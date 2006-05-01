//  $Id: ConfigControls.cxx,v 1.1 2005/05/25 21:47:54 joh Exp $
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

#include "ConfigControls.h"
#include "WidgetSet.h"

ConfigControls::ConfigControls()
{
	menu_id = widgetSet -> vstack(0);
	widgetSet -> label(menu_id, "Edit controls for which player?", GUI_LRG, GUI_ALL, 0, 0);

	int va = widgetSet -> varray(menu_id);
	widgetSet -> start(va, "Player 1",  GUI_MED, 1, 0);
	widgetSet -> state(va, "Player 2",  GUI_MED, 2, 0);
	widgetSet -> state(va, "Player 3",  GUI_MED, 3, 0);
	widgetSet -> state(va, "Player 4",  GUI_MED, 4, 0);

	widgetSet -> layout(menu_id, 0, 0);
}

ConfigControls::~ConfigControls()
{
	widgetSet -> delete_widget(menu_id) ;
}

void ConfigControls::update(float dt)
{
	widgetSet -> timer(menu_id, dt) ;
  // This menu can be triggered from the game, when it is paused
  // so we have to check it and draw it as in pause
  if(widgetSet -> get_paused())
    widgetSet -> blank() ;

  widgetSet -> paint(menu_id) ;
}

void ConfigControls::select()
{
	switch ( widgetSet -> token (widgetSet -> click()) )
	{
	case 1:
		guiStack.push_back(GUIS_CONFIGP1);
		break;
	case 2:
		guiStack.push_back(GUIS_CONFIGP2);
		break;
	case 3:
		guiStack.push_back(GUIS_CONFIGP3);
		break;
	case 4:
		guiStack.push_back(GUIS_CONFIGP4);
		break;
	default: break;
	}
}



