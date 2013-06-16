#include "protocol.hpp"

Protocol::Protocol()
{
}

Protocol::~Protocol()
{
}

PROTOCOL_TYPE Protocol::getProtocolType()
{
    return m_type;
}
