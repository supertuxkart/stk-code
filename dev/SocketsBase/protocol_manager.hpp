#ifndef PROTOCOL_MANAGER_HPP
#define PROTOCOL_MANAGER_HPP

#include "protocol_listener.hpp"

class ProtocolManager : public ProtocolListener
{
    public:
        ProtocolManager();
        virtual ~ProtocolManager();
        
        virtual void messageReceived(uint8_t* data);
        
        virtual void runProtocol(Protocol* protocol);
        virtual void stopProtocol(Protocol* protocol);
        
        virtual void update();
    protected:
        std::vector<uint8_t*> m_messagesToProcess;
};

#endif // PROTOCOL_MANAGER_HPP
