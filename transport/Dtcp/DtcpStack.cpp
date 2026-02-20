///////
///
///  Copyright (C) 2026  sonoransun
///
///  Permission is hereby granted, free of charge, to any person obtaining a copy
///  of this software and associated documentation files (the "Software"), to deal
///  in the Software without restriction, including without limitation the rights
///  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
///  copies of the Software, and to permit persons to whom the Software is
///  furnished to do so, subject to the following conditions:
///
///  The above copyright notice and this permission notice shall be included in all
///  copies or substantial portions of the Software.
///
///  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
///  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
///  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
///  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
///  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
///  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
///  SOFTWARE.
///
///////


#include <DtcpStack.h>
#include <DtcpBaseUdpTransport.h>
#include <DtcpBaseConnTransport.h>
#include <DtcpBaseConnConnector.h>
#include <DtcpBroadcastMgr.h>
#include <WriteLock.h>
#include <ReadLock.h>
#include <Log.h>
#include <StringUtils.h>
#include <NetUtils.h>


DtcpBaseUdpTransport *                  DtcpStack::udpTransport_s = nullptr;
DtcpBaseConnMux *                       DtcpStack::connMux_s = nullptr;
ulong                                   DtcpStack::currTransportId_s = 0;
ReadWriteSem                            DtcpStack::dataLock_s;

std::unique_ptr<DtcpStack::t_IpAddressSet>        DtcpStack::ipAddressFilter_s;
std::unique_ptr<DtcpStack::t_SubnetAddressIndex>  DtcpStack::subnetFilter_s;
ReadWriteSem                            DtcpStack::filterLock_s;

std::unique_ptr<DtcpStack::t_ConnTransportIndex>  DtcpStack::transportIndex_s;
ReadWriteSem                            DtcpStack::transportIndexLock_s;



// Ctor defaulted in header


// Dtor defaulted in header



bool  
DtcpStack::initialize ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStack::initialize invoked.");
#endif

    WriteLock  filterLock(filterLock_s);
    WriteLock  transLock(transportIndexLock_s);


    // Initialize the broadcast manager
    //
    bool status;
    status = DtcpBroadcastMgr::initialize ();

    if (!status) {
        Log::Error ("initialization of DtcpBroadcastMgr failed in DtcpStack::initialize!");
        return false;
    }
    if (ipAddressFilter_s) {
        Log::Error ("Invalid duplicate call to DtcpStack::initialize.");
        return false;
    }
    ipAddressFilter_s  = std::make_unique<t_IpAddressSet>();
    subnetFilter_s     = std::make_unique<t_SubnetAddressIndex>();
    transportIndex_s   = std::make_unique<t_ConnTransportIndex>();
    

    return true;
}



uint  
DtcpStack::numConnTransports ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStack::numConnTransports invoked.");
#endif

    uint transportCount = 0;

    ReadLock  lock(transportIndexLock_s);

    if (transportIndex_s) {
        transportCount = transportIndex_s->size ();
    }


    return transportCount;
}



bool  
DtcpStack::exists (ulong   ipAddress,
                   ushort  port)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStack::exists invoked.");
#endif


    return true;
}



bool
DtcpStack::exists (ulong  transportId)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStack::exists invoked.  Transport ID: "s +
                std::to_string(transportId));
#endif


    return true;
}



bool  
DtcpStack::getAllConnTransports (t_ConnTransportList & transportList)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStack::getAllConnTransports invoked.");
#endif

    transportList.clear ();

    ReadLock  lock(transportIndexLock_s);

    if (transportIndex_s) {
        for (auto& [key, value] : *transportIndex_s) {
            transportList.push_back (value);
        }
    }


    return true;
}



bool  
DtcpStack::locateTransport (ulong                    transportId,
                            DtcpBaseConnTransport *& transport)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStack::locateTransport invoked.");
#endif

    ReadLock  lock(transportIndexLock_s);

    if (!transportIndex_s) {
        return false;
    }
    auto iter = transportIndex_s->find (transportId);

    if (iter == transportIndex_s->end ()) {
        return false;
    }
    transport = (*iter).second;


    return true;
}



bool  
DtcpStack::locateTransport (ulong                    ipAddress,
                            ushort                   port,
                            DtcpBaseConnTransport *& transport)
{
#ifdef _VERBOSE
    string  ipAddressStr;
    NetUtils::longIpToString (ipAddress, ipAddressStr);
    Log::Debug ("DtcpStack::locateTransport invoked."s +
                "\nIP Address: "s + ipAddressStr +
                "\nPort: "s + std::to_string (ntohs(port)) +
                "\n");
#endif


    return true;
}



bool  
DtcpStack::createTransport  (ulong                    ipAddress,
                             ushort                   port,
                             DtcpBaseConnTransport *& transport)
{
#ifdef _VERBOSE
    string  ipAddressStr;
    NetUtils::longIpToString (ipAddress, ipAddressStr);
    Log::Debug ("DtcpStack::createTransport invoked."s +
                "\nIP Address: "s + ipAddressStr +
                "\nPort: "s + std::to_string (ntohs(port)) +
                "\n");
#endif

    // Try creating transport with connector
    //
    bool  status;
    DtcpBaseConnConnector * connector;
    status = udpTransport_s->createConnector (connector);

    if (!status) {
        Log::Error ("Call to createConnector failed in DtcpStack::createTransport!");
        return false;
    }
    status = connector->requestConnection (ipAddress, port);

    if (!status) {
        Log::Error ("Connector requestConnection failed in DtcpStack::createTransport!");
        return false;
    }
    // MRP_TEMP should wait for completion


    return true;
}



bool  
DtcpStack::hostIsExcluded (ulong  ipAddress)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStack::hostIsExcluded invoked.");
#endif


    return true;
}



bool  
DtcpStack::subnetIsExcluded (ulong  subnetIpAddress)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStack::subnetIsExcluded invoked.");
#endif


    return true;
}



bool  
DtcpStack::excludeHost (ulong ipAddress)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStack::excludeHost invoked.");
#endif


    return true;
}



bool  
DtcpStack::excludeSubnet (ulong  subnetIpAddress,
                          ulong  subnetMask)
{
#ifdef _VERBOSE  
    Log::Debug ("DtcpStack::excludeSubnet invoked.");
#endif


    return true;
}



bool  
DtcpStack::allowHost (ulong ipAddress)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStack::allowHost invoked.");
#endif


    return true;
}



bool  
DtcpStack::allowSubnet (ulong  subnetIpAddress)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStack::allowHost invoked.");
#endif


    return true;
}



bool  
DtcpStack::getAllExcludedHosts (t_IpAddressList & ipAddressList)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStack::getAllExcludedHosts invoked.");
#endif


    return true;
}



bool  
DtcpStack::getAllExcludedSubnets (t_SubnetAddressList & subnetList)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStack::getAllExcludedSubnets invoked.");
#endif
    
    
    return true;
}                      
    
    

bool  
DtcpStack::getNextTransportId (ulong & transportId)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStack::getNextTransportId invoked.");
#endif

    WriteLock  lock(dataLock_s);

    transportId = currTransportId_s++;


    return true;
}



bool  
DtcpStack::registerUdpTransport (DtcpBaseUdpTransport * udpTransport,
                                 DtcpBaseConnMux *      connMux)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStack::registerUdpTransport invoked.");
#endif

    WriteLock  lock(dataLock_s);

    if (udpTransport_s) {
        // only one transport is allowed!
        Log::Error ("Duplicate call to DtcpStack::registerUdpTransport.  Multiple UdpTransports?");
        return false;
    }
    udpTransport_s = udpTransport;
    connMux_s      = connMux;


    return true;
}



bool  
DtcpStack::registerConnTransport (DtcpBaseConnTransport * transport)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStack::registerConnTransport invoked.");
#endif

    ulong transportId;
    bool  status;

    status = transport->getTransportId (transportId);

    if (!status) {
        Log::Error ("getTransportId failed in DtcpStack::registerConnTransport.");
        return false;
    }
    auto iter = transportIndex_s->find (transportId);

    if (iter != transportIndex_s->end ()) {
        Log::Error ("Duplicate transport ID encountered in DtcpStack::registerConnTransport.");
        return false;
    }
    transportIndex_s->emplace(transportId, transport);
    

    return true;
}



bool  
DtcpStack::deactivateConnTransport (DtcpBaseConnTransport * transport)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStack::deactivateConnTransport invoked.");
#endif


    return true;
}



