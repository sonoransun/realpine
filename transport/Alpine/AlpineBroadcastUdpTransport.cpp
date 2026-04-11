/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineBroadcastUdpTransport.h>
#include <AlpineDtcpConnMux.h>
#include <BroadcastUdpConnection.h>
#include <Log.h>


AlpineBroadcastUdpTransport::AlpineBroadcastUdpTransport(const ulong ipAddress, const int port)
    : DtcpBaseUdpTransport(ipAddress, port)
{
#ifdef _VERBOSE
    Log::Debug("AlpineBroadcastUdpTransport constructor invoked.");
#endif
}


AlpineBroadcastUdpTransport::~AlpineBroadcastUdpTransport()
{
#ifdef _VERBOSE
    Log::Debug("AlpineBroadcastUdpTransport destructor invoked.");
#endif
}


bool
AlpineBroadcastUdpTransport::createMux(DtcpBaseConnMux *& connMux)
{
#ifdef _VERBOSE
    Log::Debug("AlpineBroadcastUdpTransport::createMux invoked.");
#endif

    AlpineDtcpConnMux * alpineMux;
    alpineMux = new AlpineDtcpConnMux();

    connMux = static_cast<DtcpBaseConnMux *>(alpineMux);

    return true;
}


UdpConnection *
AlpineBroadcastUdpTransport::createConnection()
{
    return new BroadcastUdpConnection();
}


bool
AlpineBroadcastUdpTransport::handleData(const byte * data, uint dataLength)
{
#ifdef _VERBOSE
    Log::Debug("AlpineBroadcastUdpTransport::handleData invoked.");
#endif

    return true;
}
