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
#include "guiengine/engine.hpp"
#include "guiengine/screen.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "modes/world.hpp"

using namespace GUIEngine;


AbstractStateManager::AbstractStateManager()
{
    m_game_mode = MENU;
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

// -----------------------------------------------------------------------------
void AbstractStateManager::enterGameState()
{
    if (getCurrentScreen() != NULL) getCurrentScreen()->tearDown();
    m_menu_stack.clear();
    m_menu_stack.push_back("race");
    setGameState(GAME);
    cleanForGame();
    input_manager->setMode(InputManager::INGAME);
}

// -----------------------------------------------------------------------------
GameState AbstractStateManager::getGameState()
{
    return m_game_mode;
}


#if 0
#pragma mark -
#pragma mark Push/pop menus
#endif

// -----------------------------------------------------------------------------
void AbstractStateManager::pushMenu(std::string name)
{
    // currently, only a single in-game menu is supported
    assert(m_game_mode != INGAME_MENU);
    
    // Send tear-down event to previous menu
    if (m_menu_stack.size() > 0 && m_game_mode != GAME) getCurrentScreen()->tearDown();
    
    input_manager->setMode(InputManager::MENU);
    m_menu_stack.push_back(name);
    if (m_game_mode == GAME)
    {
        setGameState(INGAME_MENU);
        if (World::getWorld() != NULL) World::getWorld()->pause();
    }
    else
    {
        setGameState(MENU);
    }
    switchToScreen(name.c_str());
}

// -----------------------------------------------------------------------------
void AbstractStateManager::pushScreen(Screen* screen)
{
    pushMenu(screen->getName());
    screen->init();
}

// -----------------------------------------------------------------------------
void AbstractStateManager::replaceTopMostScreen(Screen* screen)
{
    assert(m_game_mode != GAME);
    
    std::string name = screen->getName();
    
    // Send tear-down event to previous menu
    if (m_menu_stack.size() > 0) getCurrentScreen()->tearDown();
    
    input_manager->setMode(InputManager::MENU);
    m_menu_stack[m_menu_stack.size()-1] = name;
    switchToScreen(name.c_str());
    
    // Send init event to new menu
    getCurrentScreen()->init();
}

// -----------------------------------------------------------------------------
void AbstractStateManager::reshowTopMostMenu()
{
    assert(m_game_mode != GAME);
    
    // Send tear-down event to previous menu
    if (m_menu_stack.size() > 0) 
    {
        Screen* currScreen = getCurrentScreen();
        if (currScreen != NULL) getCurrentScreen()->tearDown();
    }
    
    switchToScreen( m_menu_stack[m_menu_stack.size()-1].c_str() );
    
    // Send init event to new menu
    getCurrentScreen()->init();
}

// -----------------------------------------------------------------------------
void AbstractStateManager::popMenu()
{
    assert(m_game_mode != GAME);
    
    // Send tear-down event to menu
    getCurrentScreen()->tearDown();
    m_menu_stack.pop_back();
    
    if (m_menu_stack.size() == 0)
    {
        main_loop->abort();
        return;
    }
        
    std::cout << "-- switching to screen " << m_menu_stack[m_menu_stack.size()-1].c_str() << std::endl;
    
    if (m_menu_stack[m_menu_stack.size()-1] == "race")
    {
        m_menu_stack.push_back("race");
        if (m_game_mode == INGAME_MENU)
        {
            if (World::getWorld() != NULL) World::getWorld()->unpause();
        }
        setGameState(GAME);
        cleanForGame();
        input_manager->setMode(InputManager::INGAME);
    }
    else
    {
        setGameState(MENU);
        switchToScreen(m_menu_stack[m_menu_stack.size()-1].c_str());
        getCurrentScreen()->init();
    }
}

// -----------------------------------------------------------------------------
void AbstractStateManager::setGameState(GameState state)
{
    m_game_mode = state;
    
    if (m_game_mode == GAME) irr_driver->hidePointer();
    else                     irr_driver->showPointer();
}

// -----------------------------------------------------------------------------

void AbstractStateManager::resetAndGoToScreen(Screen* screen)
{
    std::string name = screen->getName();

    assert(World::getWorld()==NULL);
    
    if (m_game_mode != GAME) getCurrentScreen()->tearDown();
    m_menu_stack.clear();

    input_manager->setMode(InputManager::MENU);
    m_menu_stack.push_back(name);
    setGameState(MENU);
    sound_manager->positionListener( Vec3(0,0,0), Vec3(0,1,0) );
    switchToScreen(name.c_str());
    getCurrentScreen()->init();
}

// -----------------------------------------------------------------------------

void AbstractStateManager::resetAndSetStack(Screen* screens[])
{
    assert(screens != NULL);
    assert(screens[0] != NULL);
    
    input_manager->setMode(InputManager::MENU);
    
    if (m_game_mode != GAME) getCurrentScreen()->tearDown();
    m_menu_stack.clear();
    
    for (int n=0; screens[n] != NULL; n++)
    {
        m_menu_stack.push_back(screens[n]->getName());
    }
    
    setGameState(MENU);
    
    sound_manager->positionListener( Vec3(0,0,0), Vec3(0,1,0) );
    switchToScreen(m_menu_stack[m_menu_stack.size()-1].c_str());
    getCurrentScreen()->init();
}


