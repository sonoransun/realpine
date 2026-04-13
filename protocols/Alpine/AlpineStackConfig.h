/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>


class AlpineStackConfig
{
  public:
    AlpineStackConfig();

    AlpineStackConfig(const AlpineStackConfig & copy);

    ~AlpineStackConfig() = default;

    AlpineStackConfig & operator=(const AlpineStackConfig & copy);


    // Various configuration options
    //
    bool setLocalEndpoint(const string & ipAddress,
                          ushort port);  // for now, only a single endpoint supported.

    bool getLocalEndpoint(string & ipAddress, ushort & port);

    bool setLocalEndpoint(ulong ipAddress, ushort port);

    bool getLocalEndpoint(ulong & ipAddress, ushort & port);

    bool setMaxConcurrentQueries(ulong max);

    bool getMaxConcurrentQueries(ulong & max);


    // Multicast transport
    //
    bool setMulticastEndpoint(const string & group, ushort port);
    bool getMulticastEndpoint(string & group, ushort & port);
    bool multicastEnabled();

    // Broadcast transport
    //
    bool setBroadcastEndpoint(ushort port);
    bool getBroadcastEndpoint(ushort & port);
    bool broadcastEnabled();

    // Raw 802.11 transport (Linux only)
    //
    bool setRawWifiInterface(const string & interfaceName);
    bool getRawWifiInterface(string & interfaceName);
    bool rawWifiEnabled();

    // Unicast 802.11 transport (Linux only)
    //
    bool setUnicastWifiInterface(const string & interfaceName);
    bool getUnicastWifiInterface(string & interfaceName);
    bool unicastWifiEnabled();

    // RTL-SDR receive-only transport
    //
#ifdef ALPINE_RTLSDR_ENABLED
    bool setRtlSdrConfig(uint centerFreqHz, uint sampleRate, int gainTenths, const string & modulation);
    bool getRtlSdrConfig(uint & centerFreqHz, uint & sampleRate, int & gainTenths, string & modulation);
    bool rtlSdrEnabled();
#endif


  private:
    ulong localIpAddress_;
    ushort localPort_;
    ulong maxConcurrentQueries_;

    // Multicast
    bool multicastEnabled_;
    string multicastGroup_;
    ushort multicastPort_;

    // Broadcast
    bool broadcastEnabled_;
    ushort broadcastPort_;

    // Raw 802.11
    bool rawWifiEnabled_;
    string rawWifiInterface_;

    // Unicast 802.11
    bool unicastWifiEnabled_;
    string unicastWifiInterface_;

    // RTL-SDR
#ifdef ALPINE_RTLSDR_ENABLED
    bool rtlSdrEnabled_;
    uint rtlSdrCenterFreqHz_;
    uint rtlSdrSampleRate_;
    int rtlSdrGainTenths_;
    string rtlSdrModulation_;
#endif
};
