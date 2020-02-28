//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 Glenn De Jonghe
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

#include "states_screens/online/online_profile_servers.hpp"

#include "guiengine/screen.hpp"
#include "network/network_config.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/online/create_server_screen.hpp"
#include "states_screens/online/online_lan.hpp"
#include "states_screens/online/server_selection.hpp"
#include "utils/translation.hpp"

#include <IGUIButton.h>

#include <iostream>
#include <sstream>

using namespace GUIEngine;
using namespace irr::core;
using namespace irr::gui;
using namespace Online;

// -----------------------------------------------------------------------------

OnlineLanScreen::OnlineLanScreen() : GUIEngine::Screen("online/lan.stkgui")
{
}   // OnlineLanScreen

// -----------------------------------------------------------------------------

void OnlineLanScreen::init()
{
    RibbonWidget* ribbon = getWidget<RibbonWidget>("lan");
    assert(ribbon != NULL);
    ribbon->select("find_lan_server", PLAYER_ID_GAME_MASTER);
    ribbon->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
}   // init

// -----------------------------------------------------------------------------

void OnlineLanScreen::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    if (name == "back")
    {
        StateManager::get()->escapePressed();
        return;
    }
    if (name == "lan")
    {
        RibbonWidget* ribbon = dynamic_cast<RibbonWidget*>(widget);
        std::string selection = ribbon->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        if (selection == "find_lan_server")
        {
            NetworkConfig::get()->setIsLAN();
            NetworkConfig::get()->setIsServer(false);
            ServerSelection::getInstance()->push();
        }
        else if (selection == "create_lan_server")
        {
            NetworkConfig::get()->setIsLAN();
            CreateServerScreen::getInstance()->push();
        }
    }
    
}   // eventCallback

// ----------------------------------------------------------------------------
/** Also called when pressing the back button. It resets the flags to indicate
 *  a networked game.
 */
bool OnlineLanScreen::onEscapePressed()
{
    NetworkConfig::get()->unsetNetworking();
    return true;
}   // onEscapePressed

