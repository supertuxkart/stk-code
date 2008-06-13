//  $Id$
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

#ifndef HEADER_PLAYERCONTROLS_H
#define HEADER_PLAYERCONTROLS_H

#include <string>
#include <SDL/SDL.h>
#include "base_gui.hpp"
#include "player.hpp"

class PlayerControls: public BaseGUI
{
public:
    PlayerControls(int whichPlayer);
    ~PlayerControls();

    void select();
    void clearMapping();

	void handle(GameAction, int);
	void inputKeyboard(SDLKey, int);
	void addKeyLabel(int change_id, KartAction control, bool start);
	void setKeyInfoString(KartAction control);

private:
	void updateAllKeyLabels();

    int m_player_index;
    bool m_grab_input;

	/** Stores the KartAction for which the input is being sensed. */
    KartAction m_edit_action;

    std::string m_name;
    std::string m_key_names[KC_COUNT];

	static const size_t PLAYER_NAME_MAX;
};

#endif
