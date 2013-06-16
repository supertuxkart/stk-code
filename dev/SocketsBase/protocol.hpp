#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include "protocol_listener.hpp"

#include <stdint.h>

enum PROTOCOL_TYPE
{
    GET_PUBLIC_ADDRESS = 0
};

class Protocol
{
    public:
        Protocol();
        virtual ~Protocol();
        
        virtual void messageReceived(uint8_t* data) = 0;
        
        virtual void setup() = 0;
        virtual void start() = 0;
        virtual void update() = 0;
        
        PROTOCOL_TYPE getProtocolType();
    protected:
        ProtocolListener* m_listener;
        PROTOCOL_TYPE m_type;
};

#endif // PROTOCOL_HPP
