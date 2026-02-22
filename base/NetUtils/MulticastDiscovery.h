/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <SysThread.h>


class MulticastDiscovery : public SysThread
{
  public:

    MulticastDiscovery ();
    ~MulticastDiscovery ();

    bool  initialize (const string & multicastGroup, ushort multicastPort,
                      const string & peerId, const string & ipAddress,
                      ushort port, ushort restPort, uint capabilities,
                      int announceIntervalSec, int peerTimeoutSec);

    void  threadMain ();


  private:

    void  sendAnnouncement ();
    void  handleReceived (const byte * data, uint length,
                          ulong srcIp, ushort srcPort);

    static const int  POLL_TIMEOUT_MS = 1000;

    int     mcastFd_;
    string  multicastGroup_;
    ushort  multicastPort_;
    string  peerId_;
    string  ipAddress_;
    ushort  port_;
    ushort  restPort_;
    uint    capabilities_;
    int     announceIntervalSec_;
    int     peerTimeoutSec_;

};
