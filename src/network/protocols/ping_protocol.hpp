#ifndef PING_PROTOCOL_HPP
#define PING_PROTOCOL_HPP

#include "network/protocol.hpp"
#include "network/transport_address.hpp"

class PingProtocol : public Protocol
{
    public:
        PingProtocol(const TransportAddress& ping_dst, double delay_between_pings);
        virtual ~PingProtocol();

        virtual bool notifyEvent(Event* event) { return true; }
        virtual bool notifyEventAsynchronous(Event* event) { return true; }
        virtual void setup();
        virtual void update() {}
        virtual void asynchronousUpdate();

    protected:
        TransportAddress m_ping_dst;
        double m_delay_between_pings;
        double m_last_ping_time;
};

#endif // PING_PROTOCOL_HPP
