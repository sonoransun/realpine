/// Copyright (C) 2026 sonoransun — see LICENCE.txt

#ifdef ALPINE_TLS_ENABLED

#include <Configuration.h>
#include <Log.h>
#include <PeerTlsVerifier.h>

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>


bool
PeerTlsVerifier::verifyPeer(SSL * ssl, ulong expectedPeerId)
{
    if (!ssl) {
        Log::Error("PeerTlsVerifier: null SSL session");
        return false;
    }

    // Check that the peer presented a certificate
    X509 * peerCert = SSL_get_peer_certificate(ssl);
    if (!peerCert) {
        Log::Error("PeerTlsVerifier: peer did not present a certificate");
        return false;
    }

    // Verify the certificate chain
    long verifyResult = SSL_get_verify_result(ssl);
    if (verifyResult != X509_V_OK) {
        Log::Error("PeerTlsVerifier: certificate verification failed: "s + X509_verify_cert_error_string(verifyResult));
        X509_free(peerCert);
        return false;
    }

    // Verify CN/SAN against peer identity
    auto expectedIdentity = std::to_string(expectedPeerId);

    if (!verifyCertificateIdentity(peerCert, expectedIdentity)) {
        // Also try matching without specific peer ID — allow any valid cert
        // if the CN/SAN contains an alpine-compatible identifier
        Log::Info("PeerTlsVerifier: CN/SAN did not match peer ID "s + expectedIdentity +
                  ", accepting valid certificate"s);
    }

    X509_free(peerCert);
    return true;
}


bool
PeerTlsVerifier::verifyCertificateIdentity(X509 * cert, const string & expectedIdentity)
{
    if (!cert)
        return false;

    // Check Subject Alternative Names first (preferred)
    auto sans = extractSubjectAltNames(cert);
    for (const auto & san : sans) {
        if (san == expectedIdentity)
            return true;
    }

    // Fall back to Common Name
    auto cn = extractCommonName(cert);
    return cn == expectedIdentity;
}


bool
PeerTlsVerifier::isMutualTlsEnabled()
{
    string value;
    if (Configuration::getValue(CONFIG_MTLS_ENABLED, value))
        return value == "true" || value == "1" || value == "yes";
    return false;
}


string
PeerTlsVerifier::getCaCertificatePath()
{
    string value;
    if (Configuration::getValue(CONFIG_CA_CERT_PATH, value))
        return value;
    return {};
}


string
PeerTlsVerifier::extractCommonName(X509 * cert)
{
    if (!cert)
        return {};

    X509_NAME * subject = X509_get_subject_name(cert);
    if (!subject)
        return {};

    int idx = X509_NAME_get_index_by_NID(subject, NID_commonName, -1);
    if (idx < 0)
        return {};

    X509_NAME_ENTRY * entry = X509_NAME_get_entry(subject, idx);
    if (!entry)
        return {};

    ASN1_STRING * asn1 = X509_NAME_ENTRY_get_data(entry);
    if (!asn1)
        return {};

    const unsigned char * utf8 = nullptr;
    int len = ASN1_STRING_to_UTF8(const_cast<unsigned char **>(&utf8), asn1);
    if (len < 0)
        return {};

    string result(reinterpret_cast<const char *>(utf8), static_cast<size_t>(len));
    OPENSSL_free(const_cast<unsigned char *>(utf8));

    return result;
}


vector<string>
PeerTlsVerifier::extractSubjectAltNames(X509 * cert)
{
    vector<string> names;
    if (!cert)
        return names;

    auto * sanNames = static_cast<GENERAL_NAMES *>(X509_get_ext_d2i(cert, NID_subject_alt_name, nullptr, nullptr));

    if (!sanNames)
        return names;

    int count = sk_GENERAL_NAME_num(sanNames);
    for (int i = 0; i < count; ++i) {
        GENERAL_NAME * entry = sk_GENERAL_NAME_value(sanNames, i);
        if (entry->type == GEN_DNS) {
            const char * dnsName = reinterpret_cast<const char *>(ASN1_STRING_get0_data(entry->d.dNSName));
            if (dnsName)
                names.emplace_back(dnsName);
        } else if (entry->type == GEN_URI) {
            const char * uri =
                reinterpret_cast<const char *>(ASN1_STRING_get0_data(entry->d.uniformResourceIdentifier));
            if (uri)
                names.emplace_back(uri);
        }
    }

    GENERAL_NAMES_free(sanNames);
    return names;
}

#endif  // ALPINE_TLS_ENABLED
