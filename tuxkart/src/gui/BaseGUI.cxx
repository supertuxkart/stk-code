//  $Id: BaseGUI.cxx,v 1.24 2004/10/11 13:40:07 jamesgregory Exp $
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
#include "MainMenu.h"
#include "CharSel.h"
#include "Difficulty.h"
#include "GameMode.h"
#include "Options.h"
#include "TrackSel.h"
#include "NumPlayers.h"
#include "ConfigControls.h"
#include "PlayerControls.h"
#include "RaceGUI.h"
#include "RaceManager.h"
#include "ScreenManager.h"
#include "StartScreen.h"
#include "RaceMenu.h"
#include "WidgetSet.h"

void updateGUI()
{
	static unsigned int rememberSize = 0;
	
	if (rememberSize != guiStack.size())
	{
		delete gui;
		gui = 0;
		
		rememberSize = guiStack.size();
		
		if (guiStack.size())
		{		
			switch (guiStack.back())
			{
			case GUIS_MAINMENU:
				gui = new MainMenu;
				break;
			case GUIS_CHARSEL:
				gui = new CharSel(0);
				break;
			case GUIS_CHARSELP2:
				gui = new CharSel(1);
				break;
			case GUIS_CHARSELP3:
				gui = new CharSel(2);
				break;
			case GUIS_CHARSELP4:
				gui = new CharSel(3);
				break;
			case GUIS_DIFFICULTYGP:
			case GUIS_DIFFICULTYSR:
				gui = new Difficulty();
				break;
			case GUIS_GAMEMODE:
				gui = new GameMode();
				break;
			case GUIS_OPTIONS:
				gui = new Options;
				break;
			case GUIS_TRACKSEL:
				gui = new TrackSel();
				break;
			case GUIS_NUMPLAYERS:
				gui = new NumPlayers();
				break;
			case GUIS_CONFIGCONTROLS:
				gui = new ConfigControls;
				break;
			case GUIS_CONFIGP1:
				gui = new PlayerControls(0);
				break;
			case GUIS_CONFIGP2:
				gui = new PlayerControls(1);
				break;
			case GUIS_CONFIGP3:
				gui = new PlayerControls(2);
				break;
			case GUIS_CONFIGP4:
				gui = new PlayerControls(3);
				break;
			
			case GUIS_RACE:
				gui = new RaceGUI;
				break;
			case GUIS_RACEMENU:
				gui = new RaceMenu;
				break;
			case GUIS_EXITRACE:
				guiStack.clear();
                                RaceManager::instance()->next();
                                break;
                        }
		}
		
		//something somewhere (most likely in the WidgetSet stuff) means the cursor will get enabled again before the game starts if you just call this when the game starts
		SDL_ShowCursor(SDL_DISABLE);
	}
	
	static int then = SDL_GetTicks();
	int now = SDL_GetTicks();
	
	if (gui)
		gui -> update( (now - then) / 1000.f );
		
	then = now;
	
	if(guiStack.empty())
		screenManager->abort();
}

void BaseGUI::keybd(const SDL_keysym& key)
{
	switch ( key.sym )
	{
	case SDLK_LEFT:
	case SDLK_RIGHT:
	case SDLK_UP:
	case SDLK_DOWN:
		widgetSet -> pulse(widgetSet -> cursor(menu_id, key.sym), 1.2f);
		break;
		
	case SDLK_RETURN: select(); break;
	
	case SDLK_ESCAPE:
		guiStack.pop_back();
		
	default: break;
	}
}

void BaseGUI::point(int x, int y)
{
	widgetSet -> pulse(widgetSet -> point(menu_id, x, y), 1.2f);
}

void BaseGUI::stick(int whichAxis, int value)
{
	widgetSet -> pulse(widgetSet -> stick(menu_id, whichAxis, value), 1.2f);
}

void BaseGUI::joybutton(int whichJoy, int button)
{
    if (button == 0)
        select();
    else if (guiStack.size() > 1 && button == 1)
        guiStack.pop_back();
}

