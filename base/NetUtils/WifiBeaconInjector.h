/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <SysThread.h>


class WifiBeaconInjector : public SysThread
{
  public:

    WifiBeaconInjector ();
    ~WifiBeaconInjector ();

    bool  initialize (const string & interfaceName, const string & peerId,
                      const string & ipAddress, ushort port, ushort restPort,
                      uint capabilities, int beaconIntervalMs);

    void  threadMain ();


  private:

#ifdef __linux__
    bool  openRawSocket ();
    uint  buildBeaconFrame (byte * buffer, uint bufferSize);
    uint  buildVendorIE (byte * buffer, uint offset);
    void  parseBeaconFrame (const byte * data, uint length);
    bool  parseVendorIE (const byte * ie, uint ieLen);

    static constexpr byte ALPINE_OUI[3] = { 0x02, 0xA1, 0xE1 };
    static constexpr byte ALPINE_OUI_TYPE = 0x01;

    int     rawFd_;
    int     ifIndex_;
    byte    localMac_[6];
#endif

    static const int  POLL_TIMEOUT_MS = 500;

    string  interfaceName_;
    string  peerId_;
    string  ipAddress_;
    ushort  port_;
    ushort  restPort_;
    uint    capabilities_;
    int     beaconIntervalMs_;

};
