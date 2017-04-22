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

#ifndef CONNECT_TO_PEER_HPP
#define CONNECT_TO_PEER_HPP

#include "network/protocol.hpp"
#include "network/transport_address.hpp"
#include "utils/cpp2011.hpp"

/** One instance of this is started for every peer who tries to
 *  connect to this server.
 */
class ConnectToPeer : public Protocol, public CallbackObject
{
protected:

    TransportAddress m_peer_address;
    uint32_t m_peer_id;

    /** Pointer to the protocol which is monitored for state changes. */
    Protocol *m_current_protocol;

    /** True if this is a LAN connection. */
    bool m_is_lan;

    /** We might need to broadcast several times (in case the client is not
     *  ready in time). This keep track of broadcastst. */
    float m_time_last_broadcast;

    int m_broadcast_count;

    enum STATE
    {
        NONE,
        RECEIVED_PEER_ADDRESS,
        WAIT_FOR_LAN,
        CONNECTING,
        CONNECTED,
        DONE,
        EXITING
    }  m_state;

public:
             ConnectToPeer(uint32_t peer_id);
             ConnectToPeer(const TransportAddress &address);
    virtual ~ConnectToPeer();

    virtual bool notifyEventAsynchronous(Event* event) OVERRIDE;
    virtual void setup() OVERRIDE;
    virtual void update(float dt) OVERRIDE {}
    virtual void asynchronousUpdate() OVERRIDE;
    virtual void callback(Protocol *protocol) OVERRIDE;
};   // class ConnectToPeer

#endif // CONNECT_TO_SERVER_HPP
