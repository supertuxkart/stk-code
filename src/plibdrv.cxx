//  $Id: plibdrv.cxx 252 2005-08-19 20:51:56Z joh $
//
//  SuperTuxKart - a fun racing game with go-kart
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
#include <plib/pu.h>
#include <plib/ssg.h>
#include <plib/js.h>

#include "Config.h"
#include "plibdrv.h"

#include "gui/MenuManager.h"
#include "KartControl.h"

/*********************************\
*                                 *
* These functions capture mouse   *
* and keystrokes and pass them on *
* to PUI.                         *
*                                 *
\*********************************/

void keyfn ( int key, int updown, int, int ) {
  puKeyboard ( key, updown ) ;
}

void gui_motionfn ( int x, int y ) {
  BaseGUI* menu= menu_manager->getCurrentMenu();
  if(menu != NULL) {
    menu->point(x,config->height-y);
  }
  puMouse ( x, y ) ;
}

void gui_mousefn ( int button, int updown, int x, int y ) {
  if (button==0 && updown==0) {
    BaseGUI* menu= menu_manager->getCurrentMenu();
    if (menu != NULL) {
      menu->select();
    }
  } else if (button==2 && updown==0) {
    menu_manager->popMenu();
  }
  puMouse(button, updown, x, y ) ;
}

static jsJoystick *joystick ;

void pollEvents() {
  BaseGUI* menu= menu_manager->getCurrentMenu();
  if(menu != NULL) {
    int k= getKeystroke();
    if(k) {
      menu->keybd(k);
    }   // if k

    if( !( joystick -> notWorking () ) )
    {
      static KartControl controls;
      int prev_buttons = controls.buttons;
      joystick->read ( &controls.buttons, controls.data) ;
      menu->stick( 0, controls.data[0]);
      menu->stick( 1, controls.data[1]);

      int changed_states = prev_buttons ^ controls.buttons;
      controls.presses = controls.buttons & changed_states;
      controls.releases = !controls.buttons & changed_states;
      menu->joybuttons(0, controls.buttons, controls.presses, controls.releases);
    }
  }
}

static unsigned int lastKeystroke = 0 ;

static char keyIsDown [ MAXKEYS ] ;

void keystroke ( int key, int updown, int, int ) {
  if ( updown == PW_DOWN )
    lastKeystroke = key ;

  keyIsDown [ key ] = (updown == PW_DOWN) ;
}


int isKeyDown ( unsigned int k ) {
  return keyIsDown [ k ] ;
}

int getKeystroke () {
  int k = lastKeystroke ;
  lastKeystroke = 0 ;
  return k ;
}

//printkeys() is not currently used, should be erased or used in debug mode.
void printkeys() {
  int flag=0;
  for(int i=0; i<512; i++) {
    if(keyIsDown[i]) {
      printf("%d, ",i);
      flag=1;
    }
  }
  if(flag) printf("\n");
}

void InitPlib() {
  pwInit ( 0, 0, config->width, config->height,
	   FALSE, "Tux Kart by Steve Baker", TRUE, 0 ) ;

  puInit () ;
  ssgInit () ;
  fntInit();
  jsInit();

  for ( int i = 0 ; i < 512 ; i++ )
    keyIsDown [ i ] = FALSE ;
  joystick = new jsJoystick ( 0 );
  if( joystick -> notWorking () ) config->player[0].setUseJoystick(false);
    else
  {
    joystick -> setDeadBand( 0, 0.1 );
    joystick -> setDeadBand( 1, 0.1 );
  }
}
