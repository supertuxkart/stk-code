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
#include <string>

class ConnectToServer : public Protocol, public CallbackObject
{
private:
    TransportAddress m_server_address;
    uint32_t m_server_id;
    uint32_t m_host_id;

    /** Protocol currently being monitored. */
    Protocol *m_current_protocol;
    bool m_quick_join;

    /** State for finite state machine. */
    enum
    {
        NONE,
        GETTING_SELF_ADDRESS,
        GOT_SERVER_ADDRESS,
        REQUESTING_CONNECTION,
        QUICK_JOIN,
        CONNECTING,
        CONNECTED,
        HIDING_ADDRESS,
        DONE,
        EXITING
    } m_state;

    void registerWithSTKServer();
    void handleQuickConnect();
    void handleSameLAN();

public:
             ConnectToServer();
             ConnectToServer(uint32_t server_id, uint32_t host_id);
    virtual ~ConnectToServer();

    virtual bool notifyEventAsynchronous(Event* event) OVERRIDE;
    virtual void setup() OVERRIDE;
    virtual void asynchronousUpdate() OVERRIDE;
    virtual void callback(Protocol *protocol) OVERRIDE;
    virtual void update(float dt) OVERRIDE {}
    void setServerAddress(const TransportAddress &address);
};   // class ConnectToServer

#endif // CONNECT_TO_SERVER_HPP
