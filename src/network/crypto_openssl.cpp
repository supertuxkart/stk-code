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

#include "network/crypto_openssl.hpp"
#include "network/network_config.hpp"
#include "network/network_string.hpp"

#include <openssl/aes.h>
#include <openssl/buffer.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>

// ============================================================================
// AES GCM modes never writes anything when finalize, it only handles the tag
// authentication, so we write to this dummy block instead of out of bounds
// pointer, and AES GCM mode has the same ciphertext and plaintext size due to
// block cipher in stream cipher mode.
std::array<uint8_t, 16> unused_16_blocks;
// ============================================================================
std::string Crypto::base64(const std::vector<uint8_t>& input)
{
    BIO *bmem, *b64;
    BUF_MEM* bptr;
    std::string result;

    b64 = BIO_new(BIO_f_base64());
    bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);

    BIO_set_flags(bmem, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(b64, input.data(), (int)input.size());
    BIO_flush(b64);
    BIO_get_mem_ptr(b64, &bptr);
    result.resize(bptr->length - 1);
    memcpy(&result[0], bptr->data, bptr->length - 1);
    BIO_free_all(b64);

    return result;
}   // base64

// ============================================================================
std::vector<uint8_t> Crypto::decode64(std::string input)
{
    BIO *b64, *bmem;
    size_t decode_len = calcDecodeLength(input);
    std::vector<uint8_t> result(decode_len, 0);
    b64 = BIO_new(BIO_f_base64());

    bmem = BIO_new_mem_buf(&input[0], (int)input.size());
    bmem = BIO_push(b64, bmem);

    BIO_set_flags(bmem, BIO_FLAGS_BASE64_NO_NL);
#ifdef DEBUG
    size_t read_l = BIO_read(bmem, result.data(), (int)input.size());
    assert(read_l == decode_len);
#else
    BIO_read(bmem, result.data(), (int)input.size());
#endif
    BIO_free_all(bmem);

    return result;
}   // decode64

// ============================================================================
std::array<uint8_t, 32> Crypto::sha256(const std::string& input)
{
    std::array<uint8_t, SHA256_DIGEST_LENGTH> result;
    SHA256_CTX sha256_ctx;
    SHA256_Init(&sha256_ctx);
    SHA256_Update(&sha256_ctx, (const unsigned char*)input.c_str(),
        input.size());
    SHA256_Final(result.data(), &sha256_ctx);
    return result;
}   // sha256

// ============================================================================
std::string Crypto::m_client_key;
std::string Crypto::m_client_iv;
// ============================================================================
bool Crypto::encryptConnectionRequest(BareNetworkString& ns)
{
    std::vector<uint8_t> cipher(ns.m_buffer.size() + 4, 0);

    int elen;
    if (EVP_EncryptInit_ex(m_encrypt, NULL, NULL, NULL, NULL) != 1)
        return false;
    if (EVP_EncryptUpdate(m_encrypt, cipher.data() + 4, &elen,
        ns.m_buffer.data(), (int)ns.m_buffer.size()) != 1)
        return false;
    if (EVP_EncryptFinal_ex(m_encrypt, unused_16_blocks.data(), &elen) != 1)
        return false;
    if (EVP_CIPHER_CTX_ctrl(m_encrypt, EVP_CTRL_GCM_GET_TAG, 4, cipher.data())
        != 1)
        return false;

    std::swap(ns.m_buffer, cipher);
    return true;
}   // encryptConnectionRequest

// ----------------------------------------------------------------------------
bool Crypto::decryptConnectionRequest(BareNetworkString& ns)
{
    std::vector<uint8_t> pt(ns.m_buffer.size() - 4, 0);

    if (EVP_DecryptInit_ex(m_decrypt, NULL, NULL, NULL, NULL) != 1)
        return false;

    int dlen;
    if (!EVP_CIPHER_CTX_ctrl(m_decrypt, EVP_CTRL_GCM_SET_TAG, 4,
        ns.m_buffer.data()))
        return false;

    if (EVP_DecryptUpdate(m_decrypt, pt.data(), &dlen, ns.m_buffer.data() + 4,
        (int)(ns.m_buffer.size() - 4)) != 1)
        return false;

    if (EVP_DecryptFinal_ex(m_decrypt, unused_16_blocks.data(), &dlen) > 0)
    {
        assert(dlen == 0);
        std::swap(ns.m_buffer, pt);
        return true;
    }

    return false;
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

    if (EVP_EncryptInit_ex(m_encrypt, NULL, NULL, NULL, iv.data()) != 1)
    {
        enet_packet_destroy(p);
        return NULL;
    }

    int elen;
    if (EVP_EncryptUpdate(m_encrypt, packet_start, &elen, ns.m_buffer.data(),
        (int)ns.m_buffer.size()) != 1)
    {
        enet_packet_destroy(p);
        return NULL;
    }
    if (EVP_EncryptFinal_ex(m_encrypt, unused_16_blocks.data(), &elen) != 1)
    {
        enet_packet_destroy(p);
        return NULL;
    }
    if (EVP_CIPHER_CTX_ctrl(m_encrypt, EVP_CTRL_GCM_GET_TAG, 4, p->data + 4)
        != 1)
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
    if (EVP_DecryptInit_ex(m_decrypt, NULL, NULL, NULL, iv.data()) != 1)
    {
        throw std::runtime_error("Failed to set IV.");
    }
    if (!EVP_CIPHER_CTX_ctrl(m_decrypt, EVP_CTRL_GCM_SET_TAG, 4, tag))
    {
        throw std::runtime_error("Failed to set tag.");
    }

    int dlen;
    if (EVP_DecryptUpdate(m_decrypt, ns->m_buffer.data(), &dlen,
        packet_start, clen) != 1)
    {
        throw std::runtime_error("Failed to decrypt.");
    }
    if (EVP_DecryptFinal_ex(m_decrypt, unused_16_blocks.data(), &dlen) > 0)
    {
        assert(dlen == 0);
        NetworkString* result = ns.get();
        ns.release();
        return result;
    }
    throw std::runtime_error("Failed to finalize decryption.");
}   // decryptRecieve

#endif
