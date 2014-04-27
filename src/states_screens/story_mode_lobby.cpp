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
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
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
    PlayerProfile *player = PlayerManager::getCurrentPlayer();
    if (player)
    {
        //StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());
        //return;
    }

    //CheckBoxWidget* cb = getWidget<CheckBoxWidget>("rememberme");
    //cb->setState(false);

    ListWidget* list = getWidget<ListWidget>("gameslots");

    //PtrVector<PlayerProfile>& players = UserConfigParams::m_all_players;
#if 0

    if (UserConfigParams::m_default_player.toString().size() > 0)
    {
        for (unsigned int n=0; n<players.size(); n++)
        {
            if (players[n].getName() == UserConfigParams::m_default_player.toString())
            {
                unlock_manager->setCurrentSlot(players[n].getUniqueID());
                StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());
                return;
            }
        }
    }.
#endif

    DynamicRibbonWidget* local = getWidget<DynamicRibbonWidget>("local");
    assert( local != NULL );

    for (unsigned int n=0; n<PlayerManager::get()->getNumPlayers(); n++)
    {
        const PlayerProfile *player = PlayerManager::get()->getPlayer(n);
        if (player->isGuestAccount()) continue;
        std::string s = StringUtils::toString(n);
        local->addItem(player->getName(), s, "", 0, IconButtonWidget::ICON_PATH_TYPE_RELATIVE);
    }

    local->addItem("Create new user", "local_new", 
                   "karts/sara/icon-sara.png", 0,
                   IconButtonWidget::ICON_PATH_TYPE_RELATIVE);
    local->updateItemDisplay();

    DynamicRibbonWidget* online = this->getWidget<DynamicRibbonWidget>("online");
    assert( online != NULL );
    const std::vector<core::stringw> &online_ids = 
        PlayerManager::get()->getAllOnlineIds();
    for (unsigned int i = 0; i < online_ids.size(); i++)
    {
        std::string s = StringUtils::toString(i);
        online->addItem(online_ids[i], s, "karts/nolok/nolokicon.png", 0,
                        IconButtonWidget::ICON_PATH_TYPE_RELATIVE);

    }
    online->addItem("Create new online user", "online_new",
                     "karts/sara/icon-sara.png", 0,
                     IconButtonWidget::ICON_PATH_TYPE_RELATIVE);
    online->updateItemDisplay();


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
    if (name == "local")
    {
        // FIXME nothing to do
    }
    else if (name == "options")
    {
        const std::string &button =
            getWidget<RibbonWidget>("options")->getSelectionIDString(player_id);
        if (button == "ok" || button == "ok_and_save")
        {
            DynamicRibbonWidget *local = getWidget<DynamicRibbonWidget>("local");
            const std::string &name = local->getSelectionIDString(player_id);
            if (name == "local_new")
            {
                // create new local player
                return;
            }
            unsigned int id;
            StringUtils::fromString(name, id);
            PlayerProfile *profile = PlayerManager::get()->getPlayer(id);
            PlayerManager::get()->setCurrentPlayer(profile, button=="ok_and_save");
            StateManager::get()->pushScreen(MainMenuScreen::getInstance());
            return;
        }   // button==ok || ok_and_save
    }   // options

    return;


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

        for (unsigned int n=0; n<PlayerManager::get()->getNumPlayers(); n++)
        {
            PlayerProfile *player = PlayerManager::get()->getPlayer(n);
            if (list->getSelectionLabel() == player->getName())
            {
                PlayerManager::get()->setCurrentPlayer(player, false);
                slot_found = true;
                break;
            }
        }

        if (!slot_found)
        {
            Log::error("StoryModeLobby",
                       "Cannot find player corresponding to slot '%s'.",
                     core::stringc(list->getSelectionLabel().c_str()).c_str());
        }
        else
        {
//            CheckBoxWidget* cb = getWidget<CheckBoxWidget>("rememberme");
//            if (cb->getState())
            {
//                UserConfigParams::m_default_player = list->getSelectionLabel();
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

void StoryModeLobbyScreen::onNewPlayerWithName(const core::stringw& newName)
{
    bool slot_found = false;

    for (unsigned int n=0; n<PlayerManager::get()->getNumPlayers(); n++)
    {
        PlayerProfile *player = PlayerManager::get()->getPlayer(n);
        if (player->getName() == newName)
        {
            PlayerManager::get()->setCurrentPlayer(player, false);
            slot_found = true;
            break;
        }
    }

    if (!slot_found)
    {
        Log::error("StoryModeLobbyScreen",
                   "Cannot find player corresponding to slot '%s'.",
                   core::stringc(newName.c_str()).c_str());
    }

    StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());
}   // onNewPlayerWithName

// -----------------------------------------------------------------------------

