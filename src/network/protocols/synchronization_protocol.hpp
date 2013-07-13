#ifndef SYNCHRONIZATION_PROTOCOL_HPP
#define SYNCHRONIZATION_PROTOCOL_HPP

#include "network/protocol.hpp"
#include <list>
#include <utility>

class SynchronizationProtocol : public Protocol
{
    public:
        SynchronizationProtocol(uint32_t* ping, bool* successed);
        virtual ~SynchronizationProtocol();

        virtual void notifyEvent(Event* event);
        virtual void setup();
        virtual void update();

    protected:
        //!< stores the start time / arrival time of packets for each peer
        std::vector<std::vector<std::pair<double, double> > > m_pings;
        uint32_t* m_average_ping;
        uint32_t m_pings_count;
        uint32_t m_successed_pings;
        double m_total_diff;
        bool* m_successed;
};

#endif // SYNCHRONIZATION_PROTOCOL_HPP
