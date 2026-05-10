/// Copyright (C) 2026 sonoransun — see LICENCE.txt
///
/// fanotify-based filesystem observation.  Marks the root mount with
/// FAN_OPEN | FAN_CLOSE_NOWRITE.  Carries pid (and on FAN_REPORT_FID-capable
/// kernels, uid) per event without requiring a recursive watch.  Needs
/// CAP_SYS_ADMIN; start() returns false otherwise so the factory falls back.


#pragma once
#include <AutoThread.h>
#include <FsObserver.h>
#include <atomic>
#include <memory>
#include <unordered_map>


class FanotifyObserver : public FsObserver
{
  public:
    explicit FanotifyObserver(const string & root);
    ~FanotifyObserver() override;


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
        explicit WorkerThread(FanotifyObserver & owner)
            : owner_(owner)
        {}
        void threadMain() override;

      private:
        FanotifyObserver & owner_;
    };


    void watchLoop();
    string resolveFdToPath(int fd) const;


    string root_;
    string backendName_;
    t_Sink sink_;

    std::unique_ptr<WorkerThread> worker_;
    std::atomic<bool> running_{false};
    std::atomic<bool> stopRequested_{false};
    int fanotifyFd_{-1};

    // pid -> (path -> open-time) for synthesizing dwell on close
    std::unordered_map<int, std::unordered_map<string, uint64_t>> openTimes_;
};
