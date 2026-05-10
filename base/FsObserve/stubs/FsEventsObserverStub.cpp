/// Copyright (C) 2026 sonoransun — see LICENCE.txt
///
/// Compiled only when ALPINE_ENABLE_FSOBSERVE_FSEVENTS=OFF.


#include <FsEventsObserver.h>
#include <Log.h>


// Out-of-line definition so the header's std::unique_ptr<FsEventsObserverState>
// has a complete type at the destruction point.
struct FsEventsObserverState
{};


FsEventsObserver::FsEventsObserver(const string & root)
    : root_(root),
      backendName_("fsevents"s),
      state_(std::make_unique<FsEventsObserverState>())
{}
FsEventsObserver::~FsEventsObserver() = default;
bool
FsEventsObserver::start()
{
    Log::Info("FsEventsObserver: disabled at build time"s);
    return false;
}
void
FsEventsObserver::stop()
{}
void
FsEventsObserver::onPath(const string &, uint32_t)
{}
