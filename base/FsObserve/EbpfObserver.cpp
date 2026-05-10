/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <EbpfObserver.h>
#include <Log.h>

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#if defined(__linux__)
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
extern char ** environ;
#endif


EbpfObserver::EbpfObserver(const string & root)
    : root_(root),
      backendName_("ebpf"s),
      sidecarPath_("alpine-fsprobe"s)
{}


EbpfObserver::~EbpfObserver()
{
    stop();
}


bool
EbpfObserver::start()
{
#if !defined(__linux__)
    Log::Error("EbpfObserver: not supported on this platform"s);
    return false;
#else
    if (running_.load())
        return true;

    if (!sink_) {
        Log::Error("EbpfObserver: sink not set"s);
        return false;
    }

    if (!spawnSidecar())
        return false;

    stopRequested_.store(false);
    worker_ = std::make_unique<WorkerThread>(*this);

    if (!worker_->create()) {
        Log::Error("EbpfObserver: failed to start worker thread"s);
        if (childPid_ > 0) {
            ::kill(childPid_, SIGTERM);
            int status;
            ::waitpid(childPid_, &status, 0);
            childPid_ = -1;
        }
        if (parentFd_ >= 0) {
            ::close(parentFd_);
            parentFd_ = -1;
        }
        worker_.reset();
        return false;
    }
    worker_->resume();

    running_.store(true);
    Log::Info("EbpfObserver: sidecar pid="s + std::to_string(childPid_) + " watching "s + root_);
    return true;
#endif
}


void
EbpfObserver::stop()
{
#if defined(__linux__)
    if (!running_.load() && !worker_)
        return;

    stopRequested_.store(true);

    if (childPid_ > 0) {
        ::kill(childPid_, SIGTERM);
        int status;
        ::waitpid(childPid_, &status, 0);
        childPid_ = -1;
    }

    if (worker_) {
        worker_->destroy();
        worker_.reset();
    }

    if (parentFd_ >= 0) {
        ::close(parentFd_);
        parentFd_ = -1;
    }

    running_.store(false);
#endif
}


bool
EbpfObserver::spawnSidecar()
{
#if defined(__linux__)
    int sv[2];
    if (::socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, sv) < 0) {
        Log::Error("EbpfObserver: socketpair failed: "s + std::strerror(errno));
        return false;
    }

    pid_t pid = ::fork();
    if (pid < 0) {
        Log::Error("EbpfObserver: fork failed: "s + std::strerror(errno));
        ::close(sv[0]);
        ::close(sv[1]);
        return false;
    }

    if (pid == 0) {
        // Child — clear CLOEXEC on the socket the sidecar will inherit, set
        // env vars, exec.
        int childFd = sv[1];
        ::close(sv[0]);

        int flags = ::fcntl(childFd, F_GETFD);
        if (flags >= 0)
            ::fcntl(childFd, F_SETFD, flags & ~FD_CLOEXEC);

        ::setenv("ALPINE_FSPROBE_FD", std::to_string(childFd).c_str(), 1);
        ::setenv("ALPINE_FSPROBE_ROOT", root_.c_str(), 1);

        ::execlp(sidecarPath_.c_str(), sidecarPath_.c_str(), nullptr);

        // exec failed
        std::fprintf(stderr, "EbpfObserver child: execlp(%s) failed: %s\n", sidecarPath_.c_str(), std::strerror(errno));
        _exit(127);
    }

    // Parent
    ::close(sv[1]);
    parentFd_ = sv[0];
    childPid_ = pid;

    // Brief sanity check — verify the child didn't immediately exec-fail.
    struct pollfd pfd;
    pfd.fd = parentFd_;
    pfd.events = POLLIN | POLLHUP;
    int ret = ::poll(&pfd, 1, 100);
    if (ret > 0 && (pfd.revents & POLLHUP) && !(pfd.revents & POLLIN)) {
        Log::Error("EbpfObserver: sidecar exited immediately (exec failure?)"s);
        int status;
        ::waitpid(childPid_, &status, 0);
        ::close(parentFd_);
        parentFd_ = -1;
        childPid_ = -1;
        return false;
    }

    return true;
#else
    return false;
#endif
}


void
EbpfObserver::WorkerThread::threadMain()
{
    owner_.readLoop();
}


namespace {

bool
readExact(int fd, void * out, size_t n)
{
    auto * cursor = static_cast<uint8_t *>(out);
    while (n > 0) {
        ssize_t got = ::read(fd, cursor, n);
        if (got <= 0)
            return false;
        cursor += got;
        n -= static_cast<size_t>(got);
    }
    return true;
}

}  // namespace


void
EbpfObserver::readLoop()
{
#if defined(__linux__)
    while (!stopRequested_.load()) {
        struct pollfd pfd;
        pfd.fd = parentFd_;
        pfd.events = POLLIN;

        int ret = ::poll(&pfd, 1, 250);
        if (stopRequested_.load())
            break;
        if (ret <= 0)
            continue;
        if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
            Log::Error("EbpfObserver: sidecar socket closed"s);
            break;
        }
        if (!(pfd.revents & POLLIN))
            continue;

        uint32_t frameLen = 0;
        if (!readExact(parentFd_, &frameLen, sizeof(frameLen)))
            break;
        if (frameLen == 0 || frameLen > 65536) {
            Log::Error("EbpfObserver: bogus frame length "s + std::to_string(frameLen));
            break;
        }

        std::vector<uint8_t> body(frameLen);
        if (!readExact(parentFd_, body.data(), body.size()))
            break;

        if (body.size() < 1 + 4 + 4 + 8 + 8 + 4)
            continue;

        size_t off = 0;
        FsEvent ev;
        ev.ts = std::chrono::steady_clock::now();

        uint8_t opByte = body[off];
        off += 1;
        ev.op = (opByte == 0) ? FsEvent::Op::Open : (opByte == 1 ? FsEvent::Op::Read : FsEvent::Op::Close);

        uint32_t pid;
        std::memcpy(&pid, body.data() + off, 4);
        off += 4;
        ev.pid = static_cast<int>(pid);

        int32_t uid;
        std::memcpy(&uid, body.data() + off, 4);
        off += 4;
        ev.uid = uid;

        std::memcpy(&ev.bytes, body.data() + off, 8);
        off += 8;
        std::memcpy(&ev.durationNs, body.data() + off, 8);
        off += 8;

        uint32_t pathLen;
        std::memcpy(&pathLen, body.data() + off, 4);
        off += 4;

        if (off + pathLen > body.size())
            continue;
        ev.fsPath.assign(reinterpret_cast<const char *>(body.data() + off), pathLen);

        if (!ev.fsPath.empty() && ev.fsPath.rfind(root_, 0) != 0)
            continue;

        try {
            sink_(ev);
        } catch (...) {
            Log::Error("EbpfObserver: sink threw"s);
        }
    }
#endif
}
