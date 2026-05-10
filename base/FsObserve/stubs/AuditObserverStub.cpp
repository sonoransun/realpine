/// Copyright (C) 2026 sonoransun — see LICENCE.txt
///
/// Compiled only when ALPINE_ENABLE_FSOBSERVE_AUDIT=OFF.


#include <AuditObserver.h>
#include <Log.h>


AuditObserver::AuditObserver(const string & root)
    : root_(root),
      backendName_("audit"s)
{}
AuditObserver::~AuditObserver() = default;
bool
AuditObserver::start()
{
    Log::Info("AuditObserver: disabled at build time"s);
    return false;
}
void
AuditObserver::stop()
{}
void
AuditObserver::WorkerThread::threadMain()
{}
void
AuditObserver::watchLoop()
{}
void
AuditObserver::parseAndEmit(const string &)
{}
