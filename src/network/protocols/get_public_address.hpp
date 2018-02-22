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

#ifndef GET_PUBLIC_ADDRESS_HPP
#define GET_PUBLIC_ADDRESS_HPP

#include "network/protocol.hpp"
#include "network/transport_address.hpp"
#include "utils/cpp2011.hpp"

#include <string>

class Network;

class GetPublicAddress : public Protocol
{
private:
    Network* createStunRequest();
    std::string parseStunResponse(Network* transaction_host);

    std::vector<std::string> m_untried_server;

    // Constants
    static const uint32_t m_stun_magic_cookie;
    static const int m_stun_server_port = 3478;

    /** The user can specify its own IP address to make the use of stun
     *  unnecessary (though that means that the user has to take care of
     *  opening the firewall). */
    static TransportAddress m_my_address;

    uint8_t m_stun_tansaction_id[12];
    uint32_t m_stun_server_ip;
    Network* m_transaction_host;

public:
                GetPublicAddress();
    virtual    ~GetPublicAddress() {}

    virtual void asynchronousUpdate() OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void update(float dt) OVERRIDE {}
    // ------------------------------------------------------------------------
    virtual bool notifyEvent(Event* event) OVERRIDE { return true; }
    // ------------------------------------------------------------------------
    virtual bool notifyEventAsynchronous(Event* event) OVERRIDE { return true; }
    // ------------------------------------------------------------------------
    virtual void setup() { }
    // ------------------------------------------------------------------------

};   // class GetPublicAddress

#endif // GET_PUBLIC_ADDRESS_HPP
