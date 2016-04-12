//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Marianne Gagnon
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

#include "config/user_config.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/modaldialog.hpp"
#include "guiengine/screen.hpp"
#include "input/device_manager.hpp"

#include <vector>
#include <iostream>


using namespace GUIEngine;


static const char RACE_STATE_NAME[] = "race";

AbstractStateManager::AbstractStateManager()
{
    m_game_mode = MENU;
}   // AbstractStateManager

#if 0
#pragma mark -
#pragma mark Game State Management
#endif

// ----------------------------------------------------------------------------

void AbstractStateManager::enterGameState()
{
     // you need to close any dialog before calling this
    assert(!ModalDialog::isADialogActive());

    if (getCurrentScreen() != NULL) getCurrentScreen()->tearDown();
    m_menu_stack.clear();
    m_menu_stack.push_back(RACE_STATE_NAME);
    setGameState(GAME);
    GUIEngine::cleanForGame();
}   // enterGameState

// ----------------------------------------------------------------------------

GameState AbstractStateManager::getGameState()
{
    return m_game_mode;
}   // getGameState

// ----------------------------------------------------------------------------

void AbstractStateManager::setGameState(GameState state)
{
    if (m_game_mode == state) return; // no change

    m_game_mode = state;

    onGameStateChange(state);
}   // setGameState


// ----------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark Push/pop menus
#endif

void AbstractStateManager::pushMenu(std::string name)
{
    // currently, only a single in-game menu is supported
    assert(m_game_mode != INGAME_MENU);

    // you need to close any dialog before calling this
    assert(!ModalDialog::isADialogActive());

    if (UserConfigParams::logGUI())
    {
        Log::info("AbstractStateManager::pushMenu", "Switching to screen %s",
            name.c_str());
    }

    // Send tear-down event to previous menu
    if (m_menu_stack.size() > 0 && m_game_mode != GAME)
        getCurrentScreen()->tearDown();

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


// ----------------------------------------------------------------------------

void AbstractStateManager::pushScreen(Screen* screen)
{
    // you need to close any dialog before calling this
    assert(!ModalDialog::isADialogActive());

    if (UserConfigParams::logGUI())
    {
        Log::info("AbstractStateManager::pushScreen", "Switching to screen %s",
            screen->getName().c_str());
    }

    if (!screen->isLoaded()) screen->loadFromFile();
    pushMenu(screen->getName());
    screen->init();

    onTopMostScreenChanged();
}   // pushScreen%

// ----------------------------------------------------------------------------

void AbstractStateManager::replaceTopMostScreen(Screen* screen, GUIEngine::GameState gameState)
{
    if (gameState == GUIEngine::CURRENT)
        gameState = getGameState();

    //assert(m_game_mode != GAME);
    // you need to close any dialog before calling this
    assert(!ModalDialog::isADialogActive());

    if (!screen->isLoaded()) screen->loadFromFile();
    std::string name = screen->getName();

    if (UserConfigParams::logGUI())
    {
        Log::info("AbstractStateManager::replaceTopMostScreen", "Switching to screen %s",
            name.c_str());
    }

    assert(m_menu_stack.size() > 0);

    // Send tear-down event to previous menu
    if (getCurrentScreen() != NULL)
        getCurrentScreen()->tearDown();

    m_menu_stack[m_menu_stack.size()-1] = name;
    setGameState(gameState);
    switchToScreen(name.c_str());

    // Send init event to new menu
    getCurrentScreen()->init();

    onTopMostScreenChanged();
}   // replaceTopMostScreen

// ----------------------------------------------------------------------------

void AbstractStateManager::reshowTopMostMenu()
{
    assert(m_game_mode != GAME);
    // you need to close any dialog before calling this
    assert(!ModalDialog::isADialogActive());

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

// ----------------------------------------------------------------------------

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

    if (UserConfigParams::logGUI())
    {
        Log::info("AbstractStateManager::popMenu", "Switching to screen %s",
            m_menu_stack[m_menu_stack.size()-1].c_str());
    }

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

// ----------------------------------------------------------------------------

void AbstractStateManager::resetAndGoToScreen(Screen* screen)
{
    // you need to close any dialog before calling this
    assert(!ModalDialog::isADialogActive());

    std::string name = screen->getName();

    if (UserConfigParams::logGUI())
        Log::info("AbstractStateManager::resetAndGoToScreen", "Switching to screen %s",
            name.c_str());

    if (m_game_mode != GAME) getCurrentScreen()->tearDown();
    m_menu_stack.clear();

    if (!screen->isLoaded()) screen->loadFromFile();
    m_menu_stack.push_back(name);
    setGameState(MENU);

    switchToScreen(name.c_str());
    getCurrentScreen()->init();

    onTopMostScreenChanged();
}   // resetAndGoToScreen

// ----------------------------------------------------------------------------

void AbstractStateManager::resetAndSetStack(Screen* screens[])
{
    assert(screens != NULL);
    assert(screens[0] != NULL);
    // you need to close any dialog before calling this
    assert(!ModalDialog::isADialogActive());

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

