/// Copyright (C) 2026 sonoransun — see LICENCE.txt

#pragma once

#ifdef ALPINE_TLS_ENABLED

#include <Common.h>

struct ssl_st;
using SSL = ssl_st;

struct x509_st;
using X509 = x509_st;


/// Verifies peer TLS certificates for mutual authentication.
/// Validates certificate chain, CN/SAN against peer identity,
/// and enforces mutual verification policy.
///
class PeerTlsVerifier
{
  public:

    /// Verify the peer certificate from an established SSL session.
    /// Checks certificate chain validity and CN/SAN matching.
    /// Returns true if the peer is verified.
    ///
    static bool  verifyPeer (SSL * ssl, ulong expectedPeerId);

    /// Verify a peer certificate's CN or SAN against an expected identity.
    /// The identity is matched as a string representation of the peer ID.
    ///
    static bool  verifyCertificateIdentity (X509 * cert, const string & expectedIdentity);

    /// Check if mutual TLS is enabled via Configuration.
    ///
    static bool  isMutualTlsEnabled ();

    /// Get the configured CA certificate path from Configuration.
    /// Returns empty string if not configured.
    ///
    static string  getCaCertificatePath ();


  private:

    /// Extract the Common Name from a certificate subject.
    ///
    static string  extractCommonName (X509 * cert);

    /// Extract Subject Alternative Names from a certificate.
    ///
    static vector<string>  extractSubjectAltNames (X509 * cert);

    static constexpr const char * CONFIG_MTLS_ENABLED  = "TLS_MUTUAL_AUTH";
    static constexpr const char * CONFIG_CA_CERT_PATH  = "TLS_CA_CERT_PATH";
};

#endif // ALPINE_TLS_ENABLED
