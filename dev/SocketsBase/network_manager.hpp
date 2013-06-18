#ifndef NETWORKMANAGER_HPP
#define NETWORKMANAGER_HPP

#include "stk_peer.hpp"
#include "stk_host.hpp"
#include <vector>

#include "protocol_manager.hpp"

class NetworkManager 
{
    public:
        NetworkManager();
        virtual ~NetworkManager();
        
        virtual void run() = 0; 
        
        static void setManualSocketsMode(bool manual);
        static void sendRawPacket(uint8_t* data, int length, unsigned int dstIp, unsigned short dstPort);
        
        static void receptionCallback(char* data);
        virtual void packetReceived(char* data) = 0;
        
        static STKHost* getHost();
    protected:
        std::vector<STKPeer*> m_peers;
        STKHost* m_localhost;
        
        static NetworkManager* instance;
};

#endif // NETWORKMANAGER_HPP
