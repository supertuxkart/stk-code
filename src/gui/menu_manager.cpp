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

#include <assert.h>

#include "menu_manager.hpp"

#include "main_menu.hpp"
#include "char_sel.hpp"
#include "difficulty.hpp"
#include "game_mode.hpp"
#include "options.hpp"
#include "track_sel.hpp"
#include "num_laps.hpp"
#include "num_players.hpp"
#include "config_controls.hpp"
#include "config_display.hpp"
#include "config_sound.hpp"
#include "player_controls.hpp"
#include "race_gui.hpp"
#include "race_results_gui.hpp"
#include "grand_prix_ending.hpp"
#include "race_manager.hpp"
#include "screen_manager.hpp"
#include "start_screen.hpp"
#include "race_menu.hpp"
#include "help_menu.hpp"
#include "credits_menu.hpp"
#include "grand_prix_select.hpp"

MenuManager* menu_manager= new MenuManager();

MenuManager::MenuManager()
{
    m_current_menu= NULL;
    m_handled_size= 0;
}

//-----------------------------------------------------------------------------
MenuManager::~MenuManager()
{
    delete m_current_menu;
}

//-----------------------------------------------------------------------------
void MenuManager::pushMenu(MenuManagerIDs id)
{
    m_menu_stack.push_back(id);
}

//-----------------------------------------------------------------------------
void MenuManager::popMenu()
{
    m_menu_stack.pop_back();
}

//-----------------------------------------------------------------------------
void MenuManager::update()
{

    if (m_handled_size != m_menu_stack.size())
    {
        delete m_current_menu;
        m_current_menu= NULL;

        m_handled_size= m_menu_stack.size();
        if (m_handled_size > 0)
        {
            MenuManagerIDs id= m_menu_stack.back();
            switch (id)
            {
            case MENUID_MAINMENU:
                m_current_menu= new MainMenu();
                break;
            case MENUID_CHARSEL_P1:
            case MENUID_CHARSEL_P2:
            case MENUID_CHARSEL_P3:
            case MENUID_CHARSEL_P4:
                m_current_menu= new CharSel(id - MENUID_CHARSEL_P1);
                break;
            case MENUID_DIFFICULTY:
                m_current_menu= new Difficulty();
                break;
            case MENUID_GAMEMODE:
                m_current_menu= new GameMode();
                break;
            case MENUID_OPTIONS:
                m_current_menu= new Options();
                break;
            case MENUID_TRACKSEL:
                m_current_menu= new TrackSel();
                break;
            case MENUID_NUMLAPS:
                m_current_menu= new NumLaps();
                break;
            case MENUID_NUMPLAYERS:
                m_current_menu= new NumPlayers();
                break;
            case MENUID_RACE:
                m_current_menu= new RaceGUI();
                break;
            case MENUID_RACERESULT:
                m_current_menu= new RaceResultsGUI();
                break;
            case MENUID_GRANDPRIXEND:
                m_current_menu= new GrandPrixEnd();
                break;
            case MENUID_GRANDPRIXSELECT:
                m_current_menu= new GrandPrixSelect();
                break;
#if 0
            case MENUID_NEXTRACE:
                race_manager->next();
                break;
#endif
            case MENUID_RACEMENU:
                m_current_menu= new RaceMenu();
                break;
            case MENUID_EXITGAME:
                m_menu_stack.clear();
                screen_manager->abort();
                break;

            case MENUID_CONFIG_CONTROLS:
                m_current_menu= new ConfigControls();
                break;
            case MENUID_CONFIG_P1:
            case MENUID_CONFIG_P2:
            case MENUID_CONFIG_P3:
            case MENUID_CONFIG_P4:
                m_current_menu= new PlayerControls(id - MENUID_CONFIG_P1);
                break;
            case MENUID_CONFIG_DISPLAY:
                m_current_menu= new ConfigDisplay();
                break;
            case MENUID_CONFIG_SOUND:
                m_current_menu= new ConfigSound();
                break;
            case MENUID_HELP:
                m_current_menu = new HelpMenu();
                break;
            case MENUID_CREDITS:
                m_current_menu = new CreditsMenu();
                break;
            default:
                break;
            }   // switch
        }
    }

    static ulClock now  = ulClock();
    now.update();

    if (m_current_menu != NULL)
    {
        m_current_menu->update(now.getDeltaTime());
    }
}   // update

//-----------------------------------------------------------------------------
void MenuManager::switchToGrandPrixEnding()
{
    if (m_current_menu != NULL)
    {
        delete m_current_menu;
        m_current_menu= NULL;
    }
    m_handled_size= 0;

    m_menu_stack.clear();
    pushMenu(MENUID_GRANDPRIXEND);
}

//-----------------------------------------------------------------------------
void MenuManager::switchToRace()
{
    m_menu_stack.clear();
    pushMenu(MENUID_RACE);
}

//-----------------------------------------------------------------------------
// Returns true if the id is somewhere on the stack. This can be used to detect
// if the config_display menu was called from the race_gui, or main_menu
bool MenuManager::isSomewhereOnStack(MenuManagerIDs id)
{
    return std::find(m_menu_stack.begin(), m_menu_stack.end(), id) !=
           m_menu_stack.end();
}   // isSomewhereOnStack

// -----------------------------------------------------------------------------
void MenuManager::switchToMainMenu()
{
    if (m_current_menu != NULL)
    {
        delete m_current_menu;
        m_current_menu= NULL;
    }
    m_handled_size= 0;

    m_menu_stack.clear();
    pushMenu(MENUID_MAINMENU);
}
