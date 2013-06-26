//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 SuperTuxKart-Team
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

#ifndef EVENT_HPP
#define EVENT_HPP

#include "stk_peer.hpp"
#include <stdint.h>
#include <string>

/*!
 * \enum EVENT_TYPE
 * \brief Represents a network event type.
 */
enum EVENT_TYPE
{
    EVENT_TYPE_CONNECTED,   //!< A peer is connected
    EVENT_TYPE_DISCONNECTED,//!< A peer is disconnected
    EVENT_TYPE_MESSAGE      //!< A message between server and client protocols
};

/*!
 * \class Event
 * \brief Class representing an event that need to pass trough the system.
 * This is used to remove ENet dependency in the network. 
 * It interfaces the ENetEvent structure.
 */
class Event
{
    public:
        /*! 
         * \brief Constructor
         * \param event : The event that needs to be translated.
         */
        Event(ENetEvent* event);
        /*! 
         * \brief Destructor
         * frees the memory of the ENetPacket.
         */
        ~Event();
    
        EVENT_TYPE type;    //!< Type of the event
        std::string data;   //!< Copy of the data passed by the event
        STKPeer* peer;      //!< Pointer to the peer that triggered that event
    
    private:
        ENetPacket* m_packet; //!< A pointer on the ENetPacket to be deleted.
};

#endif // EVENT_HPP
