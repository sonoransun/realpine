/// Copyright (C) 2026 sonoransun — see LICENCE.txt
///
/// Compiled only when ALPINE_ENABLE_FSOBSERVE_FANOTIFY=OFF.


#include <FanotifyObserver.h>
#include <Log.h>


FanotifyObserver::FanotifyObserver(const string & root)
    : root_(root),
      backendName_("fanotify"s)
{}
FanotifyObserver::~FanotifyObserver() = default;
bool
FanotifyObserver::start()
{
    Log::Info("FanotifyObserver: disabled at build time"s);
    return false;
}
void
FanotifyObserver::stop()
{}
void
FanotifyObserver::WorkerThread::threadMain()
{}
string
FanotifyObserver::resolveFdToPath(int) const
{
    return ""s;
}
void
FanotifyObserver::watchLoop()
{}
