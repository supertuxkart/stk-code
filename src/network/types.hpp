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

/*! \file types.hpp
 *  \brief Declares the general types that are used by the network.
 */
#ifndef TYPES_HPP
#define TYPES_HPP

#include "utils/types.hpp"

#include <string>

/*! functions to write easily addresses in logs. */
#define ADDRESS_FORMAT "%d.%d.%d.%d:%d"
#define ADDRESS_ARGS(ip,port) ((ip>>24)&0xff),((ip>>16)&0xff),((ip>>8)&0xff),((ip>>0)&0xff),port

/*! \class CallbackObject
 *  \brief Class that must be inherited to pass objects to protocols.
 */
class CallbackObject
{
    public:
        CallbackObject() {}
        ~CallbackObject() {}

};

/*! \class TransportAddress
 *  \brief Describes a transport-layer address.
 *  For IP networks, a transport address is the couple ip:port.
 */
class TransportAddress : public CallbackObject
{
    public:
    TransportAddress(uint32_t p_ip = 0, uint16_t p_port = 0)
    { ip = p_ip; port = p_port; }
    ~TransportAddress() {}

    bool operator==(const TransportAddress& other) const
    { return other.ip == ip && other.port == port; }

    bool operator!=(const TransportAddress& other) const
    { return other.ip != ip || other.port != port; }

    uint32_t ip;    //!< The IPv4 address
    uint16_t port;  //!< The port number
};

/*! \class PlayerLogin
 *  \brief Contains the information needed to authenticate a user.
 */
class PlayerLogin : public CallbackObject
{
    public:
    PlayerLogin() {}
    ~PlayerLogin() { username.clear(); password.clear(); }

    std::string username;   //!< Username of the player
    std::string password;   //!< Password of the player
};


#endif // TYPES_HPP
