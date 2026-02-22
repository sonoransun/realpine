/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineStackConfig.h>
#include <NetUtils.h>
#include <Log.h>
#include <StringUtils.h>



AlpineStackConfig::AlpineStackConfig ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackConfig constructor invoked.");
#endif

    localIpAddress_        = 0;
    localPort_             = 0;
    maxConcurrentQueries_  = 0;

    multicastEnabled_      = false;
    multicastPort_         = 0;
    broadcastEnabled_      = false;
    broadcastPort_         = 0;
    rawWifiEnabled_        = false;
}



AlpineStackConfig::AlpineStackConfig (const AlpineStackConfig & copy)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackConfig copy constructor invoked.");
#endif

    localIpAddress_        = copy.localIpAddress_;
    localPort_             = copy.localPort_;
    maxConcurrentQueries_  = copy.maxConcurrentQueries_;

    multicastEnabled_      = copy.multicastEnabled_;
    multicastGroup_        = copy.multicastGroup_;
    multicastPort_         = copy.multicastPort_;
    broadcastEnabled_      = copy.broadcastEnabled_;
    broadcastPort_         = copy.broadcastPort_;
    rawWifiEnabled_        = copy.rawWifiEnabled_;
    rawWifiInterface_      = copy.rawWifiInterface_;
}



// Dtor defaulted in header



AlpineStackConfig &
AlpineStackConfig::operator = (const AlpineStackConfig & copy)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackConfig assignment invoked.");
#endif

    if (&copy == this) {
        return *this;
    }

    localIpAddress_        = copy.localIpAddress_;
    localPort_             = copy.localPort_;
    maxConcurrentQueries_  = copy.maxConcurrentQueries_;

    multicastEnabled_      = copy.multicastEnabled_;
    multicastGroup_        = copy.multicastGroup_;
    multicastPort_         = copy.multicastPort_;
    broadcastEnabled_      = copy.broadcastEnabled_;
    broadcastPort_         = copy.broadcastPort_;
    rawWifiEnabled_        = copy.rawWifiEnabled_;
    rawWifiInterface_      = copy.rawWifiInterface_;

    return *this;
}



bool  
AlpineStackConfig::setLocalEndpoint (const string &  ipAddress,
                                     ushort          port)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackConfig::setLocalEndpoint invoked.  Values: "s +
                "\n IP Address: "s + ipAddress +
                "\n Port: "s + std::to_string (port) +
                "\n");
#endif

    bool   status;
    ulong  localIp;

    status = NetUtils::stringIpToLong (ipAddress, localIp);

    if (!status) {
        Log::Error ("Invalid IP address passed in call to AlpineStackConfig::setLocalEndpoint!");
        return false;
    }

    if (port == 0) {
        Log::Error ("Invalid port passed in call to AlpineStackConfig::setLocalEndpoint!");
        return false;
    }

    localIpAddress_ = localIp;
    localPort_      = port;
    

    return true;
}



bool  
AlpineStackConfig::getLocalEndpoint (string &   ipAddress,
                                     ushort &  port)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackConfig::getLocalEndpoint invoked.");
#endif

    if (localIpAddress_ == 0) {
        Log::Error ("Call to AlpineStackConfig::getLocalEndpoint before initialization!");
        return false;
    }

    NetUtils::longIpToString (localIpAddress_, ipAddress);
    port = localPort_;


    return true;
}



bool  
AlpineStackConfig::setLocalEndpoint (ulong   ipAddress,
                                     ushort  port)
{
#ifdef _VERBOSE
    string ipAddressString;
    NetUtils::longIpToString (ipAddress, ipAddressString);

    Log::Debug ("AlpineStackConfig::setLocalEndpoint invoked.  Values: "s +
                "\n IP Address: "s + ipAddressString +
                "\n Port: "s + std::to_string (port) +
                "\n");
#endif

    if (ipAddress == 0) {
        Log::Error ("Invalid IP address passed in call to AlpineStackConfig::setLocalEndpoint!");
        return false;
    }

    if (port == 0) {
        Log::Error ("Invalid port passed in call to AlpineStackConfig::setLocalEndpoint!");
        return false;
    }

    localIpAddress_ = ipAddress;
    localPort_      = port;


    return true;
}



bool  
AlpineStackConfig::getLocalEndpoint (ulong &   ipAddress,
                                     ushort &  port)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackConfig::getLocalEndpoint invoked.");
#endif

    if (localIpAddress_ == 0) {
        Log::Error ("Call to AlpineStackConfig::getLocalEndpoint before initialization!");
        return false;
    }

    ipAddress = localIpAddress_;
    port      = localPort_;


    return true;
}



bool  
AlpineStackConfig::setMaxConcurrentQueries (ulong  max)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackConfig::setMaxConcurrentQueries invoked.  Max: "s +
                std::to_string (max));
#endif

    if (max == 0) {
        Log::Error ("Invalid maximum passed in call to AlpineStackConfig::setMaxConcurrentQueries!");
        return false;
    }

    maxConcurrentQueries_ = max;
    

    return true;
}



bool
AlpineStackConfig::getMaxConcurrentQueries (ulong &  max)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackConfig::getMaxConcurrentQueries invoked.");
#endif

    if (maxConcurrentQueries_ == 0) {
        Log::Error ("Call to AlpineStackConfig::getMaxConcurrentQueries before initialization!");
        return false;
    }

    max = maxConcurrentQueries_;


    return true;
}



bool
AlpineStackConfig::setMulticastEndpoint (const string & group, ushort port)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackConfig::setMulticastEndpoint invoked.");
#endif

    if (group.empty() || port == 0) {
        Log::Error ("Invalid multicast parameters in AlpineStackConfig::setMulticastEndpoint!");
        return false;
    }

    multicastGroup_   = group;
    multicastPort_    = port;
    multicastEnabled_ = true;

    return true;
}



bool
AlpineStackConfig::getMulticastEndpoint (string & group, ushort & port)
{
    if (!multicastEnabled_) {
        return false;
    }

    group = multicastGroup_;
    port  = multicastPort_;

    return true;
}



bool
AlpineStackConfig::multicastEnabled ()
{
    return multicastEnabled_;
}



bool
AlpineStackConfig::setBroadcastEndpoint (ushort port)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackConfig::setBroadcastEndpoint invoked.");
#endif

    if (port == 0) {
        Log::Error ("Invalid broadcast port in AlpineStackConfig::setBroadcastEndpoint!");
        return false;
    }

    broadcastPort_    = port;
    broadcastEnabled_ = true;

    return true;
}



bool
AlpineStackConfig::getBroadcastEndpoint (ushort & port)
{
    if (!broadcastEnabled_) {
        return false;
    }

    port = broadcastPort_;

    return true;
}



bool
AlpineStackConfig::broadcastEnabled ()
{
    return broadcastEnabled_;
}



bool
AlpineStackConfig::setRawWifiInterface (const string & interfaceName)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackConfig::setRawWifiInterface invoked.");
#endif

    if (interfaceName.empty()) {
        Log::Error ("Invalid interface name in AlpineStackConfig::setRawWifiInterface!");
        return false;
    }

    rawWifiInterface_ = interfaceName;
    rawWifiEnabled_   = true;

    return true;
}



bool
AlpineStackConfig::getRawWifiInterface (string & interfaceName)
{
    if (!rawWifiEnabled_) {
        return false;
    }

    interfaceName = rawWifiInterface_;

    return true;
}



bool
AlpineStackConfig::rawWifiEnabled ()
{
    return rawWifiEnabled_;
}



