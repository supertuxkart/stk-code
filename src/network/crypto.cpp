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

#include "network/crypto.hpp"
#include "network/network_config.hpp"
#include "network/network_string.hpp"

#include <openssl/aes.h>

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
        ns.m_buffer.data(), ns.m_buffer.size()) != 1)
        return false;
    if (EVP_EncryptFinal_ex(m_encrypt, cipher.data() + 4 + elen, &elen) != 1)
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
    if (EVP_DecryptUpdate(m_decrypt, pt.data(), &dlen, ns.m_buffer.data() + 4,
        ns.m_buffer.size() - 4) != 1)
        return false;
    if (!EVP_CIPHER_CTX_ctrl(m_decrypt, EVP_CTRL_GCM_SET_TAG, 4,
        ns.m_buffer.data()))
        return false;

    if (!(EVP_DecryptFinal_ex(m_decrypt, pt.data() + dlen, &dlen) > 0))
    {
        assert(dlen == 0);
        return false;
    }

    std::swap(ns.m_buffer, pt);
    return true;
}   // decryptConnectionRequest

// ----------------------------------------------------------------------------
ENetPacket* Crypto::encryptSend(BareNetworkString& ns, bool reliable)
{
    // 4 bytes counter and 4 bytes tag
    ENetPacket* p = enet_packet_create(NULL, ns.m_buffer.size() + 8,
        (reliable ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNSEQUENCED));
    if (p == NULL)
        return NULL;

    std::array<uint8_t, 12> iv = m_iv;
    std::unique_lock<std::mutex> ul(m_crypto_mutex);
    uint32_t val = NetworkConfig::get()->isClient() ?
        m_packet_counter++ : m_packet_counter--;
    memcpy(iv.data(), &val, 4);
    uint8_t* packet_start = p->data + 8;

    if (EVP_EncryptInit_ex(m_encrypt, NULL, NULL, NULL, iv.data()) != 1)
    {
        enet_packet_destroy(p);
        return NULL;
    }

    int elen;
    if (EVP_EncryptUpdate(m_encrypt, packet_start, &elen, ns.m_buffer.data(),
        ns.m_buffer.size()) != 1)
    {
        enet_packet_destroy(p);
        return NULL;
    }
    if (EVP_EncryptFinal_ex(m_encrypt, packet_start, &elen) != 1)
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

    memcpy(p->data, iv.data(), 4);
    return p;
}   // encryptSend

// ----------------------------------------------------------------------------
NetworkString* Crypto::decryptRecieve(ENetPacket* p)
{
    int clen = (int)(p->dataLength - 8);
    auto ns = std::unique_ptr<NetworkString>(new NetworkString(p->data, clen));

    std::array<uint8_t, 12> iv = m_iv;
    memcpy(iv.data(), p->data, 4);
    uint8_t* packet_start = p->data + 8;
    uint8_t* tag = p->data + 4;
    if (EVP_DecryptInit_ex(m_decrypt, NULL, NULL, NULL, iv.data()) != 1)
    {
        throw std::runtime_error("Failed to set IV.");
    }

    int dlen;
    if (EVP_DecryptUpdate(m_decrypt, ns->m_buffer.data(), &dlen,
        packet_start, clen) != 1)
    {
        throw std::runtime_error("Failed to decrypt.");
    }
    if (!EVP_CIPHER_CTX_ctrl(m_decrypt, EVP_CTRL_GCM_SET_TAG, 4, tag))
    {
        throw std::runtime_error("Failed to set tag.");
    }
    if (EVP_DecryptFinal_ex(m_decrypt, ns->m_buffer.data(), &dlen) > 0)
    {
        assert(dlen == 0);
        NetworkString* result = ns.get();
        ns.release();
        return result;
    }
    throw std::runtime_error("Failed to finalize decryption.");
}   // decryptRecieve
