/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <UdpConnection.h>


class RawWifiConnection : public UdpConnection
{
  public:
    RawWifiConnection(const string & interfaceName);
    ~RawWifiConnection() override;

    bool create(ulong ipAddress = 0, ushort port = 0) override;
    void close() override;
    int getFd() override;

    bool
    sendData(const ulong destIpAddress, const ushort destPort, const byte * dataBuffer, const uint dataLength) override;

    bool receiveData(byte * dataBuffer,
                     const uint bufferLength,
                     ulong & sourceIpAddress,
                     ushort & sourcePort,
                     uint & dataLength) override;

  private:
#ifdef __linux__
    int rawFd_;
    int ifIndex_;
    byte localMac_[6];

    static constexpr byte ALPINE_OUI[3] = {0x02, 0xA1, 0xE1};

    // Radiotap header size (minimal)
    static constexpr uint RADIOTAP_HDR_SIZE = 8;

    // 802.11 data frame header (3 address format)
    static constexpr uint DOT11_HDR_SIZE = 24;

    // LLC/SNAP header: DSAP(1) + SSAP(1) + Control(1) + OUI(3) + Type(2)
    static constexpr uint LLC_SNAP_HDR_SIZE = 8;

    // Total overhead per frame
    static constexpr uint FRAME_OVERHEAD = RADIOTAP_HDR_SIZE + DOT11_HDR_SIZE + LLC_SNAP_HDR_SIZE;

    // Alpine EtherType for DTCP-over-802.11
    static constexpr ushort ALPINE_ETHERTYPE = 0xA1E1;

    uint buildDataFrame(const byte * payload, uint payloadLen, const byte * destMac, byte * frame, uint frameSize);

    bool parseDataFrame(
        const byte * frame, uint frameLen, byte * payload, uint & payloadLen, ulong & sourceIp, ushort & sourcePort);
#endif

    string interfaceName_;
};
