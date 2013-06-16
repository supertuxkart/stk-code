#ifndef SERVER_NETWORK_MANAGER_HPP
#define SERVER_NETWORK_MANAGER_HPP

#include "network_manager.hpp"


class ServerNetworkManager : public NetworkManager
{
    public:
        ServerNetworkManager();
        virtual ~ServerNetworkManager();
        
        virtual void run();
        
        void start();
        
        virtual void packetReceived(char* data);
        virtual void sendPacket(char* data);
    protected:
    
};

#endif // SERVER_NETWORK_MANAGER_HPP
