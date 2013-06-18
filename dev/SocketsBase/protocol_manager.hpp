#ifndef PROTOCOL_MANAGER_HPP
#define PROTOCOL_MANAGER_HPP

#include <vector>
#include <stdint.h>

class Protocol;

class ProtocolManager
{
    typedef struct
    {
        bool paused;
        Protocol* protocol;
    } ProtocolInfo;
    public:
        ProtocolManager();
        virtual ~ProtocolManager();
        
        virtual void messageReceived(uint8_t* data);
        
        virtual void runProtocol(Protocol* protocol);
        virtual void stopProtocol(Protocol* protocol);
        virtual void pauseProtocol(Protocol* protocol);
        virtual void unpauseProtocol(Protocol* protocol);
        virtual void protocolTerminated(Protocol* protocol);
         
        virtual void update();
        
        virtual int runningProtocolsCount();
        
    protected:
        std::vector<ProtocolInfo> m_protocols;
        std::vector<uint8_t*> m_messagesToProcess;
};

#endif // PROTOCOL_MANAGER_HPP
