#ifdef __SWITCH__

#ifndef HEADER_CRYPTO_LIBNX_HPP
#define HEADER_CRYPTO_LIBNX_HPP

#include <enet/enet.h>
extern "C" {
    #define u64 uint64_t
    #define s64 int64_t
    #include <switch/crypto/aes.h>
    #include <switch/crypto/sha256.h>
    #include <switch/kernel/random.h>
    #undef u64
    #undef s64
}

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

    Aes128Context m_aes_context;

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
        uint8_t* randData;
        // TODO: libnx linking
        // randomGet(randData, 28);
        m_client_key = base64({ randData, &randData[16] });
        m_client_iv = base64({ &randData[16], &randData[28] });
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

        // aes128CtrContextCreate(&m_aes_encrypt_context, key.data(), true);
        // aes128CtrContextCreate(&m_aes_decrypt_context, key.data(), false);

        // gcm_aes128_set_key(&m_aes_encrypt_context, key.data());
        // gcm_aes128_set_iv(&m_aes_encrypt_context, 12, iv.data());
        // gcm_aes128_set_key(&m_aes_decrypt_context, key.data());
        // gcm_aes128_set_iv(&m_aes_decrypt_context, 12, iv.data());
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

#endif

#endif
