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
#include "network/network.hpp"
#include "network/rewind_manager.hpp"
#include "network/server_config.hpp"
#include "network/stk_host.hpp"
#include "network/stk_ipv6.hpp"
#include "network/transport_address.hpp"
#include "online/xml_request.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "states_screens/online/networking_lobby.hpp"
#include "states_screens/online/online_lan.hpp"
#include "states_screens/online/online_profile_servers.hpp"
#include "states_screens/online/online_screen.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/time.hpp"

#ifdef WIN32
#  include <winsock2.h>
#  include <ws2tcpip.h>
#else
#  include <netdb.h>
#endif

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
    m_ip_type               = IP_NONE;
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

// ----------------------------------------------------------------------------
/** Use stun servers to detect current ip type.
 */
void NetworkConfig::detectIPType()
{
#ifdef ENABLE_IPV6
    ENetAddress addr;
    addr.host = STKHost::HOST_ANY;
    addr.port = STKHost::PORT_ANY;
    // We don't need to result of stun, just to check if the socket can be
    // used in ipv4 or ipv6
    uint8_t stun_tansaction_id[16] = {};
    BareNetworkString s = STKHost::getStunRequest(stun_tansaction_id);
    setIPv6Socket(0);
    auto ipv4 = std::unique_ptr<Network>(new Network(1, 1, 0, 0, &addr));
    setIPv6Socket(1);
    auto ipv6 = std::unique_ptr<Network>(new Network(1, 1, 0, 0, &addr));
    setIPv6Socket(0);

    auto ipv4_it = UserConfigParams::m_stun_servers_v4.begin();
    int adv = rand() % UserConfigParams::m_stun_servers_v4.size();
    std::advance(ipv4_it, adv);
    auto ipv6_it = UserConfigParams::m_stun_servers.begin();
    adv = rand() % UserConfigParams::m_stun_servers.size();
    std::advance(ipv6_it, adv);

    std::vector<std::string> addrv4_and_port =
        StringUtils::split(ipv4_it->first, ':');
    if (addrv4_and_port.size() != 2)
    {
        Log::error("NetworkConfig", "Wrong server address and port");
        return;
    }

    struct addrinfo hints;
    struct addrinfo* res = NULL;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int status = getaddrinfo_compat(addrv4_and_port[0].c_str(),
        addrv4_and_port[1].c_str(), &hints, &res);
    bool sent_ipv4 = false;
    if (status == 0 && res != NULL)
    {
        if (res->ai_family == AF_INET)
        {
            sendto(ipv4->getENetHost()->socket, s.getData(), s.size(), 0,
                res->ai_addr, sizeof(sockaddr_in));
            sent_ipv4 = true;
        }
        freeaddrinfo(res);
    }

    std::vector<std::string> addrv6_and_port =
        StringUtils::split(ipv6_it->first, ':');
    if (addrv6_and_port.size() != 2)
    {
        Log::error("NetworkConfig", "Wrong server address and port");
        return;
    }

    res = NULL;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    status = getaddrinfo_compat(addrv6_and_port[0].c_str(),
        addrv6_and_port[1].c_str(), &hints, &res);
    bool sent_ipv6 = false;
    if (status == 0 && res != NULL)
    {
        if (res->ai_family == AF_INET6)
        {
            sendto(ipv6->getENetHost()->socket, s.getData(), s.size(), 0,
                res->ai_addr, sizeof(sockaddr_in6));
            sent_ipv6 = true;
        }
        freeaddrinfo(res);
    }

    bool has_ipv4 = false;
    bool has_ipv6 = false;

    ENetSocketSet socket_set;
    ENET_SOCKETSET_EMPTY(socket_set);
    ENET_SOCKETSET_ADD(socket_set, ipv4->getENetHost()->socket);
    if (sent_ipv4)
    {
        // 1.5 second timeout
        has_ipv4 = enet_socketset_select(
            ipv4->getENetHost()->socket, &socket_set, NULL, 1500) > 0;
    }

    ENET_SOCKETSET_EMPTY(socket_set);
    ENET_SOCKETSET_ADD(socket_set, ipv6->getENetHost()->socket);
    if (sent_ipv6 && enet_socketset_select(
        ipv6->getENetHost()->socket, &socket_set, NULL, 1500) > 0)
    {
        has_ipv6 = true;
        // For non dual stack IPv6 we try to get a NAT64 prefix to connect
        // to IPv4 only servers
        if (!has_ipv4)
        {
            // Detect NAT64 prefix by using the IPv4 only stun:
            // All IPv4 only stun servers are *.supertuxkart.net which only
            // have A record, so the below code which forces to get an AF_INET6
            // will return their NAT64 addresses
            res = NULL;
            memset(&hints, 0, sizeof hints);
            hints.ai_family = AF_INET6;
            hints.ai_socktype = SOCK_STREAM;
            status = getaddrinfo_compat(addrv4_and_port[0].c_str(),
                addrv4_and_port[1].c_str(), &hints, &res);
            if (status == 0 && res != NULL)
            {
                if (res->ai_family == AF_INET6)
                {
                    struct sockaddr_in6 nat64 = {};
                    // Copy first 12 bytes
                    struct sockaddr_in6* out =
                        (struct sockaddr_in6*)res->ai_addr;
                    memcpy(nat64.sin6_addr.s6_addr, out->sin6_addr.s6_addr,
                        12);
                    m_nat64_prefix = getIPV6ReadableFromIn6(&nat64);
                }
                freeaddrinfo(res);
            }
        }
    }

    if (has_ipv4 && has_ipv6)
    {
        Log::info("NetworkConfig", "System is dual stack network.");
        m_ip_type = IP_DUAL_STACK;
    }
    else if (has_ipv4)
    {
        Log::info("NetworkConfig", "System is IPv4 only.");
        m_ip_type = IP_V4;
    }
    else if (has_ipv6)
    {
        Log::info("NetworkConfig", "System is IPv6 only.");
        if (m_nat64_prefix.empty())
            m_ip_type = IP_V6;
    }
    else
    {
        Log::error("NetworkConfig", "Cannot detect network type using stun.");
    }
    if (has_ipv6)
    {
        if (!has_ipv4 && m_nat64_prefix.empty())
        {
            Log::warn("NetworkConfig", "NAT64 prefix not found, "
                "you may not be able to join any IPv4 only servers.");
        }
        if (!m_nat64_prefix.empty())
        {
            m_ip_type = IP_V6_NAT64;
            Log::info("NetworkConfig",
                "NAT64 prefix is %s.", m_nat64_prefix.c_str());
        }
    }
#else
    m_ip_type = IP_V4;
#endif
}   // detectIPType
