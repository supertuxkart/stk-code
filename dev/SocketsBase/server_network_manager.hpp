#ifndef SERVER_NETWORK_MANAGER_HPP
#define SERVER_NETWORK_MANAGER_HPP

#include "network_manager.hpp"


class ServerNetworkManager : public NetworkManager
{
    friend class Singleton<NetworkManager>;
    public:
        static ServerNetworkManager* getInstance()
        {
            return Singleton<NetworkManager>::getInstance<ServerNetworkManager>();
        }
        
        virtual void run();
        
        void start();
        
        virtual void packetReceived(char* data);
        virtual void sendPacket(char* data);
    protected:
        ServerNetworkManager();
        virtual ~ServerNetworkManager();
    
};

#endif // SERVER_NETWORK_MANAGER_HPP
