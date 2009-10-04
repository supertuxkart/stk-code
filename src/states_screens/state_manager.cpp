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


#include "states_screens/state_manager.hpp"

#include <vector>

#include "main_loop.hpp"
#include "audio/sound_manager.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/player.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widget.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "input/input_device.hpp"
#include "io/file_manager.hpp"
#include "network/network_manager.hpp"
#include "race/race_manager.hpp"
#include "states_screens/kart_selection.hpp"
#include "states_screens/credits.hpp"
#include "states_screens/dialogs/track_info_dialog.hpp"
#include "states_screens/dialogs/race_paused_dialog.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/translation.hpp"

// FIXME : remove, temporary test
#include "states_screens/feature_unlocked.hpp"

using namespace GUIEngine;

StateManager* state_manager_singleton = NULL;

StateManager* StateManager::get()
{
    if (state_manager_singleton == NULL) state_manager_singleton = new StateManager();
    return state_manager_singleton;
}

#if 0
#pragma mark -
#pragma mark Player Management
#endif

ptr_vector<ActivePlayer, HOLD>& StateManager::getActivePlayers()
{
    return m_active_players;
}
ActivePlayer* StateManager::getActivePlayer(const int id)
{
    ActivePlayer *returnPlayer = NULL;
    if (id < m_active_players.size() && id >= 0)
    {
        returnPlayer = m_active_players.get(id);
    }
    else
    {
        fprintf(stderr, "getActivePlayer(): id out of bounds\n");
    }
    
    assert( returnPlayer->m_id == id );
    
    return returnPlayer;
}
void StateManager::updateActivePlayerIDs()
{
    const int amount = m_active_players.size();
    for (int n=0; n<amount; n++)
    {
        m_active_players[n].m_id = n;
    }
}

int StateManager::createActivePlayer(PlayerProfile *profile, InputDevice *device)
{
    ActivePlayer *p;
    int i;
    p = new ActivePlayer(profile, device);
    i = m_active_players.size();
    m_active_players.push_back(p);
    
    updateActivePlayerIDs();
    
    return i;
}

void StateManager::removeActivePlayer(int id)
{
    m_active_players.erase(id);
    updateActivePlayerIDs();
}
int StateManager::activePlayerCount()
{
    return m_active_players.size();
}

void StateManager::resetActivePlayers()
{
    const int amount = m_active_players.size();
    for(int i=0; i<amount; i++)
    {
        m_active_players[i].setDevice(NULL);
    }
    m_active_players.clearWithoutDeleting();
}



// -------------------------------------------------------------------------
/**
 * All widget events will be dispatched to this function; arguments are
 * a pointer to the widget from which the event originates, and its internal
 * name. There is one exception : right after showing a new screen, an event with
 * name 'init' and widget set to NULL will be fired, so the screen can be filled
 * with the right values or so.
 */
/*
void StateManager::eventCallback(Widget* widget, const std::string& name)
{
    std::cout << "event!! " << name.c_str() << std::endl;

    if (name == "lock")
    {
        unlock_manager->playLockSound();
    }
    
    Screen* topScreen = getCurrentScreen();
    if (topScreen == NULL) return;
    const std::string& screen_name = topScreen->getName();

    if( screen_name == "main.stkgui" )
        menuEventMain(widget, name);
    else if( screen_name == "karts.stkgui" )
        KartSelectionScreen::menuEventKarts(widget, name);
    else if( screen_name == "racesetup.stkgui" )
        menuEventRaceSetup(widget, name);
    else if( screen_name == "tracks.stkgui" )
        menuEventTracks(widget, name);
    else if( screen_name == "options_av.stkgui" || screen_name == "options_input.stkgui" || screen_name == "options_players.stkgui")
        OptionsScreen::menuEventOptions(widget, name);
    else if( screen_name == "help1.stkgui" || screen_name == "help2.stkgui" || screen_name == "help3.stkgui")
        menuEventHelp(widget, name);
    else if( screen_name == "credits.stkgui" )
    {
        if(name == "init")
        {
            Widget* w = getCurrentScreen()->getWidget<Widget>("animated_area");
            assert(w != NULL);
        
            Credits* credits = Credits::getInstance();
            credits->reset();
            credits->setArea(w->x, w->y, w->w, w->h);
        }
        else if(name == "back")
        {
            StateManager::escapePressed();
        }
    }
    else
        std::cerr << "Warning, unknown menu " << screen_name << " in event callback\n";

}*/

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
        ModalDialog::dismiss();
    }
    // In-game
    else if(m_game_mode == GAME)
    {
        new RacePausedDialog(0.8f, 0.6f);
        //resetAndGoToMenu("main.stkgui");
        //input_manager->setMode(InputManager::MENU);
    }
    // In menus
    else
    {
        popMenu();
    }
}


