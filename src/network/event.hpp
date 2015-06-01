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

/*! \file event.hpp
 *  \brief Contains an interface to store network events, like connections,
 *  disconnections and messages.
 */

#ifndef EVENT_HPP
#define EVENT_HPP

#include "network/stk_peer.hpp"
#include "network/network_string.hpp"
#include "utils/types.hpp"

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
 * The user has to be extremely careful about the peer.
 * Indeed, when packets are logged, the state of the peer cannot be stored at
 * all times, and then the user of this class can rely only on the address/port
 * of the peer, and not on values that might change over time.
 */
class Event
{
    public:
        /*! \brief Constructor
         *  \param event : The event that needs to be translated.
         */
        Event(ENetEvent* event);
        /*! \brief Constructor
         *  \param event : The event to copy.
         */
        Event(const Event& event);
        /*! \brief Destructor
         *  frees the memory of the ENetPacket.
         */
        ~Event();

        /*! \brief Remove bytes at the beginning of data.
         *  \param size : The number of bytes to remove.
         */
        void removeFront(int size);

        /*! \brief Get a copy of the data.
         *  \return A copy of the message data. This is empty for events like
         *  connection or disconnections.
         */
        NetworkString data() const { return m_data; }

        EVENT_TYPE type;    //!< Type of the event.
        STKPeer** peer;     //!< Pointer to the peer that triggered that event.

    private:
        NetworkString m_data; //!< Copy of the data passed by the event.
        ENetPacket* m_packet; //!< A pointer on the ENetPacket to be deleted.
};

#endif // EVENT_HPP
