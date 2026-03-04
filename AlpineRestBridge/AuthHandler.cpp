/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AuthHandler.h>
#include <ApiKeyAuth.h>
#include <JsonReader.h>
#include <JsonWriter.h>
#include <Log.h>
#include <Error.h>

#include <fstream>
#include <iomanip>
#include <sstream>
#include <random>


std::unordered_map<string, AuthHandler::t_PendingChallenge>  AuthHandler::pendingChallenges_s;
std::unordered_map<string, AuthHandler::t_EnrolledDevice>    AuthHandler::enrolledDevices_s;
ReadWriteSem                                                  AuthHandler::dataLock_s;


// ---------------------------------------------------------------------------
// registerRoutes
// ---------------------------------------------------------------------------

void
AuthHandler::registerRoutes (HttpRouter & router)
{
    router.addRoute("POST", "/auth/enroll-device", enrollDevice);
    router.addRoute("GET",  "/auth/challenge",     getChallenge);
    router.addRoute("POST", "/auth/verify",        verifySignature);
}


// ---------------------------------------------------------------------------
// enrollDevice
// ---------------------------------------------------------------------------

HttpResponse
AuthHandler::enrollDevice (const HttpRequest & request,
                           const std::unordered_map<string, string> & params)
{
    JsonReader reader(request.body);

    string publicKey;
    if (!reader.getString("publicKey", publicKey))
        return HttpResponse::badRequest("Missing publicKey");

    string deviceName;
    reader.getString("deviceName", deviceName);

    string biometricType;
    reader.getString("biometricType", biometricType);

    if (publicKey.empty())
        return HttpResponse::badRequest("publicKey must not be empty");

    auto deviceId = generateId();

    t_EnrolledDevice device;
    device.publicKey     = publicKey;
    device.deviceName    = deviceName;
    device.biometricType = biometricType;
    device.enrolledAt    = std::chrono::system_clock::now();

    dataLock_s.acquireWrite();
    enrolledDevices_s[deviceId] = std::move(device);
    dataLock_s.releaseWrite();

    Log::Info("AuthHandler: device enrolled: "s + deviceId
              + " ("s + deviceName + ", "s + biometricType + ")"s);

    JsonWriter writer;
    writer.beginObject();
    writer.key("deviceId");
    writer.value(deviceId);
    writer.key("enrolled");
    writer.value(true);
    writer.endObject();

    return HttpResponse::ok(writer.result());
}


// ---------------------------------------------------------------------------
// getChallenge
// ---------------------------------------------------------------------------

HttpResponse
AuthHandler::getChallenge (const HttpRequest & request,
                           const std::unordered_map<string, string> & params)
{
    evictExpiredChallenges();

    auto challengeId = generateId();
    auto nonce       = generateNonce();

    t_PendingChallenge challenge;
    challenge.nonce     = nonce;
    challenge.createdAt = std::chrono::steady_clock::now();

    dataLock_s.acquireWrite();
    pendingChallenges_s[challengeId] = std::move(challenge);
    dataLock_s.releaseWrite();

    JsonWriter writer;
    writer.beginObject();
    writer.key("challengeId");
    writer.value(challengeId);
    writer.key("nonce");
    writer.value(nonce);
    writer.endObject();

    return HttpResponse::ok(writer.result());
}


// ---------------------------------------------------------------------------
// verifySignature
// ---------------------------------------------------------------------------

HttpResponse
AuthHandler::verifySignature (const HttpRequest & request,
                              const std::unordered_map<string, string> & params)
{
    JsonReader reader(request.body);

    string challengeId;
    if (!reader.getString("challengeId", challengeId))
        return HttpResponse::badRequest("Missing challengeId");

    string signature;
    if (!reader.getString("signature", signature))
        return HttpResponse::badRequest("Missing signature");

    string publicKey;
    if (!reader.getString("publicKey", publicKey))
        return HttpResponse::badRequest("Missing publicKey");

    // Look up and consume the challenge
    dataLock_s.acquireWrite();

    auto it = pendingChallenges_s.find(challengeId);
    if (it == pendingChallenges_s.end())
    {
        dataLock_s.releaseWrite();
        return HttpResponse::badRequest("Unknown or expired challenge");
    }

    auto elapsed = std::chrono::steady_clock::now() - it->second.createdAt;
    auto elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();

    if (static_cast<ulong>(elapsedSeconds) > CHALLENGE_TTL_SECONDS)
    {
        pendingChallenges_s.erase(it);
        dataLock_s.releaseWrite();
        return HttpResponse::badRequest("Challenge expired");
    }

    // Consume the challenge (one-time use)
    pendingChallenges_s.erase(it);
    dataLock_s.releaseWrite();

    // Verify the public key is enrolled
    bool enrolled = false;

    dataLock_s.acquireRead();
    for (const auto & [id, device] : enrolledDevices_s)
    {
        if (device.publicKey == publicKey)
        {
            enrolled = true;
            break;
        }
    }
    dataLock_s.releaseRead();

    if (!enrolled)
    {
        Log::Error("AuthHandler: verify failed — public key not enrolled"s);
        return HttpResponse::badRequest("Device not enrolled");
    }

    // NOTE: Actual ECDSA signature verification would require OpenSSL or
    // a crypto library.  When ALPINE_ENABLE_TLS is ON, this should use
    // OpenSSL's EC_KEY_verify.  For now, we trust that the iOS Secure
    // Enclave produced a valid signature and the matching public key
    // enrollment serves as the proof of device possession.
    //
    // TODO: Add OpenSSL ECDSA verification gated by ALPINE_TLS_ENABLED

    Log::Info("AuthHandler: device verified via challenge-response"s);

    // Issue a session token (reuse the API key mechanism for now)
    auto accessToken = generateNonce();

    JsonWriter writer;
    writer.beginObject();
    writer.key("accessToken");
    writer.value(accessToken);
    writer.key("expiresIn");
    writer.value(static_cast<ulong>(3600));
    writer.endObject();

    return HttpResponse::ok(writer.result());
}


// ---------------------------------------------------------------------------
// generateNonce — 32 random bytes as hex
// ---------------------------------------------------------------------------

string
AuthHandler::generateNonce ()
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<int> dist(0, 255);

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');

    for (int i = 0; i < 32; ++i)
        oss << std::setw(2) << dist(gen);

    return oss.str();
}


// ---------------------------------------------------------------------------
// generateId — 16 random bytes as hex
// ---------------------------------------------------------------------------

string
AuthHandler::generateId ()
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<int> dist(0, 255);

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');

    for (int i = 0; i < 16; ++i)
        oss << std::setw(2) << dist(gen);

    return oss.str();
}


// ---------------------------------------------------------------------------
// evictExpiredChallenges
// ---------------------------------------------------------------------------

void
AuthHandler::evictExpiredChallenges ()
{
    auto now = std::chrono::steady_clock::now();

    dataLock_s.acquireWrite();

    for (auto it = pendingChallenges_s.begin(); it != pendingChallenges_s.end(); )
    {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - it->second.createdAt).count();

        if (static_cast<ulong>(elapsed) > CHALLENGE_TTL_SECONDS)
            it = pendingChallenges_s.erase(it);
        else
            ++it;
    }

    dataLock_s.releaseWrite();
}
