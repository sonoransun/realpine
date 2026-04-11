///////
///
///  Copyright (C) 2026  sonoransun
///
///  Broadcast UDP test over mac80211_hwsim virtual wireless stations.
///
///  Tests BroadcastUdpConnection by sending broadcast datagrams
///  between two virtual WiFi stations on an ad-hoc network created
///  with mac80211_hwsim.
///
///  Runs in two modes selected by the first argument:
///
///    wifiBroadcastConnTest send <bind_ip> <port> <bcast_ip>
///        Sends test datagrams to the broadcast address.
///
///    wifiBroadcastConnTest recv <bind_ip> <port>
///        Receives broadcast datagrams and validates them.
///        Prints "BROADCAST_RECV_OK" on success (used by runner script).
///
///  Requires: Linux, mac80211_hwsim with ad-hoc IBSS network configured
///
///////


#include <BroadcastUdpConnection.h>
#include <Common.h>
#include <Log.h>
#include <NetUtils.h>

#include <chrono>
#include <cstring>
#include <poll.h>
#include <thread>
#include <unistd.h>


static const char MAGIC[] = "ALPINE_BCAST_TEST";
static constexpr uint MAGIC_LEN = sizeof(MAGIC);
static constexpr uint NUM_PACKETS = 5;
static constexpr uint PAYLOAD_SIZE = 256;
static constexpr int RECV_TIMEOUT_MS = 8000;


static bool
runSender(ulong bindIp, ushort port, ulong bcastIp)
{
    Log::Info("Sender: Creating BroadcastUdpConnection.");

    BroadcastUdpConnection conn;

    if (!conn.create(bindIp, port)) {
        Log::Error("Sender: Failed to create broadcast connection.");
        return false;
    }

    Log::Info("Sender: Connection created, fd="s + std::to_string(conn.getFd()));

    // Give receiver time to bind
    usleep(1000000);  // 1 second

    for (uint i = 0; i < NUM_PACKETS; ++i) {
        byte payload[PAYLOAD_SIZE];
        memset(payload, 0, PAYLOAD_SIZE);

        // Magic + sequence number + fill pattern
        memcpy(payload, MAGIC, MAGIC_LEN);
        payload[MAGIC_LEN] = static_cast<byte>(i);
        for (uint j = MAGIC_LEN + 1; j < PAYLOAD_SIZE; ++j) {
            payload[j] = static_cast<byte>((i + j) & 0xFF);
        }

        Log::Info("Sender: Sending packet "s + std::to_string(i) + " to broadcast address.");

        if (!conn.sendData(bcastIp, port, payload, PAYLOAD_SIZE)) {
            Log::Error("Sender: sendData failed for packet "s + std::to_string(i));
            return false;
        }

        usleep(200000);  // 200ms between packets
    }

    Log::Info("Sender: All packets sent.");
    return true;
}


static bool
runReceiver(ulong bindIp, ushort port)
{
    Log::Info("Receiver: Creating BroadcastUdpConnection.");

    BroadcastUdpConnection conn;

    if (!conn.create(bindIp, port)) {
        Log::Error("Receiver: Failed to create broadcast connection.");
        return false;
    }

    Log::Info("Receiver: Listening on fd="s + std::to_string(conn.getFd()));

    uint packetsReceived = 0;
    int fd = conn.getFd();

    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(RECV_TIMEOUT_MS);

    while (packetsReceived < NUM_PACKETS) {
        auto remaining =
            std::chrono::duration_cast<std::chrono::milliseconds>(deadline - std::chrono::steady_clock::now()).count();

        if (remaining <= 0) {
            Log::Error("Receiver: Timeout. Got "s + std::to_string(packetsReceived) + " of " +
                       std::to_string(NUM_PACKETS));
            return false;
        }

        struct pollfd pfd;
        pfd.fd = fd;
        pfd.events = POLLIN;
        pfd.revents = 0;

        int ret = poll(&pfd, 1, static_cast<int>(std::min<long long>(remaining, 500LL)));

        if (ret <= 0) {
            continue;
        }

        byte buffer[2048];
        ulong srcIp = 0;
        ushort srcPort = 0;
        uint dataLen = 0;

        if (!conn.receiveData(buffer, sizeof(buffer), srcIp, srcPort, dataLen)) {
            continue;
        }

        // Check magic
        if (dataLen < MAGIC_LEN + 1 || memcmp(buffer, MAGIC, MAGIC_LEN) != 0) {
            continue;  // not our packet
        }

        uint seqNum = buffer[MAGIC_LEN];

        if (dataLen != PAYLOAD_SIZE) {
            Log::Error("Receiver: Packet "s + std::to_string(seqNum) + " size mismatch: expected " +
                       std::to_string(PAYLOAD_SIZE) + ", got " + std::to_string(dataLen));
            return false;
        }

        // Verify fill pattern
        bool payloadOk = true;
        for (uint j = MAGIC_LEN + 1; j < PAYLOAD_SIZE; ++j) {
            byte expected = static_cast<byte>((seqNum + j) & 0xFF);
            if (buffer[j] != expected) {
                Log::Error("Receiver: Packet "s + std::to_string(seqNum) + " payload mismatch at byte " +
                           std::to_string(j));
                payloadOk = false;
                break;
            }
        }

        if (!payloadOk) {
            return false;
        }

        packetsReceived++;
        Log::Info("Receiver: Packet "s + std::to_string(seqNum) + " OK.");
    }

    Log::Info("Receiver: All "s + std::to_string(packetsReceived) + " packets received.");

    // Signal success to the runner script
    printf("BROADCAST_RECV_OK\n");
    fflush(stdout);

    return true;
}


int
main(int argc, char * argv[])
{
    if (argc < 4) {
        fprintf(stderr,
                "Usage:\n"
                "  %s send <bind_ip> <port> <broadcast_ip>\n"
                "  %s recv <bind_ip> <port>\n",
                argv[0],
                argv[0]);
        return 1;
    }

    string mode = argv[1];
    string ipStr = argv[2];
    int portNum = atoi(argv[3]);

    Log::initialize("/tmp/alpine_wifi_tests/broadcast_conn_test.log", Log::t_LogLevel::Debug);

    ulong ip;
    if (!NetUtils::stringIpToLong(ipStr, ip)) {
        Log::Error("Invalid IP address: "s + ipStr);
        return 1;
    }
    ushort port = htons(static_cast<ushort>(portNum));

    if (mode == "send") {
        if (argc < 5) {
            fprintf(stderr, "send mode requires broadcast IP.\n");
            return 1;
        }
        string bcastStr = argv[4];
        ulong bcastIp;
        if (!NetUtils::stringIpToLong(bcastStr, bcastIp)) {
            Log::Error("Invalid broadcast IP: "s + bcastStr);
            return 1;
        }

        Log::Info("=== Broadcast UDP Send Test ===");
        return runSender(ip, port, bcastIp) ? 0 : 1;
    }

    if (mode == "recv") {
        Log::Info("=== Broadcast UDP Receive Test ===");
        return runReceiver(ip, port) ? 0 : 1;
    }

    fprintf(stderr, "Unknown mode: %s\n", mode.c_str());
    return 1;
}
