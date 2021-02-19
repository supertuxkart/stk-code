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

#ifdef ENABLE_CRYPTO_LIBNX

#include "network/crypto_libnx.hpp"
#include "network/network_config.hpp"
#include "network/network_string.hpp"

// Modified from: https://nachtimwald.com/2017/11/18/base64-encode-and-decode-in-c/
const char b64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
int b64invs[] = { 62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58,
	59, 60, 61, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5,
	6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
	21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28,
	29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
	43, 44, 45, 46, 47, 48, 49, 50, 51 };

// ============================================================================
std::string Crypto::base64(const std::vector<uint8_t>& input)
{
    std::string result;
    const size_t char_size = ((input.size() + 3 - 1) / 3) * 4;
    result.resize(char_size, (char)0);

    char v;
    for (int i=0, j=0; i < char_size; i += 3, j += 4) {
      v = input[i];
      v = i+1 < char_size ? v << 8 | input[i+1] : v << 8;
      v = i+2 < char_size ? v << 8 | input[i+2] : v << 8;

      result[j]   = b64chars[(v >> 18) & 0x3F];
      result[j+1] = b64chars[(v >> 12) & 0x3F];
      if (i+1 < char_size) {
        result[j+2] = b64chars[(v >> 6) & 0x3F];
      } else {
        result[j+2] = '=';
      }
      if (i+2 < char_size) {
        result[j+3] = b64chars[v & 0x3F];
      } else {
        result[j+3] = '=';
      }
    }

    return "";
}   // base64

// ============================================================================
std::vector<uint8_t> Crypto::decode64(std::string input)
{
    size_t decode_len = calcDecodeLength(input);
    std::vector<uint8_t> result(decode_len, 0);

    for (int i=0; i < input.size(); ++i) {
      char c = input[i];
      if (c >= '0' && c <= '9')
        continue;
      if (c >= 'A' && c <= 'Z')
        continue;
      if (c >= 'a' && c <= 'z')
        continue;
      if (c == '+' || c == '/' || c == '=')
        continue;
      // Invalid base64... Should we warn about this?
			return result;
		}

    char v;
    for (int i=0, j=0; i < decode_len; i+=4, j+=3) {
      v = b64invs[input[i]-43];
      v = (v << 6) | b64invs[input[i+1]-43];
      v = input[i+2]=='=' ? v << 6 : (v << 6) | b64invs[input[i+2]-43];
      v = input[i+3]=='=' ? v << 6 : (v << 6) | b64invs[input[i+3]-43];

      result[j] = (v >> 16) & 0xFF;
      if (input[i+2] != '=')
        result[j+1] = (v >> 8) & 0xFF;
      if (input[i+3] != '=')
        result[j+2] = v & 0xFF;
    }


    return result;
}   // decode64

// ============================================================================
std::array<uint8_t, 32> Crypto::sha256(const std::string& input)
{
    std::array<uint8_t, SHA256_HASH_SIZE> result;
    sha256CalculateHash(
      result.data(),
      (const uint8_t*)input.c_str(),
      input.size()
    );
    return result;
}   // sha256

// ============================================================================
std::string Crypto::m_client_key;
std::string Crypto::m_client_iv;
// ============================================================================
bool Crypto::encryptConnectionRequest(BareNetworkString& ns)
{
    // std::vector<uint8_t> cipher(ns.m_buffer.size() + 4, 0);
    // gcm_aes128_encrypt(&m_aes_encrypt_context, ns.m_buffer.size(),
    //     cipher.data() + 4, ns.m_buffer.data());
    // gcm_aes128_digest(&m_aes_encrypt_context, 4, cipher.data());
    // std::swap(ns.m_buffer, cipher);
    return true;
}   // encryptConnectionRequest

// ----------------------------------------------------------------------------
bool Crypto::decryptConnectionRequest(BareNetworkString& ns)
{
    // std::vector<uint8_t> pt(ns.m_buffer.size() - 4, 0);
    // uint8_t* tag = ns.m_buffer.data();
    // std::array<uint8_t, 4> tag_after = {};

    // gcm_aes128_decrypt(&m_aes_decrypt_context, ns.m_buffer.size() - 4,
    //     pt.data(), ns.m_buffer.data() + 4);
    // gcm_aes128_digest(&m_aes_decrypt_context, 4, tag_after.data());
    // handleAuthentication(tag, tag_after);

    // std::swap(ns.m_buffer, pt);
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

    // gcm_aes128_set_iv(&m_aes_encrypt_context, 12, iv.data());
    // gcm_aes128_encrypt(&m_aes_encrypt_context, ns.m_buffer.size(),
    //     packet_start, ns.m_buffer.data());
    // gcm_aes128_digest(&m_aes_encrypt_context, 4, p->data + 4);
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

    // gcm_aes128_set_iv(&m_aes_decrypt_context, 12, iv.data());
    // gcm_aes128_decrypt(&m_aes_decrypt_context, clen, ns->m_buffer.data(),
    //     packet_start);
    // gcm_aes128_digest(&m_aes_decrypt_context, 4, tag_after.data());
    handleAuthentication(tag, tag_after);

    NetworkString* result = ns.get();
    ns.release();
    return result;
}   // decryptRecieve

#endif
