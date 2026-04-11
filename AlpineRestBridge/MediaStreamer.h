/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <ContentStore.h>


class MediaStreamer
{
  public:
    static bool serveFile(int socketFd, ContentStore & store, const string & mediaId, const string & rangeHeader);

    static bool serveTranscode(int socketFd, ContentStore & store, const string & mediaId);


  private:
    static bool parseRange(const string & rangeHeader, ulong fileSize, ulong & start, ulong & end);

    static void sendFileHeaders(int fd,
                                int status,
                                const string & statusText,
                                const string & mime,
                                ulong contentLen,
                                ulong rangeStart,
                                ulong rangeEnd,
                                ulong totalSize);

    static void sendStreamHeaders(int fd, const string & mime);

    static const ulong SEND_BUFFER_SIZE = 65536;
};
