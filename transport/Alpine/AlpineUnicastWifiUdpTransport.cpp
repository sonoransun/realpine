/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineDtcpConnMux.h>
#include <AlpineUnicastWifiUdpTransport.h>
#include <Log.h>
#include <UnicastWifiConnection.h>


AlpineUnicastWifiUdpTransport::AlpineUnicastWifiUdpTransport(const ulong ipAddress,
                                                             const int port,
                                                             const string & interfaceName)
    : DtcpBaseUdpTransport(ipAddress, port),
      interfaceName_(interfaceName)
{
#ifdef _VERBOSE
    Log::Debug("AlpineUnicastWifiUdpTransport constructor invoked.");
#endif
}


AlpineUnicastWifiUdpTransport::~AlpineUnicastWifiUdpTransport()
{
#ifdef _VERBOSE
    Log::Debug("AlpineUnicastWifiUdpTransport destructor invoked.");
#endif
}


bool
AlpineUnicastWifiUdpTransport::createMux(DtcpBaseConnMux *& connMux)
{
#ifdef _VERBOSE
    Log::Debug("AlpineUnicastWifiUdpTransport::createMux invoked.");
#endif

    AlpineDtcpConnMux * alpineMux;
    alpineMux = new AlpineDtcpConnMux();

    connMux = static_cast<DtcpBaseConnMux *>(alpineMux);

    return true;
}


UdpConnection *
AlpineUnicastWifiUdpTransport::createConnection()
{
#ifndef __linux__
    Log::Error("AlpineUnicastWifiUdpTransport: Unicast 802.11 is only supported on Linux.");
    return nullptr;
#else
    try {
        return new UnicastWifiConnection(interfaceName_);
    } catch (const std::exception & e) {
        Log::Error("AlpineUnicastWifiUdpTransport: Failed to allocate UnicastWifiConnection: "s + e.what());
        return nullptr;
    } catch (...) {
        Log::Error("AlpineUnicastWifiUdpTransport: Failed to allocate UnicastWifiConnection (unknown error).");
        return nullptr;
    }
#endif
}


bool
AlpineUnicastWifiUdpTransport::handleData(const byte * data, uint dataLength)
{
#ifdef _VERBOSE
    Log::Debug("AlpineUnicastWifiUdpTransport::handleData invoked.");
#endif

    return true;
}
