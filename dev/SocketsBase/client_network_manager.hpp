#ifndef CLIENT_NETWORK_MANAGER_HPP
#define CLIENT_NETWORK_MANAGER_HPP

#include "network_manager.hpp"


class ClientNetworkManager : public NetworkManager
{
    public:
        ClientNetworkManager();
        virtual ~ClientNetworkManager();
        
        virtual void run();
        
        void connect(uint32_t ip, uint16_t port);
        
        virtual void packetReceived(char* data);
        virtual void sendPacket(char* data);
        
    protected:
    
};

#endif // CLIENT_NETWORK_MANAGER_HPP
