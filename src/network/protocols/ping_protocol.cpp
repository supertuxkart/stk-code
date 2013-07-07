#include "network/protocols/ping_protocol.hpp"

#include "network/network_manager.hpp"
#include "utils/time.hpp"

PingProtocol::PingProtocol(const TransportAddress& ping_dst, double delay_between_pings) : Protocol(NULL, PROTOCOL_SILENT)
{
    m_ping_dst = ping_dst;
    m_delay_between_pings = delay_between_pings;
}

PingProtocol::~PingProtocol()
{
}

void PingProtocol::notifyEvent(Event* event)
{
}

void PingProtocol::setup()
{
    m_last_ping_time = 0;
}

void PingProtocol::update()
{
    if (Time::getRealTime() > m_last_ping_time+m_delay_between_pings)
    {
        m_last_ping_time = Time::getRealTime();
        uint8_t data = 0;
        NetworkManager::getInstance()->getHost()->sendRawPacket(&data, 0, m_ping_dst);
    }
}
