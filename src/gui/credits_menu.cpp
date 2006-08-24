//  $Id: credits_menu.hpp, v 1.3 2005/05/31 00:49:50 joh Exp $
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

#include "credits_menu.hpp"
#include "widget_set.hpp"
#include "race_manager.hpp"
#include "menu_manager.hpp"
#include "config.hpp"
#include "player.hpp"

CreditsMenu::CreditsMenu(){
  menu_id = widgetSet->vstack(0);
  widgetSet->multi(menu_id, 
"Project leader:\n\
Joerg Henrichs (hiker); Eduardo Hernandez Munoz (coz)\n\
Developers:\n\
Patrick Ammann, ???\n\
Original Tuxkart Developer:\n\
Steve Baker\n\
GotM Team:\n\
Ingo Ruhnke (grumbel), ???\n\
Art work:\n\
Steve Baker, Oliver Baker, ???\n\
Music:\n\
Matt Thomas", GUI_SML);
  widgetSet->start(menu_id,"Press <ESC> to go back", GUI_SML, 1, 0);
  widgetSet->layout(menu_id, 0, 0);
}   // CreditsMenu

// -----------------------------------------------------------------------------
CreditsMenu::~CreditsMenu() {
  widgetSet -> delete_widget(menu_id) ;
}   // ~CreditsMenu
	
// -----------------------------------------------------------------------------
void CreditsMenu::select() {
  // must be esc, nothing else is available. So just pop this menu
  menu_manager->popMenu();
}   // select

// -----------------------------------------------------------------------------
/* EOF */
