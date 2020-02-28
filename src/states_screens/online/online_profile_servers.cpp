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

#include "audio/sfx_manager.hpp"
#include "config/player_manager.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widget.hpp"
#include "network/network_config.hpp"
#include "network/stk_host.hpp"
#include "network/server_config.hpp"
#include "network/servers_manager.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/online/create_server_screen.hpp"
#include "states_screens/online/networking_lobby.hpp"
#include "states_screens/online/server_selection.hpp"

#include <IGUIButton.h>

#include <iostream>
#include <sstream>

using namespace GUIEngine;
using namespace irr::core;
using namespace irr::gui;
using namespace Online;

// -----------------------------------------------------------------------------

OnlineProfileServers::OnlineProfileServers() : GUIEngine::Screen("online/profile_servers.stkgui")
{
}   // OnlineProfileServers

// -----------------------------------------------------------------------------

void OnlineProfileServers::init()
{
    if (!PlayerManager::getCurrentOnlineId())
    {
        getWidget("back")->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
        getWidget<IconButtonWidget>("find_wan_server")->setActive(false);
        getWidget<IconButtonWidget>("create_wan_server")->setActive(false);
        getWidget<IconButtonWidget>("quick_wan_play")->setActive(false);
    }
    else
    {
        getWidget<IconButtonWidget>("find_wan_server")->setActive(true);
        getWidget<IconButtonWidget>("create_wan_server")->setActive(true);
        getWidget<IconButtonWidget>("quick_wan_play")->setActive(true);
        RibbonWidget* ribbon = getWidget<RibbonWidget>("wan");
        assert(ribbon != NULL);
        ribbon->select("find_wan_server", PLAYER_ID_GAME_MASTER);
        ribbon->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    }
}   // init

// -----------------------------------------------------------------------------

void OnlineProfileServers::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    if (name == "back")
    {
        StateManager::get()->escapePressed();
        return;
    }
    if (name == "wan")
    {
        RibbonWidget* ribbon = dynamic_cast<RibbonWidget*>(widget);
        std::string selection = ribbon->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        if (selection == "find_wan_server")
        {
            NetworkConfig::get()->setIsWAN();
            NetworkConfig::get()->setIsServer(false);
            ServerSelection::getInstance()->push();
        }
        else if (selection == "create_wan_server")
        {
            NetworkConfig::get()->setIsWAN();
            CreateServerScreen::getInstance()->push();
        }
        else if (selection == "quick_wan_play")
        {
            NetworkConfig::get()->setIsWAN();
            NetworkConfig::get()->setIsServer(false);
            doQuickPlay();
        }
    }

}   // eventCallback

// ----------------------------------------------------------------------------
void OnlineProfileServers::doQuickPlay()
{
    ServerConfig::m_private_server_password = "";
    STKHost::create();
    NetworkingLobby::getInstance()->setJoinedServer(nullptr);
    NetworkingLobby::getInstance()->push();
}   // doQuickPlay

// ----------------------------------------------------------------------------
/** Also called when pressing the back button. It resets the flags to indicate
 *  a networked game.
 */
bool OnlineProfileServers::onEscapePressed()
{
    NetworkConfig::get()->unsetNetworking();
    return true;
}   // onEscapePressed

