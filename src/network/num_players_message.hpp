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

#ifndef HEADER_NUM_PLAYERS_MESSAGE_HPP
#define HEADER_NUM_PLAYERS_MESSAGE_HPP

#include <string>
#include <sstream>
#ifndef WIN32
#  include <unistd.h>
#endif

#include "network/message.hpp"
#include "race/race_manager.hpp"

class NumPlayersMessage : public Message
{
private:
    int m_num_players
public:
                NumPlayersMessage():Message(Message::MT_CONNECT) { m_num_players=race }
                NumPlayersMessage(ENetPacket* pkt):Message(pkt)
                              { m_id=getString(); }
    const std::string&
                getNumPlayers()       { return m_num_players;      }
};   // ConnectMessage
#endif
