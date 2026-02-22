/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <MdnsService.h>
#include <Log.h>

#include <Platform.h>
#include <cstring>


MdnsService::MdnsService ()
    : mdnsFd_(-1),
      ipAddrBinary_(0),
      port_(0)
{
}


MdnsService::~MdnsService ()
{
    stop();

    if (mdnsFd_ >= 0)
        alpine_close_socket(mdnsFd_);
}


bool
MdnsService::initialize (const string & hostname,
                          const string & ipAddress,
                          ushort port)
{
    hostname_  = hostname;
    ipAddress_ = ipAddress;
    port_      = port;

    struct in_addr addr;

    if (inet_aton(ipAddress_.c_str(), &addr) == 0) {
        Log::Error("MdnsService: Invalid IP address: "s + ipAddress_);
        return false;
    }

    ipAddrBinary_ = ntohl(addr.s_addr);

    mdnsFd_ = socket(AF_INET, SOCK_DGRAM, 0);

    if (mdnsFd_ < 0) {
        Log::Error("MdnsService: Failed to create socket.");
        return false;
    }

    int reuse = 1;
    setsockopt(mdnsFd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

#ifdef SO_REUSEPORT
    setsockopt(mdnsFd_, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse));
#endif

    struct sockaddr_in bindAddr;
    memset(&bindAddr, 0, sizeof(bindAddr));
    bindAddr.sin_family      = AF_INET;
    bindAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bindAddr.sin_port        = htons(MDNS_PORT);

    if (bind(mdnsFd_, (struct sockaddr *)&bindAddr, sizeof(bindAddr)) < 0) {
        Log::Error("MdnsService: Failed to bind to port 5353.");
        alpine_close_socket(mdnsFd_);
        mdnsFd_ = -1;
        return false;
    }

    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr("224.0.0.251");
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(mdnsFd_, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                   &mreq, sizeof(mreq)) < 0) {
        Log::Error("MdnsService: Failed to join mDNS multicast group.");
        alpine_close_socket(mdnsFd_);
        mdnsFd_ = -1;
        return false;
    }

    Log::Info("MdnsService: Initialized.");
    return true;
}


void
MdnsService::threadMain ()
{
    Log::Info("MdnsService: Thread started.");

    sendAnnouncement();

    int tickCount = 0;

    while (isActive())
    {
        struct pollfd pfd;
        pfd.fd      = mdnsFd_;
        pfd.events  = POLLIN;
        pfd.revents = 0;

        alpine_poll(&pfd, 1, POLL_TIMEOUT_MS);

        tickCount++;

        if (tickCount >= ANNOUNCE_INTERVAL_SEC) {
            sendAnnouncement();
            tickCount = 0;
        }
    }

    Log::Info("MdnsService: Thread exiting.");
}


void
MdnsService::sendAnnouncement ()
{
    byte packet[1500];
    memset(packet, 0, sizeof(packet));
    ulong offset = 0;

    // DNS header: ID=0, flags=0x8400 (QR=1, AA=1), 5 answers
    packet[2] = 0x84;
    packet[7] = 5;
    offset = 12;

    string upnpService = "_upnp._tcp.local";
    string httpService = "_http._tcp.local";
    string instanceUpnp = "Alpine Media Server._upnp._tcp.local";
    string instanceHttp = "Alpine Media Server._http._tcp.local";
    string hostLocal = hostname_ + ".local";
    string txtData = "path=/device.xml";

    offset = writePtrRecord(packet, offset, upnpService, instanceUpnp);
    offset = writePtrRecord(packet, offset, httpService, instanceHttp);
    offset = writeSrvRecord(packet, offset, instanceUpnp, hostLocal, port_);
    offset = writeTxtRecord(packet, offset, instanceUpnp, txtData);
    offset = writeARecord(packet, offset, hostLocal, ipAddrBinary_);

    struct sockaddr_in destAddr;
    memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin_family      = AF_INET;
    destAddr.sin_addr.s_addr = inet_addr("224.0.0.251");
    destAddr.sin_port        = htons(MDNS_PORT);

    sendto(mdnsFd_, packet, offset, 0,
           (struct sockaddr *)&destAddr, sizeof(destAddr));
}


ulong
MdnsService::writeDnsName (byte * buf, ulong offset, const string & name)
{
    ulong pos = 0;

    while (pos < name.length())
    {
        auto dotPos = name.find('.', pos);

        if (dotPos == string::npos)
            dotPos = name.length();

        ulong labelLen = dotPos - pos;
        buf[offset++] = (byte)labelLen;
        memcpy(buf + offset, name.c_str() + pos, labelLen);
        offset += labelLen;

        pos = dotPos + 1;
    }

    buf[offset++] = 0;
    return offset;
}


static void
writeRecordHeader (byte * buf, ulong & offset, ushort type, ulong ttl)
{
    buf[offset++] = (byte)(type >> 8);
    buf[offset++] = (byte)(type & 0xFF);
    buf[offset++] = 0x80;
    buf[offset++] = 1;          // Class IN with cache-flush
    buf[offset++] = (byte)(ttl >> 24);
    buf[offset++] = (byte)(ttl >> 16);
    buf[offset++] = (byte)(ttl >>  8);
    buf[offset++] = (byte)(ttl & 0xFF);
}


ulong
MdnsService::writePtrRecord (byte * buf, ulong offset,
                              const string & serviceName,
                              const string & instanceName)
{
    offset = writeDnsName(buf, offset, serviceName);

    writeRecordHeader(buf, offset, 12, 120);  // Type PTR

    ulong rdLenOffset = offset;
    offset += 2;

    ulong rdStart = offset;
    offset = writeDnsName(buf, offset, instanceName);

    ulong rdLen = offset - rdStart;
    buf[rdLenOffset]     = (byte)(rdLen >> 8);
    buf[rdLenOffset + 1] = (byte)(rdLen & 0xFF);

    return offset;
}


ulong
MdnsService::writeSrvRecord (byte * buf, ulong offset,
                              const string & instanceName,
                              const string & target,
                              ushort port)
{
    offset = writeDnsName(buf, offset, instanceName);

    writeRecordHeader(buf, offset, 33, 120);  // Type SRV

    ulong rdLenOffset = offset;
    offset += 2;
    ulong rdStart = offset;

    buf[offset++] = 0; buf[offset++] = 0;  // Priority
    buf[offset++] = 0; buf[offset++] = 0;  // Weight
    buf[offset++] = (byte)(port >> 8);
    buf[offset++] = (byte)(port & 0xFF);

    offset = writeDnsName(buf, offset, target);

    ulong rdLen = offset - rdStart;
    buf[rdLenOffset]     = (byte)(rdLen >> 8);
    buf[rdLenOffset + 1] = (byte)(rdLen & 0xFF);

    return offset;
}


ulong
MdnsService::writeTxtRecord (byte * buf, ulong offset,
                              const string & instanceName,
                              const string & txt)
{
    offset = writeDnsName(buf, offset, instanceName);

    writeRecordHeader(buf, offset, 16, 120);  // Type TXT

    ulong rdLen = 1 + txt.length();
    buf[offset++] = (byte)(rdLen >> 8);
    buf[offset++] = (byte)(rdLen & 0xFF);

    buf[offset++] = (byte)txt.length();
    memcpy(buf + offset, txt.c_str(), txt.length());
    offset += txt.length();

    return offset;
}


ulong
MdnsService::writeARecord (byte * buf, ulong offset,
                            const string & hostname, ulong ipAddr)
{
    offset = writeDnsName(buf, offset, hostname);

    writeRecordHeader(buf, offset, 1, 120);  // Type A

    buf[offset++] = 0; buf[offset++] = 4;   // RDLENGTH

    buf[offset++] = (byte)((ipAddr >> 24) & 0xFF);
    buf[offset++] = (byte)((ipAddr >> 16) & 0xFF);
    buf[offset++] = (byte)((ipAddr >>  8) & 0xFF);
    buf[offset++] = (byte)((ipAddr      ) & 0xFF);

    return offset;
}
