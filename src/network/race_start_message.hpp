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

#ifndef HEADER_RACE_START_MESSAGE_HPP
#define HEADER_RACE_START_MESSAGE_HPP

#include "network/message.hpp"
#include "network/remote_kart_info.hpp"
#include "race/race_manager.hpp"

class RaceStartMessage : public Message
{
private:
// For now this is an empty message
public:
    RaceStartMessage() : Message(Message::MT_RACE_START) 
    {
        allocate(0);
    }   // RaceStartMessage

    RaceStartMessage(ENetPacket* pkt):Message(pkt, MT_RACE_START)
    {
    }
};   // RaceStartMessage
#endif
