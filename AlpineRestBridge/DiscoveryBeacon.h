/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <SysThread.h>
#include <UdpConnection.h>


class DiscoveryBeacon : public SysThread
{
  public:

    DiscoveryBeacon ();
    ~DiscoveryBeacon ();

    bool initialize (ushort restPort, ushort beaconPort);

    /// Initialize with an IPv6 multicast group for beacon announcements.
    /// Falls back to IPv4 broadcast if multicastGroup is empty.
    bool initialize (ushort restPort, ushort beaconPort,
                     const string & multicastGroup);

    void threadMain ();


  private:

    static const int    BEACON_INTERVAL_SEC = 3;
    static const int    SLEEP_INCREMENT_MS  = 1000;

    UdpConnection       udpSocket_;
    ushort              restPort_;
    ushort              beaconPort_;
    string              multicastGroup_;

};
