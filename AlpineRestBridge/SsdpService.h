/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <SysThread.h>


class SsdpService : public SysThread
{
  public:
    SsdpService();
    ~SsdpService();

    bool initialize(const string & deviceUuid, const string & locationUrl, const string & serverString);

    void threadMain();


  private:
    void sendNotifyAlive();
    void sendNotifyByebye();
    void handleMSearch(const byte * data, ulong len, ulong srcIp, ushort srcPort);

    static const int NOTIFY_INTERVAL_SEC = 30;
    static const int POLL_TIMEOUT_MS = 1000;
    static const ulong SSDP_MULTICAST_ADDR = 0xEFFFFFFA;  // 239.255.255.250
    static const ushort SSDP_PORT = 1900;

    int ssdpFd_;
    string deviceUuid_;
    string locationUrl_;
    string serverString_;
};
