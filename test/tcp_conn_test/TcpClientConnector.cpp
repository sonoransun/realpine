/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <TcpClientConnector.h>
#include <TcpTransport.h>
#include <TcpClientThread.h>
#include <Log.h>
#include <StringUtils.h>



void  
TcpClientConnector::receiveTransport (ulong           requestId,
                                      TcpTransport *  transport)
{
    Log::Info ("Requested transport received for ID: "s + std::to_string(requestId));

    TcpClientThread *  clientThread;
    clientThread = new TcpClientThread (transport);
    clientThread->run ();
}



void  
TcpClientConnector::handleRequestFailure (ulong  requestId)
{
    Log::Error ("Requested transport failed for ID: "s + std::to_string(requestId));
}



