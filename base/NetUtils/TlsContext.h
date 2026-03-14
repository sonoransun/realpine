/// Copyright (C) 2026 sonoransun — see LICENCE.txt

#pragma once
#include <Common.h>
#include <string>
#include <memory>

// Forward declarations for OpenSSL types
struct ssl_ctx_st;
using SSL_CTX = ssl_ctx_st;

class TlsContext
{
  public:

    enum class t_Mode { Server, Client };

    TlsContext () = default;
    ~TlsContext ();

    bool  initialize (t_Mode mode);

    bool  loadCertificate (const string & certPath);
    bool  loadPrivateKey (const string & keyPath);
    bool  loadCaCertificate (const string & caPath);

    bool  setVerifyPeer (bool require);
    bool  setMinVersion (int version);  // TLS1_2_VERSION, TLS1_3_VERSION
    bool  setCipherSuites (const string & tls13Ciphers, const string & tls12Ciphers);

    SSL_CTX *  context ();

    bool  isInitialized () const;

  private:

    SSL_CTX *  ctx_{nullptr};
    bool       initialized_{false};
};
