/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <KqueueObserver.h>
#include <Log.h>

#include <chrono>
#include <cstring>
#include <filesystem>

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
#include <fcntl.h>
#include <sys/event.h>
#include <sys/stat.h>
#include <unistd.h>
#endif


KqueueObserver::KqueueObserver(const string & root)
    : root_(root),
      backendName_("kqueue"s)
{}


KqueueObserver::~KqueueObserver()
{
    stop();
}


bool
KqueueObserver::start()
{
#if !(defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || \
      defined(__DragonFly__))
    Log::Error("KqueueObserver: not supported on this platform"s);
    return false;
#else
    if (running_.load())
        return true;

    if (!sink_) {
        Log::Error("KqueueObserver: sink not set"s);
        return false;
    }

    kqueueFd_ = ::kqueue();
    if (kqueueFd_ < 0) {
        Log::Error("KqueueObserver: kqueue() failed: "s + std::strerror(errno));
        return false;
    }

    // Mark fd close-on-exec; kqueue() does not accept SOCK_CLOEXEC-style flags.
    int flags = ::fcntl(kqueueFd_, F_GETFD);
    if (flags >= 0)
        ::fcntl(kqueueFd_, F_SETFD, flags | FD_CLOEXEC);

    rescanRoot();

    stopRequested_.store(false);
    worker_ = std::make_unique<WorkerThread>(*this);

    if (!worker_->create()) {
        Log::Error("KqueueObserver: failed to start worker thread"s);
        for (auto & [_, fd] : fileFds_)
            ::close(fd);
        fileFds_.clear();
        fdPaths_.clear();
        ::close(kqueueFd_);
        kqueueFd_ = -1;
        worker_.reset();
        return false;
    }
    worker_->resume();

    running_.store(true);
    Log::Info("KqueueObserver: started watching "s + root_);
    return true;
#endif
}


void
KqueueObserver::stop()
{
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
    if (!running_.load() && !worker_)
        return;

    stopRequested_.store(true);

    if (worker_) {
        worker_->destroy();
        worker_.reset();
    }

    for (auto & [_, fd] : fileFds_)
        ::close(fd);
    fileFds_.clear();
    fdPaths_.clear();

    if (kqueueFd_ >= 0) {
        ::close(kqueueFd_);
        kqueueFd_ = -1;
    }

    running_.store(false);
#endif
}


void
KqueueObserver::WorkerThread::threadMain()
{
    owner_.watchLoop();
}


#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)


bool
KqueueObserver::registerFile(const string & path)
{
    if (fileFds_.find(path) != fileFds_.end())
        return false;

    int fd = ::open(path.c_str(),
#ifdef O_EVTONLY
                    O_RDONLY | O_EVTONLY | O_CLOEXEC
#else
                    O_RDONLY | O_CLOEXEC
#endif
    );
    if (fd < 0)
        return false;

    struct kevent change;
    EV_SET(&change,
           fd,
           EVFILT_VNODE,
           EV_ADD | EV_ENABLE | EV_CLEAR,
           NOTE_ATTRIB | NOTE_WRITE | NOTE_DELETE | NOTE_RENAME | NOTE_REVOKE,
           0,
           nullptr);

    if (::kevent(kqueueFd_, &change, 1, nullptr, 0, nullptr) < 0) {
        ::close(fd);
        return false;
    }

    fileFds_[path] = fd;
    fdPaths_[fd] = path;
    return true;
}


void
KqueueObserver::unregisterFile(const string & path)
{
    auto it = fileFds_.find(path);
    if (it == fileFds_.end())
        return;

    int fd = it->second;
    // Closing the fd auto-removes the kevent registration.
    ::close(fd);
    fdPaths_.erase(fd);
    fileFds_.erase(it);
}


void
KqueueObserver::rescanRoot()
{
    std::error_code ec;
    if (!std::filesystem::is_directory(root_, ec))
        return;

    for (const auto & entry : std::filesystem::directory_iterator(root_, ec)) {
        if (ec)
            break;
        if (!entry.is_regular_file(ec))
            continue;
        registerFile(entry.path().string());
    }
}


void
KqueueObserver::watchLoop()
{
    constexpr int kRescanIntervalSec = 30;
    auto lastRescan = std::chrono::steady_clock::now();

    while (!stopRequested_.load()) {
        struct timespec timeout;
        timeout.tv_sec = 0;
        timeout.tv_nsec = 250 * 1000 * 1000;  // 250ms

        struct kevent event;
        int n = ::kevent(kqueueFd_, nullptr, 0, &event, 1, &timeout);

        if (stopRequested_.load())
            break;

        if (n > 0 && (event.filter == EVFILT_VNODE)) {
            auto fdIt = fdPaths_.find(static_cast<int>(event.ident));
            if (fdIt != fdPaths_.end()) {
                FsEvent ev;
                ev.fsPath = fdIt->second;
                ev.ts = std::chrono::steady_clock::now();

                if (event.fflags & NOTE_DELETE) {
                    unregisterFile(fdIt->second);
                } else {
                    if (event.fflags & (NOTE_ATTRIB | NOTE_WRITE)) {
                        // Synthesize a minimal Open + Close pair so the
                        // aggregator's session state machine fires.  The
                        // intermediate Read is reported when NOTE_WRITE
                        // is also set (something actually changed bytes).
                        ev.op = FsEvent::Op::Open;
                        try {
                            sink_(ev);
                        } catch (...) {
                            Log::Error("KqueueObserver: sink threw on Open"s);
                        }

                        if (event.fflags & NOTE_WRITE) {
                            ev.op = FsEvent::Op::Read;
                            try {
                                sink_(ev);
                            } catch (...) {
                                Log::Error("KqueueObserver: sink threw on Read"s);
                            }
                        }

                        ev.op = FsEvent::Op::Close;
                        try {
                            sink_(ev);
                        } catch (...) {
                            Log::Error("KqueueObserver: sink threw on Close"s);
                        }
                    }
                }
            }
        }

        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - lastRescan).count() >= kRescanIntervalSec) {
            rescanRoot();
            lastRescan = now;
        }
    }
}


#else  // unsupported platform — watchLoop is empty


void
KqueueObserver::watchLoop()
{}

bool
KqueueObserver::registerFile(const string &)
{
    return false;
}

void
KqueueObserver::unregisterFile(const string &)
{}

void
KqueueObserver::rescanRoot()
{}


#endif
