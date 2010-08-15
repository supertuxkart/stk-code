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
 
#include "guiengine/engine.hpp"
#include "guiengine/screen.hpp"
#include "input/device_manager.hpp"

using namespace GUIEngine;


const char* RACE_STATE_NAME = "race";

AbstractStateManager::AbstractStateManager()
{
    m_game_mode = MENU;
}   // AbstractStateManager

#if 0
#pragma mark -
#pragma mark Game State Management
#endif

// -----------------------------------------------------------------------------

void AbstractStateManager::enterGameState()
{
    if (getCurrentScreen() != NULL) getCurrentScreen()->tearDown();
    m_menu_stack.clear();
    m_menu_stack.push_back(RACE_STATE_NAME);
    setGameState(GAME);
    GUIEngine::cleanForGame();
}   // enterGameState

// -----------------------------------------------------------------------------

GameState AbstractStateManager::getGameState()
{
    return m_game_mode;
}   // getGameState

// -----------------------------------------------------------------------------

void AbstractStateManager::setGameState(GameState state)
{
    if (m_game_mode == state) return; // no change
    
    m_game_mode = state;
    
    onGameStateChange(state);
}   // setGameState


// -----------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark Push/pop menus
#endif

void AbstractStateManager::pushMenu(std::string name)
{
    // currently, only a single in-game menu is supported
    assert(m_game_mode != INGAME_MENU);
    
    // Send tear-down event to previous menu
    if (m_menu_stack.size() > 0 && m_game_mode != GAME) getCurrentScreen()->tearDown();
    
    m_menu_stack.push_back(name);
    if (m_game_mode == GAME)
    {
        setGameState(INGAME_MENU);
    }
    else
    {
        setGameState(MENU);
    }
    switchToScreen(name.c_str());
    
    onTopMostScreenChanged();
}   // pushMenu


// -----------------------------------------------------------------------------

void AbstractStateManager::pushScreen(Screen* screen)
{
    if (!screen->isLoaded()) screen->loadFromFile();
    pushMenu(screen->getName());
    screen->init();
    
    onTopMostScreenChanged();
}   // pushScreen

// -----------------------------------------------------------------------------

void AbstractStateManager::replaceTopMostScreen(Screen* screen)
{
    assert(m_game_mode != GAME);

    if (!screen->isLoaded()) screen->loadFromFile();
    std::string name = screen->getName();
    
    assert(m_menu_stack.size() > 0);
    
    // Send tear-down event to previous menu
    getCurrentScreen()->tearDown();
    
    m_menu_stack[m_menu_stack.size()-1] = name;
    switchToScreen(name.c_str());
    
    // Send init event to new menu
    getCurrentScreen()->init();
    
    onTopMostScreenChanged();
}   // replaceTopMostScreen

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
    Screen* screen = getCurrentScreen();
    if (!screen->isLoaded()) screen->loadFromFile();
    screen->init();
    
    onTopMostScreenChanged();
}   // reshowTopMostMenu

// -----------------------------------------------------------------------------

void AbstractStateManager::popMenu()
{
    assert(m_game_mode != GAME);
    
    // Send tear-down event to menu
    getCurrentScreen()->tearDown();
    m_menu_stack.pop_back();
    
    if (m_menu_stack.size() == 0)
    {
        onStackEmptied();
        return;
    }
        
    std::cout << "-- switching to screen " << m_menu_stack[m_menu_stack.size()-1].c_str() << std::endl;
    
    if (m_menu_stack[m_menu_stack.size()-1] == RACE_STATE_NAME)
    {
        setGameState(GAME);
        GUIEngine::cleanForGame();
    }
    else
    {
        setGameState(MENU);
        switchToScreen(m_menu_stack[m_menu_stack.size()-1].c_str());
        
        Screen* screen = getCurrentScreen();
        if (!screen->isLoaded()) screen->loadFromFile();
        screen->init();
    }
    
    onTopMostScreenChanged();
}   // popMenu

// -----------------------------------------------------------------------------

void AbstractStateManager::resetAndGoToScreen(Screen* screen)
{
    std::string name = screen->getName();
    
    if (m_game_mode != GAME) getCurrentScreen()->tearDown();
    m_menu_stack.clear();
    
    m_menu_stack.push_back(name);
    setGameState(MENU);
    
    switchToScreen(name.c_str());
    getCurrentScreen()->init();
    
    onTopMostScreenChanged();
}   // resetAndGoToScreen

// -----------------------------------------------------------------------------

void AbstractStateManager::resetAndSetStack(Screen* screens[])
{
    assert(screens != NULL);
    assert(screens[0] != NULL);
    
    if (m_game_mode != GAME) getCurrentScreen()->tearDown();
    m_menu_stack.clear();
    
    for (int n=0; screens[n] != NULL; n++)
    {
        m_menu_stack.push_back(screens[n]->getName());
    }
    
    setGameState(MENU);
    
    switchToScreen(m_menu_stack[m_menu_stack.size()-1].c_str());
    getCurrentScreen()->init();
    
    onTopMostScreenChanged();
}   // resetAndSetStack

