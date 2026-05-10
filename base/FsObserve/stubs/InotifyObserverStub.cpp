/// Copyright (C) 2026 sonoransun — see LICENCE.txt
///
/// Compiled only when ALPINE_ENABLE_FSOBSERVE_INOTIFY=OFF, so the factory
/// can still link.  Always reports start() failure.


#include <InotifyObserver.h>
#include <Log.h>


InotifyObserver::InotifyObserver(const string & root)
    : root_(root),
      backendName_("inotify"s)
{}
InotifyObserver::~InotifyObserver() = default;
bool
InotifyObserver::start()
{
    Log::Info("InotifyObserver: disabled at build time"s);
    return false;
}
void
InotifyObserver::stop()
{}
void
InotifyObserver::WorkerThread::threadMain()
{}
void
InotifyObserver::watchLoop()
{}
