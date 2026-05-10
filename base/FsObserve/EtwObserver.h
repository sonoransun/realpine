/// Copyright (C) 2026 sonoransun — see LICENCE.txt
///
/// Windows ETW-based filesystem observation.  Subscribes to the
/// Microsoft-Windows-Kernel-File provider for real-time Create/Close/Read
/// events with full per-process attribution.
///
/// Runtime requirements:
///   - Process token must include SE_SYSTEM_PROFILE_NAME or be a member of
///     the "Performance Log Users" group; admin satisfies both.
///   - Tdh.lib must be available at link time (CMake-gated).
///
/// When `StartTraceW` returns ERROR_ACCESS_DENIED, `start()` returns false
/// and FsObserverFactory falls back to ReadDirChangesObserver.


#pragma once
#include <AutoThread.h>
#include <FsObserver.h>
#include <atomic>
#include <memory>
#include <mutex>
#include <unordered_map>


// Opaque to keep Windows + ETW headers out of the public header.
struct EtwObserverState;


class EtwObserver : public FsObserver
{
  public:
    explicit EtwObserver(const string & root);
    ~EtwObserver() override;


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
        explicit WorkerThread(EtwObserver & owner)
            : owner_(owner)
        {}
        void threadMain() override;

      private:
        EtwObserver & owner_;
    };


    void processLoop();


    // FileObject pointer (carried in Create/Read/Close records) -> path.
    // Mutex-protected because the ETW callback runs on a Windows-managed
    // thread that overlaps with start()/stop() bookkeeping.
    void rememberFile(uint64_t fileObject, const string & path);
    bool lookupFile(uint64_t fileObject, string & path);
    void forgetFile(uint64_t fileObject);


    string root_;
    string backendName_;
    t_Sink sink_;

    std::unique_ptr<WorkerThread> worker_;
    std::unique_ptr<EtwObserverState> state_;
    std::atomic<bool> running_{false};

    std::mutex pathMutex_;
    std::unordered_map<uint64_t, string> filePaths_;


    friend struct EtwObserverState;
};
