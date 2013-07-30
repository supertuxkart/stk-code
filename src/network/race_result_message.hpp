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

#ifndef HEADER_RACE_RESULT_MESSAGE_HPP
#define HEADER_RACE_RESULT_MESSAGE_HPP

#include <vector>

#include "network/message.hpp"


/** This message is from the server to all clients to inform them about the
 *  result of a race. The clients wait for this message before they finish
 *  a race.
 */
class RaceResultMessage : public Message
{
    struct RaceResult {
        float m_time;
        int   m_score;
    };   // RaceResult
private:
    std::vector<RaceResult> m_all_results;
public:
         RaceResultMessage();
         RaceResultMessage(ENetPacket* pkt);
    void addRaceResult(int kart_id, float time, int points);
    void getRaceResult(int kart_id, float &time, int &points);
};   // RaceResultMessage
#endif
