/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <ApiKeyAuth.h>
#include <MutexLock.h>
#include <Log.h>
#include <Platform.h>
#include <StringUtils.h>
#include <fstream>
#include <cstdlib>
#include <cstring>

#ifdef ALPINE_TLS_ENABLED
#include <jwt-cpp/jwt.h>
#endif


SecureString             ApiKeyAuth::apiKey_s;
bool                     ApiKeyAuth::initialized_s = false;
vector<ApiKeyAuth::t_ApiKeyEntry>  ApiKeyAuth::keys_s;
Mutex                    ApiKeyAuth::keysMutex_s;

#ifdef ALPINE_TLS_ENABLED
SecureString ApiKeyAuth::jwtSecret_s;
string       ApiKeyAuth::jwtIssuer_s;
string       ApiKeyAuth::jwtAudience_s;
bool         ApiKeyAuth::jwtConfigured_s = false;
#endif


void
ApiKeyAuth::initialize ()
{
    if (initialized_s)
        return;

    // First check environment variable
    const char * envKey = getenv("ALPINE_API_KEY");
    if (envKey && strlen(envKey) > 0) {
        apiKey_s.assign(string(envKey));
        initialized_s = true;

        // Add initial key to rotation list (non-deprecated, no expiry)
        MutexLock lock(keysMutex_s);
        t_ApiKeyEntry entry;
        entry.key.assign(apiKey_s.value());
        entry.expiresAt = std::chrono::steady_clock::time_point::max();
        entry.deprecated = false;
        keys_s.push_back(std::move(entry));

        Log::Info("API key loaded from environment variable.");
        return;
    }

    // Try to read from file
    string homeDir = alpine_home_dir();

    string keyDir  = homeDir + "/.alpine";
    string keyFile = keyDir + "/api.key";

    std::ifstream ifs(keyFile);
    if (ifs.good()) {
        string fileKey;
        std::getline(ifs, fileKey);
        if (!fileKey.empty()) {
            apiKey_s.assign(std::move(fileKey));
            initialized_s = true;

            // Add initial key to rotation list
            MutexLock lock(keysMutex_s);
            t_ApiKeyEntry entry;
            entry.key.assign(apiKey_s.value());
            entry.expiresAt = std::chrono::steady_clock::time_point::max();
            entry.deprecated = false;
            keys_s.push_back(std::move(entry));

            Log::Info("API key loaded from file.");
            return;
        }
    }

    // Generate a random 32-byte hex key
    byte randomBytes[32];
    if (!alpine_random_bytes(randomBytes, sizeof(randomBytes))) {
        Log::Error("CRITICAL: Failed to generate random bytes for API key. "
                   "All API requests will be rejected."s);
        initialized_s = true;
        return;
    }

    string hexKey;
    hexKey.reserve(64);
    static const char hexChars[] = "0123456789abcdef";
    for (ulong i = 0; i < sizeof(randomBytes); ++i) {
        hexKey += hexChars[(randomBytes[i] >> 4) & 0x0F];
        hexKey += hexChars[randomBytes[i] & 0x0F];
    }

    // Zero the random bytes buffer
    volatile auto * p = randomBytes;
    memset(const_cast<byte *>(p), 0, sizeof(randomBytes));

    apiKey_s.assign(hexKey);

    // Create directory if needed
    alpine_mkdir(keyDir.c_str(), 0700);

    // Write key to file
    std::ofstream ofs(keyFile);
    if (ofs.good()) {
        ofs << apiKey_s.value();
        alpine_chmod(keyFile.c_str(), 0600);
        Log::Info("API key generated and saved to file.");
    } else {
        Log::Error("Failed to write API key to file.");
    }

    // Add initial key to rotation list
    {
        MutexLock lock(keysMutex_s);
        t_ApiKeyEntry entry;
        entry.key.assign(apiKey_s.value());
        entry.expiresAt = std::chrono::steady_clock::time_point::max();
        entry.deprecated = false;
        keys_s.push_back(std::move(entry));
    }

    initialized_s = true;
}


bool
ApiKeyAuth::validate (const HttpRequest & request, HttpResponse & response)
{
    // Exempt GET requests to /status paths
    if (request.method == "GET" && request.path.starts_with("/status"))
        return true;

    // If no API key was generated (urandom failure), reject all requests
    if (apiKey_s.empty())
    {
        Log::Error("ApiKeyAuth: rejecting request — no API key available"s);
        response.setJsonBody("{\"error\":\"Service unavailable — authentication not configured\"}");
        return false;
    }

    // Look for Authorization header (try both cases)
    string authHeader;
    auto it = request.headers.find("Authorization");
    if (it != request.headers.end())
        authHeader = it->second;
    else {
        it = request.headers.find("authorization");
        if (it != request.headers.end())
            authHeader = it->second;
    }

    // Expect "Bearer <token>"
    constexpr std::string_view prefix = "Bearer ";
    if (authHeader.starts_with(prefix)) {
        string token = authHeader.substr(prefix.length());

        // Try primary API key first
        if (apiKey_s.equals(token))
            return true;

        // Check rotated keys (deprecated but not yet expired)
        {
            MutexLock lock(keysMutex_s);
            auto now = std::chrono::steady_clock::now();
            for (const auto & entry : keys_s) {
                if (entry.expiresAt > now && entry.key.equals(token))
                    return true;
            }
        }

#ifdef ALPINE_TLS_ENABLED
        // Try JWT validation if API key didn't match
        if (jwtConfigured_s && validateJwt(token))
            return true;
#endif
    }

    response.setJsonBody("{\"error\":\"Unauthorized\"}");
    return false;
}


string
ApiKeyAuth::getKey ()
{
    return apiKey_s.value();
}


string
ApiKeyAuth::rotateKey (std::chrono::seconds gracePeriod)
{
    // Generate new 32-byte random key
    byte randomBytes[32];
    if (!alpine_random_bytes(randomBytes, sizeof(randomBytes))) {
        Log::Error("rotateKey: failed to generate random bytes"s);
        return {};
    }

    string hexKey;
    hexKey.reserve(64);
    static const char hexChars[] = "0123456789abcdef";
    for (ulong i = 0; i < sizeof(randomBytes); ++i) {
        hexKey += hexChars[(randomBytes[i] >> 4) & 0x0F];
        hexKey += hexChars[randomBytes[i] & 0x0F];
    }

    // Zero the random bytes buffer
    volatile auto * p = randomBytes;
    memset(const_cast<byte *>(p), 0, sizeof(randomBytes));

    auto now = std::chrono::steady_clock::now();

    MutexLock lock(keysMutex_s);

    // Mark all current keys as deprecated with expiry
    for (auto & entry : keys_s) {
        if (!entry.deprecated) {
            entry.deprecated = true;
            entry.expiresAt = now + gracePeriod;
        }
    }

    // Create new key entry (non-deprecated, no expiry)
    t_ApiKeyEntry newEntry;
    newEntry.key.assign(hexKey);
    newEntry.expiresAt = std::chrono::steady_clock::time_point::max();
    newEntry.deprecated = false;
    keys_s.insert(keys_s.begin(), std::move(newEntry));

    // Update primary key
    apiKey_s.assign(hexKey);

    // Purge any already-expired keys
    purgeExpiredKeys();

    Log::Info("API key rotated successfully."s);

    return hexKey;
}


void
ApiKeyAuth::purgeExpiredKeys ()
{
    auto now = std::chrono::steady_clock::now();
    std::erase_if(keys_s, [&now](const t_ApiKeyEntry & entry) {
        return entry.deprecated && entry.expiresAt <= now;
    });
}


ulong
ApiKeyAuth::activeKeyCount ()
{
    MutexLock lock(keysMutex_s);
    auto now = std::chrono::steady_clock::now();
    ulong count = 0;
    for (const auto & entry : keys_s) {
        if (entry.expiresAt > now)
            ++count;
    }
    return count;
}


#ifdef ALPINE_TLS_ENABLED

void
ApiKeyAuth::configureJwt (const string & secret,
                          const string & issuer,
                          const string & audience)
{
    jwtSecret_s.assign(secret);
    jwtIssuer_s = issuer;
    jwtAudience_s = audience;
    jwtConfigured_s = true;
    Log::Info("JWT authentication configured.");
}


bool
ApiKeyAuth::validateJwt (const string & token)
{
    try {
        auto decoded = jwt::decode(token);

        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{jwtSecret_s.value()})
            .with_issuer(jwtIssuer_s);

        if (!jwtAudience_s.empty())
            verifier.with_audience({jwtAudience_s});

        verifier.verify(decoded);
        return true;

    } catch (const std::exception & e) {
        Log::Debug("JWT validation failed"s);
#ifdef _VERBOSE
        Log::Debug("JWT detail: "s + StringUtils::sanitizeForLog(e.what()));
#endif
        return false;
    }
}


bool
ApiKeyAuth::extractJwtScopes (const string & token, std::vector<string> & scopes)
{
    try {
        auto decoded = jwt::decode(token);

        if (!decoded.has_payload_claim("scope"))
            return false;

        auto scopeClaim = decoded.get_payload_claim("scope").as_string();

        // Parse space-delimited scopes (RFC 8693)
        scopes.clear();
        string current;
        for (char c : scopeClaim) {
            if (c == ' ') {
                if (!current.empty()) {
                    scopes.push_back(std::move(current));
                    current.clear();
                }
            } else {
                current += c;
            }
        }
        if (!current.empty())
            scopes.push_back(std::move(current));

        return true;

    } catch (const std::exception &) {
        return false;
    }
}


bool
ApiKeyAuth::hasScope (const HttpRequest & request, const string & scope)
{
    // Extract Bearer token
    string authHeader;
    auto it = request.headers.find("Authorization");
    if (it != request.headers.end())
        authHeader = it->second;
    else {
        it = request.headers.find("authorization");
        if (it != request.headers.end())
            authHeader = it->second;
    }

    constexpr std::string_view prefix = "Bearer ";
    if (!authHeader.starts_with(prefix))
        return false;

    string token = authHeader.substr(prefix.length());

    // API key has all scopes
    if (apiKey_s.equals(token))
        return true;

    std::vector<string> scopes;
    if (!extractJwtScopes(token, scopes))
        return false;

    for (const auto & s : scopes) {
        if (s == scope || s == "admin")
            return true;
    }

    return false;
}

#endif
