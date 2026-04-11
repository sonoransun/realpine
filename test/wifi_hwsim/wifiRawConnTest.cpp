///////
///
///  Copyright (C) 2026  sonoransun
///
///  Raw 802.11 frame exchange test using mac80211_hwsim.
///
///  Tests RawWifiConnection by sending a frame on one monitor-mode
///  interface and receiving it on the other.  The mac80211_hwsim
///  kernel module provides a simulated wireless medium so frames
///  injected on one virtual radio arrive on the other.
///
///  Usage:
///      wifiRawConnTest <sender_iface> <receiver_iface>
///
///  Example:
///      wifiRawConnTest mon0 mon1
///
///  Requires: Linux, root/CAP_NET_RAW, mac80211_hwsim loaded
///
///////


#ifdef __linux__

#include <Common.h>
#include <Log.h>
#include <NetUtils.h>
#include <RawWifiConnection.h>

#include <chrono>
#include <cstring>
#include <poll.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>


// Magic payload to verify frame integrity
static const char MAGIC[] = "ALPINE_RAW_TEST_FRAME";
static constexpr uint MAGIC_LEN = sizeof(MAGIC);

// Test payload sizes: small, medium, MTU-ish
static constexpr uint TEST_SIZES[] = {64, 512, 1400};
static constexpr uint NUM_SIZES = 3;

// Timeout for receiving a frame (milliseconds)
static constexpr int RECV_TIMEOUT_MS = 3000;


static bool
runSender(const string & ifaceName)
{
    Log::Info("Sender: Creating RawWifiConnection on "s + ifaceName);

    RawWifiConnection conn(ifaceName);

    if (!conn.create()) {
        Log::Error("Sender: Failed to create connection.");
        return false;
    }

    Log::Info("Sender: Connection created, fd="s + std::to_string(conn.getFd()));

    // Give the receiver time to set up
    usleep(500000);  // 500ms

    for (uint i = 0; i < NUM_SIZES; ++i) {
        uint payloadLen = TEST_SIZES[i];
        vector<byte> payload(payloadLen);

        // Fill payload: magic header + sequential bytes
        memcpy(payload.data(), MAGIC, MAGIC_LEN);
        payload[MAGIC_LEN] = static_cast<byte>(i);  // test index
        for (uint j = MAGIC_LEN + 1; j < payloadLen; ++j) {
            payload[j] = static_cast<byte>(j & 0xFF);
        }

        // Use dummy IP/port (RawWifiConnection encodes these in payload prefix)
        ulong destIp = htonl(0x0A370001);  // 10.55.0.1
        ushort destPort = htons(44100);

        Log::Info("Sender: Sending frame "s + std::to_string(i) + ", size=" + std::to_string(payloadLen));

        if (!conn.sendData(destIp, destPort, payload.data(), payloadLen)) {
            Log::Error("Sender: sendData failed for frame "s + std::to_string(i));
            return false;
        }

        usleep(100000);  // 100ms between frames
    }

    Log::Info("Sender: All frames sent.");
    return true;
}


static bool
runReceiver(const string & ifaceName)
{
    Log::Info("Receiver: Creating RawWifiConnection on "s + ifaceName);

    RawWifiConnection conn(ifaceName);

    if (!conn.create()) {
        Log::Error("Receiver: Failed to create connection.");
        return false;
    }

    Log::Info("Receiver: Connection created, fd="s + std::to_string(conn.getFd()));

    uint framesReceived = 0;
    int fd = conn.getFd();

    // Try to receive all test frames within a generous timeout
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(RECV_TIMEOUT_MS * NUM_SIZES);

    while (framesReceived < NUM_SIZES) {
        auto remaining =
            std::chrono::duration_cast<std::chrono::milliseconds>(deadline - std::chrono::steady_clock::now()).count();

        if (remaining <= 0) {
            Log::Error("Receiver: Timeout waiting for frames. Got "s + std::to_string(framesReceived) + " of " +
                       std::to_string(NUM_SIZES));
            return false;
        }

        struct pollfd pfd;
        pfd.fd = fd;
        pfd.events = POLLIN;
        pfd.revents = 0;

        int ret = poll(&pfd, 1, static_cast<int>(std::min<long long>(remaining, 1000LL)));

        if (ret <= 0) {
            continue;  // timeout or error, retry
        }

        byte buffer[4096];
        ulong srcIp = 0;
        ushort srcPort = 0;
        uint dataLen = 0;

        if (!conn.receiveData(buffer, sizeof(buffer), srcIp, srcPort, dataLen)) {
            // Not all frames on the medium are ours — silently skip
            continue;
        }

        // Check if this is one of our test frames
        if (dataLen < MAGIC_LEN + 1) {
            continue;  // too short, not ours
        }

        if (memcmp(buffer, MAGIC, MAGIC_LEN) != 0) {
            continue;  // not our magic, skip
        }

        uint testIdx = buffer[MAGIC_LEN];
        if (testIdx >= NUM_SIZES) {
            continue;  // invalid index
        }

        uint expectedLen = TEST_SIZES[testIdx];
        if (dataLen != expectedLen) {
            Log::Error("Receiver: Frame "s + std::to_string(testIdx) + " size mismatch: expected " +
                       std::to_string(expectedLen) + ", got " + std::to_string(dataLen));
            return false;
        }

        // Verify sequential payload bytes
        bool payloadOk = true;
        for (uint j = MAGIC_LEN + 1; j < dataLen; ++j) {
            if (buffer[j] != static_cast<byte>(j & 0xFF)) {
                Log::Error("Receiver: Frame "s + std::to_string(testIdx) + " payload mismatch at byte " +
                           std::to_string(j));
                payloadOk = false;
                break;
            }
        }

        if (!payloadOk) {
            return false;
        }

        framesReceived++;
        Log::Info("Receiver: Frame "s + std::to_string(testIdx) + " OK (size=" + std::to_string(dataLen) + ")");
    }

    Log::Info("Receiver: All "s + std::to_string(framesReceived) + " frames received and verified.");
    return true;
}


int
main(int argc, char * argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <sender_iface> <receiver_iface>\n", argv[0]);
        return 1;
    }

    string senderIface = argv[1];
    string receiverIface = argv[2];

    Log::initialize("/tmp/alpine_wifi_tests/raw_conn_test.log", Log::t_LogLevel::Debug);

    Log::Info("=== RawWifiConnection Frame Exchange Test ===");
    Log::Info("Sender interface:   "s + senderIface);
    Log::Info("Receiver interface: "s + receiverIface);

    // Fork: child = receiver, parent = sender
    pid_t pid = fork();

    if (pid < 0) {
        Log::Error("fork() failed.");
        return 1;
    }

    if (pid == 0) {
        // Child: receiver
        bool ok = runReceiver(receiverIface);
        _exit(ok ? 0 : 1);
    }

    // Parent: sender
    bool sendOk = runSender(senderIface);

    // Wait for receiver child
    int status = 0;
    waitpid(pid, &status, 0);
    bool recvOk = WIFEXITED(status) && WEXITSTATUS(status) == 0;

    if (sendOk && recvOk) {
        Log::Info("=== TEST PASSED ===");
        return 0;
    }

    if (!sendOk)
        Log::Error("Sender failed.");
    if (!recvOk)
        Log::Error("Receiver failed.");
    Log::Error("=== TEST FAILED ===");
    return 1;
}


#else  // Non-Linux


#include <cstdio>

int
main(int argc, char * argv[])
{
    fprintf(stderr, "SKIP: Raw 802.11 tests require Linux.\n");
    return 77;  // autotools-style skip
}


#endif
