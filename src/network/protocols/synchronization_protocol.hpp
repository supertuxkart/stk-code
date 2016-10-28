#ifndef SYNCHRONIZATION_PROTOCOL_HPP
#define SYNCHRONIZATION_PROTOCOL_HPP

#include "network/protocol.hpp"
#include "utils/cpp2011.hpp"

#include <vector>
#include <map>

class SynchronizationProtocol : public Protocol
{
private:
    std::vector<std::map<uint32_t, double> > m_pings;
    std::vector<uint32_t> m_average_ping;

    /** Counts the number of pings sent. */
    uint32_t m_pings_count;
    std::vector<uint32_t> m_successed_pings;
    std::vector<double> m_total_diff;

    /** True if the countdown has started, i.e. all clients have loaded
     *  the track and karts. */
    bool m_countdown_activated;

    /** The countdown timer value. */
    float m_countdown;
    float  m_last_countdown_update;
    bool m_has_quit;

    /** Keeps track of last time that an update was sent. */
    double m_last_time;


public:
             SynchronizationProtocol();
    virtual ~SynchronizationProtocol();

    virtual bool notifyEventAsynchronous(Event* event) OVERRIDE;
    virtual void setup() OVERRIDE;
    virtual void asynchronousUpdate() OVERRIDE;
    void startCountdown(float ms_countdown);

    // ------------------------------------------------------------------------
    virtual void update(float dt) OVERRIDE {}
    // ------------------------------------------------------------------------
    /** Returns the current countdown value. */
    float getCountdown() { return m_countdown; }

};   // class SynchronizationProtocol

#endif // SYNCHRONIZATION_PROTOCOL_HPP
