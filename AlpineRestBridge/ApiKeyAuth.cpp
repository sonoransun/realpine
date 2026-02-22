/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <ApiKeyAuth.h>
#include <Log.h>
#include <fstream>
#include <cstdlib>
#include <sys/stat.h>


string ApiKeyAuth::apiKey_s;
bool   ApiKeyAuth::initialized_s = false;


void
ApiKeyAuth::initialize()
{
    if (initialized_s)
        return;

    // First check environment variable
    const char* envKey = getenv("ALPINE_API_KEY");
    if (envKey && strlen(envKey) > 0) {
        apiKey_s = envKey;
        initialized_s = true;
        Log::Info("API key loaded from environment variable.");
        return;
    }

    // Try to read from file
    string homeDir;
    const char* home = getenv("HOME");
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
    urandom.read(reinterpret_cast<char*>(randomBytes), sizeof(randomBytes));

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
ApiKeyAuth::validate(const HttpRequest& request, HttpResponse& response)
{
    // Exempt GET requests to /status paths
    if (request.method == "GET" && request.path.find("/status") == 0)
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

    // Expect "Bearer <key>"
    const string prefix = "Bearer ";
    if (authHeader.length() > prefix.length() &&
        authHeader.substr(0, prefix.length()) == prefix)
    {
        string providedKey = authHeader.substr(prefix.length());
        if (constantTimeCompare(providedKey, apiKey_s))
            return true;
    }

    response.setJsonBody("{\"error\":\"Unauthorized\"}");
    return false;
}


bool
ApiKeyAuth::constantTimeCompare(const string& a, const string& b)
{
    if (a.length() != b.length())
        return false;

    volatile byte result = 0;
    for (ulong i = 0; i < a.length(); ++i)
        result |= static_cast<byte>(a[i]) ^ static_cast<byte>(b[i]);

    return result == 0;
}


const string&
ApiKeyAuth::getKey()
{
    return apiKey_s;
}
