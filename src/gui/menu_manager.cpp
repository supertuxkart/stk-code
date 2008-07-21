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

#include <cassert>

//This is needed in various platforms, but not all
# include <algorithm>

#include "menu_manager.hpp"

#include "main_menu.hpp"
#include "char_sel.hpp"
#include "game_mode.hpp"
#include "race_options.hpp"
#include "options.hpp"
#include "track_sel.hpp"
#include "num_players.hpp"
#include "config_controls.hpp"
#include "config_display.hpp"
#include "display_res_confirm.hpp"
#include "config_sound.hpp"
#include "player_controls.hpp"
#include "race_gui.hpp"
#include "race_results_gui.hpp"
#include "grand_prix_ending.hpp"
#include "race_manager.hpp"
#include "game_manager.hpp"
#include "race_menu.hpp"
#include "help_page_one.hpp"
#include "help_page_two.hpp"
#include "help_page_three.hpp"
#include "credits_menu.hpp"
#include "grand_prix_select.hpp"
#include "sound_manager.hpp"
#include "sdldrv.hpp"
#include "user_config.hpp"
#include "widget_manager.hpp"
#include "challenges_menu.hpp"
#include "feature_unlocked.hpp"
#include "start_race_feedback.hpp"

using namespace std;

MenuManager* menu_manager= new MenuManager();

MenuManager::MenuManager()
{
    m_current_menu = NULL;
    m_RaceGUI      = NULL;
    m_change_menu = false;
}

//-----------------------------------------------------------------------------
MenuManager::~MenuManager()
{
    delete m_current_menu;
}

/** Puts the given menu into the menu stack and saves the widgetToken of
  * the last selected widget for later reactivation.
  */
void MenuManager::pushMenu(MenuManagerIDs id)
{
    // If we store the selected widget when we have send the order to change
    // the menu but haven't done it, we would change the previous menu, not
    // the current menu.
	if ( m_menu_stack.size() && m_change_menu != true )
    {
		m_menu_stack.back().second = widget_manager->getSelectedWgt();
    }

	// used to suppress select-sound on startup
    static bool is_startup = true;

    if( MENUID_EXITGAME == id )
    {
        sound_manager->playSfx(SOUND_BACK_MENU);
    }
    else
    {
        if( !is_startup ) sound_manager->playSfx(SOUND_SELECT_MENU);
        else is_startup = false;
    }

	// Creates a new entry for the to be displayed menu.
	pair <MenuManagerIDs, int> element;
	element.first = id;
	element.second = WidgetManager::WGT_NONE;

    m_menu_stack.push_back(element);

    if( m_current_menu) m_current_menu->lockInput();
    m_change_menu = true;
}

//-----------------------------------------------------------------------------
void MenuManager::popMenu()
{
    sound_manager->playSfx(SOUND_BACK_MENU);

	m_menu_stack.pop_back();
    if( m_current_menu ) m_current_menu->lockInput();
    m_change_menu = true;
}

//-----------------------------------------------------------------------------
void MenuManager::update()
{

    if ( m_change_menu )
    {
        m_change_menu = false;

        if (m_RaceGUI
            && m_current_menu == m_RaceGUI)
        {
            m_RaceGUI = 0;
            inputDriver->setMode(MENU);
        }

        delete m_current_menu;
        m_current_menu= NULL;

        if (!m_menu_stack.empty())
        {
			pair<MenuManagerIDs, int> saved = m_menu_stack.back();
            MenuManagerIDs id = saved.first;
			int saved_widget = saved.second;

            switch (id)
            {
            case MENUID_MAINMENU:
                m_current_menu= new MainMenu();
                break;
            case MENUID_CHALLENGES:
                m_current_menu= new ChallengesMenu();
                break;
            case MENUID_CHARSEL_P1:
            case MENUID_CHARSEL_P2:
            case MENUID_CHARSEL_P3:
            case MENUID_CHARSEL_P4:
                m_current_menu= new CharSel(id - MENUID_CHARSEL_P1);
                break;
            case MENUID_RACE_OPTIONS:
                m_current_menu= new RaceOptions();
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
            case MENUID_UNLOCKED_FEATURE:
                m_current_menu = new FeatureUnlocked();
                break;
            case MENUID_NUMPLAYERS:
                m_current_menu= new NumPlayers();
                break;
            case MENUID_RACE:
                inputDriver->setMode(INGAME);
                m_current_menu = new RaceGUI();
                m_RaceGUI      = m_current_menu;
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
            case MENUID_RACEMENU:
                m_current_menu= new RaceMenu();
                break;
            case MENUID_EXITGAME:
                m_menu_stack.clear();
                game_manager->abort();
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
            case MENUID_RESOLUTION_CONFIRM_FS:
                {
                    const bool FROM_FULLSCREEN = false;
                    m_current_menu= new DisplayResConfirm( FROM_FULLSCREEN );
                }
                break;
            case MENUID_RESOLUTION_CONFIRM_WIN:
                {
                    const bool FROM_WINDOW = true;
                    m_current_menu= new DisplayResConfirm( FROM_WINDOW );
                }
                break;
            case MENUID_CONFIG_SOUND:
                m_current_menu= new ConfigSound();
                break;
            case MENUID_HELP1:
                m_current_menu = new HelpPageOne();
                break;
            case MENUID_HELP2:
                m_current_menu = new HelpPageTwo();
                break;
            case MENUID_HELP3:
                m_current_menu = new HelpPageThree();
                break;
            case MENUID_CREDITS:
                m_current_menu = new CreditsMenu();
                break;
            case MENUID_START_RACE_FEEDBACK:
                m_current_menu = new StartRaceFeedback();
            default:
                break;
            }   // switch


            if( id != MENUID_EXITGAME )
            {
                // Restores the previously selected widget if there was one.
                if (saved_widget != WidgetManager::WGT_NONE)
                {
                    widget_manager->lightenWgtColor( saved_widget );
                    widget_manager->pulseWgt( saved_widget );
                    widget_manager->setSelectedWgt(saved_widget);
                } else if( widget_manager->getSelectedWgt() != WidgetManager::WGT_NONE )
                {
                    widget_manager->lightenWgtColor (
                        widget_manager->getSelectedWgt() );
                }
            }
        }
    }

    static ulClock now  = ulClock();
    now.update();

    if (m_current_menu != NULL)
    {
        m_current_menu->update((float)now.getDeltaTime());
    }
}   // update

//----------k-------------------------------------------------------------------
void MenuManager::switchToGrandPrixEnding()
{
    if (m_current_menu != NULL)
    {
        if(m_current_menu==m_RaceGUI) m_RaceGUI=0;
        delete m_current_menu;
        m_current_menu= NULL;
    }

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
	for(vector<pair<MenuManagerIDs,int> >::iterator i = m_menu_stack.begin(); i != m_menu_stack.end(); i++)
	{
		if ((*i).first == id)
			return true;
	}
	
	return false;
}   // isSomewhereOnStack

// -----------------------------------------------------------------------------
void MenuManager::switchToMainMenu()
{
    if (m_current_menu != NULL)
    {
        if(m_current_menu == m_RaceGUI) m_RaceGUI = 0;
        delete m_current_menu;
        m_current_menu= NULL;
    }

    m_menu_stack.clear();
    pushMenu(MENUID_MAINMENU);
}
