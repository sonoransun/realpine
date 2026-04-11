/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <DtcpStack.h>
#include <DtcpStackTest.h>
#include <signal.h>


// Ctor defaulted in header


// Dtor defaulted in header


void
DtcpStackTest::initialize()
{
    // Register signal handlers to dispatch messages
    //
    ApplCore::addSignalHandler(&sendWelcomeMessage, SIGUSR1);
    ApplCore::addSignalHandler(&sendHelloMessage, SIGUSR2);
}


void
DtcpStackTest::sendWelcomeMessage()
{
    string message("\n\n"
                   "-= Welcome new DTCP transport! =-\n"
                   "---------------------------------\n");

    sendStringMessages(message);
}


void
DtcpStackTest::sendHelloMessage()
{
    string message("\n\n"
                   "-= This is a test of the DTCP transport stack.  Did it work? =-\n"
                   "---------------------------------------------------------------\n");

    sendStringMessages(message);
}


void
DtcpStackTest::sendStringMessages(const string & message)
{
    uint numTransports;
    numTransports = DtcpStack::numConnTransports();

    if (numTransports == 0) {
        // no transports created yet...
        return;
    }

    bool status;
    DtcpStack::t_ConnTransportList transportList;
    status = DtcpStack::getAllConnTransports(transportList);

    if (!status) {
        // no transports?
        Log::Error("Unable to retreive transport list from DtcpStack::getAllConnTransports.");
        return;
    }

    if (transportList.empty()) {
        // no transports?
        Log::Error("No transports in transport list from DtcpStack::getAllConnTransports.");
        return;
    }

    Log::Debug("- Sending message to: "s + std::to_string(transportList.size()) + " transports.");

    uint messageLength = message.length() + 1;
    const byte * msgData = reinterpret_cast<const byte *>(message.c_str());

    AlpineDtcpConnTransport * currTransport;

    for (auto & transItem : transportList) {

        currTransport = dynamic_cast<AlpineDtcpConnTransport *>(transItem);
        if (!currTransport) {
            Log::Error("Invalid transport type in transport list!");
            continue;
        }

        Log::Debug("- Sending message via current transport.");

        status = currTransport->sendData(msgData, messageLength);

        Log::Debug("- Sent...");

        if (!status) {
            Log::Error("Send data failed!");
            return;
        }
    }
}
