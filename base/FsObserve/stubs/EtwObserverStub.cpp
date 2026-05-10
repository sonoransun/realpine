/// Copyright (C) 2026 sonoransun — see LICENCE.txt
///
/// Compiled only when ALPINE_ENABLE_FSOBSERVE_ETW=OFF.


#include <EtwObserver.h>
#include <Log.h>


struct EtwObserverState
{};


EtwObserver::EtwObserver(const string & root)
    : root_(root),
      backendName_("etw"s),
      state_(std::make_unique<EtwObserverState>())
{}
EtwObserver::~EtwObserver() = default;
bool
EtwObserver::start()
{
    Log::Info("EtwObserver: disabled at build time"s);
    return false;
}
void
EtwObserver::stop()
{}
void
EtwObserver::WorkerThread::threadMain()
{}
void
EtwObserver::processLoop()
{}
void
EtwObserver::rememberFile(uint64_t, const string &)
{}
bool
EtwObserver::lookupFile(uint64_t, string &)
{
    return false;
}
void
EtwObserver::forgetFile(uint64_t)
{}
