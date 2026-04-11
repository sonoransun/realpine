/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <ContentStore.h>
#include <SysThread.h>
#include <TcpAcceptor.h>
#include <TcpTransport.h>
#include <memory>


class DlnaServer : public SysThread
{
  public:
    DlnaServer(ContentStore & store);
    ~DlnaServer();

    bool initialize(ushort port, const string & hostAddress, bool transcodeEnabled);

    void threadMain();

    ushort getPort();
    string getBaseUrl();
    string getDeviceUuid();


  private:
    void handleConnection(std::unique_ptr<TcpTransport> transport);

    void sendResponse(TcpTransport * t, int status, const string & contentType, const string & body);

    static string generateUuid(const string & hostname, ushort port);

    ContentStore & store_;
    TcpAcceptor acceptor_;
    ushort port_;
    string hostAddress_;
    string baseUrl_;
    string deviceUuid_;
    bool transcodeEnabled_;
};
