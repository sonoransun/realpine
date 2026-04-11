/// Copyright (C) 2026 sonoransun — see LICENCE.txt

#pragma once

#ifdef ALPINE_TLS_ENABLED

#include <Common.h>

class TlsContext;

struct ssl_st;
using SSL = ssl_st;

struct bio_st;
using BIO = bio_st;


/// Wraps OpenSSL DTLS for UDP sockets and TLS for TCP connections.
/// Provides encrypt/decrypt operations for transport-layer data.
///
class DtlsWrapper
{
  public:
    enum class t_Mode { DtlsClient, DtlsServer, TlsClient, TlsServer };

    DtlsWrapper() = default;
    ~DtlsWrapper();

    /// Initialize the wrapper with a TlsContext and operating mode.
    ///
    bool initialize(TlsContext & tlsCtx, t_Mode mode);

    /// Set the underlying socket file descriptor for TLS mode.
    ///
    bool setSocketFd(int fd);

    /// Perform the handshake (connect for client, accept for server).
    /// Returns true on success.
    ///
    bool handshake();

    /// Encrypt and send data through the SSL session.
    /// Returns number of bytes written, or -1 on error.
    ///
    int wrapSend(const byte * data, uint len);

    /// Receive and decrypt data from the SSL session.
    /// Returns number of bytes read, or -1 on error.
    ///
    int unwrapRecv(byte * data, uint len);

    /// Shutdown the SSL session gracefully.
    ///
    void shutdown();

    /// Enable mutual peer certificate verification.
    /// Must be called before handshake.
    ///
    bool enablePeerVerification(bool require);

    /// Get the underlying SSL session (for certificate inspection).
    ///
    SSL * sslSession();

    bool isInitialized() const;

    bool isDtls() const;


  private:
    SSL * ssl_{nullptr};
    BIO * readBio_{nullptr};
    BIO * writeBio_{nullptr};
    t_Mode mode_{t_Mode::DtlsClient};
    bool initialized_{false};
    bool handshakeComplete_{false};
};

#endif  // ALPINE_TLS_ENABLED
