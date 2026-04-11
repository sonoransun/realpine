/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <Configuration.h>
#include <Log.h>
#include <PacketAuth.h>
#include <Platform.h>
#include <ReadLock.h>
#include <WriteLock.h>
#include <cstring>

#ifdef ALPINE_TLS_ENABLED
#include <openssl/evp.h>
#include <openssl/hmac.h>
#endif


std::unordered_map<ulong, PacketAuth::t_PeerSecret, OptHash<ulong>> PacketAuth::peerSecrets_s;
ReadWriteSem PacketAuth::secretsLock_s;
bool PacketAuth::enabled_s = false;


void
PacketAuth::initialize()
{
    string packetAuthEnabled;

    if (Configuration::getValue("Packet Auth Enabled"s, packetAuthEnabled)) {
        enabled_s = (packetAuthEnabled == "true"s || packetAuthEnabled == "1"s);
    }

    Log::Info("PacketAuth initialized, enabled="s + (enabled_s ? "true"s : "false"s));
}


bool
PacketAuth::isEnabled()
{
    return enabled_s;
}


void
PacketAuth::setEnabled(bool enabled)
{
    enabled_s = enabled;
}


bool
PacketAuth::setPeerSecret(ulong peerId, const byte * secret, uint secretLen)
{
    if (!secret || secretLen == 0) {
        return false;
    }

    WriteLock lock(secretsLock_s);

    t_PeerSecret entry;
    entry.secret.assign(string(reinterpret_cast<const char *>(secret), secretLen));

    // Erase any existing entry first (SecureString is non-copyable, move into map)
    peerSecrets_s.erase(peerId);
    peerSecrets_s.emplace(peerId, std::move(entry));

    return true;
}


bool
PacketAuth::removePeerSecret(ulong peerId)
{
    WriteLock lock(secretsLock_s);

    auto it = peerSecrets_s.find(peerId);
    if (it == peerSecrets_s.end()) {
        return false;
    }

    peerSecrets_s.erase(it);
    return true;
}


bool
PacketAuth::hasPeerSecret(ulong peerId)
{
    ReadLock lock(secretsLock_s);

    return peerSecrets_s.contains(peerId);
}


bool
PacketAuth::generateSecret(byte * secretOut, uint secretBufSize)
{
    if (!secretOut || secretBufSize == 0) {
        return false;
    }

    return alpine_random_bytes(secretOut, secretBufSize);
}


#ifdef ALPINE_TLS_ENABLED

bool
PacketAuth::computeHmac(ulong peerId, const byte * data, uint dataLen, byte * hmacOut, uint hmacBufSize)
{
    if (hmacBufSize < HMAC_SIZE) {
        return false;
    }

    ReadLock lock(secretsLock_s);

    auto it = peerSecrets_s.find(peerId);
    if (it == peerSecrets_s.end()) {
        return false;
    }

    const auto & secretValue = it->second.secret.value();
    unsigned int resultLen = 0;

    auto * result = HMAC(
        EVP_sha256(), secretValue.data(), static_cast<int>(secretValue.size()), data, dataLen, hmacOut, &resultLen);

    return result && resultLen == HMAC_SIZE;
}


bool
PacketAuth::verifyHmac(ulong peerId, const byte * data, uint dataLen, const byte * hmac, uint hmacLen)
{
    if (hmacLen != HMAC_SIZE) {
        return false;
    }

    byte computed[HMAC_SIZE];

    if (!computeHmac(peerId, data, dataLen, computed, HMAC_SIZE)) {
        return false;
    }

    // Constant-time comparison to prevent timing attacks
    volatile unsigned char accumulator = 0;
    for (uint i = 0; i < HMAC_SIZE; ++i) {
        accumulator |= (computed[i] ^ hmac[i]);
    }

    return accumulator == 0;
}

#else

bool
PacketAuth::computeHmac(
    ulong /* peerId */, const byte * /* data */, uint /* dataLen */, byte * /* hmacOut */, uint /* hmacBufSize */)
{
    return false;
}


bool
PacketAuth::verifyHmac(
    ulong /* peerId */, const byte * /* data */, uint /* dataLen */, const byte * /* hmac */, uint /* hmacLen */)
{
    return false;
}

#endif
