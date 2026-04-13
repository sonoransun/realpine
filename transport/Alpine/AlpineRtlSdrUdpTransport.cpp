/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineDtcpConnMux.h>
#include <AlpineRtlSdrUdpTransport.h>
#include <Log.h>
#include <RtlSdrConnection.h>

#ifdef ALPINE_RTLSDR_ENABLED
#include <GfskDemodulator.h>
#endif

#include <memory>


AlpineRtlSdrUdpTransport::AlpineRtlSdrUdpTransport(const ulong ipAddress,
                                                   const int port,
                                                   uint centerFreqHz,
                                                   uint sampleRate,
                                                   int gainTenths,
                                                   const string & modulation)
    : DtcpBaseUdpTransport(ipAddress, port),
      centerFreqHz_(centerFreqHz),
      sampleRate_(sampleRate),
      gainTenths_(gainTenths),
      modulation_(modulation)
{
#ifdef _VERBOSE
    Log::Debug("AlpineRtlSdrUdpTransport constructor invoked.");
#endif
}


AlpineRtlSdrUdpTransport::~AlpineRtlSdrUdpTransport()
{
#ifdef _VERBOSE
    Log::Debug("AlpineRtlSdrUdpTransport destructor invoked.");
#endif
}


bool
AlpineRtlSdrUdpTransport::createMux(DtcpBaseConnMux *& connMux)
{
#ifdef _VERBOSE
    Log::Debug("AlpineRtlSdrUdpTransport::createMux invoked.");
#endif

    AlpineDtcpConnMux * alpineMux;
    alpineMux = new AlpineDtcpConnMux();

    connMux = static_cast<DtcpBaseConnMux *>(alpineMux);

    return true;
}


UdpConnection *
AlpineRtlSdrUdpTransport::createConnection()
{
#ifndef ALPINE_RTLSDR_ENABLED
    Log::Error("AlpineRtlSdrUdpTransport: RTL-SDR not compiled in (need -DALPINE_ENABLE_RTLSDR=ON).");
    return nullptr;
#else
    try {
        // Factory: create the appropriate demodulator based on modulation name
        std::unique_ptr<SdrDemodulator> demod;

        if (modulation_ == "gfsk") {
            demod = std::make_unique<GfskDemodulator>();
        } else {
            Log::Error("AlpineRtlSdrUdpTransport: Unknown modulation '"s + modulation_ + "'.");
            return nullptr;
        }

        return new RtlSdrConnection(centerFreqHz_, sampleRate_, gainTenths_, std::move(demod));
    } catch (const std::exception & e) {
        Log::Error("AlpineRtlSdrUdpTransport: Failed to allocate RtlSdrConnection: "s + e.what());
        return nullptr;
    } catch (...) {
        Log::Error("AlpineRtlSdrUdpTransport: Failed to allocate RtlSdrConnection (unknown error).");
        return nullptr;
    }
#endif
}


bool
AlpineRtlSdrUdpTransport::handleData(const byte * data, uint dataLength)
{
#ifdef _VERBOSE
    Log::Debug("AlpineRtlSdrUdpTransport::handleData invoked.");
#endif

    return true;
}
