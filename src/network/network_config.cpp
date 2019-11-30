//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2015 Joerg Henrichs
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

#include "network/network_config.hpp"
#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "input/device_manager.hpp"
#include "modes/world.hpp"
#include "network/rewind_manager.hpp"
#include "network/server_config.hpp"
#include "network/transport_address.hpp"
#include "online/xml_request.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "states_screens/online/networking_lobby.hpp"
#include "states_screens/online/online_lan.hpp"
#include "states_screens/online/online_profile_servers.hpp"
#include "states_screens/online/online_screen.hpp"
#include "states_screens/state_manager.hpp"

NetworkConfig *NetworkConfig::m_network_config = NULL;

/** \class NetworkConfig
 *  This class is the interface between STK and the online code, particularly
 *  STKHost. It stores all online related properties (e.g. if this is a server
 *  or a host, name of the server, maximum number of players, ip address, ...).
 *  They can either be set from the GUI code, or via the command line (for a
 *  stand-alone server).
 *  When STKHost is created, it takes all necessary information from this
 *  instance.
 */
// ============================================================================
/** Constructor.
 */
NetworkConfig::NetworkConfig()
{
    m_network_type          = NETWORK_NONE;
    m_auto_connect          = false;
    m_is_server             = false;
    m_is_public_server      = false;
    m_done_adding_network_players = false;
    m_cur_user_id           = 0;
    m_cur_user_token        = "";
    m_client_port = UserConfigParams::m_random_client_port ?
        0 : stk_config->m_client_port;
    m_joined_server_version = 0;
    m_network_ai_tester = false;
    m_state_frequency = 10;
}   // NetworkConfig

// ----------------------------------------------------------------------------
/** Set that this is not a networked game.
 */
void NetworkConfig::unsetNetworking()
{
    clearServerCapabilities();
    m_network_type = NETWORK_NONE;
    ServerConfig::m_private_server_password = "";
}   // unsetNetworking

// ----------------------------------------------------------------------------
void NetworkConfig::setUserDetails(std::shared_ptr<Online::XMLRequest> r,
                                   const std::string& name)
{
    assert(!m_cur_user_token.empty());
    r->setApiURL(Online::API::USER_PATH, name);
    r->addParameter("userid", m_cur_user_id);
    r->addParameter("token", m_cur_user_token);
}   // setUserDetails

// ----------------------------------------------------------------------------
void NetworkConfig::setServerDetails(std::shared_ptr<Online::XMLRequest> r,
                                     const std::string& name)
{
    assert(!m_cur_user_token.empty());
    r->setApiURL(Online::API::SERVER_PATH, name);
    r->addParameter("userid", m_cur_user_id);
    r->addParameter("token", m_cur_user_token);
}   // setServerDetails

// ----------------------------------------------------------------------------
std::vector<GUIEngine::Screen*>
    NetworkConfig::getResetScreens(bool lobby) const
{
    if (lobby)
    {
        if (isWAN())
        {
            return
                {
                    MainMenuScreen::getInstance(),
                    OnlineScreen::getInstance(),
                    OnlineProfileServers::getInstance(),
                    NetworkingLobby::getInstance(),
                    nullptr
                };
        }
        else
        {
            return
                {
                    MainMenuScreen::getInstance(),
                    OnlineScreen::getInstance(),
                    OnlineLanScreen::getInstance(),
                    NetworkingLobby::getInstance(),
                    nullptr
                };
        }
    }
    else
    {
        if (isWAN())
        {
            return
                {
                    MainMenuScreen::getInstance(),
                    OnlineScreen::getInstance(),
                    OnlineProfileServers::getInstance(),
                    nullptr
                };
        }
        else
        {
            return
                {
                    MainMenuScreen::getInstance(),
                    OnlineScreen::getInstance(),
                    OnlineLanScreen::getInstance(),
                    nullptr
                };
        }
    }
}   // getResetScreens

// ----------------------------------------------------------------------------
/** Called before (re)starting network race, must be used before adding
 *  split screen players. */
void NetworkConfig::clearActivePlayersForClient() const
{
    if (!isClient())
        return;
    StateManager::get()->resetActivePlayers();
    if (input_manager)
    {
        input_manager->getDeviceManager()->setAssignMode(NO_ASSIGN);
        input_manager->getDeviceManager()->setSinglePlayer(NULL);
        input_manager->setMasterPlayerOnly(false);
        input_manager->getDeviceManager()->clearLatestUsedDevice();
    }
}   // clearActivePlayersForClient

// ----------------------------------------------------------------------------
/** True when client needs to round the bodies phyiscal info for current
 *  ticks, server doesn't as it will be done implictly in save state. */
bool NetworkConfig::roundValuesNow() const
{
    return isNetworking() && !isServer() && RewindManager::get()
        ->shouldSaveState(World::getWorld()->getTicksSinceStart());
}   // roundValuesNow
