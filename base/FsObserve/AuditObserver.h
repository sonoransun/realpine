/// Copyright (C) 2026 sonoransun — see LICENCE.txt
///
/// Linux audit subsystem (libaudit) backend.  Reads audit netlink messages,
/// optionally installing a path watch on the root.  Needs CAP_AUDIT_READ at
/// minimum, plus CAP_AUDIT_CONTROL to install rules.  Useful when the host
/// already has auditd configured to watch the resource store directory.


#pragma once
#include <AutoThread.h>
#include <FsObserver.h>
#include <atomic>
#include <memory>


class AuditObserver : public FsObserver
{
  public:
    explicit AuditObserver(const string & root);
    ~AuditObserver() override;


    const string &
    root() const override
    {
        return root_;
    }
    const string &
    backendName() const override
    {
        return backendName_;
    }
    void
    setSink(t_Sink sink) override
    {
        sink_ = std::move(sink);
    }
    bool start() override;
    void stop() override;
    bool
    isRunning() const override
    {
        return running_.load();
    }


  private:
    class WorkerThread : public AutoThread
    {
      public:
        explicit WorkerThread(AuditObserver & owner)
            : owner_(owner)
        {}
        void threadMain() override;

      private:
        AuditObserver & owner_;
    };


    void watchLoop();
    void parseAndEmit(const string & message);


    string root_;
    string backendName_;
    t_Sink sink_;

    std::unique_ptr<WorkerThread> worker_;
    std::atomic<bool> running_{false};
    std::atomic<bool> stopRequested_{false};
    int auditFd_{-1};
    int ruleKey_{0};
};
