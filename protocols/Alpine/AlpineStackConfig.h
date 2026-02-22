/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>


class AlpineStackConfig
{
  public:

    AlpineStackConfig ();

    AlpineStackConfig (const AlpineStackConfig & copy);

    ~AlpineStackConfig () = default;

    AlpineStackConfig & operator = (const AlpineStackConfig & copy);



    // Various configuration options
    //
    bool  setLocalEndpoint (const string &  ipAddress,
                            ushort          port);      // for now, only a single endpoint supported.

    bool  getLocalEndpoint (string &   ipAddress,
                            ushort &  port);

    bool  setLocalEndpoint (ulong   ipAddress,
                            ushort  port);

    bool  getLocalEndpoint (ulong &   ipAddress,
                            ushort &  port);

    bool  setMaxConcurrentQueries (ulong  max);

    bool  getMaxConcurrentQueries (ulong &  max);


    // Multicast transport
    //
    bool  setMulticastEndpoint (const string & group, ushort port);
    bool  getMulticastEndpoint (string & group, ushort & port);
    bool  multicastEnabled ();

    // Broadcast transport
    //
    bool  setBroadcastEndpoint (ushort port);
    bool  getBroadcastEndpoint (ushort & port);
    bool  broadcastEnabled ();

    // Raw 802.11 transport (Linux only)
    //
    bool  setRawWifiInterface (const string & interfaceName);
    bool  getRawWifiInterface (string & interfaceName);
    bool  rawWifiEnabled ();



  private:

    ulong   localIpAddress_;
    ushort  localPort_;
    ulong   maxConcurrentQueries_;

    // Multicast
    bool    multicastEnabled_;
    string  multicastGroup_;
    ushort  multicastPort_;

    // Broadcast
    bool    broadcastEnabled_;
    ushort  broadcastPort_;

    // Raw 802.11
    bool    rawWifiEnabled_;
    string  rawWifiInterface_;

};

