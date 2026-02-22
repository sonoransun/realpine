/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineDtcpUdpTransport.h>
#include <AlpineDtcpConnMux.h>
#include <Log.h>
#include <StringUtils.h>


AlpineDtcpUdpTransport::AlpineDtcpUdpTransport (const ulong ipAddress,
                                                const int   port)
    : DtcpBaseUdpTransport (ipAddress, port)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineDtcpUdpTransport constructor invoked.");
#endif

}


    
AlpineDtcpUdpTransport::~AlpineDtcpUdpTransport ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineDtcpUdpTransport destructor invoked.");
#endif
}



bool 
AlpineDtcpUdpTransport::createMux (DtcpBaseConnMux *& connMux)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineDtcpUdpTransport::createMux invoked.");
#endif

    AlpineDtcpConnMux * alpineMux;
    alpineMux = new AlpineDtcpConnMux ();

    connMux = static_cast<DtcpBaseConnMux *>(alpineMux);


    return true;
}



bool 
AlpineDtcpUdpTransport::handleData (const byte * data,
                                    uint         dataLength)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineDtcpUdpTransport::handleData invoked.");
#endif

    return true;
}




