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
#include "guiengine/screen_keyboard.hpp"
#include "guiengine/screen.hpp"
#include "input/device_manager.hpp"
#include "utils/debug.hpp"

#include <vector>
#include <iostream>
#include <IGUIEnvironment.h>

using namespace GUIEngine;


static const char RACE_STATE_NAME[] = "race";

AbstractStateManager::AbstractStateManager()
{
    m_game_mode.store(MENU);
}   // AbstractStateManager

#if 0
#pragma mark -
#pragma mark Game State Management
#endif

// ----------------------------------------------------------------------------

void AbstractStateManager::enterGameState()
{
    if (GUIEngine::isNoGraphics())
    {
        // No graphics STK won't push dialog
        setGameState(GAME);
        return;
    }

     // you need to close any dialog before calling this
    assert(!ModalDialog::isADialogActive());
    assert(!ScreenKeyboard::isActive());

    if (getCurrentScreen() != NULL) getCurrentScreen()->tearDown();
    m_menu_stack.clear();
    m_menu_stack.emplace_back(RACE_STATE_NAME, (Screen*)NULL);

    Debug::closeDebugMenu();
    setGameState(GAME);
    GUIEngine::cleanForGame();
}   // enterGameState

// ----------------------------------------------------------------------------

GameState AbstractStateManager::getGameState()
{
    return m_game_mode.load();
}   // getGameState

// ----------------------------------------------------------------------------

void AbstractStateManager::setGameState(GameState state)
{
    if (m_game_mode.load() == state) return; // no change

    m_game_mode.store(state);

    onGameStateChange(state);
}   // setGameState


// ----------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark Push/pop menus
#endif

void AbstractStateManager::pushMenu(Screen* screen)
{
    // currently, only a single in-game menu is supported
    assert(m_game_mode.load() != INGAME_MENU);

    // you need to close any dialog before calling this
    assert(!ModalDialog::isADialogActive());
    assert(!ScreenKeyboard::isActive());

    if (UserConfigParams::logGUI())
    {
        Log::info("AbstractStateManager::pushMenu", "Switching to screen %s",
            screen->getName().c_str());
    }

    // Send tear-down event to previous menu
    if (m_menu_stack.size() > 0 && m_game_mode.load() != GAME)
        getCurrentScreen()->tearDown();

    m_menu_stack.emplace_back(screen->getName(), screen);
    if (m_game_mode.load() == GAME)
    {
        setGameState(INGAME_MENU);
    }
    else
    {
        setGameState(MENU);
    }
    switchToScreen(screen);

    onTopMostScreenChanged();
}   // pushMenu


// ----------------------------------------------------------------------------

void AbstractStateManager::pushScreen(Screen* screen)
{
    // you need to close any dialog before calling this
    assert(!ModalDialog::isADialogActive());
    assert(!ScreenKeyboard::isActive());

    if (UserConfigParams::logGUI())
    {
        Log::info("AbstractStateManager::pushScreen", "Switching to screen %s",
            screen->getName().c_str());
    }

    if (!screen->isLoaded()) screen->loadFromFile();
    pushMenu(screen);
    screen->init();

    onTopMostScreenChanged();
}   // pushScreen%

// ----------------------------------------------------------------------------

void AbstractStateManager::replaceTopMostScreen(Screen* screen, GUIEngine::GameState gameState)
{
    if (gameState == GUIEngine::CURRENT)
        gameState = getGameState();

    //assert(m_game_mode.load() != GAME);
    // you need to close any dialog before calling this
    assert(!ModalDialog::isADialogActive());
    assert(!ScreenKeyboard::isActive());

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

    m_menu_stack[m_menu_stack.size()-1] = std::make_pair(name, screen);
    setGameState(gameState);
    switchToScreen(screen);

    // Send init event to new menu
    getCurrentScreen()->init();

    onTopMostScreenChanged();
}   // replaceTopMostScreen

// ----------------------------------------------------------------------------

void AbstractStateManager::reshowTopMostMenu()
{
    assert(m_game_mode.load() != GAME);
    // you need to close any dialog before calling this
    assert(!ModalDialog::isADialogActive());
    assert(!ScreenKeyboard::isActive());

    // Send tear-down event to previous menu
    if (m_menu_stack.size() > 0)
    {
        Screen* currScreen = getCurrentScreen();
        if (currScreen != NULL) getCurrentScreen()->tearDown();
    }

    switchToScreen(m_menu_stack[m_menu_stack.size()-1].second);

    // Send init event to new menu
    Screen* screen = getCurrentScreen();
    if (!screen->isLoaded()) screen->loadFromFile();
    screen->init();

    onTopMostScreenChanged();
}   // reshowTopMostMenu

// ----------------------------------------------------------------------------

void AbstractStateManager::popMenu()
{
    assert(m_game_mode.load() != GAME);

    if (m_menu_stack.empty())
        return;

    // Send tear-down event to menu
    getCurrentScreen()->tearDown();
    m_menu_stack.pop_back();

    if (m_menu_stack.empty())
    {
        getGUIEnv()->clear();
        getCurrentScreen()->elementsWereDeleted();
        onStackEmptied();
        return;
    }

    if (UserConfigParams::logGUI())
    {
        Log::info("AbstractStateManager::popMenu", "Switching to screen %s",
            m_menu_stack[m_menu_stack.size()-1].first.c_str());
    }

    if (m_menu_stack[m_menu_stack.size()-1].first == RACE_STATE_NAME)
    {
        setGameState(GAME);
        GUIEngine::cleanForGame();
    }
    else
    {
        setGameState(MENU);
        switchToScreen(m_menu_stack[m_menu_stack.size()-1].second);

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
    assert(!ScreenKeyboard::isActive());

    std::string name = screen->getName();

    if (UserConfigParams::logGUI())
        Log::info("AbstractStateManager::resetAndGoToScreen", "Switching to screen %s",
            name.c_str());

    if (m_game_mode.load() != GAME) getCurrentScreen()->tearDown();
    m_menu_stack.clear();

    if (!screen->isLoaded()) screen->loadFromFile();
    m_menu_stack.emplace_back(name, screen);
    setGameState(MENU);

    switchToScreen(screen);
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
    assert(!ScreenKeyboard::isActive());

    if (m_game_mode.load() != GAME && getCurrentScreen())
        getCurrentScreen()->tearDown();
    m_menu_stack.clear();

    for (int n=0; screens[n] != NULL; n++)
    {
        m_menu_stack.emplace_back(screens[n]->getName(), screens[n]);
    }

    setGameState(MENU);

    switchToScreen(m_menu_stack[m_menu_stack.size()-1].second);
    getCurrentScreen()->init();

    onTopMostScreenChanged();
}   // resetAndSetStack

// ----------------------------------------------------------------------------

void AbstractStateManager::onResize()
{
    // Happens in the first resize in main.cpp
    if (m_menu_stack.empty())
        return;

    // In game resizing
    if (m_menu_stack[0].first == RACE_STATE_NAME)
    {
        if (m_menu_stack.size() == 1)
        {
            clearScreenCache();
            m_menu_stack.emplace_back(RACE_STATE_NAME, (Screen*)NULL);
        }
        return;
    }

    // For some window manager it sends resize event when STK is not focus
    // even if the screen is not resizable, prevent it from resizing if wrong
    // screen
    if (!m_menu_stack.back().second ||
        !m_menu_stack.back().second->isResizable())
        return;

    std::vector<std::function<Screen*()> > screen_function;
    for (auto& p : m_menu_stack)
        screen_function.push_back(p.second->getNewScreenPointer());
    clearScreenCache();
    std::vector<Screen*> new_screen;
    for (auto& screen : screen_function)
        new_screen.push_back(screen());
    new_screen.push_back(NULL);
    resetAndSetStack(new_screen.data());
}   // onResize
