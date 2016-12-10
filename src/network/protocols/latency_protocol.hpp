#ifndef LATENCY_PROTOCOL_HPP
#define LATENCY_PROTOCOL_HPP

#include "network/protocol.hpp"
#include "utils/cpp2011.hpp"

#include <vector>
#include <map>

class LatencyProtocol : public Protocol
{
private:
    std::vector<std::map<uint32_t, double> > m_pings;
    std::vector<uint32_t> m_average_ping;

    /** Counts the number of pings sent. */
    uint32_t m_pings_count;
    std::vector<uint32_t> m_successed_pings;
    std::vector<double> m_total_diff;

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
