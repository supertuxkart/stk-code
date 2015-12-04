#ifndef SYNCHRONIZATION_PROTOCOL_HPP
#define SYNCHRONIZATION_PROTOCOL_HPP

#include "network/protocol.hpp"
#include <vector>
#include <map>

class SynchronizationProtocol : public Protocol
{
private:
    std::vector<std::map<uint32_t, double> > m_pings;
    std::vector<uint32_t> m_average_ping;
    std::vector<uint32_t> m_pings_count;
    std::vector<uint32_t> m_successed_pings;
    std::vector<double> m_total_diff;
    bool m_countdown_activated;
    double m_countdown;
    double m_last_countdown_update;
    bool m_has_quit;

public:
             SynchronizationProtocol();
    virtual ~SynchronizationProtocol();

    virtual bool notifyEventAsynchronous(Event* event);
    virtual void setup();
    virtual void update() {}
    virtual void asynchronousUpdate();

    void startCountdown(int ms_countdown);

    int getCountdown() { return (int)(m_countdown*1000.0); }

};   // class SynchronizationProtocol

#endif // SYNCHRONIZATION_PROTOCOL_HPP
