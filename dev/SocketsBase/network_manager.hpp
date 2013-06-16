#ifndef NETWORKMANAGER_HPP
#define NETWORKMANAGER_HPP

#include "stk_peer.hpp"
#include "stk_host.hpp"
#include <vector>

#include "protocol_listener.hpp"

class NetworkManager 
{
    public:
        NetworkManager();
        virtual ~NetworkManager();
        
        virtual void run() = 0;
        
        static void sendRawPacket(uint8_t* data, int length, unsigned int dstIp, unsigned short dstPort);
        static uint8_t* receiveRawPacket();
        static void receptionCallback(char* data);
        virtual void packetReceived(char* data) = 0;
        
        STKHost* getHost();
    protected:
        std::vector<STKPeer*> m_peers;
        STKHost* m_localhost;
        
        static NetworkManager* instance;
};

#endif // NETWORKMANAGER_HPP
