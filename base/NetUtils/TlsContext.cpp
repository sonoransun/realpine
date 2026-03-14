/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <TlsContext.h>
#include <Log.h>

#include <openssl/ssl.h>
#include <openssl/err.h>


TlsContext::~TlsContext ()
{
    if (ctx_) {
        SSL_CTX_free(ctx_);
        ctx_ = nullptr;
    }
}


bool
TlsContext::initialize (t_Mode mode)
{
    if (initialized_)
        return true;

    const SSL_METHOD * method = nullptr;

    if (mode == t_Mode::Server)
        method = TLS_server_method();
    else
        method = TLS_client_method();

    ctx_ = SSL_CTX_new(method);
    if (!ctx_) {
        Log::Error("TlsContext: failed to create SSL_CTX");
        return false;
    }

    // Set reasonable defaults
    SSL_CTX_set_options(ctx_, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
    SSL_CTX_set_min_proto_version(ctx_, TLS1_2_VERSION);

    // Harden cipher suites: modern AEAD ciphers only
    SSL_CTX_set_ciphersuites(ctx_,
        "TLS_AES_256_GCM_SHA384:"
        "TLS_AES_128_GCM_SHA256:"
        "TLS_CHACHA20_POLY1305_SHA256");

    SSL_CTX_set_cipher_list(ctx_,
        "ECDHE+AESGCM:ECDHE+CHACHA20:DHE+AESGCM:"
        "!RC4:!DES:!3DES:!MD5:!aNULL:!eNULL:!EXPORT");

    SSL_CTX_set_options(ctx_, SSL_OP_CIPHER_SERVER_PREFERENCE);

    initialized_ = true;
    return true;
}


bool
TlsContext::loadCertificate (const string & certPath)
{
    if (!ctx_) {
        Log::Error("TlsContext: not initialized");
        return false;
    }

    if (SSL_CTX_use_certificate_chain_file(ctx_, certPath.c_str()) != 1) {
        Log::Error("TlsContext: failed to load certificate: " + certPath);
        return false;
    }

    return true;
}


bool
TlsContext::loadPrivateKey (const string & keyPath)
{
    if (!ctx_) {
        Log::Error("TlsContext: not initialized");
        return false;
    }

    if (SSL_CTX_use_PrivateKey_file(ctx_, keyPath.c_str(), SSL_FILETYPE_PEM) != 1) {
        Log::Error("TlsContext: failed to load private key: " + keyPath);
        return false;
    }

    if (SSL_CTX_check_private_key(ctx_) != 1) {
        Log::Error("TlsContext: private key does not match certificate");
        return false;
    }

    return true;
}


bool
TlsContext::loadCaCertificate (const string & caPath)
{
    if (!ctx_) {
        Log::Error("TlsContext: not initialized");
        return false;
    }

    if (SSL_CTX_load_verify_locations(ctx_, caPath.c_str(), nullptr) != 1) {
        Log::Error("TlsContext: failed to load CA certificate: " + caPath);
        return false;
    }

    return true;
}


bool
TlsContext::setVerifyPeer (bool require)
{
    if (!ctx_) {
        Log::Error("TlsContext: not initialized");
        return false;
    }

    int mode = require ? (SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT)
                       : SSL_VERIFY_NONE;
    SSL_CTX_set_verify(ctx_, mode, nullptr);
    return true;
}


bool
TlsContext::setMinVersion (int version)
{
    if (!ctx_) {
        Log::Error("TlsContext: not initialized");
        return false;
    }

    if (SSL_CTX_set_min_proto_version(ctx_, version) != 1) {
        Log::Error("TlsContext: failed to set minimum protocol version");
        return false;
    }

    return true;
}


SSL_CTX *
TlsContext::context ()
{
    return ctx_;
}


bool
TlsContext::setCipherSuites (const string & tls13Ciphers,
                              const string & tls12Ciphers)
{
    if (!ctx_) {
        Log::Error("TlsContext: not initialized");
        return false;
    }

    if (!tls13Ciphers.empty()) {
        if (SSL_CTX_set_ciphersuites(ctx_, tls13Ciphers.c_str()) != 1) {
            Log::Error("TlsContext: failed to set TLS 1.3 cipher suites");
            return false;
        }
    }

    if (!tls12Ciphers.empty()) {
        if (SSL_CTX_set_cipher_list(ctx_, tls12Ciphers.c_str()) != 1) {
            Log::Error("TlsContext: failed to set TLS 1.2 cipher list");
            return false;
        }
    }

    return true;
}


bool
TlsContext::isInitialized () const
{
    return initialized_;
}
