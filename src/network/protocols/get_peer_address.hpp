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

#ifndef GET_PEER_ADDRESS_HPP
#define GET_PEER_ADDRESS_HPP

#include "network/protocol.hpp"
#include "network/transport_address.hpp"

namespace Online { class XMLRequest; }

class GetPeerAddress : public Protocol
{
private:
    uint32_t m_peer_id;
    Online::XMLRequest* m_request;

    /** Stores the address found. Used in a callback from the parent protocol
     *  to get the result. */
    TransportAddress m_address;
public:
             GetPeerAddress(uint32_t peer_id, CallbackObject* callback_object);
    virtual ~GetPeerAddress();

    virtual void setup();
    virtual void asynchronousUpdate();
    void setPeerID(uint32_t m_peer_id);

    // ------------------------------------------------------------------------
    /** Returns the address found. */
    const TransportAddress &getAddress() const { return m_address;  }
    // ------------------------------------------------------------------------
    virtual void update() {}
    // ------------------------------------------------------------------------
    virtual bool notifyEvent(Event* event) { return true; }
    // ------------------------------------------------------------------------
    virtual bool notifyEventAsynchronous(Event* event) { return true; }

};   // class GetPeerAddress

#endif // GET_PEER_ADDRESS_HPP
