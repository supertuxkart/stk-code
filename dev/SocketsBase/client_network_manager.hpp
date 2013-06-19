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
        
        void connect(uint32_t ip, uint16_t port);
        
        virtual void packetReceived(char* data);
        virtual void sendPacket(char* data);
        
    protected:
        ClientNetworkManager();
        virtual ~ClientNetworkManager();
};

#endif // CLIENT_NETWORK_MANAGER_HPP
