/// Copyright (C) 2026 sonoransun — see LICENCE.txt
///
/// inotify-based filesystem observation.  Always available on Linux; needs no
/// special capability.  Watches the root directory non-recursively for
/// IN_OPEN | IN_ACCESS | IN_CLOSE_NOWRITE on its child files.


#pragma once
#include <AutoThread.h>
#include <FsObserver.h>
#include <atomic>
#include <memory>


class InotifyObserver : public FsObserver
{
  public:
    explicit InotifyObserver(const string & root);
    ~InotifyObserver() override;


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
        explicit WorkerThread(InotifyObserver & owner)
            : owner_(owner)
        {}
        void threadMain() override;

      private:
        InotifyObserver & owner_;
    };


    void watchLoop();


    string root_;
    string backendName_;
    t_Sink sink_;

    std::unique_ptr<WorkerThread> worker_;
    std::atomic<bool> running_{false};
    std::atomic<bool> stopRequested_{false};
    int inotifyFd_{-1};
    int watchDescriptor_{-1};
};
