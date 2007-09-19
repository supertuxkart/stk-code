//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
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

#ifndef HEADER_PLAYERCONTROLS_H
#define HEADER_PLAYERCONTROLS_H

#include "base_gui.hpp"
#include "player.hpp"

#include <string>

class PlayerControls: public BaseGUI
{
public:
    PlayerControls(int whichPlayer);
    ~PlayerControls();

    void select();
    void input(InputType type, int id0, int id1, int id2, int value);
    void addKeyLabel(int change_id, KartActions control, bool start);
    void changeKeyLabel(int grab_id, KartActions control);
    void setKeyInfoString(KartActions control);

private:
    int m_grab_id;
    int m_player_index;
    int m_name_id;
    bool m_grab_input;
    KartActions m_edit_action;
    // Stores the heading - making this an attribute here avoids
    // memory leaks or complicated memory management
    char m_heading[60];
    std::string m_name;
    std::string m_key_names[KC_LAST+1];
};

#endif
