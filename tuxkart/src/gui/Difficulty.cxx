//  $Id: Difficulty.cxx,v 1.13 2004/08/24 21:01:45 grumbel Exp $
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

#include "Difficulty.h"
#include "tuxkart.h"
#include "RaceManager.h"
#include "WidgetSet.h"

#include <algorithm>

Difficulty::Difficulty()
{
	menu_id = widgetSet -> vstack(0);

	widgetSet -> label(menu_id, "Choose a Difficulty", GUI_LRG, GUI_ALL, 0, 0);

	int va = widgetSet -> varray(menu_id);
	widgetSet -> start(va, "Easy", GUI_MED, MENU_EASY, 0);
	widgetSet -> state(va, "Medium", GUI_MED, MENU_MEDIUM, 0);
	widgetSet -> state(va, "Hard", GUI_MED, MENU_HARD, 0);
	
	if (0)
	{
		if (std::find(guiStack.begin(), guiStack.end(), GUIS_DIFFICULTYSR) != guiStack.end())
		{
			widgetSet -> state(menu_id, "Number of Laps", GUI_SML, 0, 0);
			widgetSet -> state(menu_id, "Reverse Track", GUI_SML, 0, 0);
#ifdef SSG_BACKFACE_COLLISIONS_SUPPORTED
			widgetSet -> state(menu_id, "Mirror Track", GUI_SML, 0, 0);
#endif
		}
	}

	widgetSet -> layout(menu_id, 0, 0);
}

Difficulty::~Difficulty()
{
	widgetSet -> delete_widget(menu_id) ;
}
	
void Difficulty::update(float dt)
{
	
	widgetSet -> timer(menu_id, dt) ;
	widgetSet -> paint(menu_id) ;
}

void Difficulty::select()
{
	switch ( widgetSet -> token (widgetSet -> click()) )
	{
	case MENU_EASY:
                RaceManager::instance()->setDifficulty(RD_EASY);
		guiStack.push_back(GUIS_CHARSEL);
		break;
	case MENU_MEDIUM:
                RaceManager::instance()->setDifficulty(RD_MEDIUM);
		guiStack.push_back(GUIS_CHARSEL);
		break;
	case MENU_HARD:
                RaceManager::instance()->setDifficulty(RD_HARD);
		guiStack.push_back(GUIS_CHARSEL);
		break;
	default: break;
	}
}

void Difficulty::keybd(const SDL_keysym& key)
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

void Difficulty::point(int x, int y)
{
	widgetSet -> pulse(widgetSet -> point(menu_id, x, y), 1.2f);
}

void Difficulty::stick(int x, int y)
{
	widgetSet -> pulse(widgetSet -> stick(menu_id, x, y), 1.2f);
}

