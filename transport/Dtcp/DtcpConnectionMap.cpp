/// Copyright (C) 2026 sonoransun — see LICENCE.txt



#include <DtcpConnectionMap.h>
#include <DtcpBaseConnTransport.h>
#include <Platform.h>
#include <Log.h>
#include <StringUtils.h>



DtcpConnectionMap::DtcpConnectionMap ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpConnectionMap constructor invoked.");
#endif

    numConnections_   = 0;
    currConnectionId_ = time (nullptr);

    peerIndex_.clear ();
    addressIndex_.clear ();
    pendingIndex_.clear ();
}



DtcpConnectionMap::~DtcpConnectionMap ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpConnectionMap destructor invoked.");
#endif

}



bool 
DtcpConnectionMap::exists (ulong ipAddress,
                           int   port)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpConnectionMap::exists invoked.");
#endif

    auto ipAddrIter = addressIndex_.find (ipAddress);

    if (ipAddrIter == addressIndex_.end ()) {
        return false;
    }

    t_PortIndex * portIndex;
    portIndex = (*ipAddrIter).second;

    return portIndex->find (port) != portIndex->end ();
}



bool 
DtcpConnectionMap::exists (ulong peerId)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpConnectionMap::exists invoked.");
#endif

    if (peerIndex_.find (peerId) == peerIndex_.end ()) {

        // Check the pending index...
        if (pendingIndex_.find (peerId) == pendingIndex_.end ()) {
            return false;
        }
    }

    return true;
}



bool 
DtcpConnectionMap::createConnection (ulong    ipAddress,
                                     int      port,
                                     ulong &  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpConnectionMap::createConnection invoked.");
#endif

    if (exists (ipAddress, port)) {
        Log::Error ("Duplicate ipAddress / port for request in DtcpConnectionMap::createConnection.");

        return false;
    }

    peerId = currConnectionId_++;
    numConnections_++;

    // No transport is created until the connection is accepted.
    //
    DtcpBaseConnTransport * connection = nullptr;

    // Add to index containers
    //
    peerIndex_.insert ( t_PeerIdIndexPair (peerId, connection) );

    t_PortIndex * portIndex;

    auto ipAddrIter = addressIndex_.find (ipAddress);

    if (ipAddrIter == addressIndex_.end ()) {
        portIndex = new t_PortIndex;
        addressIndex_.insert ( t_IpAddrPortIndexPair (ipAddress, portIndex) );
    }
    else {
        portIndex = (*ipAddrIter).second;
    }

    portIndex->insert ( t_PortIndexPair (port, connection) );

    t_IpPortPair * pendingRecord;
    pendingRecord = new t_IpPortPair;
    pendingRecord->ipAddress = ipAddress;
    pendingRecord->port      = port;
    gettimeofday(&(pendingRecord->timestamp), nullptr);

    pendingIndex_.insert ( t_PendingAcceptIndexPair (peerId, pendingRecord) );

    return true;
}



bool 
DtcpConnectionMap::pendingAccept (ulong peerId)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpConnectionMap::pendingAccept invoked.");
#endif

    return pendingIndex_.find (peerId) != pendingIndex_.end ();
}



bool 
DtcpConnectionMap::indexConnection (ulong                    peerId,
                                    DtcpBaseConnTransport *  connection)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpConnectionMap::indexConnection invoked.");
#endif

    t_IpPortPair * pendingRecord;
    auto pendingIter = pendingIndex_.find (peerId);

    if (pendingIter == pendingIndex_.end ()) {
        // Nope, no connection pending here...
        Log::Error ("DtcpConnectionMap::indexConnection invoked without pending accept for ID.");

        return false;
    }

    pendingRecord = (*pendingIter).second;


    // Update connection indexes
    //
    auto peerIndexIter = peerIndex_.find (peerId);

    if (peerIndexIter == peerIndex_.end ()) {
        // serious problem?
        Log::Error ("Unable to locate peer index record in DtcpConnectionMap::indexConnection.");

        return false;
    }

    (*peerIndexIter).second = connection;


    t_PortIndex * portIndex;

    auto ipAddrIter = addressIndex_.find (pendingRecord->ipAddress);

    if (ipAddrIter == addressIndex_.end ()) {
        // Should be here, big problem...
        return false;
    }
    else {
        portIndex = (*ipAddrIter).second;
    }

    auto portIter = portIndex->find (pendingRecord->port);

    if (portIter == portIndex->end ()) {
        // This should always work...  ;)
        return false;
    }

    (*portIter).second = connection;
        
    pendingIndex_.erase (peerId);
    delete pendingRecord;


    return true;
}



bool 
DtcpConnectionMap::locateConnection (ulong                     peerId,
                                     DtcpBaseConnTransport *&  connection)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpConnectionMap::locateConnection invoked.");
#endif

    auto iter = peerIndex_.find (peerId);

    if (iter == peerIndex_.end ()) {
        return false;
    }

    connection = (*iter).second;

    return true;
}



bool 
DtcpConnectionMap::locateConnection (ulong                     ipAddress,
                                     int                       port,
                                     DtcpBaseConnTransport *&  connection)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpConnectionMap::locateConnection invoked.");
#endif

    auto ipAddrIter = addressIndex_.find (ipAddress);

    if (ipAddrIter == addressIndex_.end ()) {
        return false;
    }

    t_PortIndex * portIndex;
    portIndex = (*ipAddrIter).second;

    auto portIter = portIndex->find (port);

    if (portIter == portIndex->end ()) {
        return false;
    }                  

    connection = (*portIter).second;

    // Dont return null pointer if connection pending...
    return connection != nullptr;
}



int  
DtcpConnectionMap::numConnections (ulong ipAddress)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpConnectionMap::numConnections invoked.");
#endif

    return numConnections_;
}



bool 
DtcpConnectionMap::removePendingConnection (ulong  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpConnectionMap::removePendingConnection invoked.");
#endif

    return true;
}



bool 
DtcpConnectionMap::removeConnection (DtcpBaseConnTransport * connection)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpConnectionMap::removeConnection invoked.");
#endif

    return true;
}



