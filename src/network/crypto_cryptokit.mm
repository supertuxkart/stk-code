#ifdef APPLE_NETWORK_LIBRARIES

#include "network/crypto_cryptokit.hpp"
#include "network/network_config.hpp"
#include "network/network_string.hpp"
#include "utils/log.hpp"

#include <cassert>
#include <cstring>
#include <stdexcept>
#import <cryptokit_wrapper-Swift.h>

// ============================================================================
std::string Crypto::base64(const std::vector<uint8_t>& input)
{
    if (input.empty()) return "";

    NSData *s = [NSData dataWithBytes:input.data() length:input.size()];
    if (!s) return "";

    NSString *b = [CryptoKitWrapper base64Encode:s];
    if (!b) return "";

    const char *utf8 = [b UTF8String];
    if (!utf8) return "";
    return utf8;
}   // base64

// ============================================================================
std::vector<uint8_t> Crypto::decode64(std::string input)
{
    std::vector<uint8_t> out;

    NSString *s = [NSString stringWithUTF8String:input.c_str()];
    if (!s) return out;

    NSData *d = [CryptoKitWrapper base64Decode:s];
    if (!d) return out;

    NSUInteger len = [d length];
    if (len == 0) return out;

    const uint8_t *bytes = static_cast<const uint8_t *>([d bytes]);
    if (!bytes) return out;

    out.resize(len);
    memcpy(out.data(), bytes, len);
    return out;
}   // decode64

// ============================================================================
std::array<uint8_t, 32> Crypto::sha256(const std::string& input)
{
    std::array<uint8_t, 32> out = {};

    NSData* d = [NSData dataWithBytes:input.c_str() length:input.size()];
    if (!d) return out;

    NSData* hash = [CryptoKitWrapper sha256Hash:d];
    if (!hash) return out;

    assert([hash length] == out.size());
    const uint8_t *bytes = static_cast<const uint8_t *>([hash bytes]);
    if (!bytes) return out;

    memcpy(out.data(), bytes, out.size());
    return out;
}   // sha256

// ============================================================================
std::string Crypto::m_client_key;
std::string Crypto::m_client_iv;

// ============================================================================
void Crypto::initClientAES()
{
    NSData* key = [CryptoKitWrapper generateSymmetricKey];
    NSData* iv = [CryptoKitWrapper generateRandomIV];
    if (key && iv)
    {
        m_client_key = [CryptoKitWrapper base64Encode:key].UTF8String;
        m_client_iv = [CryptoKitWrapper base64Encode:iv].UTF8String;
    }
}   // initClientAES

// ============================================================================
bool Crypto::encryptConnectionRequest(BareNetworkString& ns)
{
    NSData* plaintext = [NSData dataWithBytes:ns.m_buffer.data() length:ns.m_buffer.size()];
    NSData* key = [NSData dataWithBytes:m_key.data() length:m_key.size()];
    NSData* iv = [NSData dataWithBytes:m_iv.data() length:m_iv.size()];
    NSData* encrypted = [CryptoKitWrapper aesGcmEncryptWithPlaintext:plaintext key:key iv:iv];
    if (!encrypted)
        return false;

    // Replace buffer with encrypted data (tag + ciphertext)
    std::vector<uint8_t> cipher(encrypted.length);
    memcpy(cipher.data(), encrypted.bytes, encrypted.length);
    std::swap(ns.m_buffer, cipher);
    return true;
}   // encryptConnectionRequest

// ----------------------------------------------------------------------------
bool Crypto::decryptConnectionRequest(BareNetworkString& ns)
{
    NSData* ciphertext = [NSData dataWithBytes:ns.m_buffer.data() length:ns.m_buffer.size()];
    NSData* key = [NSData dataWithBytes:m_key.data() length:m_key.size()];
    NSData* iv = [NSData dataWithBytes:m_iv.data() length:m_iv.size()];

    NSData* decrypted = [CryptoKitWrapper aesGcmDecryptWithCiphertext:ciphertext key:key iv:iv];
    if (!decrypted)
        throw std::runtime_error("Failed authentication.");

    std::vector<uint8_t> pt(decrypted.length);
    memcpy(pt.data(), decrypted.bytes, decrypted.length);
    std::swap(ns.m_buffer, pt);
    return true;
}   // decryptConnectionRequest

// ----------------------------------------------------------------------------
ENetPacket* Crypto::encryptSend(BareNetworkString& ns, bool reliable)
{
    // 4 bytes counter and 16 bytes tag
    ENetPacket* p = enet_packet_create(NULL, ns.m_buffer.size() + 4 + 16,
        (reliable ? ENET_PACKET_FLAG_RELIABLE :
        (ENET_PACKET_FLAG_UNSEQUENCED | ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT))
        );
    if (p == NULL)
        return NULL;

    std::array<uint8_t, 12> iv = {};
    uint32_t val = ++m_packet_counter;
    if (NetworkConfig::get()->isClient())
    {
        iv[0] = (val >> 24) & 0xff;
        iv[1] = (val >> 16) & 0xff;
        iv[2] = (val >> 8) & 0xff;
        iv[3] = val & 0xff;
    }
    else
    {
        iv[4] = (val >> 24) & 0xff;
        iv[5] = (val >> 16) & 0xff;
        iv[6] = (val >> 8) & 0xff;
        iv[7] = val & 0xff;
    }

    NSData* plaintext = [NSData dataWithBytes:ns.m_buffer.data() length:ns.m_buffer.size()];
    NSData* key = [NSData dataWithBytes:m_key.data() length:m_key.size()];
    NSData* ivData = [NSData dataWithBytes:iv.data() length:iv.size()];

    NSData* encrypted = [CryptoKitWrapper aesGcmEncryptWithPlaintext:plaintext key:key iv:ivData];
    if (!encrypted)
    {
        enet_packet_destroy(p);
        return NULL;
    }

    // Copy encrypted data: [counter][16-byte tag][ciphertext]
    p->data[0] = (val >> 24) & 0xff;
    p->data[1] = (val >> 16) & 0xff;
    p->data[2] = (val >> 8) & 0xff;
    p->data[3] = val & 0xff;
    memcpy(p->data + 4, encrypted.bytes, encrypted.length);
    return p;
}   // encryptSend

// ----------------------------------------------------------------------------
NetworkString* Crypto::decryptRecieve(ENetPacket* p)
{

    std::array<uint8_t, 12> iv = {};
    if (NetworkConfig::get()->isClient())
        memcpy(iv.data() + 4, p->data, 4);
    else
        memcpy(iv.data(), p->data, 4);
    NSData* ciphertext = [NSData dataWithBytes:p->data + 4 length:p->dataLength - 4];
    NSData* key = [NSData dataWithBytes:m_key.data() length:m_key.size()];
    NSData* ivData = [NSData dataWithBytes:iv.data() length:iv.size()];

    NSData* decrypted = [CryptoKitWrapper aesGcmDecryptWithCiphertext:ciphertext key:key iv:ivData];
    if (!decrypted)
        throw std::runtime_error("Failed authentication.");
    int clen = (int)(p->dataLength - 4 - 16);
    if (clen != decrypted.length)
        throw std::runtime_error("aesGcmDecryptWithCiphertext produced different size.");

    NetworkString* ns = new NetworkString(p->data, clen);
    memcpy(ns->m_buffer.data(), decrypted.bytes, decrypted.length);
    return ns;
}   // decryptRecieve

#endif
