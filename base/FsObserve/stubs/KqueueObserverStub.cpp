/// Copyright (C) 2026 sonoransun — see LICENCE.txt
///
/// Compiled only when ALPINE_ENABLE_FSOBSERVE_KQUEUE=OFF.


#include <KqueueObserver.h>
#include <Log.h>


KqueueObserver::KqueueObserver(const string & root)
    : root_(root),
      backendName_("kqueue"s)
{}
KqueueObserver::~KqueueObserver() = default;
bool
KqueueObserver::start()
{
    Log::Info("KqueueObserver: disabled at build time"s);
    return false;
}
void
KqueueObserver::stop()
{}
void
KqueueObserver::WorkerThread::threadMain()
{}
void
KqueueObserver::watchLoop()
{}
bool
KqueueObserver::registerFile(const string &)
{
    return false;
}
void
KqueueObserver::unregisterFile(const string &)
{}
void
KqueueObserver::rescanRoot()
{}
