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

#include "states_screens/story_mode_lobby.hpp"

#include "challenges/unlock_manager.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "states_screens/dialogs/enter_player_name_dialog.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "states_screens/state_manager.hpp"


using namespace GUIEngine;

DEFINE_SCREEN_SINGLETON( StoryModeLobbyScreen );

// ----------------------------------------------------------------------------

StoryModeLobbyScreen::StoryModeLobbyScreen() : Screen("story_mode_lobby.stkgui")
{
}   // StoryModeLobbyScreen

// ----------------------------------------------------------------------------

void StoryModeLobbyScreen::loadedFromFile()
{
}   // loadedFromFile

// ----------------------------------------------------------------------------

void StoryModeLobbyScreen::init()
{
    Screen::init();
    
    CheckBoxWidget* cb = getWidget<CheckBoxWidget>("rememberme");
    cb->setState(false);
    
    ListWidget* list = getWidget<ListWidget>("gameslots");
    list->clear();
    
    PtrVector<PlayerProfile>& players = UserConfigParams::m_all_players;
    
    if (UserConfigParams::m_default_player.toString().size() > 0)
    {
        for (int n=0; n<players.size(); n++)
        {
            if (players[n].getName() == UserConfigParams::m_default_player.toString())
            {
                unlock_manager->setCurrentSlot(players[n].getUniqueID());
                StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());
                return;
            }
        }
    }
    
    for (int n=0; n<players.size(); n++)
    {
        if (players[n].isGuestAccount()) continue;
        
        // FIXME: we're using a trunacted ascii version of the player name as
        //        identifier, let's hope this causes no issues...
        list->addItem(core::stringc(players[n].getName().c_str()).c_str(),
                      players[n].getName() );
    }
    
    list->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    list->setSelectionID(0);
    
}   // init

// ----------------------------------------------------------------------------

void StoryModeLobbyScreen::tearDown()
{
    Screen::tearDown();
}   // tearDown

// ----------------------------------------------------------------------------

void StoryModeLobbyScreen::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    if (name == "back")
    {
        StateManager::get()->escapePressed();
    }
    else if (name == "creategame")
    {
        new EnterPlayerNameDialog(this, 0.5f, 0.4f);
    }
    else if (name == "gameslots")
    {
        ListWidget* list = getWidget<ListWidget>("gameslots");
        
        bool slot_found = false;
        
        PtrVector<PlayerProfile>& players = UserConfigParams::m_all_players;
        for (int n=0; n<players.size(); n++)
        {
            if (list->getSelectionLabel() == players[n].getName())
            {
                unlock_manager->setCurrentSlot(players[n].getUniqueID());
                slot_found = true;
                break;
            }
        }
        
        if (!slot_found)
        {
            fprintf(stderr, "[StoryModeLobbyScreen] ERROR: cannot find player corresponding to slot '%s'\n",
                    core::stringc(list->getSelectionLabel().c_str()).c_str());
        }
        else
        {
            CheckBoxWidget* cb = getWidget<CheckBoxWidget>("rememberme");
            if (cb->getState())
            {
                UserConfigParams::m_default_player = list->getSelectionLabel();
            }
        }
            
        StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());
    }
}   // eventCallback

// ----------------------------------------------------------------------------

void StoryModeLobbyScreen::unloaded()
{
}   // unloaded

// ----------------------------------------------------------------------------

void StoryModeLobbyScreen::onNewPlayerWithName(const stringw& newName)
{
    bool slot_found = false;
    
    PtrVector<PlayerProfile>& players = UserConfigParams::m_all_players;
    for (int n=0; n<players.size(); n++)
    {
        if (players[n].getName() == newName)
        {
            unlock_manager->setCurrentSlot(players[n].getUniqueID());
            slot_found = true;
            break;
        }
    }
    
    if (!slot_found)
    {
        fprintf(stderr, "[StoryModeLobbyScreen] ERROR: cannot find player corresponding to slot '%s'\n",
                core::stringc(newName.c_str()).c_str());
    }
    
    StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());
}

// -----------------------------------------------------------------------------

