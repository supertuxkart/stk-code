//  $Id: PlayerControls.h,v 1.3 2005/07/13 17:17:47 joh Exp $
//
//  TuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
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
	void keybd(int key);
	void point(int x, int y);
	void stick(const int &whichAxis, const float &value);
	void joybuttons(int whichJoy, int hold, int presses, int releases);
	void addKeyLabel(int change_id, KartActions control, bool start);
	void changeKeyLabel(int grab_id, KartActions control);
	void setKeyInfoString(KartActions control);

private:
	int grab_id;
	int player_index;
	bool grabInput;
	KartActions editAction;
	// Stores the heading - making this an attribute here avoids
	// memory leaks or complicated memory management
	char Heading[60];
	std::string KeyNames[KC_FIRE+1];
};

#endif
