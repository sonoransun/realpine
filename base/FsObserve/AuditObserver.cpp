/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AuditObserver.h>
#include <Log.h>

#include <chrono>
#include <cstring>

#if defined(ALPINE_HAVE_LIBAUDIT)
#include <libaudit.h>
#include <poll.h>
#include <unistd.h>
#endif


AuditObserver::AuditObserver(const string & root)
    : root_(root),
      backendName_("audit"s)
{}


AuditObserver::~AuditObserver()
{
    stop();
}


bool
AuditObserver::start()
{
#if !defined(ALPINE_HAVE_LIBAUDIT)
    Log::Error("AuditObserver: libaudit not available at build time"s);
    return false;
#else
    if (running_.load())
        return true;

    if (!sink_) {
        Log::Error("AuditObserver: sink not set"s);
        return false;
    }

    auditFd_ = audit_open();
    if (auditFd_ < 0) {
        Log::Error("AuditObserver: audit_open failed: "s + std::strerror(errno));
        return false;
    }

    // Try to install a directory watch.  Requires CAP_AUDIT_CONTROL — fall
    // through and rely on pre-existing auditd rules if this fails.
    struct audit_rule_data rule;
    std::memset(&rule, 0, sizeof(rule));
    int rc = audit_add_watch_dir(AUDIT_DIR, reinterpret_cast<struct audit_rule_data **>(&rule), root_.c_str());
    if (rc != 0) {
        Log::Info("AuditObserver: could not install dir watch ("s + std::strerror(-rc) +
                  "); relying on existing audit rules"s);
    }

    stopRequested_.store(false);
    worker_ = std::make_unique<WorkerThread>(*this);

    if (!worker_->create()) {
        Log::Error("AuditObserver: failed to start worker thread"s);
        audit_close(auditFd_);
        auditFd_ = -1;
        worker_.reset();
        return false;
    }
    worker_->resume();

    running_.store(true);
    Log::Info("AuditObserver: started watching "s + root_);
    return true;
#endif
}


void
AuditObserver::stop()
{
#if defined(ALPINE_HAVE_LIBAUDIT)
    if (!running_.load() && !worker_)
        return;

    stopRequested_.store(true);

    if (worker_) {
        worker_->destroy();
        worker_.reset();
    }

    if (auditFd_ >= 0) {
        audit_close(auditFd_);
        auditFd_ = -1;
    }

    running_.store(false);
#endif
}


void
AuditObserver::WorkerThread::threadMain()
{
    owner_.watchLoop();
}


void
AuditObserver::watchLoop()
{
#if defined(ALPINE_HAVE_LIBAUDIT)
    while (!stopRequested_.load()) {
        struct pollfd pfd;
        pfd.fd = auditFd_;
        pfd.events = POLLIN;

        int ret = poll(&pfd, 1, 250);
        if (stopRequested_.load())
            break;
        if (ret <= 0)
            continue;
        if (!(pfd.revents & POLLIN))
            continue;

        struct audit_reply reply;
        if (audit_get_reply(auditFd_, &reply, GET_REPLY_NONBLOCKING, 0) <= 0)
            continue;

        if (reply.type == AUDIT_PATH || reply.type == AUDIT_SYSCALL) {
            string msg(reply.message, reply.len);
            parseAndEmit(msg);
        }
    }
#endif
}


void
AuditObserver::parseAndEmit(const string & message)
{
    // Audit records are key=value tokens.  Extract name= and syscall= and
    // emit a coarse FsEvent when the path falls under our root.
    auto namePos = message.find(" name=\""s);
    if (namePos == string::npos)
        return;

    auto nameStart = namePos + 7;
    auto nameEnd = message.find('"', nameStart);
    if (nameEnd == string::npos)
        return;

    string path = message.substr(nameStart, nameEnd - nameStart);
    if (path.rfind(root_, 0) != 0)
        return;

    FsEvent ev;
    ev.fsPath = path;
    ev.ts = std::chrono::steady_clock::now();

    // Best-effort op classification by syscall name appearing in the record.
    if (message.find("syscall=openat"s) != string::npos || message.find("syscall=open "s) != string::npos)
        ev.op = FsEvent::Op::Open;
    else if (message.find("syscall=close"s) != string::npos)
        ev.op = FsEvent::Op::Close;
    else if (message.find("syscall=read"s) != string::npos)
        ev.op = FsEvent::Op::Read;
    else
        return;

    auto pidPos = message.find(" pid="s);
    if (pidPos != string::npos)
        ev.pid = std::atoi(message.c_str() + pidPos + 5);

    auto uidPos = message.find(" uid="s);
    if (uidPos != string::npos)
        ev.uid = std::atoi(message.c_str() + uidPos + 5);

    try {
        sink_(ev);
    } catch (...) {
        Log::Error("AuditObserver: sink threw"s);
    }
}
