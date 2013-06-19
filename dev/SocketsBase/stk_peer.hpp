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
        
        bool connectToServer(STKHost* host, uint32_t ip, uint16_t port, uint32_t channelCount, uint32_t data);
        
        bool isConnected();
    protected:
        ENetPeer* m_peer;
};

#endif // STK_PEER_HPP
