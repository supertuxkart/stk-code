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

#ifdef ENABLE_CRYPTO_OPENSSL

#ifndef HEADER_CRYPTO_OPENSSL_HPP
#define HEADER_CRYPTO_OPENSSL_HPP

#include "utils/log.hpp"

#include <enet/enet.h>

#define OPENSSL_API_COMPAT 0x09800000L
#include <openssl/evp.h>
#include <openssl/rand.h>

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

    EVP_CIPHER_CTX* m_encrypt;

    EVP_CIPHER_CTX* m_decrypt;

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
        std::random_device rd;
        std::mt19937 g(rd());

        // Default key and if RAND_bytes failed
        std::vector<uint8_t> key;
        for (int i = 0; i < 16; i++)
            key.push_back((uint8_t)(g() % 255));
        std::vector<uint8_t> iv;
        for (int i = 0; i < 12; i++)
            iv.push_back((uint8_t)(g() % 255));
        if (!RAND_bytes(key.data(), 16))
        {
            Log::warn("Crypto",
                "Failed to generate cryptographically strong key");
        }
        m_client_key = base64(key);
        m_client_iv = base64(iv);
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
        m_encrypt = EVP_CIPHER_CTX_new();
        EVP_CIPHER_CTX_init(m_encrypt);
        EVP_EncryptInit_ex(m_encrypt, EVP_aes_128_gcm(), NULL, key.data(),
            iv.data());
        m_decrypt = EVP_CIPHER_CTX_new();
        EVP_CIPHER_CTX_init(m_decrypt);
        EVP_DecryptInit_ex(m_decrypt, EVP_aes_128_gcm(), NULL, key.data(),
            iv.data());
    }
    // ------------------------------------------------------------------------
    ~Crypto()
    {
        EVP_CIPHER_CTX_free(m_encrypt);
        EVP_CIPHER_CTX_free(m_decrypt);
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

#endif // HEADER_CRYPTO_OPENSSL_HPP

#endif
