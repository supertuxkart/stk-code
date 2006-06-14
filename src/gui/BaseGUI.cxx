//  $Id: BaseGUI.cxx,v 1.5 2005/08/19 20:51:07 joh Exp $
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

#include <plib/pw.h>

#include "BaseGUI.h"
#include "WidgetSet.h"
#include "MenuManager.h"

void BaseGUI::keybd(int key) {
  switch ( key ) {
    case PW_KEY_LEFT:
    case PW_KEY_RIGHT:
    case PW_KEY_UP:
    case PW_KEY_DOWN:
      widgetSet->pulse(widgetSet->cursor(menu_id, key), 1.2f);
      break;

    case ' ' :
    case '\r':
      select();
      break;
    
    case 27:  // ESC
      menu_manager->popMenu();
      break;

    default:
      break;
  }   // switch
}   // keybd

void BaseGUI::point(int x, int y) {
  if(widgetSet) {
    widgetSet -> pulse(widgetSet -> point(menu_id, x, y), 1.2f);
  }
}   // point

void BaseGUI::stick(const int &whichAxis, const float &value) {
  if(widgetSet) {
    widgetSet -> pulse(widgetSet -> stick(menu_id, whichAxis, (int)value), 1.2f);
  }
}   // stick

void BaseGUI::joybuttons( int whichJoy, int hold, int presses, int releases ) {
  (void)whichJoy; (void)hold; (void)releases;

  if( presses & 2 ) {
    select();
  }

  if (presses & 1) {
    menu_manager->popMenu();
  }
}   // joybuttons

/* EOF */
