//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008 Joerg Henrichs
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

#ifndef HEADER_RACE_RESULT_ACK_MESSAGE_HPP
#define HEADER_RACE_RESULT_ACK_MESSAGE_HPP

#include <string>

#include "network/message.hpp"


/** This message is sent from the clients to the server when the race result
 *  screen was acknowledged, and then from the server to all clients to
 *  finish synchronisation.
 */
class RaceResultAckMessage : public Message
{
private:
    char m_menu_selected;
public:
    /** Constructor, creates an empty message
     */
    RaceResultAckMessage(char menu_choice) : Message(Message::MT_RACE_RESULT_ACK)
    {
        allocate(getCharLength());
        addChar(menu_choice);
    }   // RaceResultAckMessage

    // ------------------------------------------------------------------------
    /** Receives the ack message.
     *  \param pkt Received enet packet.
     */
    RaceResultAckMessage(ENetPacket* pkt):Message(pkt, MT_RACE_RESULT_ACK)
    {
        m_menu_selected = getChar();
    }   // RaceResultAckMessage(EnetPacket)
    // ------------------------------------------------------------------------
    /** Returns the menu selected on the server after this message is received
     *  on a client. */
    char getSelectedMenu() const {return m_menu_selected; }

};   // RaceResultAckMessageMessage
#endif
