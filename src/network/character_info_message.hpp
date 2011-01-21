//  $Id$
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

#ifndef HEADER_CHARACTER_INFO_MESSAGE_HPP
#define HEADER_CHARACTER_INFO_MESSAGE_HPP

#include "karts/kart_properties_manager.hpp"
#include "network/message.hpp"

/** This message is sent from the server to the clients and contains the list
 *  of available characters. Additionally, it contains the clients id.
 */
class CharacterInfoMessage : public Message
{
// Add the remote host id to this message (to avoid sending this separately)
public:
    CharacterInfoMessage(int hostid) : Message(Message::MT_CHARACTER_INFO) 
    {
        std::vector<std::string> all_karts =
                               kart_properties_manager->getAllAvailableKarts();
        allocate(getCharLength()+getStringVectorLength(all_karts));
        addChar(hostid);
        addStringVector(all_karts);
    }
    // ------------------------------------------------------------------------
    CharacterInfoMessage(ENetPacket* pkt):Message(pkt, MT_CHARACTER_INFO)
    {
        int hostid=getChar();
        network_manager->setHostId(hostid);
        std::vector<std::string> all_karts;
        all_karts = getStringVector();
        kart_properties_manager->setUnavailableKarts(all_karts);
    }
};   // CharacterInfoMessage
#endif
