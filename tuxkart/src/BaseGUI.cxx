//  $Id: BaseGUI.cxx,v 1.3 2004/08/05 18:33:00 jamesgregory Exp $
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

#include "BaseGUI.h"
#include "tuxkart.h"
#include "RaceGUI.h"
#include "MainMenu.h"

void updateGUI()
{
	if (guiSwitch != GUIS_CURRENT)
	{
		delete gui;
		gui = NULL;
		
		switch (guiSwitch)
		{
		case GUIS_CURRENT:
			break;
		case GUIS_MAINMENU:
			gui = new MainMenu;
			break;
		
		case GUIS_RACE:
			gui = new RaceGUI;
			break;
		}
		
		guiSwitch = GUIS_CURRENT;
		SDL_ShowCursor(SDL_DISABLE);
	}
	
	static int then = SDL_GetTicks();
	int now = SDL_GetTicks();
	
	glEnable(GL_TEXTURE_2D);
	
	if (gui)
		gui -> update( (now - then) / 1000.f );
		
	then = now;
}

/*---------------------------------------------------------------------------*/
