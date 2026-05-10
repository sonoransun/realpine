/// Copyright (C) 2026 sonoransun — see LICENCE.txt
///
/// Constructs the most capable FsObserver the host supports.
///
/// Selection order (env-var override wins):
///   ALPINE_FSOBSERVE_BACKEND in {ebpf, fanotify, audit, inotify} → that backend
///   else: try ebpf, then fanotify, then audit, then inotify; first start()
///         that returns true wins.


#pragma once
#include <Common.h>
#include <FsObserver.h>
#include <memory>


class FsObserverFactory
{
  public:
    // Construct and start an observer with `sink` already attached.  Returns
    // nullptr only when *every* backend failed to start.
    //
    static std::unique_ptr<FsObserver> detectAndStart(const string & root, FsObserver::t_Sink sink);


    // Construct without starting (useful for tests).  Caller must call
    // setSink() and start().
    //
    static std::unique_ptr<FsObserver> create(const string & backend, const string & root);
};
