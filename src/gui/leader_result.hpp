//  $Id: grand_prix_ending.hpp 1023 2007-04-29 22:23:46Z coz $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008 Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#ifndef HEADER_LEADER_RESULT_H
#define HEADER_LEADER_RESULT_H

#include "base_gui.hpp"

class ssgBranch;
class ssgTransform;
class ssgContext;

class LeaderResult: public BaseGUI
{
private:
    ssgContext* m_context;
    ssgTransform* m_kart;
    int m_current_kart;
    int m_kart_name_label;
    char* m_score;
    float m_clock;

public:
    LeaderResult();
    ~LeaderResult();

    void update(float dt);
    void select();
    void handle(GameAction ga, int value);
};

#endif
