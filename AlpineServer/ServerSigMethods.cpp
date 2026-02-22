/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <ServerSigMethods.h>
#include <DtcpStack.h>
#include <Platform.h>



// Ctor defaulted in header


// Dtor defaulted in header



void  
ServerSigMethods::initialize ()
{
    // Register signal handlers to dispatch messages
    //
    ApplCore::addSignalHandler (&sendWelcomeMessage, SIGUSR1);
    ApplCore::addSignalHandler (&sendHelloMessage, SIGUSR2);
}



void  
ServerSigMethods::sendWelcomeMessage ()
{
    string message ("\n\n"
                    "-= Welcome to the ALPINE Network! =-\n"
                    "------------------------------------\n");

    sendStringMessages (message, false);
}



void  
ServerSigMethods::sendHelloMessage ()
{
    string message ("\n\n"
                    "-= This is a test of the DTCP transport stack.  Did it work? =-\n"
                    "---------------------------------------------------------------\n");

    sendStringMessages (message, true);
}



void  
ServerSigMethods::sendStringMessages (const string &  message,
                                      bool            reliable)
{
    uint numTransports;
    numTransports = DtcpStack::numConnTransports ();

    if (numTransports == 0) {
        // no transports, nothing to send.
        return;
    }

    bool status;
    DtcpStack::t_ConnTransportList  transportList;
    status = DtcpStack::getAllConnTransports (transportList);

    if (!status) {
        // no transports?
        Log::Error ("Unable to retreive transport list from DtcpStack::getAllConnTransports during "
                             "ServerSigMethods::sendStringMessages.");
        return;
    }

    if (transportList.empty()) {
        // no transports?
        Log::Error ("No transports in transport list from DtcpStack::getAllConnTransports during "
                             "ServerSigMethods::sendStringMessages.");
        return;
    }

    Log::Debug ("- Sending message to: "s + std::to_string(transportList.size()) +
                " transports.");

    uint messageLength = message.length() + 1;
    const byte * msgData = reinterpret_cast<const byte *>(message.c_str());

    AlpineDtcpConnTransport * currTransport;

    for (auto& transItem : transportList) {

        currTransport = dynamic_cast<AlpineDtcpConnTransport *>(transItem);
        if (!currTransport) {
            Log::Error ("Invalid transport type in transport list during "
                                 "ServerSigMethods::sendStringMessages!");
            continue;
        }

        Log::Debug ("- Sending message via current transport.");

        if (reliable) {
            ulong  requestId = 0;
            status = currTransport->sendReliableData (msgData,
                                                      messageLength,
                                                      requestId);
        }
        else {
            status = currTransport->sendData (msgData,
                                              messageLength);
        }

        if (status ==  false) {
            Log::Error ("Send data failed!");
        }
        else {
            Log::Debug ("- Sent...");
            return;
        }
    }
}


    
