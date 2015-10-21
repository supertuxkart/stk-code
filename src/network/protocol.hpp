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

/*! \file protocol.hpp
 *  \brief Generic protocols declarations.
 */

#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include "network/network_string.hpp"
#include "network/types.hpp"
#include "utils/types.hpp"

class Event;
class ProtocolManager;

/** \enum PROTOCOL_TYPE
  * \brief The types that protocols can have. This is used to select which protocol receives which event.
  * \ingroup network
  */
enum PROTOCOL_TYPE
{
    PROTOCOL_NONE = 0,          //!< No protocol type assigned.
    PROTOCOL_CONNECTION = 1,    //!< Protocol that deals with client-server connection.
    PROTOCOL_LOBBY_ROOM = 2,    //!< Protocol that is used during the lobby room phase.
    PROTOCOL_START_GAME = 3,    //!< Protocol used when starting the game.
    PROTOCOL_SYNCHRONIZATION = 4,//!<Protocol used to synchronize clocks.
    PROTOCOL_KART_UPDATE = 5,   //!< Protocol to update karts position, rotation etc...
    PROTOCOL_GAME_EVENTS = 6,   //!< Protocol to communicate the game events.
    PROTOCOL_CONTROLLER_EVENTS = 7,//!< Protocol to transfer controller modifications
    PROTOCOL_SILENT = 0xffff    //!< Used for protocols that do not subscribe to any network event.
};

/** \class Protocol
  * \brief Abstract class used to define the global protocol functions.
  * A protocol is an entity that is started at a point, and that is updated by a thread.
  * A protocol can be terminated by an other class, or it can terminate itself if has fulfilled its role.
  * This class must be inherited to make any network job.
  * \ingroup network
  */
class Protocol
{
    public:
        /*!
         * \brief Constructor
         *
         * Sets the basic protocol parameters, as the callback object and the protocol type.
         *
         * \param callback_object The callback object that will be used by the protocol. Protocols that do not use callback objects must set it to NULL.
         * \param type The type of the protocol.
         */
        Protocol(CallbackObject* callback_object, PROTOCOL_TYPE type);
        /*! \brief Destructor
         */
        virtual ~Protocol();

        /*! \brief Notify a protocol matching the Event type of that event.
         *  \param event : Pointer to the event.
         *  \return True if the event has been treated, false elseway
         */
        virtual bool notifyEvent(Event* event) { return false; }

        /*! \brief Notify a protocol matching the Event type of that event.
         *  This update is done asynchronously :
         *  \param event : Pointer to the event.
         *  \return True if the event has been treated, false elseway
         */
        virtual bool notifyEventAsynchronous(Event* event) { return false; }

        /*! \brief Set the protocol listener.
         *  \param listener : Pointer to the listener.
         */
        void setListener(ProtocolManager* listener);

        /*! \brief Called when the protocol is going to start. Must be re-defined by subclasses.
         */
        virtual void setup() = 0;
        /*! \brief Called when the protocol is paused (by an other entity or by itself).
         *  This function must be called by the subclasse's pause function if re-defined.
         */
        virtual void pause();
        /*! \brief Called when the protocol is unpaused.
         *  This function must be called by the subclasse's unpause function if re-defined.
         */
        virtual void unpause();
        /*! \brief Called by the protocol listener, synchronously with the main loop. Must be re-defined.
         */
        virtual void update() = 0;
        /*! \brief Called by the protocol listener as often as possible. Must be re-defined.
         */
        virtual void asynchronousUpdate() = 0;
        /*! \brief Called when the protocol is to be killed.
         */
        virtual void kill();

        /*! \brief Method to get a protocol's type.
         *  \return The protocol type.
         */
        PROTOCOL_TYPE getProtocolType();

        /// functions to check incoming data easily
        bool checkDataSizeAndToken(Event* event, int minimum_size);
        bool isByteCorrect(Event* event, int byte_nb, int value);
        void sendMessageToPeersChangingToken(NetworkString prefix, NetworkString message);


    protected:
        ProtocolManager* m_listener;        //!< The protocol listener
        PROTOCOL_TYPE m_type;               //!< The type of the protocol
        CallbackObject* m_callback_object;   //!< The callback object, if needed
};

#endif // PROTOCOL_HPP
