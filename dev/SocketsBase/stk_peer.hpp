#ifndef STK_PEER_HPP
#define STK_PEER_HPP

#include "stk_host.hpp"
#include <enet/enet.h>

class STKPeer
{
    public:
        STKPeer();
        virtual ~STKPeer();
        
        
        static void* receive_data(void* self);
        
        virtual void sendPacket(char* data);
        
        bool connectToHost(STKHost* host, uint32_t ip, uint16_t port, uint32_t channelCount, uint32_t data);
        
        bool isConnected();
        
        uint32_t getAddress();
        uint16_t getPort();
        bool operator==(ENetPeer* peer);
    protected:
        ENetPeer* m_peer;
};

#endif // STK_PEER_HPP
