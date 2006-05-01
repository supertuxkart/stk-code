//  $Id: ConfigDisplay.cxx,v 1.2 2005/05/27 10:25:52 joh Exp $
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

#include "ConfigDisplay.h"
#include "WidgetSet.h"
#include "Config.h"

ConfigDisplay::ConfigDisplay()
{
	menu_id = widgetSet -> vstack(0);
	widgetSet -> label(menu_id, "Display Settings", GUI_LRG, GUI_ALL, 0, 0);

	int va = widgetSet -> varray(menu_id);
	fullscreen_menu_id = widgetSet -> start(va, "Fullscreen mode",  GUI_MED, MENU_FULLSCREEN_TOGGLE, 0);

	widgetSet -> layout(menu_id, 0, 0);

  if(config->fullscreen)
    widgetSet->set_label(fullscreen_menu_id, "Window mode");
}

ConfigDisplay::~ConfigDisplay()
{
	widgetSet -> delete_widget(menu_id) ;
}

void ConfigDisplay::update(float dt)
{
	widgetSet -> timer(menu_id, dt) ;
  // This menu can be triggered from the game, when it is paused
  // so we have to check it and draw it as in pause
  if(widgetSet -> get_paused())
    widgetSet -> blank() ;
	widgetSet -> paint(menu_id) ;
}

void ConfigDisplay::select()
{
	switch ( widgetSet -> token (widgetSet -> click()) )
	{
	case MENU_FULLSCREEN_TOGGLE:
	  //JHtoggle_fullscreen();
    if(config->fullscreen)
      widgetSet->set_label(fullscreen_menu_id, "Window mode");
    else
      widgetSet->set_label(fullscreen_menu_id, "Fullscreen mode");

    break;
	default: break;
	}
}



