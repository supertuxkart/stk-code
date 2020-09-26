//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2015 Joerg Henrichs
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

#ifdef ENABLE_IPV6

#ifndef HEADER_STUN_DETECTION
#define HEADER_STUN_DETECTION

#include "network/socket_address.hpp"
#include <atomic>
#include <thread>

#ifdef WIN32
#  include <winsock.h>
#else
#  include <sys/socket.h>
#endif

class StunDetection
{
private:
    /** Socket connection thread */
    std::thread m_thread;

    /** Socket variable */
#ifdef WIN32
    SOCKET m_socket;
#else
    int m_socket;
#endif

    /** True if socket connected */
    std::atomic_bool m_connected;

    /** True if socket closed and the thread can be joined */
    std::atomic_bool m_socket_closed;
public:
    // ------------------------------------------------------------------------
    StunDetection(const std::string& addr, bool ipv4)
    {
        m_connected = false;
        m_socket_closed = true;
        SocketAddress sa(addr.c_str(), 0/*port specified in addr*/,
            ipv4 ? AF_INET : AF_INET6);
        if (sa.isUnset() || (ipv4 && sa.getFamily() != AF_INET) ||
            (!ipv4 && sa.getFamily() != AF_INET6))
            return;
        m_socket = socket(ipv4? AF_INET : AF_INET6, SOCK_STREAM, 0);
#ifdef WIN32
        if (m_socket == INVALID_SOCKET)
            return;
#else
        if (m_socket == -1)
            return;
#endif
        m_socket_closed = false;
        m_thread = std::thread([addr, ipv4, sa, this]()
        {
            uint64_t t = StkTime::getMonoTimeMs();
            if (connect(m_socket, sa.getSockaddr(), sa.getSocklen()) == -1)
                m_connected.store(false);
            else
                m_connected.store(true);
            shutdown(m_socket, 2);
#ifdef WIN32
            closesocket(m_socket);
#else
            close(m_socket);
#endif
            m_socket_closed.store(true);
            Log::debug("StunDetection", "Took %dms for %s.",
                (int)(StkTime::getMonoTimeMs() - t),
                (addr + (ipv4 ? " (IPv4)" : " (IPv6)")).c_str());
        });
    }
    // ------------------------------------------------------------------------
    ~StunDetection()
    {
        if (m_thread.joinable())
        {
            if (m_socket_closed)
                m_thread.join();
            else
                m_thread.detach();
        }
    }
    // ------------------------------------------------------------------------
    bool isConnecting()
                    { return m_thread.joinable() && m_socket_closed == false; }
    // ------------------------------------------------------------------------
    bool connectionSucceeded()                  { return m_connected == true; }
    // ------------------------------------------------------------------------
    bool socketClosed()                     { return m_socket_closed == true; }
};
#endif

#endif
