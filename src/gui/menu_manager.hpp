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

#ifndef HEADER_MENUMANAGER_H
#define HEADER_MENUMANAGER_H

#include <vector>
#include "gui/base_gui.hpp"

enum MenuManagerIDs
{
  // menu
  MENUID_MAINMENU,
  MENUID_CHARSEL_P1,
  MENUID_CHARSEL_P2,
  MENUID_CHARSEL_P3,
  MENUID_CHARSEL_P4,
  MENUID_DIFFICULTY,
  MENUID_GAMEMODE,
  MENUID_RACERESULT,
  MENUID_GRANDPRIXEND,
#if 0 // no needed yet
  MENUID_NEXTRACE,
#endif
  MENUID_RACEMENU,
  MENUID_TRACKSEL,
  MENUID_NUMLAPS,
  MENUID_NUMPLAYERS,
  MENUID_OPTIONS,
  MENUID_EXITGAME,

  // menu configuration
  MENUID_CONFIG_DISPLAY,
  MENUID_CONFIG_SOUND,
  MENUID_CONFIG_CONTROLS,
  MENUID_CONFIG_P1,
  MENUID_CONFIG_P2,
  MENUID_CONFIG_P3,
  MENUID_CONFIG_P4,
  
  // race gui
  MENUID_RACE,
};

class MenuManager
{
public:
  MenuManager();
  virtual ~MenuManager();

  // general functions
  void switchToGrandPrixEnding();
  void switchToRace();
  void switchToMainMenu();

  // use this function within menu classes
  void pushMenu(MenuManagerIDs id);
  void popMenu();

  int getMenuStackSize() {return m_menuStack.size();}

  bool isCurrentMenu(MenuManagerIDs id) {return (m_menuStack.back() == id);}
  BaseGUI* getCurrentMenu() {return m_currentMenu;}

  void update();

private:
  std::vector<MenuManagerIDs> m_menuStack;
  BaseGUI* m_currentMenu;
  unsigned int m_handeldSize;
};

extern MenuManager* menu_manager;

#endif // HEADER_MENUMANAGER_H
