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
#include "MainMenu.h"
#include "CharSel.h"
#include "Difficulty.h"
#include "GameMode.h"
#include "Options.h"
#include "TrackSel.h"
#include "NumLaps.h"
#include "NumPlayers.h"
#include "ConfigControls.h"
#include "ConfigDisplay.h"
#include "ConfigSound.h"
#include "PlayerControls.h"
#include "RaceGUI.h"
#include "RaceResultsGUI.h"
#include "RaceManager.h"
#include "ScreenManager.h"
#include "StartScreen.h"
#include "RaceMenu.h"
#include "WidgetSet.h"

BaseGUI *gui=NULL;
std::vector<GUISwitch> guiStack;

void updateGUI() {
  static unsigned int rememberSize = 0;

  if (rememberSize != guiStack.size()) {
    delete gui;
    gui = 0;

    rememberSize = guiStack.size();
    if (guiStack.size()) {
      switch (guiStack.back()) {
        case GUIS_MAINMENU:       gui = new MainMenu;                     break;
        case GUIS_CHARSEL:        gui = new CharSel(0);                   break;
        case GUIS_CHARSELP2:      gui = new CharSel(1);                   break;
        case GUIS_CHARSELP3:      gui = new CharSel(2);                   break;
        case GUIS_CHARSELP4:      gui = new CharSel(3);                   break;
        case GUIS_DIFFICULTY:     gui = new Difficulty();                 break;
        case GUIS_GAMEMODE:       gui = new GameMode();                   break;
        case GUIS_OPTIONS:        gui = new Options;                      break;
        case GUIS_TRACKSEL:       gui = new TrackSel();                   break;
        case GUIS_NUMLAPS:        gui = new NumLaps();                    break;
        case GUIS_NUMPLAYERS:     gui = new NumPlayers();                 break;
        case GUIS_CONFIGCONTROLS: gui = new ConfigControls;               break;
        case GUIS_CONFIGP1:       gui = new PlayerControls(0);            break;
        case GUIS_CONFIGP2:       gui = new PlayerControls(1);            break;
        case GUIS_CONFIGP3:       gui = new PlayerControls(2);            break;
        case GUIS_CONFIGP4:       gui = new PlayerControls(3);            break;
        case GUIS_CONFIGDISPLAY:  gui = new ConfigDisplay();              break;
        case GUIS_CONFIGSOUND:    gui = new ConfigSound();                break;
        case GUIS_RACE:           gui = new RaceGUI;                      break;
        case GUIS_RACERESULT:     gui = new RaceResultsGUI();             break;
        case GUIS_NEXTRACE:       race_manager->next();                   break;
        case GUIS_RACEMENU:       gui = new RaceMenu;                     break;
        case GUIS_EXITRACE:       guiStack.clear(); race_manager->next(); break;
      }   // switch
    }   // if guiStack.size()
  }   // if rememberSize!=guiStack.size()

  static ulClock now  = ulClock();

  now.update();
  if (gui) gui -> update( now.getDeltaTime());

  if(guiStack.empty()) screen_manager->abort();
}   // updateGUI

// -----------------------------------------------------------------------------
void BaseGUI::keybd(int key) {
  switch ( key ) {
    case PW_KEY_LEFT:
    case PW_KEY_RIGHT:
    case PW_KEY_UP:
    case PW_KEY_DOWN: widgetSet->pulse(widgetSet->cursor(menu_id, key), 1.2f);
                      break;

    case ' ' :
    case '\r': select(); break;
    case 27:   guiStack.pop_back(); break;   // ESC
    default: break;
  }   // switch
}   // keybd

// -----------------------------------------------------------------------------
void BaseGUI::point(int x, int y) {
    if(widgetSet)
	widgetSet -> pulse(widgetSet -> point(menu_id, x, y), 1.2f);
}   // point

// -----------------------------------------------------------------------------
void BaseGUI::stick(const int &whichAxis, const float &value) {
    if(widgetSet)
	widgetSet -> pulse(widgetSet -> stick(menu_id, whichAxis, (int)value), 1.2f);
}   // stick

// -----------------------------------------------------------------------------
void BaseGUI::joybuttons( int whichJoy, int hold, int presses, int releases ) {
    (void)whichJoy; (void)hold; (void)releases;

    if( presses & 2 ) select();

    if (guiStack.size() > 1 && (presses & 1)) guiStack.pop_back();
}   // joybuttons

/* EOF */
