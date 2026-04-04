/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AuthHandler.h>
#include <ApiKeyAuth.h>
#include <JsonReader.h>
#include <JsonWriter.h>
#include <Log.h>
#include <Error.h>
#include <Platform.h>
#include <ReadLock.h>
#include <WriteLock.h>

#include <iomanip>
#include <sstream>

#ifdef ALPINE_TLS_ENABLED
#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/pem.h>
#endif


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

    {
        WriteLock guard(dataLock_s);
        enrolledDevices_s[deviceId] = std::move(device);
    }

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

    {
        WriteLock guard(dataLock_s);

        if (pendingChallenges_s.size() >= MAX_PENDING_CHALLENGES)
        {
            Log::Error("AuthHandler: pending challenge limit reached"s);
            return HttpResponse::tooManyRequests("Too many pending challenges");
        }

        t_PendingChallenge challenge;
        challenge.nonce     = nonce;
        challenge.createdAt = std::chrono::steady_clock::now();

        pendingChallenges_s[challengeId] = std::move(challenge);
    }

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
    string challengeNonce;

    {
        WriteLock guard(dataLock_s);

        auto it = pendingChallenges_s.find(challengeId);
        if (it == pendingChallenges_s.end())
            return HttpResponse::badRequest("Unknown or expired challenge");

        auto elapsed = std::chrono::steady_clock::now() - it->second.createdAt;
        auto elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();

        if (static_cast<ulong>(elapsedSeconds) > CHALLENGE_TTL_SECONDS)
        {
            pendingChallenges_s.erase(it);
            return HttpResponse::badRequest("Challenge expired");
        }

        // Save nonce before consuming (needed for signature verification)
        challengeNonce = it->second.nonce;

        // Consume the challenge (one-time use)
        pendingChallenges_s.erase(it);
    }

    // Verify the public key is enrolled
    bool enrolled = false;

    {
        ReadLock guard(dataLock_s);
        for (const auto & [id, device] : enrolledDevices_s)
        {
            if (device.publicKey == publicKey)
            {
                enrolled = true;
                break;
            }
        }
    }

    if (!enrolled)
    {
        Log::Error("AuthHandler: verify failed — public key not enrolled"s);
        return HttpResponse::badRequest("Device not enrolled");
    }

#ifdef ALPINE_TLS_ENABLED
    // ECDSA signature verification using OpenSSL EVP API
    {
        auto * bio = BIO_new_mem_buf(publicKey.data(),
                                     static_cast<int>(publicKey.size()));
        if (!bio)
        {
            Log::Error("AuthHandler: BIO_new_mem_buf failed"s);
            return HttpResponse::serverError("Signature verification failed");
        }

        auto * pkey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
        BIO_free(bio);

        if (!pkey)
        {
            Log::Error("AuthHandler: failed to parse public key"s);
            return HttpResponse::badRequest("Invalid public key format");
        }

        auto * mdCtx = EVP_MD_CTX_new();
        if (!mdCtx)
        {
            EVP_PKEY_free(pkey);
            Log::Error("AuthHandler: EVP_MD_CTX_new failed"s);
            return HttpResponse::serverError("Signature verification failed");
        }

        bool verified = false;

        if (EVP_DigestVerifyInit(mdCtx, nullptr, EVP_sha256(), nullptr, pkey) == 1)
        {
            if (EVP_DigestVerifyUpdate(mdCtx,
                                       challengeNonce.data(),
                                       challengeNonce.size()) == 1)
            {
                // Decode hex signature to binary
                string sigBytes;
                sigBytes.reserve(signature.size() / 2);
                for (ulong i = 0; i + 1 < signature.size(); i += 2)
                {
                    auto byte_val = static_cast<unsigned char>(
                        std::stoi(signature.substr(i, 2), nullptr, 16));
                    sigBytes += static_cast<char>(byte_val);
                }

                verified = EVP_DigestVerifyFinal(
                    mdCtx,
                    reinterpret_cast<const unsigned char *>(sigBytes.data()),
                    sigBytes.size()) == 1;
            }
        }

        EVP_MD_CTX_free(mdCtx);
        EVP_PKEY_free(pkey);

        if (!verified)
        {
            Log::Error("AuthHandler: ECDSA signature verification failed"s);
            return HttpResponse::badRequest("Signature verification failed");
        }
    }
#else
    // Without TLS/OpenSSL, signature verification is not available
    Log::Error("AuthHandler: signature verification requires TLS support"s);
    return HttpResponse::serverError("Signature verification not available (TLS disabled)");
#endif

    Log::Info("AuthHandler: device verified via challenge-response"s);

    // Issue a session token
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
// readUrandom — read numBytes from /dev/urandom and return as hex string
// ---------------------------------------------------------------------------

string
AuthHandler::readUrandom (ulong numBytes)
{
    std::vector<byte> buf(numBytes);
    if (!alpine_random_bytes(buf.data(), static_cast<unsigned long>(numBytes)))
    {
        Log::Error("AuthHandler: failed to generate random bytes"s);
        return {};
    }

    static const char hexChars[] = "0123456789abcdef";
    string result;
    result.reserve(numBytes * 2);
    for (auto b : buf)
    {
        result += hexChars[(b >> 4) & 0x0F];
        result += hexChars[b & 0x0F];
    }

    return result;
}


// ---------------------------------------------------------------------------
// generateNonce — 32 random bytes as hex
// ---------------------------------------------------------------------------

string
AuthHandler::generateNonce ()
{
    return readUrandom(32);
}


// ---------------------------------------------------------------------------
// generateId — 16 random bytes as hex
// ---------------------------------------------------------------------------

string
AuthHandler::generateId ()
{
    return readUrandom(16);
}


// ---------------------------------------------------------------------------
// evictExpiredChallenges
// ---------------------------------------------------------------------------

void
AuthHandler::evictExpiredChallenges ()
{
    auto now = std::chrono::steady_clock::now();

    WriteLock guard(dataLock_s);

    for (auto it = pendingChallenges_s.begin(); it != pendingChallenges_s.end(); )
    {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - it->second.createdAt).count();

        if (static_cast<ulong>(elapsed) > CHALLENGE_TTL_SECONDS)
            it = pendingChallenges_s.erase(it);
        else
            ++it;
    }
}
