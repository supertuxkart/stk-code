//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2013 Marianne Gagnon
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


#include "states_screens/state_manager.hpp"

#include "audio/sfx_manager.hpp"
#include "audio/music_manager.hpp"
#include "config/stk_config.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/modaldialog.hpp"
#include "guiengine/screen.hpp"
#include "input/input_device.hpp"
#include "input/input_manager.hpp"
#include "main_loop.hpp"
#include "modes/profile_world.hpp"
#include "modes/world.hpp"
#include "utils/translation.hpp"
#include "utils/log.hpp"

using namespace GUIEngine;

static StateManager* state_manager_singleton = NULL;

StateManager* StateManager::get()
{
    if (state_manager_singleton == NULL)
        state_manager_singleton = new StateManager();
    return state_manager_singleton;
}   // get

void StateManager::deallocate()
{
    delete state_manager_singleton;
    state_manager_singleton = NULL;
}   // deallocate


// ============================================================================

#if 0
#pragma mark -
#pragma mark Player Management
#endif

// ----------------------------------------------------------------------------

StateManager::ActivePlayer* StateManager::getActivePlayer(const int id)
{
    ActivePlayer *returnPlayer = NULL;
    if (id < (int)m_active_players.size() && id >= 0)
    {
        returnPlayer = m_active_players.get(id);
    }
    else
    {
        Log::error("StateManager", "getActivePlayer(): id %d out of bounds", id);
        assert(false);
        return NULL;
    }

    assert( returnPlayer->m_id == id );

    return returnPlayer;
}   // getActivePlayer

// ----------------------------------------------------------------------------

const PlayerProfile* StateManager::getActivePlayerProfile(const int id)
{
    ActivePlayer* a = getActivePlayer(id);
    if (a == NULL) return NULL;
    return a->getProfile();
}   // getActivePlayerProfile

// ----------------------------------------------------------------------------

void StateManager::updateActivePlayerIDs()
{
    const int amount = m_active_players.size();
    for (int n=0; n<amount; n++)
    {
        m_active_players[n].m_id = n;
    }
}   // updateActivePlayerIDs

// ----------------------------------------------------------------------------

int StateManager::createActivePlayer(PlayerProfile *profile,
                                     InputDevice *device)
{
    ActivePlayer *p;
    int i;
    p = new ActivePlayer(profile, device);
    i = m_active_players.size();
    m_active_players.push_back(p);

    updateActivePlayerIDs();

    return i;
}   // createActivePlayer

// ----------------------------------------------------------------------------

void StateManager::removeActivePlayer(int id)
{
    m_active_players.erase(id);
    updateActivePlayerIDs();
}   // removeActivePlayer

// ----------------------------------------------------------------------------

unsigned int StateManager::activePlayerCount()
{
    return m_active_players.size();
}   // activePlayerCount

// ----------------------------------------------------------------------------

void StateManager::resetActivePlayers()
{
    const int amount = m_active_players.size();
    for(int i=0; i<amount; i++)
    {
        m_active_players[i].setDevice(NULL);
    }
    m_active_players.clearAndDeleteAll();
}   // resetActivePlayers

// ----------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark misc stuff
#endif

bool StateManager::throttleFPS()
{
    return m_game_mode != GUIEngine::GAME  &&
           GUIEngine::getCurrentScreen()->throttleFPS();
}   // throttleFPS

// ----------------------------------------------------------------------------

void StateManager::escapePressed()
{
    // in input sensing mode
    if(input_manager->isInMode(InputManager::INPUT_SENSE_KEYBOARD) ||
       input_manager->isInMode(InputManager::INPUT_SENSE_GAMEPAD) )
    {
        ModalDialog::dismiss();
        input_manager->setMode(InputManager::MENU);
    }
    // when another modal dialog is visible
    else if(ModalDialog::isADialogActive())
    {
        if(ModalDialog::getCurrent()->onEscapePressed())
            ModalDialog::getCurrent()->dismiss();
    }
    // In-game
    else if(m_game_mode == GAME)
    {
        if(World::getWorld()->getPhase()!=WorldStatus::RESULT_DISPLAY_PHASE
            && !ProfileWorld::isProfileMode())
            World::getWorld()->escapePressed();
    }
    // In menus
    else
    {
        if (getCurrentScreen() != NULL &&
            getCurrentScreen()->onEscapePressed()) popMenu();
    }
}   // escapePressed

// ----------------------------------------------------------------------------

void StateManager::onGameStateChange(GameState new_state)
{
    if (new_state == GAME)
    {
        if (race_manager->getMinorMode() != RaceManager::MINOR_MODE_OVERWORLD)
            irr_driver->hidePointer();
        input_manager->setMode(InputManager::INGAME);
    }
    else  // menu (including in-game menu)
    {
        irr_driver->showPointer();
        input_manager->setMode(InputManager::MENU);
        SFXManager::get()->positionListener( Vec3(0,0,0), Vec3(0,1,0), Vec3(0, 1, 0) );

        if (new_state == MENU)
        {
            GUIEngine::Screen* screen = GUIEngine::getCurrentScreen();
            if (screen != NULL)
            {
                music_manager->startMusic(
                    GUIEngine::getCurrentScreen()->getMusic());
            }
        }
    }
}   // onGameStateChange

// ----------------------------------------------------------------------------

void StateManager::onTopMostScreenChanged()
{
    if (m_game_mode == MENU && GUIEngine::getCurrentScreen() != NULL)
    {
        if (GUIEngine::getCurrentScreen()->getMusic() != NULL)
        {
            music_manager->startMusic(GUIEngine::getCurrentScreen()->getMusic());
        }
    }
    else if (m_game_mode == INGAME_MENU && GUIEngine::getCurrentScreen() != NULL)
    {
        if (GUIEngine::getCurrentScreen()->getInGameMenuMusic() != NULL)
        {
            music_manager->startMusic(GUIEngine::getCurrentScreen()->getInGameMenuMusic());
        }
    }

}   // onTopMostScreenChanged

// ----------------------------------------------------------------------------

void StateManager::onStackEmptied()
{
    GUIEngine::cleanUp();
    GUIEngine::deallocate();
    main_loop->abort();
}   // onStackEmptied

// ============================================================================

#if 0
#pragma mark -
#pragma mark ActivePlayer
#endif

StateManager::ActivePlayer::ActivePlayer(PlayerProfile* player,
                                         InputDevice *device)
{
#ifdef DEBUG
    m_magic_number = 0xAC1EF1AE;
#endif

    m_player = player;
    m_device = NULL;
    m_kart = NULL;
    setDevice(device);
}  // ActivePlayer

// ----------------------------------------------------------------------------
StateManager::ActivePlayer::~ActivePlayer()
{
    setDevice(NULL);

#ifdef DEBUG
    m_magic_number = 0xDEADBEEF;
#endif
}   // ~ActivePlayer

// ----------------------------------------------------------------------------

void StateManager::ActivePlayer::setPlayerProfile(PlayerProfile* player)
{
#ifdef DEBUG
    assert(m_magic_number == 0xAC1EF1AE);
#endif
    m_player = player;
}   // setPlayerProfile

// ----------------------------------------------------------------------------

void StateManager::ActivePlayer::setDevice(InputDevice* device)
{
#ifdef DEBUG
    assert(m_magic_number == 0xAC1EF1AE);
#endif

    // unset player from previous device he was assigned to, if any
    if (m_device != NULL) m_device->setPlayer(NULL);

    m_device = device;

    // inform the devce of its new owner
    if (device != NULL) device->setPlayer(this);
}   // setDevice
