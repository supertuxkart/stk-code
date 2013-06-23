#ifndef PROTOCOL_MANAGER_HPP
#define PROTOCOL_MANAGER_HPP

#include "singleton.hpp"
#include "event.hpp"

#include <vector>
#include <stdint.h>

class Protocol;

enum PROTOCOL_STATE
{
    PROTOCOL_STATE_RUNNING,
    PROTOCOL_STATE_PAUSED,
    PROTOCOL_STATE_TERMINATED
};

class ProtocolManager : public Singleton<ProtocolManager>
{
    friend class Singleton<ProtocolManager>;
    typedef struct
    {
        PROTOCOL_STATE state;
        Protocol* protocol;
        uint32_t id;
    } ProtocolInfo;
    public:
        
        virtual void notifyEvent(Event* event);
        virtual void sendMessage(std::string message);
        
        virtual int  startProtocol(Protocol* protocol);
        virtual void stopProtocol(Protocol* protocol);
        virtual void pauseProtocol(Protocol* protocol);
        virtual void unpauseProtocol(Protocol* protocol);
        virtual void protocolTerminated(Protocol* protocol);
         
        virtual void update();
        
        virtual int runningProtocolsCount();
        virtual PROTOCOL_STATE getProtocolState(uint32_t id);
        
    protected:
        // protected functions
        ProtocolManager();
        virtual ~ProtocolManager();
        void assignProtocolId(ProtocolInfo& protocolInfo);
        
        // protected members
        std::vector<ProtocolInfo> m_protocols;
        std::vector<Event*> m_eventsToProcess;
        
        // mutexes:
        pthread_mutex_t m_eventsMutex;
        pthread_mutex_t m_protocolsMutex;
};

#endif // PROTOCOL_MANAGER_HPP
