#ifndef CLIENT_NETWORK_MANAGER_HPP
#define CLIENT_NETWORK_MANAGER_HPP

#include "network_manager.hpp"


class ClientNetworkManager : public NetworkManager
{
    friend class Singleton<NetworkManager>;
    public:
        static ClientNetworkManager* getInstance()
        {
            return Singleton<NetworkManager>::getInstance<ClientNetworkManager>();
        }
        
        virtual void run();
        
        bool connectToHost(std::string serverNickname);
        
        virtual void packetReceived(char* data);
        virtual void sendPacket(char* data);
        
        STKPeer* getPeer();
        
    protected:
        ClientNetworkManager();
        virtual ~ClientNetworkManager();
};

#endif // CLIENT_NETWORK_MANAGER_HPP
