//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2018 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef HEADER_NETWORK_TIMER_SYNCHRONIZER_HPP
#define HEADER_NETWORK_TIMER_SYNCHRONIZER_HPP

#include "network/stk_host.hpp"
#include "utils/log.hpp"
#include "utils/time.hpp"
#include "utils/types.hpp"

#include <atomic>
#include <cstdlib>
#include <deque>
#include <numeric>
#include <tuple>

class NetworkTimerSynchronizer
{
private:
    std::deque<std::tuple<uint32_t, uint64_t, uint64_t> > m_times;

    std::atomic_bool m_synchronised, m_force_set_timer;

public:
    NetworkTimerSynchronizer()
    {
        m_synchronised.store(false);
        m_force_set_timer.store(false);
    }
    // ------------------------------------------------------------------------
    bool isSynchronised() const               { return m_synchronised.load(); }
    // ------------------------------------------------------------------------
    void enableForceSetTimer()
    {
        if (m_synchronised.load() == true)
            return;
        m_force_set_timer.store(true);
    }
    // ------------------------------------------------------------------------
    void resynchroniseTimer()                  { m_synchronised.store(false); }
    // ------------------------------------------------------------------------
    void addAndSetTime(uint32_t ping, uint64_t server_time)
    {
        if (m_synchronised.load() == true)
            return;

        if (m_force_set_timer.load() == true)
        {
            m_force_set_timer.store(false);
            m_synchronised.store(true);
            STKHost::get()->setNetworkTimer(server_time + (uint64_t)(ping / 2));
            return;
        }

        const uint64_t cur_time = StkTime::getMonoTimeMs();
        // Discard too close time compared to last ping
        // (due to resend when packet loss)
        // 10 packets per second as seen in STKHost
        const uint64_t frequency = (uint64_t)((1.0f / 10.0f) * 1000.0f) / 2;
        if (!m_times.empty() &&
            cur_time - std::get<2>(m_times.back()) < frequency)
            return;

        // Take max 20 averaged samples from m_times, the next addAndGetTime
        // is used to determine that server_time if it's correct, if not
        // clear half in m_times until it's correct
        if (m_times.size() >= 20)
        {
            uint64_t sum = std::accumulate(m_times.begin(), m_times.end(),
                (uint64_t)0, [cur_time](const uint64_t previous,
                const std::tuple<uint32_t, uint64_t, uint64_t>& b)->uint64_t
                {
                    return previous + (uint64_t)(std::get<0>(b) / 2) +
                        std::get<1>(b) + cur_time - std::get<2>(b);
                });
            const int64_t averaged_time = sum / 20;
            const int64_t server_time_now = server_time + (uint64_t)(ping / 2);
            int difference = (int)std::abs(averaged_time - server_time_now);
            if (std::abs(averaged_time - server_time_now) <
                UserConfigParams::m_timer_sync_difference_tolerance)
            {
                STKHost::get()->setNetworkTimer(averaged_time);
                m_times.clear();
                m_force_set_timer.store(false);
                m_synchronised.store(true);
                Log::info("NetworkTimerSynchronizer", "Network "
                    "timer synchronized, difference: %dms", difference);
                return;
            }
            m_times.erase(m_times.begin(), m_times.begin() + 10);
        }
        m_times.emplace_back(ping, server_time, cur_time);
    }
};

#endif // HEADER_NETWORK_TIMER_SYNCHRONIZER_HPP
