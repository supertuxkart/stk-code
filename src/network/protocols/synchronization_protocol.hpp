#ifndef SYNCHRONIZATION_PROTOCOL_HPP
#define SYNCHRONIZATION_PROTOCOL_HPP

#include "network/protocol.hpp"
#include <vector>
#include <map>

class SynchronizationProtocol : public Protocol
{
    public:
        SynchronizationProtocol();
        virtual ~SynchronizationProtocol();

        virtual void notifyEvent(Event* event);
        virtual void setup();
        virtual void update();

        void startCountdown(bool* ready, uint32_t ms_countdown);

    protected:
        std::vector<std::map<uint32_t, double> > m_pings;
        std::vector<uint32_t> m_average_ping;
        std::vector<uint32_t> m_pings_count;
        std::vector<uint32_t> m_successed_pings;
        std::vector<double> m_total_diff;
        bool* m_ready;
        bool m_countdown_activated;
        uint32_t m_countdown;
        double m_last_countdown_update;
};

#endif // SYNCHRONIZATION_PROTOCOL_HPP
