/// Copyright (C) 2026 sonoransun — see LICENCE.txt
///
/// Compiled only when ALPINE_ENABLE_FSOBSERVE_ENDPOINTSEC=OFF.


#include <EndpointSecurityObserver.h>
#include <Log.h>


struct EndpointSecurityObserverState
{};


EndpointSecurityObserver::EndpointSecurityObserver(const string & root)
    : root_(root),
      backendName_("endpointsec"s),
      state_(std::make_unique<EndpointSecurityObserverState>())
{}
EndpointSecurityObserver::~EndpointSecurityObserver() = default;
bool
EndpointSecurityObserver::start()
{
    Log::Info("EndpointSecurityObserver: disabled at build time"s);
    return false;
}
void
EndpointSecurityObserver::stop()
{}
void
EndpointSecurityObserver::onMessage(const string &, int, int, int, uint64_t)
{}
