#ifndef PROTOCOL_LISTENER_HPP
#define PROTOCOL_LISTENER_HPP

#include <vector>
#include <stdint.h>

class Protocol;

class ProtocolListener
{
    public:
        ProtocolListener();
        virtual ~ProtocolListener();
        
        virtual void messageReceived(uint8_t* data) = 0;
        
        virtual void runProtocol(Protocol* protocol) = 0;
        virtual void stopProtocol(Protocol* protocol) = 0;
        
        virtual void update() = 0;
        
    protected:
        std::vector<Protocol*> m_protocols;
};

#endif // PROTOCOL_LISTENER_HPP
