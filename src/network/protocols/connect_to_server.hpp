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
#include "network/transport_address.hpp"
#include "utils/cpp2011.hpp"
#include <atomic>
#include <memory>
#include <string>

class Server;

class ConnectToServer : public Protocol
{
private:
    double m_timer = 0.0;
    TransportAddress m_server_address;
    std::shared_ptr<Server> m_server;
    unsigned m_tried_connection = 0;

    /** Protocol currently being monitored. */
    std::weak_ptr<Protocol> m_current_protocol;

    /** State for finite state machine. */
    enum ConnectState : unsigned int
    {
        SET_PUBLIC_ADDRESS,
        REGISTER_SELF_ADDRESS,
        GOT_SERVER_ADDRESS,
        REQUESTING_CONNECTION,
        CONNECTING,
        CONNECTED,
        HIDING_ADDRESS,
        DONE,
        EXITING
    };
    std::atomic<ConnectState> m_state;

    void registerWithSTKServer();
    void waitingAloha(bool is_wan);
public:
             ConnectToServer(std::shared_ptr<Server> server);
    virtual ~ConnectToServer();

    virtual bool notifyEventAsynchronous(Event* event) OVERRIDE;
    virtual void setup() OVERRIDE;
    virtual void asynchronousUpdate() OVERRIDE;
    virtual void update(int ticks) OVERRIDE;
    bool handleDirectConnect(int timeout = 2000);

};   // class ConnectToServer

#endif // CONNECT_TO_SERVER_HPP
