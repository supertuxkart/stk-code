#ifndef PING_PROTOCOL_HPP
#define PING_PROTOCOL_HPP

#include "network/protocol.hpp"


class PingProtocol : public Protocol
{
    public:
        PingProtocol(const TransportAddress& ping_dst, double delay_between_pings);
        virtual ~PingProtocol();
        
        virtual void notifyEvent(Event* event);
        virtual void setup();
        virtual void update();
        
    protected:
        TransportAddress m_ping_dst;
        double m_delay_between_pings;
        double m_last_ping_time;
};

#endif // PING_PROTOCOL_HPP
