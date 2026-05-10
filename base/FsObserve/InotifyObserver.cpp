/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <InotifyObserver.h>
#include <Log.h>

#include <chrono>
#include <cstring>
#include <filesystem>
#include <system_error>

#if defined(__linux__)
#include <poll.h>
#include <sys/inotify.h>
#include <unistd.h>
#endif


InotifyObserver::InotifyObserver(const string & root)
    : root_(root),
      backendName_("inotify"s)
{}


InotifyObserver::~InotifyObserver()
{
    stop();
}


bool
InotifyObserver::start()
{
#if !defined(__linux__)
    Log::Error("InotifyObserver: not supported on this platform"s);
    return false;
#else
    if (running_.load())
        return true;

    if (!sink_) {
        Log::Error("InotifyObserver: sink not set"s);
        return false;
    }

    inotifyFd_ = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
    if (inotifyFd_ < 0) {
        Log::Error("InotifyObserver: inotify_init1 failed: "s + std::strerror(errno));
        return false;
    }

    watchDescriptor_ = inotify_add_watch(inotifyFd_, root_.c_str(), IN_OPEN | IN_ACCESS | IN_CLOSE_NOWRITE);
    if (watchDescriptor_ < 0) {
        Log::Error("InotifyObserver: inotify_add_watch failed for "s + root_ + ": "s + std::strerror(errno));
        ::close(inotifyFd_);
        inotifyFd_ = -1;
        return false;
    }

    stopRequested_.store(false);
    worker_ = std::make_unique<WorkerThread>(*this);

    if (!worker_->create()) {
        Log::Error("InotifyObserver: failed to start worker thread"s);
        inotify_rm_watch(inotifyFd_, watchDescriptor_);
        ::close(inotifyFd_);
        inotifyFd_ = -1;
        watchDescriptor_ = -1;
        worker_.reset();
        return false;
    }
    worker_->resume();

    running_.store(true);
    Log::Info("InotifyObserver: started watching "s + root_);
    return true;
#endif
}


void
InotifyObserver::stop()
{
#if defined(__linux__)
    if (!running_.load() && !worker_)
        return;

    stopRequested_.store(true);

    if (worker_) {
        worker_->destroy();
        worker_.reset();
    }

    if (inotifyFd_ >= 0) {
        if (watchDescriptor_ >= 0) {
            inotify_rm_watch(inotifyFd_, watchDescriptor_);
            watchDescriptor_ = -1;
        }
        ::close(inotifyFd_);
        inotifyFd_ = -1;
    }

    running_.store(false);
#endif
}


void
InotifyObserver::WorkerThread::threadMain()
{
    owner_.watchLoop();
}


void
InotifyObserver::watchLoop()
{
#if defined(__linux__)
    constexpr size_t bufSize = 8192;
    alignas(struct inotify_event) char buf[bufSize];

    while (!stopRequested_.load()) {
        struct pollfd pfd;
        pfd.fd = inotifyFd_;
        pfd.events = POLLIN;

        int ret = poll(&pfd, 1, 250);

        if (stopRequested_.load())
            break;
        if (ret <= 0)
            continue;
        if (!(pfd.revents & POLLIN))
            continue;

        ssize_t len = ::read(inotifyFd_, buf, sizeof(buf));
        if (len <= 0)
            continue;

        ssize_t offset = 0;
        while (offset < len) {
            auto * event = reinterpret_cast<struct inotify_event *>(buf + offset);
            offset += static_cast<ssize_t>(sizeof(struct inotify_event)) + event->len;

            if (event->len == 0)
                continue;
            if (event->mask & IN_ISDIR)
                continue;

            FsEvent ev;
            ev.fsPath = root_ + "/"s + event->name;
            ev.ts = std::chrono::steady_clock::now();

            if (event->mask & IN_OPEN) {
                ev.op = FsEvent::Op::Open;
            } else if (event->mask & IN_ACCESS) {
                ev.op = FsEvent::Op::Read;
            } else if (event->mask & IN_CLOSE_NOWRITE) {
                ev.op = FsEvent::Op::Close;
            } else {
                continue;
            }

            try {
                sink_(ev);
            } catch (const std::exception & e) {
                Log::Error("InotifyObserver: sink threw: "s + e.what());
            } catch (...) {
                Log::Error("InotifyObserver: sink threw unknown exception"s);
            }
        }
    }
#endif
}
