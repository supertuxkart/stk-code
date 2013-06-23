#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include "protocol_manager.hpp"
#include "types.hpp"

#include <stdint.h>

enum PROTOCOL_TYPE
{
    PROTOCOL_NONE = 0,
    PROTOCOL_CONNECTION = 1,
    PROTOCOL_SILENT = 0xffff // used for protocols that do not subscribe to any network event.
};

class Protocol
{
    public:
        Protocol(CallbackObject* callbackObject, PROTOCOL_TYPE type);
        virtual ~Protocol(); 
        
        virtual void notifyEvent(Event* event) = 0;
        
        void setListener(ProtocolManager* listener);
        
        virtual void setup() = 0;
        virtual void pause();
        virtual void unpause();
        virtual void update() = 0;
        
        PROTOCOL_TYPE getProtocolType();
    protected:
        ProtocolManager* m_listener;
        PROTOCOL_TYPE m_type;
        CallbackObject* m_callbackObject;
};

#endif // PROTOCOL_HPP
