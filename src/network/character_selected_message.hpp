//  $Id: character_selected_message.hpp 2128 2008-06-13 00:53:52Z cosmosninja $
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

#ifndef HEADER_CHARACTER_SELECTED_MESSAGE_H
#define HEADER_CHARACTER_SELECTED_MESSAGE_H

#include "network/message.hpp"
#include "user_config.hpp"
#include "race_manager.hpp"
#include "network/remote_kart_info.hpp"

class CharacterSelectedMessage : public Message
{
private:
    int            m_num_local_players;
    RemoteKartInfo m_kart_info;
// For now this is an empty message
public:
    CharacterSelectedMessage(int player_id)  :Message(Message::MT_CHARACTER_INFO) 
    {
        m_kart_info         = race_manager->getLocalKartInfo(player_id);
        m_num_local_players = race_manager->getNumLocalPlayers();

        allocate(getCharLength()       // m_kart_info.getLocalPlayerId())
                +getStringLength(m_kart_info.getKartName())
                +getStringLength(m_kart_info.getPlayerName())
                +getCharLength());     // m_num_local_players)
        addChar(m_kart_info.getLocalPlayerId());
        addString(m_kart_info.getKartName());
        addString(m_kart_info.getPlayerName());
        // Piggy backing this information saves sending it as a separate 
        // message. It is actually only required in the first message
        addChar(race_manager->getNumLocalPlayers());
    }   // CharacterSelectedMessage
    // ------------------------------------------------------------------------
    CharacterSelectedMessage(ENetPacket* pkt):Message(pkt, MT_CHARACTER_INFO)
    {
        m_kart_info.setLocalPlayerId(getChar());
        m_kart_info.setKartName(getString());
        m_kart_info.setPlayerName(getString());
        m_num_local_players = getChar();
    }   // CharacterSelectedMessage(EnetPacket)
    // ------------------------------------------------------------------------
    const RemoteKartInfo& getKartInfo  () const { return m_kart_info;         }
    int                   getNumPlayers() const { return m_num_local_players; }
};   // CharacterSelectedMessage
#endif
