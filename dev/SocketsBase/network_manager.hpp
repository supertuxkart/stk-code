#ifndef NETWORKMANAGER_HPP
#define NETWORKMANAGER_HPP

#include "stk_peer.hpp"
#include "stk_host.hpp"
#include <vector>

#include "protocol_manager.hpp"
#include "singleton.hpp"

class NetworkManager : public Singleton<NetworkManager>
{
    friend class Singleton<NetworkManager>;
    public:
        virtual void run() = 0; 
        
        virtual void setManualSocketsMode(bool manual);
        virtual void packetReceived(char* data) = 0;
        
        STKHost* getHost();
    protected:
        NetworkManager();
        virtual ~NetworkManager();
        
        std::vector<STKPeer*> m_peers;
        STKHost* m_localhost;
};

#endif // NETWORKMANAGER_HPP
