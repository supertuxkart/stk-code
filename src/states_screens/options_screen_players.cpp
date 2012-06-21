//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Marianne Gagnon
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

#include "states_screens/options_screen_players.hpp"

#include "challenges/unlock_manager.hpp"
#include "config/player.hpp"
#include "config/device_config.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "states_screens/dialogs/enter_player_name_dialog.hpp"
#include "states_screens/dialogs/player_info_dialog.hpp"
#include "states_screens/options_screen_audio.hpp"
#include "states_screens/options_screen_input.hpp"
#include "states_screens/options_screen_video.hpp"
#include "states_screens/options_screen_ui.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/story_mode_lobby.hpp"

#include <IGUIButton.h>

#include <iostream>
#include <sstream>

using namespace GUIEngine;
using namespace irr::core;
using namespace irr::gui;

DEFINE_SCREEN_SINGLETON( OptionsScreenPlayers );

// -----------------------------------------------------------------------------

OptionsScreenPlayers::OptionsScreenPlayers() : Screen("options_players.stkgui")
{
}   // OptionsScreenPlayers

// -----------------------------------------------------------------------------

void OptionsScreenPlayers::loadedFromFile()
{
}   // loadedFromFile

// -----------------------------------------------------------------------------

void OptionsScreenPlayers::init()
{
    Screen::init();

    RibbonWidget* tabBar = this->getWidget<RibbonWidget>("options_choice");
    if (tabBar != NULL) tabBar->select( "tab_players", PLAYER_ID_GAME_MASTER );
    
    tabBar->getRibbonChildren()[0].setTooltip( _("Graphics") );
    tabBar->getRibbonChildren()[1].setTooltip( _("Audio") );
    tabBar->getRibbonChildren()[2].setTooltip( _("User Interface") );
    tabBar->getRibbonChildren()[4].setTooltip( _("Controls") );

    ListWidget* players = this->getWidget<ListWidget>("players");
    assert(players != NULL);

    refreshPlayerList();

    ButtonWidget* you = getWidget<ButtonWidget>("playername");
    const std::string& playerID = unlock_manager->getCurrentSlot()->getPlayerID();
    core::stringw playerName = L"-";
    PlayerProfile* curr;
    for_in (curr, UserConfigParams::m_all_players)
    {
        if (curr->getUniqueID() == playerID)
        {
            playerName = curr->getName();
            break;
        }
    }
    you->setText( playerName );
    ((gui::IGUIButton*)you->getIrrlichtElement())->setOverrideFont( GUIEngine::getSmallFont() );

    if (StateManager::get()->getGameState() == GUIEngine::INGAME_MENU)
    {
        players->setDeactivated();
        you->setDeactivated();
    }
    else
    {
        players->setActivated();
        you->setActivated();
    }
}   // init

// -----------------------------------------------------------------------------

bool OptionsScreenPlayers::renamePlayer(const stringw& newName, PlayerProfile* player)
{
    player->setName( newName );
    refreshPlayerList();
    return true;
}   // renamePlayer

// -----------------------------------------------------------------------------

void OptionsScreenPlayers::onNewPlayerWithName(const stringw& newName)
{
    ListWidget* players = this->getWidget<ListWidget>("players");
    if (players != NULL)
    {
        core::stringc newNameC(newName.c_str());
        players->addItem( newNameC.c_str(), translations->fribidize(newName) );
    }
}

// -----------------------------------------------------------------------------

void OptionsScreenPlayers::deletePlayer(PlayerProfile* player)
{
    UserConfigParams::m_all_players.erase(player);
    
    refreshPlayerList();
}   // deletePlayer

// -----------------------------------------------------------------------------

void OptionsScreenPlayers::tearDown()
{
    Screen::tearDown();
    user_config->saveConfig();
    bool changed = unlock_manager->createSlotsIfNeeded();
    if (changed) unlock_manager->save();
}   // tearDown

// -----------------------------------------------------------------------------

void OptionsScreenPlayers::eventCallback(Widget* widget, const std::string& name, const int playerID)
{    
    if (name == "options_choice")
    {
        std::string selection = ((RibbonWidget*)widget)->getSelectionIDString(PLAYER_ID_GAME_MASTER).c_str();
        
        if (selection == "tab_audio") StateManager::get()->replaceTopMostScreen(OptionsScreenAudio::getInstance());
        else if (selection == "tab_video") StateManager::get()->replaceTopMostScreen(OptionsScreenVideo::getInstance());
        else if (selection == "tab_players") StateManager::get()->replaceTopMostScreen(OptionsScreenPlayers::getInstance());
        else if (selection == "tab_controls") StateManager::get()->replaceTopMostScreen(OptionsScreenInput::getInstance());
        else if (selection == "tab_ui") StateManager::get()->replaceTopMostScreen(OptionsScreenUI::getInstance());
    }
    else if (name == "back")
    {
        StateManager::get()->escapePressed();
    }
    else if (name == "addplayer")
    {
        new EnterPlayerNameDialog(this, 0.5f, 0.4f);
    }
    else if (name == "players")
    {
        // Find which player in the list was clicked
        ListWidget* players = this->getWidget<ListWidget>("players");
        assert(players != NULL);
        
        core::stringw selectedPlayer = players->getSelectionLabel();
        const int playerAmount = UserConfigParams::m_all_players.size();
        for (int n=0; n<playerAmount; n++)
        {
            if (selectedPlayer == translations->fribidize(UserConfigParams::m_all_players[n].getName()))
            {
                if (!(UserConfigParams::m_all_players[n].isGuestAccount()))
                {
                    new PlayerInfoDialog( &UserConfigParams::m_all_players[n], 0.5f, 0.6f );
                }
                return;
            }
        } // end for
    } 
    else if (name == "playername")
    {
        UserConfigParams::m_default_player = L"";
        StateManager::get()->pushScreen(StoryModeLobbyScreen::getInstance());
    }
    
}   // eventCallback

// -----------------------------------------------------------------------------

void OptionsScreenPlayers::selectPlayer(const irr::core::stringw& name)
{
    ListWidget* players = this->getWidget<ListWidget>("players");
    assert(players != NULL);
    players->selectItemWithLabel(name);
    
    players->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
}

// ----------------------------------------------------------------------------
/** Refreshes the list of players.
 */
bool OptionsScreenPlayers::refreshPlayerList()
{
    ListWidget* players = this->getWidget<ListWidget>("players");
    if (players == NULL) return false;
    // Get rid of previous
    players->clear();
    // Rebuild it
    const int playerAmount = UserConfigParams::m_all_players.size();
    for (int i = 0; i < playerAmount; i++)
    {
        // FIXME: Using a truncated ASCII string for internal ID. Let's cross
        // our fingers and hope no one enters two player names that,
        // when stripped down to ASCII, give the same identifier...
        players->addItem(
            core::stringc(UserConfigParams::m_all_players[i].getName().c_str()).c_str(),
            translations->fribidize(UserConfigParams::m_all_players[i].getName()));
    }

    return true;
}
