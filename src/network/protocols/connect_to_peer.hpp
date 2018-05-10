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
class ConnectToPeer : public Protocol
{
protected:

    TransportAddress m_peer_address;
    uint32_t m_peer_id;

    /** Pointer to the protocol which is monitored for state changes, this
     *  need to be shared_ptr because we need to get the result from
     *  \ref GetPeerAddress, otherwise when it terminated the result will be
     *  gone. */
    std::shared_ptr<Protocol> m_current_protocol;

    /** Timer use for tracking broadcast. */
    double m_timer = 0.0;

    /** If greater than a certain value, terminate this protocol. */
    unsigned m_tried_connection = 0;

    enum STATE
    {
        NONE,
        RECEIVED_PEER_ADDRESS,
        WAIT_FOR_CONNECTION,
        DONE,
        EXITING
    }  m_state;

public:
             ConnectToPeer(uint32_t peer_id);
             ConnectToPeer(const TransportAddress &address);
    virtual ~ConnectToPeer();

    virtual void setup() OVERRIDE {}
    virtual void update(int ticks) OVERRIDE {}
    virtual void asynchronousUpdate() OVERRIDE;
};   // class ConnectToPeer

#endif // CONNECT_TO_SERVER_HPP
