/// Copyright (C) 2026 sonoransun — see LICENCE.txt

#pragma once
#include <Common.h>
#include <HttpRequest.h>
#include <HttpResponse.h>

class ApiKeyAuth
{
  public:

    static void  initialize ();
    [[nodiscard]] static bool  validate (const HttpRequest & request, HttpResponse & response);
    static const string &  getKey ();

#ifdef ALPINE_TLS_ENABLED
    static void  configureJwt (const string & secret,
                               const string & issuer,
                               const string & audience);

    static bool  hasScope (const HttpRequest & request, const string & scope);
#endif

  private:

    [[nodiscard]] static bool  constantTimeCompare (const string & a, const string & b);

#ifdef ALPINE_TLS_ENABLED
    static bool  validateJwt (const string & token);
    static bool  extractJwtScopes (const string & token, std::vector<string> & scopes);

    static string  jwtSecret_s;
    static string  jwtIssuer_s;
    static string  jwtAudience_s;
    static bool    jwtConfigured_s;
#endif

    static string  apiKey_s;
    static bool    initialized_s;
};
