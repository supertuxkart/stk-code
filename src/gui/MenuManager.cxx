//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Patrick Ammann <pammann@aro.ch>
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

#include "MenuManager.h"

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



MenuManager* menu_manager= new MenuManager();

MenuManager::MenuManager() {
  m_currentMenu= NULL;
  m_handeldSize= 0;
}

MenuManager::~MenuManager() {

}

void MenuManager::pushMenu(MenuManagerIDs id) {
  m_menuStack.push_back(id);
}

void MenuManager::popMenu() {
  m_menuStack.pop_back();
}

void MenuManager::clearMenus() {
  m_menuStack.clear();
}

void MenuManager::update() {

  if (m_handeldSize != m_menuStack.size()) {
    delete m_currentMenu;
    m_currentMenu= NULL;

    m_handeldSize= m_menuStack.size();
    if (m_handeldSize) {
      MenuManagerIDs id= m_menuStack.back();
      switch (id) {
        case MENUID_MAINMENU:
          m_currentMenu= new MainMenu();
          break;
        case MENUID_CHARSEL_P1:
        case MENUID_CHARSEL_P2:
        case MENUID_CHARSEL_P3:
        case MENUID_CHARSEL_P4:
          m_currentMenu= new CharSel(id - MENUID_CHARSEL_P1);
          break;
        case MENUID_DIFFICULTY:
          m_currentMenu= new Difficulty();
          break;
        case MENUID_GAMEMODE:
          m_currentMenu= new GameMode();
          break;
        case MENUID_OPTIONS:
          m_currentMenu= new Options();
          break;
        case MENUID_TRACKSEL:
          m_currentMenu= new TrackSel();
                   break;
        case MENUID_NUMLAPS:
          m_currentMenu= new NumLaps();
          break;
        case MENUID_NUMPLAYERS:
          m_currentMenu= new NumPlayers();
          break;
        case MENUID_RACE:
          m_currentMenu= new RaceGUI();
          break;
        case MENUID_RACERESULT:
          m_currentMenu= new RaceResultsGUI();
          break;
        case MENUID_NEXTRACE:
          race_manager->next();
          break;
        case MENUID_RACEMENU:
          m_currentMenu= new RaceMenu();
          break;
        case MENUID_EXITRACE:
          m_menuStack.clear(); race_manager->next();
          break;

        case MENUID_CONFIG_CONTROLS:
          m_currentMenu= new ConfigControls();
          break;
        case MENUID_CONFIG_P1:
        case MENUID_CONFIG_P2:
        case MENUID_CONFIG_P3:
        case MENUID_CONFIG_P4:
          m_currentMenu= new PlayerControls(id - MENUID_CONFIG_P1);
          break;
        case MENUID_CONFIG_DISPLAY:
          m_currentMenu= new ConfigDisplay();
          break;
        case MENUID_CONFIG_SOUND:
          m_currentMenu= new ConfigSound();
          break;
        default:
          break;
      }   // switch
    }   // if guiStack.size()
  }   // if rememberSize!=guiStack.size()

  static ulClock now  = ulClock();
  now.update();

  if (m_currentMenu != NULL) {
    m_currentMenu->update(now.getDeltaTime());
  }

  if (m_menuStack.empty()) {
    screen_manager->abort();
  }
}   // update
