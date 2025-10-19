#ifdef APPLE_NETWORK_LIBRARIES

#ifndef HEADER_CRYPTO_CRYPTOKIT_HPP
#define HEADER_CRYPTO_CRYPTOKIT_HPP

#include <enet/enet.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <cstdint>
#include <memory>
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

    std::atomic_uint m_packet_counter;

    std::vector<uint8_t> m_key;

public:
    // ------------------------------------------------------------------------
    static std::string base64(const std::vector<uint8_t>& input);
    // ------------------------------------------------------------------------
    static std::vector<uint8_t> decode64(std::string input);
    // ------------------------------------------------------------------------
    static std::array<uint8_t, 32> sha256(const std::string& input);
    // ------------------------------------------------------------------------
    static std::unique_ptr<Crypto> getClientCrypto(size_t tag_size)
    {
        assert(!m_client_key.empty());
        assert(!m_client_iv.empty());
        assert(tag_size == 16);
        auto c = std::unique_ptr<Crypto>(new Crypto(decode64(m_client_key),
            decode64(m_client_iv), tag_size));
        c->m_packet_counter = 0;
        return c;
    }
    // ------------------------------------------------------------------------
    static void initClientAES();
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
           const std::vector<uint8_t>& iv,
           size_t tag_size = 16)
    {
        assert(key.size() == 16);
        assert(iv.size() == 12);
        assert(tag_size == 16);
        std::copy_n(iv.begin(), 12, m_iv.begin());
        m_key = key;
        m_packet_counter = 0;
    }
    // ------------------------------------------------------------------------
    ~Crypto() {}
    // ------------------------------------------------------------------------
    bool encryptConnectionRequest(BareNetworkString& ns);
    // ------------------------------------------------------------------------
    bool decryptConnectionRequest(BareNetworkString& ns);
    // ------------------------------------------------------------------------
    ENetPacket* encryptSend(BareNetworkString& ns, bool reliable);
    // ------------------------------------------------------------------------
    NetworkString* decryptRecieve(ENetPacket* p);
};

#endif // HEADER_CRYPTO_CRYPTOKIT_HPP

#endif
