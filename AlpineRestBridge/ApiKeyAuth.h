/// Copyright (C) 2026 sonoransun — see LICENCE.txt

#pragma once
#include <Common.h>
#include <HttpRequest.h>
#include <HttpResponse.h>
#include <SecureString.h>
#include <Mutex.h>
#include <chrono>

class ApiKeyAuth
{
  public:

    struct t_ApiKeyEntry {
        SecureString                              key;
        std::chrono::steady_clock::time_point     expiresAt;
        bool                                      deprecated{false};
    };

    static void  initialize ();
    [[nodiscard]] static bool  validate (const HttpRequest & request, HttpResponse & response);
    static string  getKey ();

    static string  rotateKey (std::chrono::seconds gracePeriod = std::chrono::seconds(3600));
    static void    purgeExpiredKeys ();
    static ulong   activeKeyCount ();

#ifdef ALPINE_TLS_ENABLED
    static void  configureJwt (const string & secret,
                               const string & issuer,
                               const string & audience);

    static bool  hasScope (const HttpRequest & request, const string & scope);
#endif

  private:

#ifdef ALPINE_TLS_ENABLED
    static bool  validateJwt (const string & token);
    static bool  extractJwtScopes (const string & token, std::vector<string> & scopes);

    static SecureString  jwtSecret_s;
    static string        jwtIssuer_s;
    static string        jwtAudience_s;
    static bool          jwtConfigured_s;
#endif

    static SecureString          apiKey_s;
    static bool                  initialized_s;
    static vector<t_ApiKeyEntry> keys_s;
    static Mutex                 keysMutex_s;
};
