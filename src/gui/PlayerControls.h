//  $Id$
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

#include "BaseGUI.h"
#include "Player.h"

#include <string>

class PlayerControls: public BaseGUI
{
public:
	PlayerControls(int whichPlayer);
	~PlayerControls();
	
	void update(float dt);
	void select();
	void keybd(const SDL_keysym& key);
	void point(int x, int y);
	void stick(int x, int y);
      void joybutton(int whichJoy, int button);
      void addKeyLabel(int change_id, KartControl control, bool start);
      void changeKeyLabel(int grab_id, KartControl control);
      std::string getKeyInfoString(KartControl control);
	
private:
	int grab_id;
	int config_index;
	bool grabInput;
	KartControl editKey;
};

#endif
