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

#include "utils/string_utils.hpp"
#include "utils/types.hpp"

#include "enet/enet.h"

#include <string>

// ============================================================================
/*! \class CallbackObject
 *  \brief Class that must be inherited to pass objects to protocols.
 */
class CallbackObject
{
    public:
        CallbackObject() {}
        ~CallbackObject() {}

};   // CallbackObject

// ============================================================================
/*! \class TransportAddress
 *  \brief Describes a transport-layer address.
 *  For IP networks, a transport address is the couple ip:port.
 */
class TransportAddress : public CallbackObject
{
public:
    uint32_t m_ip;    //!< The IPv4 address
    uint16_t m_port;  //!< The port number

    /** Constructor. */
    TransportAddress(uint32_t ip = 0, uint16_t port = 0)
    {
        m_ip = ip;
        m_port = port; 
    }   // TransportAddress

    // ------------------------------------------------------------------------
    ~TransportAddress() {}
    // ------------------------------------------------------------------------
    /** Resets ip and port to 0. */
    void clear()
    {
        m_ip   = 0;
        m_port = 0;
    }   // clear
    // ------------------------------------------------------------------------
    /** Compares if ip address and port are identical. */
    bool operator==(const TransportAddress& other) const
    {
        return other.m_ip == m_ip && other.m_port == m_port; 
    }   // operator==

    // ------------------------------------------------------------------------
    bool operator==(const ENetAddress& other)
    {
        return other.host == ntohl(m_ip) && other.port == m_port;
    }
    // ------------------------------------------------------------------------
    /** Compares if ip address or port are different. */
    bool operator!=(const TransportAddress& other) const
    {
        return other.m_ip != m_ip || other.m_port != m_port;
    }   // operator!=
    // ------------------------------------------------------------------------
    /** Returns a std::string representing the ip address and port in human
     *  readable format. */
    std::string toString() const
    {
        return
        StringUtils::insertValues("%d.%d.%d.%d:%d",
                                  ((m_ip >> 24) & 0xff), ((m_ip >> 16) & 0xff),
                                  ((m_ip >>  8) & 0xff), ((m_ip >>  0) & 0xff),
                                  m_port                                     );
    }   // toString
};   // TransportAddress

// ============================================================================
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
};   // class PlayerLogin


#endif // TYPES_HPP
