/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <Log.h>
#include <StringUtils.h>
#include <TcpClientConnector.h>
#include <TcpClientThread.h>
#include <TcpTransport.h>


void
TcpClientConnector::receiveTransport(ulong requestId, TcpTransport * transport)
{
    Log::Info("Requested transport received for ID: "s + std::to_string(requestId));

    TcpClientThread * clientThread;
    clientThread = new TcpClientThread(transport);
    clientThread->run();
}


void
TcpClientConnector::handleRequestFailure(ulong requestId)
{
    Log::Error("Requested transport failed for ID: "s + std::to_string(requestId));
}
