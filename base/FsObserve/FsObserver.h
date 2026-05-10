/// Copyright (C) 2026 sonoransun — see LICENCE.txt
///
/// Abstract filesystem-observation interface.  Pluggable backends watch a
/// directory tree and emit FsEvent records as files are opened, read, and
/// closed by any process on the host.  Used by AlpineFeedbackAggregator to
/// translate observed activity into reputation deltas without putting Alpine
/// in the data path.


#pragma once
#include <Common.h>
#include <chrono>
#include <functional>


struct FsEvent
{
    enum class Op { Open, Read, Close };


    string fsPath;
    Op op{Op::Open};
    ulong bytes{0};          // best-effort; 0 when unknown
    int pid{0};              // -1 when unknown
    int uid{-1};             // -1 when unknown
    uint64_t durationNs{0};  // close events only
    std::chrono::steady_clock::time_point ts{std::chrono::steady_clock::now()};
};


class FsObserver
{
  public:
    using t_Sink = std::function<void(const FsEvent &)>;


    virtual ~FsObserver() = default;


    // Watched root directory (for logging / diagnostics).
    //
    virtual const string & root() const = 0;


    // Human-readable backend name (inotify, fanotify, audit, ebpf).
    //
    virtual const string & backendName() const = 0;


    // Install the event sink.  Must be called before start(); the observer may
    // invoke the sink concurrently from a worker thread, so the callback must
    // be thread-safe.
    //
    virtual void setSink(t_Sink sink) = 0;


    // Begin watching.  Returns false if the backend cannot start (e.g.
    // missing kernel support, missing capability).  Observer state is left
    // in a stopped, re-startable form on failure.
    //
    virtual bool start() = 0;


    // Stop watching and join the worker thread (if any).  Idempotent.
    //
    virtual void stop() = 0;


    // True when the worker thread is running.
    //
    virtual bool isRunning() const = 0;
};
