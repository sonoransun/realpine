/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineDtcpConnMux.h>
#include <AlpineRawWifiUdpTransport.h>
#include <Log.h>
#include <RawWifiConnection.h>


AlpineRawWifiUdpTransport::AlpineRawWifiUdpTransport(const ulong ipAddress,
                                                     const int port,
                                                     const string & interfaceName)
    : DtcpBaseUdpTransport(ipAddress, port),
      interfaceName_(interfaceName)
{
#ifdef _VERBOSE
    Log::Debug("AlpineRawWifiUdpTransport constructor invoked.");
#endif
}


AlpineRawWifiUdpTransport::~AlpineRawWifiUdpTransport()
{
#ifdef _VERBOSE
    Log::Debug("AlpineRawWifiUdpTransport destructor invoked.");
#endif
}


bool
AlpineRawWifiUdpTransport::createMux(DtcpBaseConnMux *& connMux)
{
#ifdef _VERBOSE
    Log::Debug("AlpineRawWifiUdpTransport::createMux invoked.");
#endif

    AlpineDtcpConnMux * alpineMux;
    alpineMux = new AlpineDtcpConnMux();

    connMux = static_cast<DtcpBaseConnMux *>(alpineMux);

    return true;
}


UdpConnection *
AlpineRawWifiUdpTransport::createConnection()
{
#ifndef __linux__
    Log::Error("AlpineRawWifiUdpTransport: Raw 802.11 is only supported on Linux.");
    return nullptr;
#else
    // Return a RawWifiConnection without yet binding to the AF_PACKET socket;
    // DtcpBaseUdpTransport::activate() will call create() and handle failures
    // (e.g. EPERM when CAP_NET_RAW is missing) cleanly. Wrap in try/catch so
    // an allocation failure cannot crash the stack.
    //
    try {
        return new RawWifiConnection(interfaceName_);
    } catch (const std::exception & e) {
        Log::Error("AlpineRawWifiUdpTransport: Failed to allocate RawWifiConnection: "s + e.what());
        return nullptr;
    } catch (...) {
        Log::Error("AlpineRawWifiUdpTransport: Failed to allocate RawWifiConnection (unknown error).");
        return nullptr;
    }
#endif
}


bool
AlpineRawWifiUdpTransport::handleData(const byte * data, uint dataLength)
{
#ifdef _VERBOSE
    Log::Debug("AlpineRawWifiUdpTransport::handleData invoked.");
#endif

    return true;
}
