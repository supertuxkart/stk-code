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

#ifndef HEADER_RACE_INFO_MESSAGE_HPP
#define HEADER_RACE_INFO_MESSAGE_HPP

#include <vector>

#include "network/message.hpp"
#include "network/remote_kart_info.hpp"

class RaceInfoMessage : public Message
{
public:
    RaceInfoMessage(const std::vector<RemoteKartInfo>& kart_info);
    RaceInfoMessage(ENetPacket* pkt);
};   // RaceInfoMessage
#endif
