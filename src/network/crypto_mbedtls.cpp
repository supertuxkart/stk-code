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

#ifdef ENABLE_CRYPTO_MBEDTLS

#include "network/crypto_mbedtls.hpp"
#include "network/network_config.hpp"
#include "network/network_string.hpp"

#include <mbedtls/base64.h>
#include <mbedtls/sha256.h>
#include <mbedtls/version.h>
#include <cstring>

// ============================================================================
std::string Crypto::base64(const std::vector<uint8_t>& input)
{
    size_t required_size = 0;
    mbedtls_base64_encode(NULL, 0, &required_size, &input[0], input.size());
    std::string result;
    result.resize(required_size, (char)0);
    mbedtls_base64_encode((unsigned char*)&result[0], required_size,
        &required_size, &input[0], input.size());
    // mbedtls_base64_encode includes the null terminator for required size
    result.resize(strlen(result.c_str()));
    return result;
}   // base64

// ============================================================================
std::vector<uint8_t> Crypto::decode64(std::string input)
{
    size_t required_size = 0;
    mbedtls_base64_decode(NULL, 0, &required_size, (unsigned char*)&input[0],
        input.size());
    std::vector<uint8_t> result(required_size, 0);
    mbedtls_base64_decode(result.data(), required_size,
        &required_size, (unsigned char*)&input[0], input.size());
    return result;
}   // decode64

// ============================================================================
std::array<uint8_t, 32> Crypto::sha256(const std::string& input)
{
    std::array<uint8_t, 32> result;
#if MBEDTLS_VERSION_MAJOR >= 3
    mbedtls_sha256((unsigned char*)&input[0], input.size(),
        result.data(), 0/*not 224*/);
#else
    mbedtls_sha256_ret((unsigned char*)&input[0], input.size(),
        result.data(), 0/*not 224*/);
#endif
    return result;
}   // sha256

// ============================================================================
std::string Crypto::m_client_key;
std::string Crypto::m_client_iv;
// ============================================================================
bool Crypto::encryptConnectionRequest(BareNetworkString& ns)
{
    std::vector<uint8_t> cipher(ns.m_buffer.size() + 4, 0);
    if (mbedtls_gcm_crypt_and_tag(&m_aes_encrypt_context, MBEDTLS_GCM_ENCRYPT,
        ns.m_buffer.size(), m_iv.data(), m_iv.size(), NULL, 0,
        ns.m_buffer.data(), cipher.data() + 4, 4, cipher.data()) != 0)
    {
        return false;
    }
    std::swap(ns.m_buffer, cipher);
    return true;
}   // encryptConnectionRequest

// ----------------------------------------------------------------------------
bool Crypto::decryptConnectionRequest(BareNetworkString& ns)
{
    std::vector<uint8_t> pt(ns.m_buffer.size() - 4, 0);
    uint8_t* tag = ns.m_buffer.data();
    if (mbedtls_gcm_auth_decrypt(&m_aes_decrypt_context, pt.size(),
        m_iv.data(), m_iv.size(), NULL, 0, tag, 4, ns.m_buffer.data() + 4,
        pt.data()) != 0)
    {
        throw std::runtime_error("Failed authentication.");
    }
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
    if (mbedtls_gcm_crypt_and_tag(&m_aes_encrypt_context, MBEDTLS_GCM_ENCRYPT,
        ns.m_buffer.size(), iv.data(), iv.size(), NULL, 0, ns.m_buffer.data(),
        packet_start, 4, p->data + 4) != 0)
    {
        enet_packet_destroy(p);
        return NULL;
    }
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
    if (mbedtls_gcm_auth_decrypt(&m_aes_decrypt_context, clen, iv.data(),
        iv.size(), NULL, 0, tag, 4, packet_start, ns->m_buffer.data()) != 0)
    {
        throw std::runtime_error("Failed authentication.");
    }

    NetworkString* result = ns.get();
    ns.release();
    return result;
}   // decryptRecieve

#endif
