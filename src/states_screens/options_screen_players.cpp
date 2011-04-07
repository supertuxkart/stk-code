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

#include "config/player.hpp"
#include "config/device_config.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"
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
    
    const int playerAmount = UserConfigParams::m_all_players.size();
    for(int n=0; n<playerAmount; n++)
    {
        // FIXME: Using a truncated ASCII string for internal ID. Let's cross our fingers
        //        and hope no one enters two player names that, when stripped down to ASCII,
        //        give the same identifier...
        players->addItem( core::stringc(UserConfigParams::m_all_players[n].getName().c_str()).c_str(),
                          translations->fribidize(UserConfigParams::m_all_players[n].getName()) );
    }
}   // init

// -----------------------------------------------------------------------------

bool OptionsScreenPlayers::gotNewPlayerName(const stringw& newName, PlayerProfile* player)
{
    // FIXME: Using a truncated ASCII string for internal ID. Let's cross our fingers
    //        and hope no one enters two player names that, when stripped down to ASCII,
    //        give the same identifier...
    stringc newNameC( newName );
    
    ListWidget* players = this->getWidget<ListWidget>("players");
    if (players == NULL) return false;
    
    // ---- Add new player
    if (player == NULL)
    {
        // check for duplicates
        const int amount = UserConfigParams::m_all_players.size();
        for (int n=0; n<amount; n++)
        {
            if (UserConfigParams::m_all_players[n].getName() == newName) return false;
        }
        
        // add new player
        UserConfigParams::m_all_players.push_back( new PlayerProfile(newName) );
        
        players->addItem( newNameC.c_str(), newName );
    }
    else // ---- Rename existing player
    {
        player->setName( newName );
        
        // refresh list display
        players->clear();
        const int playerAmount =  UserConfigParams::m_all_players.size();
        for(int n=0; n<playerAmount; n++)
        {
            players->addItem(newNameC.c_str(), UserConfigParams::m_all_players[n].getName());
        }
        
    }
    return true;
}   // gotNewPlayerName

// -----------------------------------------------------------------------------

void OptionsScreenPlayers::deletePlayer(PlayerProfile* player)
{
    UserConfigParams::m_all_players.erase(player);
    
    // refresh list display
    ListWidget* players = this->getWidget<ListWidget>("players");
    if(players == NULL) return;
    players->clear();
    
    const int playerAmount =  UserConfigParams::m_all_players.size();
    for(int n=0; n<playerAmount; n++)
    {
        players->addItem(core::stringc(UserConfigParams::m_all_players[n].getName().c_str()).c_str(),
                         UserConfigParams::m_all_players[n].getName());
    }
}   // deletePlayer

// -----------------------------------------------------------------------------

void OptionsScreenPlayers::tearDown()
{
    Screen::tearDown();
    user_config->saveConfig();
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
        new EnterPlayerNameDialog(0.5f, 0.4f);
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
            if (UserConfigParams::m_all_players[n].getName() == selectedPlayer)
            {
                if (!(UserConfigParams::m_all_players[n].isGuestAccount()))
                {
                    new PlayerInfoDialog( &UserConfigParams::m_all_players[n], 0.5f, 0.6f );
                }
                return;
            }
        } // end for
    }   // name=="players"
    
}   // eventCallback

// -----------------------------------------------------------------------------
