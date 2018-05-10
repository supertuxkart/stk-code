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
#include "network/transport_address.hpp"
#include "online/xml_request.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "states_screens/networking_lobby.hpp"
#include "states_screens/online_lan.hpp"
#include "states_screens/online_profile_servers.hpp"
#include "states_screens/online_screen.hpp"

NetworkConfig *NetworkConfig::m_network_config = NULL;
bool           NetworkConfig::m_disable_lan    = false;
const uint8_t  NetworkConfig::m_server_version = 1;

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
    m_max_players           = 4;
    m_cur_user_id           = 0;
    m_cur_user_token        = "";
    m_server_name           = "";
    m_password              = "";
    m_server_discovery_port = stk_config->m_server_discovery_port;
    if (UserConfigParams::m_random_ports)
    {
        m_client_port = 0;
        m_server_port = 0;
    }
    else
    {
        m_client_port = stk_config->m_client_port;
        m_server_port = stk_config->m_server_port;
    }
}   // NetworkConfig

// ----------------------------------------------------------------------------
/** Sets if this instance is a server or client. It also assigns the
 *  private port depending if this is a server or client.
 */
void NetworkConfig::setIsServer(bool b)
{
    m_is_server = b;
}   // setIsServer

// ----------------------------------------------------------------------------
void NetworkConfig::setServerMode(RaceManager::MinorRaceModeType minor,
                                  RaceManager::MajorRaceModeType major)
{
    if (major == RaceManager::MAJOR_MODE_GRAND_PRIX)
    {
        if (minor == RaceManager::MINOR_MODE_NORMAL_RACE)
            m_server_mode = 0;
        else if (minor == RaceManager::MINOR_MODE_TIME_TRIAL)
            m_server_mode = 1;
        else if (minor == RaceManager::MINOR_MODE_FOLLOW_LEADER)
            m_server_mode = 2;
    }
    else
    {
        if (minor == RaceManager::MINOR_MODE_NORMAL_RACE)
            m_server_mode = 3;
        else if (minor == RaceManager::MINOR_MODE_TIME_TRIAL)
            m_server_mode = 4;
        else if (minor == RaceManager::MINOR_MODE_FOLLOW_LEADER)
            m_server_mode = 5;
        else if (minor == RaceManager::MINOR_MODE_3_STRIKES)
            m_server_mode = 6;
        else if (minor == RaceManager::MINOR_MODE_SOCCER)
            m_server_mode = 7;
    }
}   // setServerMode

// ----------------------------------------------------------------------------
std::pair<RaceManager::MinorRaceModeType, RaceManager::MajorRaceModeType>
    NetworkConfig::getLocalGameMode()
{
    switch (m_server_mode)
    {
        case 0:
            return { RaceManager::MINOR_MODE_NORMAL_RACE,
                RaceManager::MAJOR_MODE_GRAND_PRIX };
        case 1:
            return { RaceManager::MINOR_MODE_TIME_TRIAL,
                RaceManager::MAJOR_MODE_GRAND_PRIX };
        case 2:
            return { RaceManager::MINOR_MODE_FOLLOW_LEADER,
                RaceManager::MAJOR_MODE_GRAND_PRIX };
        case 3:
            return { RaceManager::MINOR_MODE_NORMAL_RACE,
                RaceManager::MAJOR_MODE_SINGLE };
        case 4:
            return { RaceManager::MINOR_MODE_TIME_TRIAL,
                RaceManager::MAJOR_MODE_SINGLE };
        case 5:
            return { RaceManager::MINOR_MODE_FOLLOW_LEADER,
                RaceManager::MAJOR_MODE_SINGLE };
        case 6:
            return { RaceManager::MINOR_MODE_3_STRIKES,
                RaceManager::MAJOR_MODE_SINGLE };
        case 7:
            return { RaceManager::MINOR_MODE_SOCCER,
                RaceManager::MAJOR_MODE_SINGLE };
        default:
            break;
    }
    return { RaceManager::MINOR_MODE_NORMAL_RACE,
        RaceManager::MAJOR_MODE_SINGLE };

}   // getLocalGameMode

// ----------------------------------------------------------------------------
void NetworkConfig::setUserDetails(Online::XMLRequest* r,
                                   const std::string& name)
{
    assert(!m_cur_user_token.empty());
    r->setApiURL(Online::API::SERVER_PATH, name);
    r->addParameter("userid", m_cur_user_id);
    r->addParameter("token", m_cur_user_token);
}   // setUserDetails

// ----------------------------------------------------------------------------
core::stringw NetworkConfig::getModeName(unsigned id)
{
    switch(id)
    {
        case 0:
            return _("Normal Race (Grand Prix)");
        case 1:
            return _("Time Trial (Grand Prix)");
        case 3:
            return _("Normal Race");
        case 4:
            return _("Time Trial");
        case 6:
            return _("3 Strikes Battle");
        case 7:
            return _("Soccer");
        default:
            return L"";
    }
}   // getModeName

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
