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
#include "network/socket_address.hpp"
#include "utils/cpp2011.hpp"

/** One instance of this is started for every peer who tries to
 *  connect to this server.
 */
class ConnectToPeer : public Protocol
{
protected:
    SocketAddress m_peer_address;

    /** Timer use for tracking broadcast. */
    uint64_t m_timer = 0;

    /** If greater than a certain value, terminate this protocol. */
    unsigned m_tried_connection = 0;

    enum STATE
    {
        WAIT_FOR_CONNECTION,
        DONE,
        EXITING
    }  m_state;

public:
             ConnectToPeer(const SocketAddress &address);
    virtual ~ConnectToPeer() {}
    virtual void setup() OVERRIDE {}
    virtual void update(int ticks) OVERRIDE {}
    virtual void asynchronousUpdate() OVERRIDE;
};   // class ConnectToPeer

#endif // CONNECT_TO_SERVER_HPP
