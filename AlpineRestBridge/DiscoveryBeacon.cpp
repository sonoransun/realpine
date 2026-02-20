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


#include <DiscoveryBeacon.h>
#include <JsonWriter.h>
#include <Log.h>

#include <Platform.h>


DiscoveryBeacon::DiscoveryBeacon ()
    : restPort_(0),
      beaconPort_(0)
{
}


DiscoveryBeacon::~DiscoveryBeacon ()
{
    stop();
    udpSocket_.close();
}



bool
DiscoveryBeacon::initialize (ushort restPort, ushort beaconPort)
{
    restPort_   = restPort;
    beaconPort_ = beaconPort;

    if (!udpSocket_.create()) {
        Log::Error("DiscoveryBeacon: Failed to create UDP socket.");
        return false;
    }
    int broadcastEnable = 1;

    if (setsockopt(udpSocket_.getFd(), SOL_SOCKET, SO_BROADCAST,
                   &broadcastEnable, sizeof(broadcastEnable)) < 0) {
        Log::Error("DiscoveryBeacon: Failed to enable SO_BROADCAST.");
        udpSocket_.close();
        return false;
    }
    Log::Info("DiscoveryBeacon: Initialized on beacon port "s +
              std::to_string(beaconPort_));

    return true;
}



void
DiscoveryBeacon::threadMain ()
{
    Log::Info("DiscoveryBeacon: Thread started.");

    while (isActive()) {

        // Build beacon JSON payload
        //
        JsonWriter json;
        json.beginObject();
        json.key("service");    json.value("alpine-bridge");
        json.key("version");    json.value("1");
        json.key("restPort");   json.value((ulong)restPort_);
        json.key("bridgeVersion"); json.value("devel-00019");
        json.endObject();

        string payload = json.result();

        // Send to broadcast address
        //
        bool sent = udpSocket_.sendData(INADDR_BROADCAST,
                                        htons(beaconPort_),
                                        (const byte *)payload.c_str(),
                                        payload.length());

        if (!sent) {
            Log::Debug("DiscoveryBeacon: Failed to send beacon.");
            continue;
        }

        // Sleep in 1-second increments so stop() is responsive
        //
        for (int i = 0; i < BEACON_INTERVAL_SEC && isActive(); i++) {
            usleep(SLEEP_INCREMENT_MS * 1000);
        }
    }

    Log::Info("DiscoveryBeacon: Thread exiting.");
}
