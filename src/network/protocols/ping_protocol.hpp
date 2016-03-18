#ifndef PING_PROTOCOL_HPP
#define PING_PROTOCOL_HPP

#include "network/protocol.hpp"
#include "network/transport_address.hpp"
#include "utils/cpp2011.hpp"

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

    virtual void asynchronousUpdate() OVERRIDE;

    virtual void setup() OVERRIDE;
    // ------------------------------------------------------------------------
    virtual bool notifyEvent(Event* event) OVERRIDE { return true; }
    // ------------------------------------------------------------------------
    virtual bool notifyEventAsynchronous(Event* event) OVERRIDE { return true; }
    // ------------------------------------------------------------------------
    virtual void update(float dt) OVERRIDE {}
};

#endif // PING_PROTOCOL_HPP
