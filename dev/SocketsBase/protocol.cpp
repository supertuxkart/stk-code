#include "protocol.hpp"

Protocol::Protocol(CallbackObject* callbackObject, PROTOCOL_TYPE type)
{
    m_callbackObject = callbackObject;
    m_type = type;
}

Protocol::~Protocol()
{
}

void Protocol::pause()
{
    m_listener->pauseProtocol(this);
}
void Protocol::unpause()
{
    m_listener->unpauseProtocol(this);
}


void Protocol::setListener(ProtocolManager* listener)
{
    m_listener = listener; 
}

PROTOCOL_TYPE Protocol::getProtocolType()
{
    return m_type;
}
