/// Copyright (C) 2026 sonoransun — see LICENCE.txt
///
/// Compiled only when ALPINE_ENABLE_FSOBSERVE_EBPF=OFF.


#include <EbpfObserver.h>
#include <Log.h>


EbpfObserver::EbpfObserver(const string & root)
    : root_(root),
      backendName_("ebpf"s),
      sidecarPath_("alpine-fsprobe"s)
{}
EbpfObserver::~EbpfObserver() = default;
bool
EbpfObserver::start()
{
    Log::Info("EbpfObserver: disabled at build time"s);
    return false;
}
void
EbpfObserver::stop()
{}
bool
EbpfObserver::spawnSidecar()
{
    return false;
}
void
EbpfObserver::WorkerThread::threadMain()
{}
void
EbpfObserver::readLoop()
{}
