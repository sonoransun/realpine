/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <DtcpFilter.h>
#include <Log.h>
#include <NetUtils.h>
#include <ReadLock.h>
#include <StringUtils.h>
#include <WriteLock.h>


DtcpFilter::t_IpAddressSet * DtcpFilter::ipAddressSet_s = nullptr;
DtcpFilter::t_NetworkMaskSet * DtcpFilter::networkMaskSet_s = nullptr;
bool DtcpFilter::initialized_s = false;
ReadWriteSem DtcpFilter::dataLock_s;


// Ctor defaulted in header


// Dtor defaulted in header


bool
DtcpFilter::initialize()
{
#ifdef _VERBOSE
    Log::Debug("DtcpFilter::initialize invoked.");
#endif

    WriteLock lock(dataLock_s);

    if (initialized_s) {
        Log::Error("Attempt to reinitialize DtcpFiler!");
        return false;
    }

    ipAddressSet_s = new t_IpAddressSet;
    networkMaskSet_s = new t_NetworkMaskSet;
    initialized_s = true;


    return true;
}


bool
DtcpFilter::validAddress(ulong ipAddress)
{
#ifdef _VERBOSE
    Log::Debug("DtcpFilter::validAddress invoked.");
#endif

    ReadLock lock(dataLock_s);

    if (!initialized_s) {
        // Must be initialized!
        //
        Log::Error("call to DtcpFilter::validAddress before initialization!");
        return false;
    }

    auto ipIter = ipAddressSet_s->find(ipAddress);

    if (ipIter != ipAddressSet_s->end()) {
        // This host address is banned
        //
        return false;
    }

    // MRP_TEMP check network bans

    return true;
}


bool
DtcpFilter::addIpAddressBan(ulong ipAddress)
{
#ifdef _VERBOSE
    Log::Debug("DtcpFilter::addIpAddressBan invoked.");
#endif

    WriteLock lock(dataLock_s);

    if (!initialized_s) {
        // Must be initialized!
        //
        Log::Error("call to DtcpFilter::addIpAddressBan before initialization!");
        return false;
    }

    auto ipIter = ipAddressSet_s->find(ipAddress);

    if (ipIter != ipAddressSet_s->end()) {
        // This host address is already banned
        //
        return false;
    }

    ipAddressSet_s->insert(ipAddress);


    return true;
}


bool
DtcpFilter::removeIpAddressBan(ulong ipAddress)
{
#ifdef _VERBOSE
    Log::Debug("DtcpFilter::removeIpAddressBan invoked.");
#endif

    WriteLock lock(dataLock_s);

    if (!initialized_s) {
        // Must be initialized!
        //
        Log::Error("call to DtcpFilter::removeIpAddressBan before initialization!");
        return false;
    }

    auto ipIter = ipAddressSet_s->find(ipAddress);

    if (ipIter == ipAddressSet_s->end()) {
        // This host address is not banned
        //
        return false;
    }

    ipAddressSet_s->erase(ipAddress);


    return true;
}


bool
DtcpFilter::numIpAddressFiltered(ulong & count)
{
#ifdef _VERBOSE
    Log::Debug("DtcpFilter::numIpAddressFiltered invoked.");
#endif

    ReadLock lock(dataLock_s);

    if (!initialized_s) {
        // Must be initialized!
        //
        Log::Error("call to DtcpFilter::numIpAddressFiltered before initialization!");
        return false;
    }

    count = ipAddressSet_s->size();


    return true;
}


bool
DtcpFilter::getFilteredList(t_IpAddressList & list)
{
#ifdef _VERBOSE
    Log::Debug("DtcpFilter::getFilteredList invoked.");
#endif

    ReadLock lock(dataLock_s);

    if (!initialized_s) {
        // Must be initialized!
        //
        Log::Error("call to DtcpFilter::getFilteredList before initialization!");
        return false;
    }

    // MRP_TEMP ipAddressSet_s->size ();


    return true;
}


bool
DtcpFilter::addNetworkBan(ulong network, ulong mask)
{
#ifdef _VERBOSE
    Log::Debug("DtcpFilter::addNetworkBan invoked.");
#endif

    WriteLock lock(dataLock_s);

    if (!initialized_s) {
        // Must be initialized!
        //
        Log::Error("call to DtcpFilter::addNetworkBan before initialization!");
        return false;
    }

    return true;
}


bool
DtcpFilter::removeNetworkBan(ulong network, ulong mask)
{
#ifdef _VERBOSE
    Log::Debug("DtcpFilter::removeNetworkBan invoked.");
#endif

    WriteLock lock(dataLock_s);

    if (!initialized_s) {
        // Must be initialized!
        //
        Log::Error("call to DtcpFilter::removeNetworkBan before initialization!");
        return false;
    }

    return true;
}


bool
DtcpFilter::numNetworkFiltered(ulong & count)
{
#ifdef _VERBOSE
    Log::Debug("DtcpFilter::numNetworkFiltered invoked.");
#endif

    ReadLock lock(dataLock_s);

    if (!initialized_s) {
        // Must be initialized!
        //
        Log::Error("call to DtcpFilter::numNetworkFiltered before initialization!");
        return false;
    }

    return true;
}


bool
DtcpFilter::getFilteredList(t_NetworkMaskList & list)
{
#ifdef _VERBOSE
    Log::Debug("DtcpFilter::getFilteredList invoked.");
#endif

    ReadLock lock(dataLock_s);

    if (!initialized_s) {
        // Must be initialized!
        //
        Log::Error("call to DtcpFilter::getFilteredList before initialization!");
        return false;
    }

    return true;
}


void
DtcpFilter::packNetworkMask(ulong network, ulong mask, ulonglong & packedNetMask)
{
#ifdef _VERBOSE
    Log::Debug("DtcpFilter::packNetworkMask invoked.");
#endif
}


void
DtcpFilter::unpackNetworkMask(const ulonglong & packedNetMask, ulong & network, ulong & mask)
{
#ifdef _VERBOSE
    Log::Debug("DtcpFilter::unpackNetworkMask invoked.");
#endif
}
