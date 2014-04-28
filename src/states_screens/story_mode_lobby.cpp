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
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "states_screens/dialogs/enter_player_name_dialog.hpp"
#include "states_screens/login_screen.hpp"
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
    m_online_cb = getWidget<CheckBoxWidget>("online");
    assert(m_online_cb);
    m_username_tb = getWidget<TextBoxWidget >("username");
    assert(m_username_tb);
    m_password_tb = getWidget<TextBoxWidget >("password");
    assert(m_password_tb);
    //m_password_tb->setPasswordBox(true, L'*');

    Screen::init();
    PlayerProfile *player = PlayerManager::getCurrentPlayer();
    if (player)
    {
        //StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());
        //return;
    }

    DynamicRibbonWidget* local = getWidget<DynamicRibbonWidget>("local");
    assert( local != NULL );

    for (unsigned int n=0; n<PlayerManager::get()->getNumPlayers(); n++)
    {
        const PlayerProfile *player = PlayerManager::get()->getPlayer(n);
        if (player->isGuestAccount()) continue;
        std::string s = StringUtils::toString(n);
        local->addItem(player->getName(), s, "", 0, 
                       IconButtonWidget::ICON_PATH_TYPE_RELATIVE);
    }


    update();

}   // init

// ----------------------------------------------------------------------------

void StoryModeLobbyScreen::tearDown()
{
    Screen::tearDown();
}   // tearDown

// ----------------------------------------------------------------------------
void StoryModeLobbyScreen::update()
{
    bool online = m_online_cb->getState();
    getWidget<LabelWidget>("label_username")->setVisible(online);
    m_username_tb->setVisible(online);
    getWidget<LabelWidget>("label_password")->setVisible(online);
    m_password_tb->setVisible(online);
}   // update

// ----------------------------------------------------------------------------
void StoryModeLobbyScreen::eventCallback(Widget* widget,
                                         const std::string& name,
                                         const int player_id)
{
    if (name == "local")
    {
        // Clicked on a name --> Find the corresponding online data
        // and display them
        const std::string &s_index = getWidget<DynamicRibbonWidget>("local")
                                     ->getSelectionIDString(player_id);
        if (s_index == "") return;  // can happen if the list is empty

        unsigned int id;
        StringUtils::fromString(s_index, id);
        PlayerProfile *profile = PlayerManager::get()->getPlayer(id);
        assert(profile);
        getWidget<TextBoxWidget >("username")->setText(profile
                                                       ->getLastOnlineName());
        // In case of a saved session, remove the password field,
        // since it is not necessary to display it.
        getWidget<TextBoxWidget >("password")->setVisible(profile
                                                          ->hasSavedSession());

    }
    else if (name == "online")
    {
        update();  // This will make the fields (in)visible
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

    update();
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

