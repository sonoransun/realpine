/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <HttpRequest.h>
#include <HttpResponse.h>
#include <HttpRouter.h>
#include <ReadWriteSem.h>
#include <chrono>
#include <unordered_map>


class AuthHandler
{
  public:
    static void registerRoutes(HttpRouter & router);


  private:
    static HttpResponse enrollDevice(const HttpRequest & request, const std::unordered_map<string, string> & params);

    static HttpResponse getChallenge(const HttpRequest & request, const std::unordered_map<string, string> & params);

    static HttpResponse verifySignature(const HttpRequest & request, const std::unordered_map<string, string> & params);


    // In-memory challenge store with expiry
    //
    struct t_PendingChallenge
    {
        string nonce;
        std::chrono::steady_clock::time_point createdAt;
    };

    static std::unordered_map<string, t_PendingChallenge> pendingChallenges_s;

    // Enrolled device public keys (deviceId -> base64 public key)
    //
    struct t_EnrolledDevice
    {
        string publicKey;
        string deviceName;
        string biometricType;
        std::chrono::system_clock::time_point enrolledAt;
    };

    static std::unordered_map<string, t_EnrolledDevice> enrolledDevices_s;

    static ReadWriteSem dataLock_s;

    static constexpr ulong CHALLENGE_TTL_SECONDS = 60;
    static constexpr ulong MAX_PENDING_CHALLENGES = 10000;

    static string generateNonce();
    static string generateId();
    static string readUrandom(ulong numBytes);
    static void evictExpiredChallenges();
};
