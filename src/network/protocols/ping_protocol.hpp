#ifndef PING_PROTOCOL_HPP
#define PING_PROTOCOL_HPP

#include "network/protocol.hpp"
#include "network/transport_address.hpp"

class PingProtocol : public Protocol
{
private:
    /** The destination for the ping request. */
    TransportAddress m_ping_dst;

    /** How frequently to ping. */
    double m_delay_between_pings;

    /** Time of last ping. */
    double m_last_ping_time;
public:
                 PingProtocol(const TransportAddress& ping_dst,
                              double delay_between_pings);
        virtual ~PingProtocol();

        virtual bool notifyEvent(Event* event) { return true; }
        virtual bool notifyEventAsynchronous(Event* event) { return true; }
        virtual void setup();
        virtual void update() {}
        virtual void asynchronousUpdate();

};

#endif // PING_PROTOCOL_HPP
