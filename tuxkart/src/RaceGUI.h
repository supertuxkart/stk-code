
//  $Id: RaceGUI.h,v 1.2 2004/08/05 16:47:18 jamesgregory Exp $
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

#ifndef HEADER_RACEGUI_H
#define HEADER_RACEGUI_H

#include "BaseGUI.h"

class RaceGUI: public BaseGUI
{
public:
	RaceGUI();
	~RaceGUI();
	
	void update(float dt);
	void click(int button, int x, int y) { (void)button; (void)x; (void)y; }
	void point(int x, int y) { (void)x; (void)y; }
	void stick(int x, int y) { (void)x; (void)y; }
};
#endif

