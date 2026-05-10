/// Copyright (C) 2026 sonoransun — see LICENCE.txt
///
/// macOS FSEvents-based filesystem observation.  Recursively watches root_
/// via the high-level FSEventStream API with kFSEventStreamCreateFlagFileEvents
/// for per-file resolution.  Runs on a private dispatch queue (no CFRunLoop).
///
/// Like KqueueObserver, FSEvents does not carry process attribution; emitted
/// FsEvents have pid=-1 and uid=-1.


#pragma once
#include <FsObserver.h>
#include <atomic>
#include <memory>


// Opaque forwards so the header doesn't drag in CoreServices on non-Darwin.
struct FsEventsObserverState;


class FsEventsObserver : public FsObserver
{
  public:
    explicit FsEventsObserver(const string & root);
    ~FsEventsObserver() override;


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
    string root_;
    string backendName_;
    t_Sink sink_;

    std::unique_ptr<FsEventsObserverState> state_;
    std::atomic<bool> running_{false};


    // Trampoline called by the FSEvents callback (defined in .cpp).
    void onPath(const string & path, uint32_t flags);


    friend struct FsEventsObserverState;
};
