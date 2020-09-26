//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 SuperTuxKart-Team
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

#ifndef CONNECT_TO_SERVER_HPP
#define CONNECT_TO_SERVER_HPP

#include "network/protocol.hpp"
#include "utils/cpp2011.hpp"
#include <atomic>
#include <memory>
#include <string>

#include "irrString.h"
#include <enet/enet.h>

namespace Online
{
    class Request;
}

class Server;

class ConnectToServer : public Protocol
{
private:
    std::shared_ptr<Server> m_server;

    irr::core::stringw m_error_msg;

    /** State for finite state machine. */
    enum ConnectState : unsigned int
    {
        SET_PUBLIC_ADDRESS,
        GOT_SERVER_ADDRESS,
        DONE,
        EXITING
    };
    std::atomic<ConnectState> m_state;

    void getClientServerInfo();
    bool registerWithSTKServer();
    bool tryConnect(int timeout, int retry, bool another_port = false,
                    bool ipv6 = false);
    static ENetAddress m_server_address;
    static int interceptCallback(ENetHost* host, ENetEvent* event);
    static int m_retry_count;
    static bool m_done_intecept;
    bool detectPort();
public:
             ConnectToServer(std::shared_ptr<Server> server);
    virtual ~ConnectToServer();
    virtual void setup() OVERRIDE;
    virtual void asynchronousUpdate() OVERRIDE;
    virtual void update(int ticks) OVERRIDE;
};   // class ConnectToServer

#endif // CONNECT_TO_SERVER_HPP
