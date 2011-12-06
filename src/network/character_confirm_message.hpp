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

#ifndef HEADER_CHARACTER_CONFIRM_MESSAGE_HPP
#define HEADER_CHARACTER_CONFIRM_MESSAGE_HPP

#include <string>

#include "network/message.hpp"


/** This message is from the server to all clients to inform them about a 
 *  newly selected character. This means that this character is not available
 *  anymore. The message contains the hostid of the client who selected this
 *  character (0 in case of server), so that this message acts as a 
 *  confirmation for the corresponding client (or a reject if the message has
 *  a different hostid, meaning that another client selected the character
 *  earlier).
 */
class CharacterConfirmMessage : public Message
{
private:
    /** The host id. */
    int         m_host_id;
    /** Name of the selected kart. */
    std::string m_kart_name;
public:
    /** Constructor, takes the name of the kart name and the host id.
     *  \param kart_name Name of the kart.
     *  \param host_id Id of the host who selected this character.
     */
    CharacterConfirmMessage(const std::string &kart_name, int host_id)
        : Message(Message::MT_CHARACTER_CONFIRM) 
    {
        allocate(getStringLength(kart_name) + getCharLength());
        addString(kart_name);
        addChar(host_id);
    }   // CharacterConfirmMessage

    // ------------------------------------------------------------------------
    /** Unpacks a character confirm message. 
     *  \param pkt Received enet packet.
     */
    CharacterConfirmMessage(ENetPacket* pkt):Message(pkt, MT_CHARACTER_CONFIRM)
    {
        m_kart_name = getString();
        m_host_id   = getChar();
    }   // CharacterConfirmMessage(EnetPacket)
    // ------------------------------------------------------------------------
    /** Returns the kart name contained in a received message. */
    const std::string &getKartName() const { return m_kart_name; }
    /** Returns the host id contained in a received message. */
    int                getHostId() const   { return m_host_id;   }

};   // CharacterConfirmMessage
#endif
