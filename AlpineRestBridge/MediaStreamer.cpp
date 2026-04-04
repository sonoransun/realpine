/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <MediaStreamer.h>
#include <SafeParse.h>
#include <Log.h>

#include <Platform.h>
#include <cstdio>
#include <cstring>
#ifndef ALPINE_PLATFORM_WINDOWS
#include <sys/wait.h>
#endif


static void
sendErrorResponse (int socketFd, int status, const char * statusText)
{
    string resp = "HTTP/1.1 "s + std::to_string(status) + " " + statusText +
                  "\r\nContent-Length: 0\r\n\r\n";
    ::send(socketFd, resp.c_str(), resp.length(), 0);
}


static bool
sendAllBytes (int socketFd, const byte * data, ulong len)
{
    ulong totalSent = 0;

    while (totalSent < len) {
        ssize_t sent = ::send(socketFd, data + totalSent, len - totalSent, 0);

        if (sent <= 0)
            return false;

        totalSent += sent;
    }

    return true;
}


bool
MediaStreamer::serveFile (int socketFd, ContentStore & store,
                          const string & mediaId, const string & rangeHeader)
{
    ContentStore::MediaItem item;

    if (!store.getItem(mediaId, item)) {
        sendErrorResponse(socketFd, 404, "Not Found");
        return false;
    }

    FILE * fp = fopen(item.path.c_str(), "rb");

    if (!fp) {
        Log::Error("MediaStreamer: Cannot open file: "s + item.path);
        sendErrorResponse(socketFd, 500, "Internal Server Error");
        return false;
    }

    if (item.fileSize == 0) {
        sendFileHeaders(socketFd, 200, "OK", item.mimeType, 0, 0, 0, 0);
        fclose(fp);
        return true;
    }

    ulong rangeStart = 0;
    ulong rangeEnd   = item.fileSize - 1;
    bool  hasRange   = !rangeHeader.empty() &&
                       parseRange(rangeHeader, item.fileSize, rangeStart, rangeEnd);

    if (hasRange) {
        ulong contentLen = rangeEnd - rangeStart + 1;
        sendFileHeaders(socketFd, 206, "Partial Content", item.mimeType,
                        contentLen, rangeStart, rangeEnd, item.fileSize);
    } else {
        sendFileHeaders(socketFd, 200, "OK", item.mimeType,
                        item.fileSize, 0, 0, item.fileSize);
    }

    fseek(fp, rangeStart, SEEK_SET);

    byte buffer[SEND_BUFFER_SIZE];
    ulong remaining = rangeEnd - rangeStart + 1;

    while (remaining > 0)
    {
        ulong toRead = (remaining < SEND_BUFFER_SIZE) ? remaining : SEND_BUFFER_SIZE;
        ulong bytesRead = fread(buffer, 1, toRead, fp);

        if (bytesRead == 0)
            break;

        if (!sendAllBytes(socketFd, buffer, bytesRead))
            break;

        remaining -= bytesRead;
    }

    fclose(fp);
    return true;
}


bool
MediaStreamer::serveTranscode (int socketFd, ContentStore & store,
                               const string & mediaId)
{
    ContentStore::MediaItem item;

    if (!store.getItem(mediaId, item)) {
        sendErrorResponse(socketFd, 404, "Not Found");
        return false;
    }

#ifdef ALPINE_PLATFORM_WINDOWS
    Log::Error("MediaStreamer: Transcoding not supported on Windows.");
    sendErrorResponse(socketFd, 501, "Not Implemented");
    return false;
#else
    int pipefd[2];

    if (pipe(pipefd) < 0) {
        Log::Error("MediaStreamer: pipe() failed.");
        sendErrorResponse(socketFd, 500, "Internal Server Error");
        return false;
    }

    pid_t pid = fork();

    if (pid < 0) {
        Log::Error("MediaStreamer: fork() failed.");
        close(pipefd[0]);
        close(pipefd[1]);
        sendErrorResponse(socketFd, 500, "Internal Server Error");
        return false;
    }

    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        int devNull = open("/dev/null", O_WRONLY);
        if (devNull >= 0) {
            dup2(devNull, STDERR_FILENO);
            close(devNull);
        }

        const char * argv[] = {
            "ffmpeg", "-i", item.path.c_str(),
            "-c:v", "libx264", "-preset", "veryfast", "-crf", "23",
            "-c:a", "aac", "-b:a", "128k",
            "-f", "mpegts", "pipe:1",
            nullptr
        };

        execvp(argv[0], const_cast<char * const *>(argv));
        _exit(1);
    }

    close(pipefd[1]);

    sendStreamHeaders(socketFd, "video/mp2t");

    byte buffer[SEND_BUFFER_SIZE];
    ssize_t bytesRead;

    while ((bytesRead = read(pipefd[0], buffer, SEND_BUFFER_SIZE)) > 0)
    {
        if (!sendAllBytes(socketFd, buffer, (ulong)bytesRead))
            break;
    }

    close(pipefd[0]);
    kill(pid, SIGTERM);
    waitpid(pid, nullptr, 0);

    return true;
#endif
}


bool
MediaStreamer::parseRange (const string & rangeHeader, ulong fileSize,
                           ulong & start, ulong & end)
{
    if (fileSize == 0)
        return false;

    auto eqPos = rangeHeader.find('=');

    if (eqPos == string::npos)
        return false;

    string range = rangeHeader.substr(eqPos + 1);
    auto dashPos = range.find('-');

    if (dashPos == string::npos)
        return false;

    string startStr = range.substr(0, dashPos);
    string endStr   = range.substr(dashPos + 1);

    if (startStr.empty() && !endStr.empty()) {
        auto suffixOpt = parseUlong(endStr);
        if (!suffixOpt)
            return false;
        ulong suffix = *suffixOpt;

        if (suffix > fileSize)
            suffix = fileSize;

        start = fileSize - suffix;
        end   = fileSize - 1;
    }
    else if (!startStr.empty()) {
        auto startOpt = parseUlong(startStr);
        if (!startOpt)
            return false;
        start = *startOpt;

        if (!endStr.empty()) {
            auto endOpt = parseUlong(endStr);
            if (!endOpt)
                return false;
            end = *endOpt;
        } else {
            end = fileSize - 1;
        }

        if (start >= fileSize)
            return false;

        if (end >= fileSize)
            end = fileSize - 1;
    }
    else {
        return false;
    }

    return true;
}


void
MediaStreamer::sendFileHeaders (int fd, int status, const string & statusText,
                                const string & mime, ulong contentLen,
                                ulong rangeStart, ulong rangeEnd, ulong totalSize)
{
    string headers = "HTTP/1.1 "s + std::to_string(status) + " " + statusText + "\r\n"
        "Content-Type: " + mime + "\r\n"
        "Content-Length: " + std::to_string(contentLen) + "\r\n"
        "Accept-Ranges: bytes\r\n"
        "transferMode.dlna.org: Streaming\r\n"
        "contentFeatures.dlna.org: DLNA.ORG_OP=01;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000\r\n";

    if (status == 206) {
        headers += "Content-Range: bytes "s + std::to_string(rangeStart) + "-" +
                   std::to_string(rangeEnd) + "/" + std::to_string(totalSize) + "\r\n";
    }

    headers += "Connection: close\r\n\r\n";

    ::send(fd, headers.c_str(), headers.length(), 0);
}


void
MediaStreamer::sendStreamHeaders (int fd, const string & mime)
{
    string headers = "HTTP/1.1 200 OK\r\n"
        "Content-Type: "s + mime + "\r\n"
        "Transfer-Encoding: chunked\r\n"
        "transferMode.dlna.org: Streaming\r\n"
        "contentFeatures.dlna.org: DLNA.ORG_OP=00;DLNA.ORG_CI=1;DLNA.ORG_FLAGS=01700000000000000000000000000000\r\n"
        "Connection: close\r\n"
        "\r\n";

    ::send(fd, headers.c_str(), headers.length(), 0);
}
