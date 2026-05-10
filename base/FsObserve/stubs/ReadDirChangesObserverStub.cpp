/// Copyright (C) 2026 sonoransun — see LICENCE.txt
///
/// Compiled only when ALPINE_ENABLE_FSOBSERVE_READDIRCHANGES=OFF.


#include <Log.h>
#include <ReadDirChangesObserver.h>


struct ReadDirChangesObserverState
{};


ReadDirChangesObserver::ReadDirChangesObserver(const string & root)
    : root_(root),
      backendName_("readdirchanges"s),
      state_(std::make_unique<ReadDirChangesObserverState>())
{}
ReadDirChangesObserver::~ReadDirChangesObserver() = default;
bool
ReadDirChangesObserver::start()
{
    Log::Info("ReadDirChangesObserver: disabled at build time"s);
    return false;
}
void
ReadDirChangesObserver::stop()
{}
void
ReadDirChangesObserver::WorkerThread::threadMain()
{}
void
ReadDirChangesObserver::watchLoop()
{}
