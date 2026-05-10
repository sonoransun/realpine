/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AuditObserver.h>
#include <EbpfObserver.h>
#include <EndpointSecurityObserver.h>
#include <EtwObserver.h>
#include <FanotifyObserver.h>
#include <FsEventsObserver.h>
#include <FsObserverFactory.h>
#include <InotifyObserver.h>
#include <KqueueObserver.h>
#include <Log.h>
#include <ReadDirChangesObserver.h>

#include <cstdlib>


std::unique_ptr<FsObserver>
FsObserverFactory::create(const string & backend, const string & root)
{
    // Linux backends
    if (backend == "inotify"s)
        return std::make_unique<InotifyObserver>(root);
    if (backend == "fanotify"s)
        return std::make_unique<FanotifyObserver>(root);
    if (backend == "audit"s)
        return std::make_unique<AuditObserver>(root);
    if (backend == "ebpf"s)
        return std::make_unique<EbpfObserver>(root);

    // macOS backends
    if (backend == "kqueue"s)
        return std::make_unique<KqueueObserver>(root);
    if (backend == "fsevents"s)
        return std::make_unique<FsEventsObserver>(root);
    if (backend == "endpointsec"s)
        return std::make_unique<EndpointSecurityObserver>(root);

    // Windows backends
    if (backend == "readdirchanges"s)
        return std::make_unique<ReadDirChangesObserver>(root);
    if (backend == "etw"s)
        return std::make_unique<EtwObserver>(root);

    return nullptr;
}


std::unique_ptr<FsObserver>
FsObserverFactory::detectAndStart(const string & root, FsObserver::t_Sink sink)
{
    // Per-platform probe order (richest first; fallback to always-available).
    static const vector<string> kProbeOrder = {
#if defined(__linux__)
        "ebpf"s, "fanotify"s, "audit"s, "inotify"s
#elif defined(__APPLE__)
        "endpointsec"s, "fsevents"s, "kqueue"s
#elif defined(_WIN32)
        "etw"s, "readdirchanges"s
#endif
    };

    vector<string> order;
    if (const char * forced = std::getenv("ALPINE_FSOBSERVE_BACKEND"); forced && *forced) {
        order.push_back(string(forced));
    } else {
        order = kProbeOrder;
    }

    for (const auto & name : order) {
        auto observer = create(name, root);
        if (!observer) {
            Log::Error("FsObserverFactory: unknown backend "s + name);
            continue;
        }

        observer->setSink(sink);
        if (observer->start()) {
            Log::Info("FsObserverFactory: backend="s + observer->backendName() + " root="s + observer->root() +
                      " started"s);
            return observer;
        }

        Log::Info("FsObserverFactory: backend "s + name + " unavailable, trying next"s);
    }

    Log::Error("FsObserverFactory: no backend could start; implicit feedback disabled"s);
    return nullptr;
}
