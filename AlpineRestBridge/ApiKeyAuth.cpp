/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <ApiKeyAuth.h>
#include <Log.h>
#include <fstream>
#include <cstdlib>
#include <sys/stat.h>

#ifdef ALPINE_TLS_ENABLED
#include <jwt-cpp/jwt.h>
#endif


string ApiKeyAuth::apiKey_s;
bool   ApiKeyAuth::initialized_s = false;

#ifdef ALPINE_TLS_ENABLED
string ApiKeyAuth::jwtSecret_s;
string ApiKeyAuth::jwtIssuer_s;
string ApiKeyAuth::jwtAudience_s;
bool   ApiKeyAuth::jwtConfigured_s = false;
#endif


void
ApiKeyAuth::initialize ()
{
    if (initialized_s)
        return;

    // First check environment variable
    const char * envKey = getenv("ALPINE_API_KEY");
    if (envKey && strlen(envKey) > 0) {
        apiKey_s = envKey;
        initialized_s = true;
        Log::Info("API key loaded from environment variable.");
        return;
    }

    // Try to read from file
    string homeDir;
    const char * home = getenv("HOME");
    if (home)
        homeDir = home;
    else
        homeDir = "/tmp";

    string keyDir  = homeDir + "/.alpine";
    string keyFile = keyDir + "/api.key";

    std::ifstream ifs(keyFile);
    if (ifs.good()) {
        std::getline(ifs, apiKey_s);
        if (!apiKey_s.empty()) {
            initialized_s = true;
            Log::Info("API key loaded from file.");
            return;
        }
    }

    // Generate a random 32-byte hex key using /dev/urandom
    std::ifstream urandom("/dev/urandom", std::ios::binary);
    if (!urandom.good()) {
        Log::Error("Failed to open /dev/urandom for API key generation.");
        apiKey_s = "fallback-key-change-me";
        initialized_s = true;
        return;
    }

    byte randomBytes[32];
    urandom.read(reinterpret_cast<char *>(randomBytes), sizeof(randomBytes));

    apiKey_s.clear();
    apiKey_s.reserve(64);
    static const char hexChars[] = "0123456789abcdef";
    for (ulong i = 0; i < sizeof(randomBytes); ++i) {
        apiKey_s += hexChars[(randomBytes[i] >> 4) & 0x0F];
        apiKey_s += hexChars[randomBytes[i] & 0x0F];
    }

    // Create directory if needed
    mkdir(keyDir.c_str(), 0700);

    // Write key to file
    std::ofstream ofs(keyFile);
    if (ofs.good()) {
        ofs << apiKey_s;
        chmod(keyFile.c_str(), 0600);
        Log::Info("API key generated and saved to file.");
    } else {
        Log::Error("Failed to write API key to file.");
    }

    initialized_s = true;
}


bool
ApiKeyAuth::validate (const HttpRequest & request, HttpResponse & response)
{
    // Exempt GET requests to /status paths
    if (request.method == "GET" && request.path.starts_with("/status"))
        return true;

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

        // Try API key first
        if (constantTimeCompare(token, apiKey_s))
            return true;

#ifdef ALPINE_TLS_ENABLED
        // Try JWT validation if API key didn't match
        if (jwtConfigured_s && validateJwt(token))
            return true;
#endif
    }

    response.setJsonBody("{\"error\":\"Unauthorized\"}");
    return false;
}


bool
ApiKeyAuth::constantTimeCompare (const string & a, const string & b)
{
    if (a.length() != b.length())
        return false;

    volatile byte result = 0;
    for (ulong i = 0; i < a.length(); ++i)
        result |= static_cast<byte>(a[i]) ^ static_cast<byte>(b[i]);

    return result == 0;
}


const string &
ApiKeyAuth::getKey ()
{
    return apiKey_s;
}


#ifdef ALPINE_TLS_ENABLED

void
ApiKeyAuth::configureJwt (const string & secret,
                          const string & issuer,
                          const string & audience)
{
    jwtSecret_s = secret;
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
            .allow_algorithm(jwt::algorithm::hs256{jwtSecret_s})
            .with_issuer(jwtIssuer_s);

        if (!jwtAudience_s.empty())
            verifier.with_audience({jwtAudience_s});

        verifier.verify(decoded);
        return true;

    } catch (const std::exception & e) {
        Log::Debug("JWT validation failed: "s + e.what());
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
    if (constantTimeCompare(token, apiKey_s))
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
