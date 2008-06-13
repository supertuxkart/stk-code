// $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
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

#ifndef TUXKART_PLAYER_H
#define TUXKART_PLAYER_H

#include <string>
#include "input.hpp"

extern const char *sKartAction2String[KA_LAST+1];

/*class for managing player name and control configuration*/
class Player
{
private:
    std::string m_name;
    Input m_action_map[KA_LAST+1];
    int m_last_kart_id;

public:
    Player(){}
    Player(const std::string &name_):m_name(name_),m_last_kart_id(-1){}
    void setName(const std::string &name_){m_name = name_;}

    std::string getName() {return m_name;}

    int getLastKartId(){ return m_last_kart_id; }
    void setLastKartId(int newLastKartId){ m_last_kart_id = newLastKartId; }
};

#endif

/*EOF*/
