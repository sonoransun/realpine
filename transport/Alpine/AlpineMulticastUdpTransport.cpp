/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineDtcpConnMux.h>
#include <AlpineMulticastUdpTransport.h>
#include <Log.h>
#include <MulticastUdpConnection.h>


AlpineMulticastUdpTransport::AlpineMulticastUdpTransport(const ulong ipAddress,
                                                         const int port,
                                                         const string & multicastGroup,
                                                         ushort multicastPort)
    : DtcpBaseUdpTransport(ipAddress, port),
      multicastGroup_(multicastGroup),
      multicastPort_(multicastPort)
{
#ifdef _VERBOSE
    Log::Debug("AlpineMulticastUdpTransport constructor invoked.");
#endif
}


AlpineMulticastUdpTransport::~AlpineMulticastUdpTransport()
{
#ifdef _VERBOSE
    Log::Debug("AlpineMulticastUdpTransport destructor invoked.");
#endif
}


bool
AlpineMulticastUdpTransport::createMux(DtcpBaseConnMux *& connMux)
{
#ifdef _VERBOSE
    Log::Debug("AlpineMulticastUdpTransport::createMux invoked.");
#endif

    AlpineDtcpConnMux * alpineMux;
    alpineMux = new AlpineDtcpConnMux();

    connMux = static_cast<DtcpBaseConnMux *>(alpineMux);

    return true;
}


UdpConnection *
AlpineMulticastUdpTransport::createConnection()
{
    return new MulticastUdpConnection(multicastGroup_, multicastPort_);
}


bool
AlpineMulticastUdpTransport::handleData(const byte * data, uint dataLength)
{
#ifdef _VERBOSE
    Log::Debug("AlpineMulticastUdpTransport::handleData invoked.");
#endif

    return true;
}
