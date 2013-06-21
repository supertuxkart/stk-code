#ifndef NETWORKMANAGER_HPP
#define NETWORKMANAGER_HPP

#include "stk_peer.hpp"
#include "stk_host.hpp"
#include <vector>

#include "protocol_manager.hpp"
#include "singleton.hpp"
#include "types.hpp"
#include "event.hpp"

class NetworkManager : public Singleton<NetworkManager>
{
    friend class Singleton<NetworkManager>;
    public:
        virtual void run(); 
        
        // network management functions
        virtual bool connect(uint32_t ip, uint16_t port);
        virtual void setManualSocketsMode(bool manual);
        virtual void notifyEvent(Event* event);
        virtual void packetReceived(char* data) = 0;

        // raw data management
        void setLogin(std::string username, std::string password);
        void setPublicAddress(uint32_t ip, uint16_t port);
        
        // getters
        virtual bool peerExists(uint32_t ip, uint16_t port);
        virtual bool isConnectedTo(uint32_t ip, uint16_t port);
        STKHost* getHost();
        std::vector<STKPeer*> getPeers();
    protected:
        NetworkManager();
        virtual ~NetworkManager();
        
        // protected members
        std::vector<STKPeer*> m_peers;
        STKHost* m_localhost;
        
        TransportAddress m_publicAddress;
        PlayerLogin m_playerLogin;
        
        NetworkManager* m_networkManager;
        pthread_t* m_protocolManagerUpdateThread;
};

#endif // NETWORKMANAGER_HPP
