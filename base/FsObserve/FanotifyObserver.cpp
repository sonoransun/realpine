/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <FanotifyObserver.h>
#include <Log.h>

#include <chrono>
#include <cstring>
#include <system_error>

#if defined(__linux__)
#include <fcntl.h>
#include <poll.h>
#include <sys/fanotify.h>
#include <sys/stat.h>
#include <unistd.h>
#endif


FanotifyObserver::FanotifyObserver(const string & root)
    : root_(root),
      backendName_("fanotify"s)
{}


FanotifyObserver::~FanotifyObserver()
{
    stop();
}


bool
FanotifyObserver::start()
{
#if !defined(__linux__)
    Log::Error("FanotifyObserver: not supported on this platform"s);
    return false;
#else
    if (running_.load())
        return true;

    if (!sink_) {
        Log::Error("FanotifyObserver: sink not set"s);
        return false;
    }

    fanotifyFd_ = fanotify_init(FAN_CLOEXEC | FAN_NONBLOCK | FAN_CLASS_NOTIF, O_RDONLY | O_LARGEFILE);
    if (fanotifyFd_ < 0) {
        // EPERM is the typical "no CAP_SYS_ADMIN" failure mode.
        Log::Error("FanotifyObserver: fanotify_init failed: "s + std::strerror(errno));
        return false;
    }

    int markRet = fanotify_mark(
        fanotifyFd_, FAN_MARK_ADD | FAN_MARK_MOUNT, FAN_OPEN | FAN_CLOSE_NOWRITE, AT_FDCWD, root_.c_str());
    if (markRet < 0) {
        Log::Error("FanotifyObserver: fanotify_mark failed for "s + root_ + ": "s + std::strerror(errno));
        ::close(fanotifyFd_);
        fanotifyFd_ = -1;
        return false;
    }

    stopRequested_.store(false);
    worker_ = std::make_unique<WorkerThread>(*this);

    if (!worker_->create()) {
        Log::Error("FanotifyObserver: failed to start worker thread"s);
        ::close(fanotifyFd_);
        fanotifyFd_ = -1;
        worker_.reset();
        return false;
    }
    worker_->resume();

    running_.store(true);
    Log::Info("FanotifyObserver: started watching mount of "s + root_);
    return true;
#endif
}


void
FanotifyObserver::stop()
{
#if defined(__linux__)
    if (!running_.load() && !worker_)
        return;

    stopRequested_.store(true);

    if (worker_) {
        worker_->destroy();
        worker_.reset();
    }

    if (fanotifyFd_ >= 0) {
        ::close(fanotifyFd_);
        fanotifyFd_ = -1;
    }

    running_.store(false);
#endif
}


void
FanotifyObserver::WorkerThread::threadMain()
{
    owner_.watchLoop();
}


string
FanotifyObserver::resolveFdToPath(int fd) const
{
#if defined(__linux__)
    char proc[64];
    std::snprintf(proc, sizeof(proc), "/proc/self/fd/%d", fd);
    char target[4096];
    ssize_t n = ::readlink(proc, target, sizeof(target) - 1);
    if (n <= 0)
        return ""s;
    target[n] = '\0';
    return string(target);
#else
    (void)fd;
    return ""s;
#endif
}


void
FanotifyObserver::watchLoop()
{
#if defined(__linux__)
    constexpr size_t bufSize = 8192;
    alignas(struct fanotify_event_metadata) char buf[bufSize];

    while (!stopRequested_.load()) {
        struct pollfd pfd;
        pfd.fd = fanotifyFd_;
        pfd.events = POLLIN;

        int ret = poll(&pfd, 1, 250);

        if (stopRequested_.load())
            break;
        if (ret <= 0)
            continue;
        if (!(pfd.revents & POLLIN))
            continue;

        ssize_t len = ::read(fanotifyFd_, buf, sizeof(buf));
        if (len <= 0)
            continue;

        auto * meta = reinterpret_cast<struct fanotify_event_metadata *>(buf);
        while (FAN_EVENT_OK(meta, len)) {
            if (meta->vers != FANOTIFY_METADATA_VERSION) {
                Log::Error("FanotifyObserver: metadata version mismatch"s);
                break;
            }

            if (meta->fd >= 0) {
                string path = resolveFdToPath(meta->fd);

                if (!path.empty() && path.rfind(root_, 0) == 0) {
                    FsEvent ev;
                    ev.fsPath = path;
                    ev.pid = meta->pid;
                    ev.ts = std::chrono::steady_clock::now();

                    auto nowNs = static_cast<uint64_t>(
                        std::chrono::duration_cast<std::chrono::nanoseconds>(ev.ts.time_since_epoch()).count());

                    if (meta->mask & FAN_OPEN) {
                        ev.op = FsEvent::Op::Open;
                        openTimes_[meta->pid][path] = nowNs;
                    } else if (meta->mask & FAN_CLOSE_NOWRITE) {
                        ev.op = FsEvent::Op::Close;
                        auto pidIt = openTimes_.find(meta->pid);
                        if (pidIt != openTimes_.end()) {
                            auto pathIt = pidIt->second.find(path);
                            if (pathIt != pidIt->second.end()) {
                                ev.durationNs = nowNs - pathIt->second;
                                pidIt->second.erase(pathIt);
                                if (pidIt->second.empty())
                                    openTimes_.erase(pidIt);
                            }
                        }

                        struct stat st;
                        if (::fstat(meta->fd, &st) == 0)
                            ev.bytes = static_cast<ulong>(st.st_size);
                    } else {
                        ::close(meta->fd);
                        meta = FAN_EVENT_NEXT(meta, len);
                        continue;
                    }

                    try {
                        sink_(ev);
                    } catch (const std::exception & e) {
                        Log::Error("FanotifyObserver: sink threw: "s + e.what());
                    } catch (...) {
                        Log::Error("FanotifyObserver: sink threw unknown exception"s);
                    }
                }

                ::close(meta->fd);
            }

            meta = FAN_EVENT_NEXT(meta, len);
        }
    }
#endif
}
