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

#include "config_display.hpp"
#include "widget_set.hpp"
#include "config.hpp"
#include "menu_manager.hpp"
#include "sdldrv.hpp"

enum WidgetTokens {
  WTOK_FULLSCREEN, WTOK_BACK
};

ConfigDisplay::ConfigDisplay() {
  CreateMenu();
}

// -----------------------------------------------------------------------------
void ConfigDisplay::CreateMenu() {
  menu_id = widgetSet -> vstack(0);
  widgetSet -> label(menu_id, "Display Settings", GUI_LRG, GUI_ALL, 0, 0);

  int va = widgetSet -> varray(menu_id);
  fullscreen_menu_id = widgetSet -> start(va, "Fullscreen mode",  GUI_MED, 
					  WTOK_FULLSCREEN);
  
  if(config->fullscreen)
    widgetSet->set_label(fullscreen_menu_id, "Window mode");
  widgetSet -> space(va);
  widgetSet -> state(va, "Press <ESC> to go back", GUI_SML, WTOK_BACK);
  widgetSet -> layout(menu_id, 0, 0);
}   // CreateMenu

// -----------------------------------------------------------------------------
ConfigDisplay::~ConfigDisplay() {
  widgetSet -> delete_widget(menu_id) ;
}

// -----------------------------------------------------------------------------
void ConfigDisplay::update(float dt) {
  widgetSet -> timer(menu_id, dt) ;
#if 0
  // This menu can be triggered from the game, when it is paused
  // so we have to check it and draw it as in pause
  if(widgetSet -> get_paused())
    widgetSet -> blank() ;
#endif
  widgetSet -> paint(menu_id) ;
}

// -----------------------------------------------------------------------------
void ConfigDisplay::select() {
  switch ( widgetSet -> token (widgetSet -> click()) )  {
  case WTOK_FULLSCREEN:
    drv_toggleFullscreen();
    widgetSet -> delete_widget(menu_id) ;
    // Since changing the video mode in drv_toggleFullscreen deletes all
    // display lists, textures etc., we have to load the menu again.
    // drv_toggleFullscreen takes care of general material, general
    // widgetSet, etc.
    CreateMenu();
    if(config->fullscreen)
      widgetSet->set_label(fullscreen_menu_id, "Window mode");
    else
      widgetSet->set_label(fullscreen_menu_id, "Fullscreen mode");
    break;
  case WTOK_BACK:
    menu_manager->popMenu();
    break;
  default: break;
  }
}



