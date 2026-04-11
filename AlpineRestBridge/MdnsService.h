/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <SysThread.h>


class MdnsService : public SysThread
{
  public:
    MdnsService();
    ~MdnsService();

    bool initialize(const string & hostname, const string & ipAddress, ushort port);

    void threadMain();


  private:
    void sendAnnouncement();
    ulong writeDnsName(byte * buf, ulong offset, const string & name);
    ulong writePtrRecord(byte * buf, ulong offset, const string & serviceName, const string & instanceName);
    ulong writeSrvRecord(byte * buf, ulong offset, const string & instanceName, const string & target, ushort port);
    ulong writeTxtRecord(byte * buf, ulong offset, const string & instanceName, const string & txt);
    ulong writeARecord(byte * buf, ulong offset, const string & hostname, ulong ipAddr);

    static const int ANNOUNCE_INTERVAL_SEC = 60;
    static const int POLL_TIMEOUT_MS = 1000;
    static const ushort MDNS_PORT = 5353;

    int mdnsFd_;
    string hostname_;
    string ipAddress_;
    ulong ipAddrBinary_;
    ushort port_;
};
