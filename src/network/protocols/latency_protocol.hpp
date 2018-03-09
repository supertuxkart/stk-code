#ifndef LATENCY_PROTOCOL_HPP
#define LATENCY_PROTOCOL_HPP

#include "network/protocol.hpp"
#include "utils/cpp2011.hpp"

#include <vector>
#include <map>
#include <memory>

class STKPeer;

class LatencyProtocol : public Protocol
{
private:
    std::map<std::weak_ptr<STKPeer>, std::map<uint32_t, double>,
        std::owner_less<std::weak_ptr<STKPeer> > > m_pings;

    std::map<std::weak_ptr<STKPeer>, uint32_t,
        std::owner_less<std::weak_ptr<STKPeer> > > m_average_ping;

    std::map<std::weak_ptr<STKPeer>, uint32_t,
        std::owner_less<std::weak_ptr<STKPeer> > > m_successed_pings;

    std::map<std::weak_ptr<STKPeer>, double,
        std::owner_less<std::weak_ptr<STKPeer> > > m_total_diff;

    /** Counts the number of pings sent. */
    uint32_t m_pings_count;

    /** Keeps track of last time that an update was sent. */
    double m_last_time;


public:
             LatencyProtocol();
    virtual ~LatencyProtocol();

    virtual bool notifyEventAsynchronous(Event* event) OVERRIDE;
    virtual void setup() OVERRIDE;
    virtual void asynchronousUpdate() OVERRIDE;

    // ------------------------------------------------------------------------
    virtual void update(float dt) OVERRIDE {}

};   // class LatencyProtocol

#endif // LATENCY_PROTOCOL_HPP
