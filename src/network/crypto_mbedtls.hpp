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

#ifndef HEADER_CRYPTO_MBEDTLS_HPP
#define HEADER_CRYPTO_MBEDTLS_HPP

#include "utils/log.hpp"

#include <enet/enet.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/gcm.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <vector>

class BareNetworkString;
class NetworkString;

class Crypto
{
private:
    static std::string m_client_key;

    static std::string m_client_iv;

    std::array<uint8_t, 12> m_iv;

    uint32_t m_packet_counter;

    mbedtls_gcm_context m_aes_encrypt_context, m_aes_decrypt_context;

    std::mutex m_crypto_mutex;

public:
    // ------------------------------------------------------------------------
    static std::string base64(const std::vector<uint8_t>& input);
    // ------------------------------------------------------------------------
    static std::vector<uint8_t> decode64(std::string input);
    // ------------------------------------------------------------------------
    static std::array<uint8_t, 32> sha256(const std::string& input);
    // ------------------------------------------------------------------------
    static std::unique_ptr<Crypto> getClientCrypto()
    {
        assert(!m_client_key.empty());
        assert(!m_client_iv.empty());
        auto c = std::unique_ptr<Crypto>(new Crypto(decode64(m_client_key),
            decode64(m_client_iv)));
        c->m_packet_counter = 0;
        return c;
    }
    // ------------------------------------------------------------------------
    static void initClientAES()
    {
        mbedtls_entropy_context entropy;
        mbedtls_entropy_init(&entropy);
        mbedtls_ctr_drbg_context ctr_drbg;
        mbedtls_ctr_drbg_init(&ctr_drbg);
        std::random_device rd;
        std::mt19937 g(rd());
        std::array<uint8_t, 28> seed, key_iv;
        for (unsigned i = 0; i < 28; i++)
            seed[i] = (uint8_t)(g() % 255);
        key_iv = seed;
        if (mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func,
            &entropy, seed.data(), seed.size()) == 0)
        {
            // If failed use the seed in the beginning
            if (mbedtls_ctr_drbg_random((void*)&ctr_drbg, key_iv.data(),
                key_iv.size()) != 0)
                key_iv = seed;
        }
        m_client_key = base64({ key_iv.begin(), key_iv.begin() + 16 });
        m_client_iv = base64({ key_iv.begin() + 16, key_iv.end() });
    }
    // ------------------------------------------------------------------------
    static void resetClientAES()
    {
        m_client_key = "";
        m_client_iv = "";
    }
    // ------------------------------------------------------------------------
    static const std::string& getClientKey()           { return m_client_key; }
    // ------------------------------------------------------------------------
    static const std::string& getClientIV()             { return m_client_iv; }
    // ------------------------------------------------------------------------
    Crypto(const std::vector<uint8_t>& key,
           const std::vector<uint8_t>& iv)
    {
        assert(key.size() == 16);
        assert(iv.size() == 12);
        std::copy_n(iv.begin(), 12, m_iv.begin());
        m_packet_counter = 0;
        mbedtls_gcm_init(&m_aes_encrypt_context);
        mbedtls_gcm_init(&m_aes_decrypt_context);
        mbedtls_gcm_setkey(&m_aes_encrypt_context, MBEDTLS_CIPHER_ID_AES,
            key.data(), key.size() * 8);
        mbedtls_gcm_setkey(&m_aes_decrypt_context, MBEDTLS_CIPHER_ID_AES,
            key.data(), key.size() * 8);
    }
    // ------------------------------------------------------------------------
    ~Crypto()
    {
        mbedtls_gcm_free(&m_aes_encrypt_context);
        mbedtls_gcm_free(&m_aes_decrypt_context);
    }
    // ------------------------------------------------------------------------
    bool encryptConnectionRequest(BareNetworkString& ns);
    // ------------------------------------------------------------------------
    bool decryptConnectionRequest(BareNetworkString& ns);
    // ------------------------------------------------------------------------
    ENetPacket* encryptSend(BareNetworkString& ns, bool reliable);
    // ------------------------------------------------------------------------
    NetworkString* decryptRecieve(ENetPacket* p);

};

#endif // HEADER_CRYPTO_MBEDTLS_HPP

#endif
