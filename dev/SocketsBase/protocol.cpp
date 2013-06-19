#include "protocol.hpp"

Protocol::Protocol(CallbackObject* callbackObject)
{
    m_callbackObject = callbackObject;
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
