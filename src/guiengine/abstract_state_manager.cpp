//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Marianne Gagnon
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


#include "guiengine/abstract_state_manager.hpp"

#include <vector>

#include "main_loop.hpp"
#include "audio/sound_manager.hpp"
#include "config/player.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/modaldialog.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widget.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "io/file_manager.hpp"
#include "network/network_manager.hpp"
#include "race/race_manager.hpp"
#include "states_screens/options_screen.hpp"
#include "states_screens/kart_selection.hpp"
#include "states_screens/credits.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;


/**
* Name of the event sent when constructing a menu
*/
const std::string g_init_event = "init";

/**
 * Name of the event sent when destructing a menu
 */
const std::string g_teardown_event = "tearDown";

AbstractStateManager::AbstractStateManager()
{
    m_game_mode = false;
}

#if 0
#pragma mark -
#pragma mark Other
#endif

/*
void initGUI()
{
    IrrlichtDevice* device = irr_driver->getDevice();
    video::IVideoDriver* driver = device->getVideoDriver();
    GUIEngine::init(device, driver, &eventCallback);
}
 */

void AbstractStateManager::enterGameState()
{
    m_menu_stack.clear();
    m_menu_stack.push_back("race");
    m_game_mode = true;
    cleanForGame();
    input_manager->setMode(InputManager::INGAME);
}

bool AbstractStateManager::isGameState()
{
    return m_game_mode;
}


#if 0
#pragma mark -
#pragma mark Push/pop menus
#endif

void AbstractStateManager::pushMenu(std::string name)
{
    // Send tear-down event to previous menu
    if (m_menu_stack.size() > 0) eventCallback(NULL, g_teardown_event);
    
    input_manager->setMode(InputManager::MENU);
    m_menu_stack.push_back(name);
    m_game_mode = false;
    switchToScreen(name.c_str());
    
    // Send init event to new menu
    eventCallback(NULL, g_init_event);
}
void AbstractStateManager::replaceTopMostMenu(std::string name)
{
    // Send tear-down event to previous menu
    if (m_menu_stack.size() > 0) eventCallback(NULL, g_teardown_event);
    
    input_manager->setMode(InputManager::MENU);
    m_menu_stack[m_menu_stack.size()-1] = name;
    m_game_mode = false;
    switchToScreen(name.c_str());
    
    // Send init event to new menu
    eventCallback(NULL, g_init_event);
}

void AbstractStateManager::reshowTopMostMenu()
{
    // Send tear-down event to previous menu
    if (m_menu_stack.size() > 0) eventCallback(NULL, g_teardown_event);
    
    switchToScreen( m_menu_stack[m_menu_stack.size()-1].c_str() );
    
    // Send init event to new menu
    eventCallback(NULL, g_init_event);
}

void AbstractStateManager::popMenu()
{
    // Send tear-down event to menu
    eventCallback(NULL, g_teardown_event);
    m_menu_stack.pop_back();
    
    if(m_menu_stack.size() == 0)
    {
        main_loop->abort();
        return;
    }
    
    m_game_mode = m_menu_stack[m_menu_stack.size()-1] == "race";
    
    std::cout << "-- switching to screen " << m_menu_stack[m_menu_stack.size()-1].c_str() << std::endl;
    switchToScreen(m_menu_stack[m_menu_stack.size()-1].c_str());
    input_manager->getDeviceList()->setAssignMode(NO_ASSIGN); // No assign mode on menus by default
    eventCallback(NULL, g_init_event);
}

void AbstractStateManager::resetAndGoToMenu(std::string name)
{
    race_manager->exitRace();
    input_manager->setMode(InputManager::MENU);
    m_menu_stack.clear();
    m_menu_stack.push_back(name);
    m_game_mode = false;
    sound_manager->positionListener( Vec3(0,0,0), Vec3(0,1,0) );
    switchToScreen(name.c_str());
    eventCallback(NULL, g_init_event);
}

