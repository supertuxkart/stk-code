#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include "protocol_manager.hpp"
#include "callback_object.hpp"

#include <stdint.h>

enum PROTOCOL_TYPE
{
    GET_PUBLIC_ADDRESS = 0
};

class Protocol
{
    public:
        Protocol(CallbackObject* callbackObject);
        virtual ~Protocol(); 
        
        virtual void messageReceived(uint8_t* data) = 0;
        
        void setListener(ProtocolManager* listener);
        
        virtual void setup() = 0;
        virtual void start() = 0;
        virtual void pause() = 0;
        virtual void unpause() = 0;
        virtual void update() = 0;
        
        PROTOCOL_TYPE getProtocolType();
    protected:
        ProtocolManager* m_listener;
        PROTOCOL_TYPE m_type;
        CallbackObject* m_callbackObject;
};

#endif // PROTOCOL_HPP
