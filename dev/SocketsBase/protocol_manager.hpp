#ifndef PROTOCOL_MANAGER_HPP
#define PROTOCOL_MANAGER_HPP

#include <vector>
#include <stdint.h>

class Protocol;

enum PROTOCOL_STATE
{
    PROTOCOL_STATE_RUNNING,
    PROTOCOL_STATE_PAUSED,
    PROTOCOL_STATE_TERMINATED
};

class ProtocolManager
{
    typedef struct
    {
        PROTOCOL_STATE state;
        Protocol* protocol;
        uint32_t id;
    } ProtocolInfo;
    public:
        ProtocolManager();
        virtual ~ProtocolManager();
        
        virtual void messageReceived(uint8_t* data);
        
        virtual int  startProtocol(Protocol* protocol);
        virtual void stopProtocol(Protocol* protocol);
        virtual void pauseProtocol(Protocol* protocol);
        virtual void unpauseProtocol(Protocol* protocol);
        virtual void protocolTerminated(Protocol* protocol);
         
        virtual void update();
        
        virtual int runningProtocolsCount();
        
    protected:
        void assignProtocolId(ProtocolInfo& protocolInfo);
    
        std::vector<ProtocolInfo> m_protocols;
        std::vector<uint8_t*> m_messagesToProcess;
};

#endif // PROTOCOL_MANAGER_HPP
