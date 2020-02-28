//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2020 SuperTuxKart-Team
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

#ifndef HEADER_SERVER_LOOP_HPP
#define HEADER_SERVER_LOOP_HPP

#include "utils/types.hpp"
#include <atomic>
#include <string>

struct ChildLoopConfig
{
    bool m_lan_server;
    uint32_t m_login_id;
    std::string m_token;
    unsigned m_server_ai;
};

class ChildLoop
{
private:
    const ChildLoopConfig* m_cl_config;

    std::atomic_bool m_abort;

    std::atomic<uint16_t> m_port;

    std::atomic<uint32_t> m_server_online_id;

    uint64_t m_curr_time;
    uint64_t m_prev_time;
    float getLimitedDt();
public:
    ChildLoop(const ChildLoopConfig& clc)
        : m_cl_config(new ChildLoopConfig(clc))
    {
        m_abort = false;
        m_prev_time = m_curr_time = 0;
        m_port = 0;
        m_server_online_id = 0;
    }
    void run();
    /** Set the abort flag, causing the mainloop to be left. */
    void abort() { m_abort = true; }
    bool isAborted() const { return m_abort; }
    uint16_t getPort() const { return m_port; }
    uint32_t getServerOnlineId() const { return m_server_online_id; }
};   // ChildLoop

#endif
