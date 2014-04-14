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

#include "states_screens/story_mode_lobby.hpp"

#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "online/current_user.hpp"
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

    PlayerProfile *player = PlayerManager::getCurrentPlayer();
    if(player)
    {
        StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());
        return;
    }

    for (unsigned int n=0; n<PlayerManager::get()->getNumPlayers(); n++)
    {
        const PlayerProfile *player = PlayerManager::get()->getPlayer(n);
        if (player->isGuestAccount()) continue;

        // FIXME: we're using a trunacted ascii version of the player name as
        //        identifier, let's hope this causes no issues...
        list->addItem(core::stringc(player->getName().c_str()).c_str(),
                                    player->getName() );
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

void StoryModeLobbyScreen::eventCallback(Widget* widget,
                                         const std::string& name,
                                         const int player_id)
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

        PlayerProfile *player = PlayerManager::get()
                              ->getPlayer(list->getSelectionLabel());
        if(player)
        {
            player->computeActive();
            CheckBoxWidget* cb = getWidget<CheckBoxWidget>("rememberme");
            PlayerManager::get()->setCurrentPlayer(player,cb->getState());
        }
        else
        {
            Log::error("StoryModeLobby",
                       "Cannot find player corresponding to slot '%s'.",
                     core::stringc(list->getSelectionLabel().c_str()).c_str());
        }

        StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());
        // Since only now the current player is defined, we have to request 
        // a login (if an online login was saved). If the current player was
        // saved, this request will be started much earlier in the startup
        // sequence from the RequestManager.
        player->getCurrentUser()->requestSavedSession();
    }
}   // eventCallback

// ----------------------------------------------------------------------------

void StoryModeLobbyScreen::unloaded()
{
}   // unloaded

// ----------------------------------------------------------------------------

void StoryModeLobbyScreen::onNewPlayerWithName(const stringw& new_name)
{
    PlayerProfile *player = PlayerManager::get()->getPlayer(new_name);
    if(player)
    {
        PlayerManager::get()->setCurrentPlayer(player,false);
        player->computeActive();
    }
    else
    {
        Log::error("StoryModeLobbyScreen",
                   "Cannot find player corresponding to slot '%s'.",
                   core::stringc(new_name.c_str()).c_str());
    }

    StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());
}   // onNewPlayerWithName

// -----------------------------------------------------------------------------

