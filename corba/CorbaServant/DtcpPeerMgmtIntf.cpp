/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <DtcpPeerMgmtIntf.h>
#include <AlpineCorbaClient.h>
#include <Log.h>
#include <StringUtils.h>



bool  
DtcpPeerMgmtIntf::addDtcpPeer (const string &  ipAddress,
                               ushort          port)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpPeerMgmtIntf::addDtcpPeer invoked.");
#endif

    ExNewEnv;

    ExTry {

        AlpineCorbaClient::DtcpPeerMgmt::addDtcpPeer (ipAddress.c_str(), port, ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::DtcpPeerMgmt::addDtcpPeer in call to "
                             "DtcpPeerMgmtIntf::addDtcpPeer.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::DtcpPeerMgmt::addDtcpPeer in call to "
                             "DtcpPeerMgmtIntf::addDtcpPeer.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



bool  
DtcpPeerMgmtIntf::getDtcpPeerId (const string &  ipAddress,
                                 ushort          port,
                                 ulong &         peerId)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpPeerMgmtIntf::getDtcpPeerId invoked.");
#endif

    ExNewEnv;

    ExTry {

        peerId = AlpineCorbaClient::DtcpPeerMgmt::getDtcpPeerId (ipAddress.c_str(), port, ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::DtcpPeerMgmt::getDtcpPeerId in call to "
                             "DtcpPeerMgmtIntf::getDtcpPeerId.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::DtcpPeerMgmt::getDtcpPeerId in call to "
                             "DtcpPeerMgmtIntf::getDtcpPeerId.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



bool  
DtcpPeerMgmtIntf::getDtcpPeerStatus (ulong               peerId,
                                     t_DtcpPeerStatus &  status)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpPeerMgmtIntf::getDtcpPeerStatus invoked.");
#endif

    ExNewEnv;

    Alpine::t_DtcpPeerStatus_var  corbaPeerStatus;

    ExTry {

        AlpineCorbaClient::DtcpPeerMgmt::getDtcpPeerStatus (peerId, corbaPeerStatus.out (), ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::DtcpPeerMgmt::getDtcpPeerStatus in call to "
                             "DtcpPeerMgmtIntf::getDtcpPeerStatus.");
        return false;
    }
    ExCatchAll {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::DtcpPeerMgmt::getDtcpPeerStatus in call to "
                             "DtcpPeerMgmtIntf::getDtcpPeerStatus.");
        return false;
    }
    ExEndTry;
    ExCheck;


    if (corbaPeerStatus->ipAddress == 0) {
        // Nothing passed back, assume error
        Log::Error ("Nothing returned in call to AlpineCorbaClient::DtcpPeerMgmt::getDtcpPeerStatus!");
        return false;
    }

    status.ipAddress      = corbaPeerStatus->ipAddress;
    status.port           = corbaPeerStatus->port;
    status.lastRecvTime   = corbaPeerStatus->lastRecvTime;
    status.lastSendTime   = corbaPeerStatus->lastSendTime;
    status.avgBandwidth   = corbaPeerStatus->avgBandwidth;
    status.peakBandwidth  = corbaPeerStatus->peakBandwidth;


    return true;
}



bool  
DtcpPeerMgmtIntf::getAllDtcpPeerIds (t_DtcpPeerIdList &  peerIdList)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpPeerMgmtIntf::getAllDtcpPeerIds invoked.");
#endif

    ExNewEnv;

    Alpine::t_DtcpPeerIdList_var  corbaPeerList;

    ExTry {

        AlpineCorbaClient::DtcpPeerMgmt::getAllDtcpPeerIds (corbaPeerList.out (), ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::DtcpPeerMgmt::getAllDtcpPeerIds in call to "
                             "DtcpPeerMgmtIntf::getAllDtcpPeerIds.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::DtcpPeerMgmt::getAllDtcpPeerIds in call to "
                             "DtcpPeerMgmtIntf::getAllDtcpPeerIds.");
        return false;
    }
    ExEndTry;
    ExCheck;


    // process returned list
    //
    peerIdList.clear ();

    uint    i;
    ulong  currId;

    for (i = 0; i < corbaPeerList->length (); i++) {
        currId = corbaPeerList[i];
        peerIdList.push_back (currId);
    }


    return true;
}



bool  
DtcpPeerMgmtIntf::activateDtcpPeer (ulong  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpPeerMgmtIntf::activateDtcpPeer invoked.");
#endif

    ExNewEnv;

    ExTry {

        AlpineCorbaClient::DtcpPeerMgmt::activateDtcpPeer (peerId, ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::DtcpPeerMgmt::activateDtcpPeer in call to "
                             "DtcpPeerMgmtIntf::activateDtcpPeer.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::DtcpPeerMgmt::activateDtcpPeer in call to "
                             "DtcpPeerMgmtIntf::activateDtcpPeer.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



bool  
DtcpPeerMgmtIntf::deactivateDtcpPeer (ulong  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpPeerMgmtIntf::deactivateDtcpPeer invoked.");
#endif

    ExNewEnv;

    ExTry {

        AlpineCorbaClient::DtcpPeerMgmt::deactivateDtcpPeer (peerId, ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::DtcpPeerMgmt::deactivateDtcpPeer in call to "
                             "DtcpPeerMgmtIntf::deactivateDtcpPeer.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::DtcpPeerMgmt::deactivateDtcpPeer in call to "
                             "DtcpPeerMgmtIntf::deactivateDtcpPeer.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



bool  
DtcpPeerMgmtIntf::pingDtcpPeer (ulong  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpPeerMgmtIntf::pingDtcpPeer invoked.");
#endif

    ExNewEnv;

    ExTry {

        AlpineCorbaClient::DtcpPeerMgmt::pingDtcpPeer (peerId, ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::DtcpPeerMgmt::pingDtcpPeer in call to "
                             "DtcpPeerMgmtIntf::pingDtcpPeer.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::DtcpPeerMgmt::pingDtcpPeer in call to "
                             "DtcpPeerMgmtIntf::pingDtcpPeer.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



bool  
DtcpPeerMgmtIntf::excludeHost (const string & ipAddress)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpPeerMgmtIntf::excludeHost invoked.");
#endif

    ExNewEnv;

    ExTry {

        AlpineCorbaClient::DtcpPeerMgmt::excludeHost (ipAddress.c_str(), ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::DtcpPeerMgmt::excludeHost in call to "
                             "DtcpPeerMgmtIntf::excludeHost.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::DtcpPeerMgmt::excludeHost in call to "
                             "DtcpPeerMgmtIntf::excludeHost.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



bool  
DtcpPeerMgmtIntf::excludeSubnet (const string &  subnetIpAddress,
                                 const string &  subnetMask)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpPeerMgmtIntf::excludeSubnet invoked.");
#endif

    ExNewEnv;

    ExTry {

        AlpineCorbaClient::DtcpPeerMgmt::excludeSubnet (subnetIpAddress.c_str(),
                                                        subnetMask.c_str(), ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::DtcpPeerMgmt::excludeSubnet in call to "
                             "DtcpPeerMgmtIntf::excludeSubnet.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::DtcpPeerMgmt::excludeSubnet in call to "
                             "DtcpPeerMgmtIntf::excludeSubnet.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



bool  
DtcpPeerMgmtIntf::allowHost (const string & ipAddress)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpPeerMgmtIntf::allowHost invoked.");
#endif

    ExNewEnv;

    ExTry {

        AlpineCorbaClient::DtcpPeerMgmt::allowHost (ipAddress.c_str(), ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::DtcpPeerMgmt::allowHost in call to "
                             "DtcpPeerMgmtIntf::allowHost.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::DtcpPeerMgmt::allowHost in call to "
                             "DtcpPeerMgmtIntf::allowHost.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



bool  
DtcpPeerMgmtIntf::allowSubnet (const string &  subnetIpAddress)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpPeerMgmtIntf::allowSubnet invoked.");
#endif

    ExNewEnv;

    ExTry {

        AlpineCorbaClient::DtcpPeerMgmt::allowSubnet (subnetIpAddress.c_str(), ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::DtcpPeerMgmt::allowSubnet in call to "
                             "DtcpPeerMgmtIntf::allowSubnet.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::DtcpPeerMgmt::allowSubnet in call to "
                             "DtcpPeerMgmtIntf::allowSubnet.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return false;
}



bool  
DtcpPeerMgmtIntf::listExcludedHosts (t_IpAddressList & ipAddressList)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpPeerMgmtIntf::listExcludedHosts invoked.");
#endif

    ExNewEnv;

    Alpine::t_IpAddressList_var  corbaIpList;

    ExTry {

        AlpineCorbaClient::DtcpPeerMgmt::listExcludedHosts (corbaIpList.out (), ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::DtcpPeerMgmt::listExcludedHosts in call to "
                             "DtcpPeerMgmtIntf::listExcludedHosts.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::DtcpPeerMgmt::listExcludedHosts in call to "
                             "DtcpPeerMgmtIntf::listExcludedHosts.");
        return false;
    }
    ExEndTry;
    ExCheck;

    // process returned list
    //
    ipAddressList.clear ();

    uint    i;
    string currIp; 

    for (i = 0; i < corbaIpList->length (); i++) {
        currIp = corbaIpList[i];
        ipAddressList.push_back (currIp);
    }


    return true;
}



bool  
DtcpPeerMgmtIntf::listExcludedSubnets (t_SubnetAddressList &  subnetAddressList)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpPeerMgmtIntf::listExcludedSubnets invoked.");
#endif

    ExNewEnv;

    Alpine::t_SubnetAddressList_var  corbaSubnetList;

    ExTry {

        AlpineCorbaClient::DtcpPeerMgmt::listExcludedSubnets (corbaSubnetList.out (), ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::DtcpPeerMgmt::listExcludedSubnets in call to "
                             "DtcpPeerMgmtIntf::listExcludedSubnets.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::DtcpPeerMgmt::listExcludedSubnets in call to "
                             "DtcpPeerMgmtIntf::listExcludedSubnets.");
        return false;
    }
    ExEndTry;
    ExCheck;

    // process returned list
    //
    subnetAddressList.clear ();

    uint    i;
    t_SubnetAddress  currAddress;

    for (i = 0; i < corbaSubnetList->length (); i++) {
        currAddress.ipAddress = corbaSubnetList[i].ipAddress;
        currAddress.netMask = corbaSubnetList[i].netMask;

        subnetAddressList.push_back (currAddress);
    }


    return true;
}



