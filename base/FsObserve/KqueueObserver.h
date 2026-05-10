/// Copyright (C) 2026 sonoransun — see LICENCE.txt
///
/// kqueue/EVFILT_VNODE-based filesystem observation for macOS and BSD.
/// Maintains an inventory of per-file fds under root_, listening for
/// NOTE_ATTRIB (atime updates from reads), NOTE_WRITE, NOTE_DELETE, etc.
/// Periodic rescan picks up new files registered after start().
///
/// kqueue does not carry process attribution; emitted FsEvents have
/// pid=-1 and uid=-1.  For real per-process fidelity on macOS use
/// EndpointSecurityObserver (opt-in, requires entitlement + root).


#pragma once
#include <AutoThread.h>
#include <FsObserver.h>
#include <atomic>
#include <memory>
#include <unordered_map>


class KqueueObserver : public FsObserver
{
  public:
    explicit KqueueObserver(const string & root);
    ~KqueueObserver() override;


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
        explicit WorkerThread(KqueueObserver & owner)
            : owner_(owner)
        {}
        void threadMain() override;

      private:
        KqueueObserver & owner_;
    };


    void watchLoop();
    bool registerFile(const string & path);  // returns true if newly enrolled
    void unregisterFile(const string & path);
    void rescanRoot();


    string root_;
    string backendName_;
    t_Sink sink_;

    std::unique_ptr<WorkerThread> worker_;
    std::atomic<bool> running_{false};
    std::atomic<bool> stopRequested_{false};
    int kqueueFd_{-1};

    // path -> fd inventory (each fd is registered with EVFILT_VNODE).
    std::unordered_map<string, int> fileFds_;
    // fd -> path (reverse lookup when an event fires; kevent::ident is the fd).
    std::unordered_map<int, string> fdPaths_;
};
