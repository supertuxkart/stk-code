#ifndef PROTOCOL_MANAGER_HPP
#define PROTOCOL_MANAGER_HPP

#include <vector>
#include <stdint.h>

class Protocol;

class ProtocolManager
{
    public:
        ProtocolManager();
        virtual ~ProtocolManager();
        
        virtual void messageReceived(uint8_t* data);
        
        virtual void runProtocol(Protocol* protocol);
        virtual void stopProtocol(Protocol* protocol);
        virtual void protocolTerminated(Protocol* protocol);
         
        virtual void update();
        
        virtual int runningProtocolsCount();
        
    protected:
        std::vector<Protocol*> m_protocols;
        std::vector<uint8_t*> m_messagesToProcess;
};

#endif // PROTOCOL_MANAGER_HPP
