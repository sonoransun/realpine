/// Copyright (C) 2026 sonoransun — see LICENCE.txt

#ifdef ALPINE_TLS_ENABLED

#include <DtlsWrapper.h>
#include <Log.h>
#include <TlsContext.h>

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>


DtlsWrapper::~DtlsWrapper()
{
    shutdown();
}


bool
DtlsWrapper::initialize(TlsContext & tlsCtx, t_Mode mode)
{
    if (initialized_)
        return true;

    if (!tlsCtx.isInitialized()) {
        Log::Error("DtlsWrapper: TlsContext not initialized");
        return false;
    }

    mode_ = mode;

    // For DTLS modes, create a separate SSL_CTX with DTLS method
    SSL_CTX * ctx = nullptr;

    if (mode == t_Mode::DtlsClient || mode == t_Mode::DtlsServer) {
        const SSL_METHOD * method = (mode == t_Mode::DtlsServer) ? DTLS_server_method() : DTLS_client_method();
        ctx = SSL_CTX_new(method);
        if (!ctx) {
            Log::Error("DtlsWrapper: failed to create DTLS SSL_CTX");
            return false;
        }
        // Copy cert/key settings from the TlsContext if available
        // The caller should configure the DTLS context separately if needed
    } else {
        // TLS mode — use the existing TlsContext directly
        ctx = tlsCtx.context();
        SSL_CTX_up_ref(ctx);
    }

    ssl_ = SSL_new(ctx);

    // Release our reference — SSL holds its own
    if (mode == t_Mode::DtlsClient || mode == t_Mode::DtlsServer)
        SSL_CTX_free(ctx);
    else
        SSL_CTX_free(ctx);

    if (!ssl_) {
        Log::Error("DtlsWrapper: failed to create SSL object");
        return false;
    }

    if (isDtls()) {
        // Create memory BIOs for DTLS (datagram-oriented)
        readBio_ = BIO_new(BIO_s_mem());
        writeBio_ = BIO_new(BIO_s_mem());

        if (!readBio_ || !writeBio_) {
            Log::Error("DtlsWrapper: failed to create BIO pair");
            SSL_free(ssl_);
            ssl_ = nullptr;
            return false;
        }

        // Set BIOs — SSL takes ownership
        SSL_set_bio(ssl_, readBio_, writeBio_);
    }

    initialized_ = true;
    return true;
}


bool
DtlsWrapper::setSocketFd(int fd)
{
    if (!initialized_) {
        Log::Error("DtlsWrapper: not initialized");
        return false;
    }

    if (isDtls()) {
        // For DTLS, create a datagram BIO from the socket fd
        BIO * dgBio = BIO_new_dgram(fd, BIO_NOCLOSE);
        if (!dgBio) {
            Log::Error("DtlsWrapper: failed to create datagram BIO");
            return false;
        }
        SSL_set_bio(ssl_, dgBio, dgBio);
        readBio_ = nullptr;
        writeBio_ = nullptr;
    } else {
        // For TLS, set the socket fd directly
        if (SSL_set_fd(ssl_, fd) != 1) {
            Log::Error("DtlsWrapper: failed to set socket fd");
            return false;
        }
    }

    return true;
}


bool
DtlsWrapper::handshake()
{
    if (!initialized_) {
        Log::Error("DtlsWrapper: not initialized");
        return false;
    }

    int ret;
    if (mode_ == t_Mode::DtlsServer || mode_ == t_Mode::TlsServer)
        ret = SSL_accept(ssl_);
    else
        ret = SSL_connect(ssl_);

    if (ret == 1) {
        handshakeComplete_ = true;
        return true;
    }

    int err = SSL_get_error(ssl_, ret);
    if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)
        return false;  // Non-blocking: try again later

    Log::Error("DtlsWrapper: handshake failed, SSL error: "s + std::to_string(err));
    return false;
}


int
DtlsWrapper::wrapSend(const byte * data, uint len)
{
    if (!initialized_ || !handshakeComplete_)
        return -1;

    int written = SSL_write(ssl_, data, static_cast<int>(len));
    if (written <= 0) {
        int err = SSL_get_error(ssl_, written);
        if (err == SSL_ERROR_WANT_WRITE)
            return 0;
        Log::Error("DtlsWrapper: SSL_write failed, error: "s + std::to_string(err));
        return -1;
    }

    return written;
}


int
DtlsWrapper::unwrapRecv(byte * data, uint len)
{
    if (!initialized_ || !handshakeComplete_)
        return -1;

    int bytesRead = SSL_read(ssl_, data, static_cast<int>(len));
    if (bytesRead <= 0) {
        int err = SSL_get_error(ssl_, bytesRead);
        if (err == SSL_ERROR_WANT_READ)
            return 0;
        if (err == SSL_ERROR_ZERO_RETURN)
            return 0;  // Peer closed connection
        Log::Error("DtlsWrapper: SSL_read failed, error: "s + std::to_string(err));
        return -1;
    }

    return bytesRead;
}


void
DtlsWrapper::shutdown()
{
    if (ssl_) {
        if (handshakeComplete_)
            SSL_shutdown(ssl_);
        SSL_free(ssl_);
        ssl_ = nullptr;
    }
    // BIOs are freed by SSL_free when set via SSL_set_bio
    readBio_ = nullptr;
    writeBio_ = nullptr;
    initialized_ = false;
    handshakeComplete_ = false;
}


bool
DtlsWrapper::enablePeerVerification(bool require)
{
    if (!initialized_ || !ssl_) {
        Log::Error("DtlsWrapper: not initialized");
        return false;
    }

    int mode = require ? (SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT) : SSL_VERIFY_NONE;
    SSL_set_verify(ssl_, mode, nullptr);
    return true;
}


SSL *
DtlsWrapper::sslSession()
{
    return ssl_;
}


bool
DtlsWrapper::isInitialized() const
{
    return initialized_;
}


bool
DtlsWrapper::isDtls() const
{
    return mode_ == t_Mode::DtlsClient || mode_ == t_Mode::DtlsServer;
}

#endif  // ALPINE_TLS_ENABLED
