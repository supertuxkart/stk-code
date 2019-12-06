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

#ifndef HEADER_CRYPTO_NETTLE_HPP
#define HEADER_CRYPTO_NETTLE_HPP

#include "utils/log.hpp"

#include <enet/enet.h>
#include <nettle/gcm.h>
#include <nettle/yarrow.h>

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

    struct gcm_aes128_ctx m_aes_encrypt_context, m_aes_decrypt_context;

    std::mutex m_crypto_mutex;

    // ------------------------------------------------------------------------
    static size_t calcDecodeLength(const std::string& input)
    {
        // Calculates the length of a decoded string
        size_t padding = 0;
        const size_t len = input.size();
        if (input[len - 1] == '=' && input[len - 2] == '=')
        {
            // last two chars are =
            padding = 2;
        }
        else if (input[len - 1] == '=')
        {
            // last char is =
            padding = 1;
        }
        return (len * 3) / 4 - padding;
    }   // calcDecodeLength
    // ------------------------------------------------------------------------
    void handleAuthentication(const uint8_t* tag,
                              const std::array<uint8_t, 4>& tag_after)
    {
        for (unsigned i = 0; i < tag_after.size(); i++)
        {
            if (tag[i] != tag_after[i])
                throw std::runtime_error("Failed authentication.");
        }
    }

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
        struct yarrow256_ctx ctx;
        yarrow256_init(&ctx, 0, NULL);
        std::random_device rd;
        std::mt19937 g(rd());
        std::array<uint8_t, YARROW256_SEED_FILE_SIZE> seed;
        for (unsigned i = 0; i < YARROW256_SEED_FILE_SIZE; i++)
            seed[i] = (uint8_t)(g() % 255);
        yarrow256_seed(&ctx, YARROW256_SEED_FILE_SIZE, seed.data());
        std::array<uint8_t, 28> key_iv;
        yarrow256_random(&ctx, key_iv.size(), key_iv.data());
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
        gcm_aes128_set_key(&m_aes_encrypt_context, key.data());
        gcm_aes128_set_iv(&m_aes_encrypt_context, 12, iv.data());
        gcm_aes128_set_key(&m_aes_decrypt_context, key.data());
        gcm_aes128_set_iv(&m_aes_decrypt_context, 12, iv.data());
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

#endif // HEADER_CRYPTO_NETTLE_HPP

#endif
