/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineRawWifiUdpTransport.h>
#include <AlpineDtcpConnMux.h>
#include <RawWifiConnection.h>
#include <Log.h>



AlpineRawWifiUdpTransport::AlpineRawWifiUdpTransport (const ulong    ipAddress,
                                                        const int      port,
                                                        const string & interfaceName)
    : DtcpBaseUdpTransport(ipAddress, port),
      interfaceName_(interfaceName)
{
#ifdef _VERBOSE
    Log::Debug("AlpineRawWifiUdpTransport constructor invoked.");
#endif
}



AlpineRawWifiUdpTransport::~AlpineRawWifiUdpTransport ()
{
#ifdef _VERBOSE
    Log::Debug("AlpineRawWifiUdpTransport destructor invoked.");
#endif
}



bool
AlpineRawWifiUdpTransport::createMux (DtcpBaseConnMux *& connMux)
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
AlpineRawWifiUdpTransport::createConnection ()
{
#ifndef __linux__
    Log::Error("AlpineRawWifiUdpTransport: Raw 802.11 is only supported on Linux.");
    return nullptr;
#else
    return new RawWifiConnection(interfaceName_);
#endif
}



bool
AlpineRawWifiUdpTransport::handleData (const byte * data,
                                        uint         dataLength)
{
#ifdef _VERBOSE
    Log::Debug("AlpineRawWifiUdpTransport::handleData invoked.");
#endif

    return true;
}
