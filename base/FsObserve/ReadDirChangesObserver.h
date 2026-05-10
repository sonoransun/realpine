/// Copyright (C) 2026 sonoransun — see LICENCE.txt
///
/// Windows ReadDirectoryChangesW-based filesystem observation.  Recursively
/// watches root_ via overlapped I/O on a backup-semantics directory handle.
/// Reports filesystem-level Modified events; synthesizes Open + Close pairs
/// to drive the aggregator's session state machine.
///
/// Like the macOS always-available backends, ReadDirectoryChangesW does not
/// carry process attribution; emitted FsEvents have pid=-1 and uid=-1.  For
/// per-process fidelity on Windows use EtwObserver (opt-in, requires admin
/// or Performance Log Users group).


#pragma once
#include <AutoThread.h>
#include <FsObserver.h>
#include <atomic>
#include <memory>


// Opaque to keep <windows.h> out of the public header.
struct ReadDirChangesObserverState;


class ReadDirChangesObserver : public FsObserver
{
  public:
    explicit ReadDirChangesObserver(const string & root);
    ~ReadDirChangesObserver() override;


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
        explicit WorkerThread(ReadDirChangesObserver & owner)
            : owner_(owner)
        {}
        void threadMain() override;

      private:
        ReadDirChangesObserver & owner_;
    };


    void watchLoop();


    string root_;
    string backendName_;
    t_Sink sink_;

    std::unique_ptr<WorkerThread> worker_;
    std::unique_ptr<ReadDirChangesObserverState> state_;
    std::atomic<bool> running_{false};
    std::atomic<bool> stopRequested_{false};
};
