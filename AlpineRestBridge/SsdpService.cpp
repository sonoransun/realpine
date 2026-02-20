///////
///
///  Copyright (C) 2026  sonoransun
///
///  Permission is hereby granted, free of charge, to any person obtaining a copy
///  of this software and associated documentation files (the "Software"), to deal
///  in the Software without restriction, including without limitation the rights
///  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
///  copies of the Software, and to permit persons to whom the Software is
///  furnished to do so, subject to the following conditions:
///
///  The above copyright notice and this permission notice shall be included in all
///  copies or substantial portions of the Software.
///
///  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
///  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
///  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
///  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
///  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
///  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
///  SOFTWARE.
///
///////


#include <SsdpService.h>
#include <Log.h>

#include <Platform.h>
#include <cstring>


static const char * NOTIFY_TYPES[] = {
    "upnp:rootdevice",
    "urn:schemas-upnp-org:device:MediaServer:1",
    "urn:schemas-upnp-org:service:ContentDirectory:1"
};


SsdpService::SsdpService ()
    : ssdpFd_(-1)
{
}


SsdpService::~SsdpService ()
{
    stop();

    if (ssdpFd_ >= 0)
        alpine_close_socket(ssdpFd_);
}


bool
SsdpService::initialize (const string & deviceUuid,
                          const string & locationUrl,
                          const string & serverString)
{
    deviceUuid_   = deviceUuid;
    locationUrl_  = locationUrl;
    serverString_ = serverString;

    ssdpFd_ = socket(AF_INET, SOCK_DGRAM, 0);

    if (ssdpFd_ < 0) {
        Log::Error("SsdpService: Failed to create socket.");
        return false;
    }

    int reuse = 1;
    setsockopt(ssdpFd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

#ifdef SO_REUSEPORT
    setsockopt(ssdpFd_, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse));
#endif

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port        = htons(SSDP_PORT);

    if (bind(ssdpFd_, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        Log::Error("SsdpService: Failed to bind to port 1900.");
        alpine_close_socket(ssdpFd_);
        ssdpFd_ = -1;
        return false;
    }

    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr("239.255.255.250");
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(ssdpFd_, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                   &mreq, sizeof(mreq)) < 0) {
        Log::Error("SsdpService: Failed to join SSDP multicast group.");
        alpine_close_socket(ssdpFd_);
        ssdpFd_ = -1;
        return false;
    }

    Log::Info("SsdpService: Initialized.");
    return true;
}


void
SsdpService::threadMain ()
{
    Log::Info("SsdpService: Thread started.");

    sendNotifyAlive();

    int tickCount = 0;

    while (isActive())
    {
        struct pollfd pfd;
        pfd.fd      = ssdpFd_;
        pfd.events  = POLLIN;
        pfd.revents = 0;

        int ret = alpine_poll(&pfd, 1, POLL_TIMEOUT_MS);

        if (ret > 0 && (pfd.revents & POLLIN))
        {
            byte buffer[2048];
            struct sockaddr_in srcAddr;
            socklen_t srcLen = sizeof(srcAddr);

            ssize_t bytesRead = recvfrom(ssdpFd_, buffer, sizeof(buffer) - 1, 0,
                                         (struct sockaddr *)&srcAddr, &srcLen);

            if (bytesRead > 0) {
                buffer[bytesRead] = 0;
                handleMSearch(buffer, bytesRead,
                              ntohl(srcAddr.sin_addr.s_addr),
                              ntohs(srcAddr.sin_port));
            }
        }

        tickCount++;

        if (tickCount >= NOTIFY_INTERVAL_SEC) {
            sendNotifyAlive();
            tickCount = 0;
        }
    }

    sendNotifyByebye();

    Log::Info("SsdpService: Thread exiting.");
}


void
SsdpService::sendNotifyAlive ()
{
    struct sockaddr_in destAddr;
    memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin_family      = AF_INET;
    destAddr.sin_addr.s_addr = inet_addr("239.255.255.250");
    destAddr.sin_port        = htons(SSDP_PORT);

    for (const auto * nt : NOTIFY_TYPES)
    {
        string usn = deviceUuid_ + "::" + nt;

        string msg =
            "NOTIFY * HTTP/1.1\r\n"
            "HOST: 239.255.255.250:1900\r\n"
            "CACHE-CONTROL: max-age=1800\r\n"
            "LOCATION: "s + locationUrl_ + "\r\n"
            "NT: " + nt + "\r\n"
            "NTS: ssdp:alive\r\n"
            "SERVER: " + serverString_ + "\r\n"
            "USN: " + usn + "\r\n"
            "\r\n";

        sendto(ssdpFd_, msg.c_str(), msg.length(), 0,
               (struct sockaddr *)&destAddr, sizeof(destAddr));
    }
}


void
SsdpService::sendNotifyByebye ()
{
    struct sockaddr_in destAddr;
    memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin_family      = AF_INET;
    destAddr.sin_addr.s_addr = inet_addr("239.255.255.250");
    destAddr.sin_port        = htons(SSDP_PORT);

    for (const auto * nt : NOTIFY_TYPES)
    {
        string usn = deviceUuid_ + "::" + nt;

        string msg =
            "NOTIFY * HTTP/1.1\r\n"
            "HOST: 239.255.255.250:1900\r\n"
            "NT: "s + nt + "\r\n"
            "NTS: ssdp:byebye\r\n"
            "USN: " + usn + "\r\n"
            "\r\n";

        sendto(ssdpFd_, msg.c_str(), msg.length(), 0,
               (struct sockaddr *)&destAddr, sizeof(destAddr));
    }
}


void
SsdpService::handleMSearch (const byte * data, ulong len,
                             ulong srcIp, ushort srcPort)
{
    string request((const char *)data, len);

    if (!request.contains("M-SEARCH"))
        return;

    bool match = request.contains("ssdp:all") ||
                 request.contains("upnp:rootdevice") ||
                 request.contains("urn:schemas-upnp-org:device:MediaServer:1") ||
                 request.contains("urn:schemas-upnp-org:service:ContentDirectory:1");

    if (!match)
        return;

    string response =
        "HTTP/1.1 200 OK\r\n"
        "CACHE-CONTROL: max-age=1800\r\n"
        "LOCATION: "s + locationUrl_ + "\r\n"
        "ST: urn:schemas-upnp-org:device:MediaServer:1\r\n"
        "SERVER: " + serverString_ + "\r\n"
        "USN: " + deviceUuid_ + "::urn:schemas-upnp-org:device:MediaServer:1\r\n"
        "EXT:\r\n"
        "\r\n";

    struct sockaddr_in destAddr;
    memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin_family      = AF_INET;
    destAddr.sin_addr.s_addr = htonl(srcIp);
    destAddr.sin_port        = htons(srcPort);

    sendto(ssdpFd_, response.c_str(), response.length(), 0,
           (struct sockaddr *)&destAddr, sizeof(destAddr));
}
