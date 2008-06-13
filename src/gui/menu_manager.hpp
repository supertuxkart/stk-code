//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Patrick Ammann <pammann@aro.ch>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
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
#include "gui/race_gui.hpp"
#include "gui/base_gui.hpp"

enum MenuManagerIDs
{
    // menu
    MENUID_MAINMENU,
    MENUID_CHARSEL_P1,
    MENUID_CHARSEL_P2,
    MENUID_CHARSEL_P3,
    MENUID_CHARSEL_P4,
    MENUID_CHALLENGES,
    MENUID_RACE_OPTIONS,
    MENUID_GAMEMODE,
    MENUID_RACERESULT,
    MENUID_LEADERRESULT,
    MENUID_GRANDPRIXEND,
    MENUID_RACEMENU,
    MENUID_TRACKSEL,
    MENUID_NUMPLAYERS,
    MENUID_OPTIONS,
    MENUID_EXITGAME,
    MENUID_GRANDPRIXSELECT,
    MENUID_UNLOCKED_FEATURE,
    MENUID_START_RACE_FEEDBACK,

    // menu configuration
    MENUID_CONFIG_DISPLAY,
    MENUID_RESOLUTION_CONFIRM_FS,
    MENUID_RESOLUTION_CONFIRM_WIN,
    MENUID_CONFIG_SOUND,
    MENUID_CONFIG_CONTROLS,
    MENUID_CONFIG_P1,
    MENUID_CONFIG_P2,
    MENUID_CONFIG_P3,
    MENUID_CONFIG_P4,

    // help and credit menu
    MENUID_HELP1,
    MENUID_HELP2,
    MENUID_HELP3,
    MENUID_CREDITS,

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
    void pushMenu(MenuManagerIDs);
    void popMenu();

    int getMenuStackSize() {return (int)m_menu_stack.size();}

    bool isCurrentMenu(MenuManagerIDs id) {return (m_menu_stack.back().first == id);}
    bool isSomewhereOnStack(MenuManagerIDs id);
    BaseGUI* getCurrentMenu() const {return m_current_menu;}
    RaceGUI* getRaceMenu   () const {return (RaceGUI*)m_RaceGUI;}

    void update();

private:
    std::vector< std::pair<MenuManagerIDs, int> > m_menu_stack;
    BaseGUI* m_current_menu;
    BaseGUI* m_RaceGUI;
    bool m_change_menu;
};

extern MenuManager* menu_manager;

#endif // HEADER_MENUMANAGER_H
