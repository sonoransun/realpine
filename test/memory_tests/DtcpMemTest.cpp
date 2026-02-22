/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <DtcpMemTest.h>



DtcpMemTest::DtcpMemTest ()
{
    currPeerId_      = 1;
    currTransportId_ = 1;

    transportList_   = new t_TransportList;
#ifndef _LEAN_N_MEAN
    locationList_    = new t_LocationList;   
#endif
    peerIdIndex_     = new t_PeerIdIndex;
    locationIndex_   = new t_LocationIndex;
    transportIndex_  = new t_TransportIdIndex;
}



DtcpMemTest::~DtcpMemTest ()
{
    cleanUp ();
}



bool  
DtcpMemTest::allocate (ulong  numPeers)
{
    t_ConnTransportData *  currTransport;
#ifndef _LEAN_N_MEAN
    t_IpPortPair *         currLocation;
#endif
    ulong i;

    for (i = 0; i < numPeers; i++) {

        currTransport = new t_ConnTransportData;
        memset (currTransport, 0, sizeof(t_ConnTransportData));

#ifndef _LEAN_N_MEAN
        currLocation = new t_IpPortPair;
        memset (currLocation, 0, sizeof(t_IpPortPair));
#endif

        transportList_->push_back (currTransport);
#ifndef _LEAN_N_MEAN
        locationList_->push_back (currLocation);
#endif

        peerIdIndex_->emplace (currPeerId_, currTransport);

#ifndef _LEAN_N_MEAN
        locationIndex_->emplace (currPeerId_, currLocation);
#else
        locationIndex_->emplace (currPeerId_, currTransportId_);
#endif

        transportIndex_->emplace (currTransportId_, currTransport);

        currPeerId_++;
        currTransportId_++;
    }


    return true;
}



bool  
DtcpMemTest::cleanUp ()
{
    for (auto& item : *transportList_) {
        delete item;
    }

#ifndef _LEAN_N_MEAN
    for (auto& item : *locationList_) {
        delete item;
    }
#endif

    transportList_->clear ();
#ifndef _LEAN_N_MEAN
    locationList_->clear ();
#endif
    peerIdIndex_->clear ();
    locationIndex_->clear ();
    transportIndex_->clear ();


    return true;
}



