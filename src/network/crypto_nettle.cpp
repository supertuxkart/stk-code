//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2018 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifdef ENABLE_CRYPTO_NETTLE

#include "network/crypto_nettle.hpp"
#include "network/network_config.hpp"
#include "network/network_string.hpp"

#include <nettle/base64.h>
#include <nettle/sha.h>
#include <nettle/version.h>

#if NETTLE_VERSION_MAJOR > 3 || \
    (NETTLE_VERSION_MAJOR == 3 && NETTLE_VERSION_MINOR > 3)
typedef const char* NETTLE_CONST_CHAR;
typedef char* NETTLE_CHAR;
#else
typedef const uint8_t* NETTLE_CONST_CHAR;
typedef uint8_t* NETTLE_CHAR;
#endif

// ============================================================================
std::string Crypto::base64(const std::vector<uint8_t>& input)
{
    std::string result;
    const size_t char_size = ((input.size() + 3 - 1) / 3) * 4;
    result.resize(char_size, (char)0);
    base64_encode_raw((NETTLE_CHAR)&result[0], input.size(), input.data());
    return result;
}   // base64

// ============================================================================
std::vector<uint8_t> Crypto::decode64(std::string input)
{
    size_t decode_len = calcDecodeLength(input);
    std::vector<uint8_t> result(decode_len, 0);
    struct base64_decode_ctx ctx;
    base64_decode_init(&ctx);
    size_t decode_len_by_nettle;
#ifdef DEBUG
    int ret = base64_decode_update(&ctx, &decode_len_by_nettle, result.data(),
        input.size(), (NETTLE_CONST_CHAR)input.c_str());
    assert(ret == 1);
    ret = base64_decode_final(&ctx);
    assert(ret == 1);
    assert(decode_len_by_nettle == decode_len);
#else
    base64_decode_update(&ctx, &decode_len_by_nettle, result.data(),
        input.size(), (NETTLE_CONST_CHAR)input.c_str());
    base64_decode_final(&ctx);
#endif
    return result;
}   // decode64

// ============================================================================
std::array<uint8_t, 32> Crypto::sha256(const std::string& input)
{
    std::array<uint8_t, SHA256_DIGEST_SIZE> result;
    struct sha256_ctx hash;
    sha256_init(&hash);
    sha256_update(&hash, input.size(), (const uint8_t*)input.c_str());
    sha256_digest(&hash, SHA256_DIGEST_SIZE, result.data());
    return result;
}   // sha256

// ============================================================================
std::string Crypto::m_client_key;
std::string Crypto::m_client_iv;
// ============================================================================
bool Crypto::encryptConnectionRequest(BareNetworkString& ns)
{
    std::vector<uint8_t> cipher(ns.m_buffer.size() + 4, 0);
    gcm_aes128_encrypt(&m_aes_encrypt_context, ns.m_buffer.size(),
        cipher.data() + 4, ns.m_buffer.data());
    gcm_aes128_digest(&m_aes_encrypt_context, 4, cipher.data());
    std::swap(ns.m_buffer, cipher);
    return true;
}   // encryptConnectionRequest

// ----------------------------------------------------------------------------
bool Crypto::decryptConnectionRequest(BareNetworkString& ns)
{
    std::vector<uint8_t> pt(ns.m_buffer.size() - 4, 0);
    uint8_t* tag = ns.m_buffer.data();
    std::array<uint8_t, 4> tag_after = {};

    gcm_aes128_decrypt(&m_aes_decrypt_context, ns.m_buffer.size() - 4,
        pt.data(), ns.m_buffer.data() + 4);
    gcm_aes128_digest(&m_aes_decrypt_context, 4, tag_after.data());
    handleAuthentication(tag, tag_after);

    std::swap(ns.m_buffer, pt);
    return true;
}   // decryptConnectionRequest

// ----------------------------------------------------------------------------
ENetPacket* Crypto::encryptSend(BareNetworkString& ns, bool reliable)
{
    // 4 bytes counter and 4 bytes tag
    ENetPacket* p = enet_packet_create(NULL, ns.m_buffer.size() + 8,
        (reliable ? ENET_PACKET_FLAG_RELIABLE :
        (ENET_PACKET_FLAG_UNSEQUENCED | ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT))
        );
    if (p == NULL)
        return NULL;

    std::array<uint8_t, 12> iv = {};
    std::unique_lock<std::mutex> ul(m_crypto_mutex);

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

    uint8_t* packet_start = p->data + 8;

    gcm_aes128_set_iv(&m_aes_encrypt_context, 12, iv.data());
    gcm_aes128_encrypt(&m_aes_encrypt_context, ns.m_buffer.size(),
        packet_start, ns.m_buffer.data());
    gcm_aes128_digest(&m_aes_encrypt_context, 4, p->data + 4);
    ul.unlock();

    p->data[0] = (val >> 24) & 0xff;
    p->data[1] = (val >> 16) & 0xff;
    p->data[2] = (val >> 8) & 0xff;
    p->data[3] = val & 0xff;
    return p;
}   // encryptSend

// ----------------------------------------------------------------------------
NetworkString* Crypto::decryptRecieve(ENetPacket* p)
{
    int clen = (int)(p->dataLength - 8);
    auto ns = std::unique_ptr<NetworkString>(new NetworkString(p->data, clen));

    std::array<uint8_t, 12> iv = {};
    if (NetworkConfig::get()->isClient())
        memcpy(iv.data() + 4, p->data, 4);
    else
        memcpy(iv.data(), p->data, 4);

    uint8_t* packet_start = p->data + 8;
    uint8_t* tag = p->data + 4;
    std::array<uint8_t, 4> tag_after = {};

    gcm_aes128_set_iv(&m_aes_decrypt_context, 12, iv.data());
    gcm_aes128_decrypt(&m_aes_decrypt_context, clen, ns->m_buffer.data(),
        packet_start);
    gcm_aes128_digest(&m_aes_decrypt_context, 4, tag_after.data());
    handleAuthentication(tag, tag_after);

    NetworkString* result = ns.get();
    ns.release();
    return result;
}   // decryptRecieve

#endif
