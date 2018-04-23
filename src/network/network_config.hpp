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

/*! \file stk_host.hpp
 *  \brief Defines an interface to use network low-level functions easily.
 */
#ifndef HEADER_NETWORK_CONFIG
#define HEADER_NETWORK_CONFIG

#include "network/transport_address.hpp"
#include "race/race_manager.hpp"
#include "utils/no_copy.hpp"

#include "irrString.h"
#include <tuple>
#include <vector>

namespace Online
{
    class XMLRequest;
}

namespace GUIEngine
{
    class Screen;
}

class InputDevice;
class PlayerProfile;

class NetworkConfig : public NoCopy
{
private:
    /** The singleton instance. */
    static NetworkConfig *m_network_config;

    enum NetworkType
    {
        NETWORK_NONE, NETWORK_WAN, NETWORK_LAN
    };

    /** Keeps the type of network connection: none (yet), LAN or WAN. */
    NetworkType m_network_type;

    /** If set it allows clients to connect directly to this server without
     *  using the stk server in between. It requires obviously that this
     *  server is accessible (through the firewall) from the outside,
     *  then you can use the \ref m_server_discovery_port and ip address for
     *  direct connection. */
    bool m_is_public_server;

    /** True if this host is a server, false otherwise. */
    bool m_is_server;

    /** The password for a server (or to authenticate to a server). */
    std::string m_password;

    /** The port number to which the server listens to detect direct socket
     *  requests. */
    uint16_t m_server_discovery_port;

    /** The port on which the server listens for connection requests from LAN. */
    uint16_t m_server_port;

    /** The LAN port on which a client is waiting for a server connection. */
    uint16_t m_client_port;

    /** Maximum number of players on the server. */
    unsigned m_max_players;

    /** True if a client should connect to the first server it finds and
     *  immediately start a race. */
    bool m_auto_connect;

    bool m_done_adding_network_players;

    /** If this is a server, the server name. */
    irr::core::stringw m_server_name;

    unsigned m_server_mode;

    /** Used by wan server. */
    uint32_t m_cur_user_id;
    std::string m_cur_user_token;

    /** Used by client server to determine if the child server is created. */
    std::string m_server_id_file;

    std::vector<std::tuple<InputDevice*, PlayerProfile*,
        /*is_handicap*/bool> > m_network_players;

    core::stringw m_motd;

    NetworkConfig();

public:
    /** Stores the command line flag to disable lan detection (i.e. force
     *  WAN code to be used when connection client and server). */
    static bool m_disable_lan;

    /** Server version, will be advanced if there are protocol changes. */
    static const uint8_t m_server_version;

    /** Singleton get, which creates this object if necessary. */
    static NetworkConfig *get()
    {
        if (!m_network_config)
            m_network_config = new NetworkConfig();
        return m_network_config;
    }   // get

    // ------------------------------------------------------------------------
    static void destroy()
    {
        delete m_network_config;   // It's ok to delete NULL
        m_network_config = NULL;
    }   // destroy

    // ------------------------------------------------------------------------
    void setIsServer(bool b);
    // ------------------------------------------------------------------------
    /** Sets the port for server discovery. */
    void setServerDiscoveryPort(uint16_t port)
    {
        m_server_discovery_port = port;
    }   // setServerDiscoveryPort
    // ------------------------------------------------------------------------
    /** Sets the port on which this server listens. */
    void setServerPort(uint16_t port) { m_server_port = port; }
    // ------------------------------------------------------------------------
    /** Sets the port on which a client listens for server connection. */
    void setClientPort(uint16_t port) { m_client_port = port; }
    // ------------------------------------------------------------------------
    /** Returns the port on which this server listenes. */
    uint16_t getServerPort() const { return m_server_port; }
    // ------------------------------------------------------------------------
    /** Returns the port for LAN server discovery. */
    uint16_t getServerDiscoveryPort() const { return m_server_discovery_port; }
    // ------------------------------------------------------------------------
    /** Returns the port on which a client listens for server connections. */
    uint16_t getClientPort() const { return m_client_port; }
    // ------------------------------------------------------------------------
    /** Sets the password for a server. */
    void setPassword(const std::string &password) { m_password = password; }
    // ------------------------------------------------------------------------
    /** Returns the password. */
    const std::string& getPassword() const { return m_password; }
    // ------------------------------------------------------------------------
    /** Sets that this server can be contacted directly. */
    void setIsPublicServer() { m_is_public_server = true; }
    // ------------------------------------------------------------------------
    /** Returns if connections directly to the server are to be accepted. */
    bool isPublicServer() const { return m_is_public_server; }
    // ------------------------------------------------------------------------
    /** Return if a network setting is happening. A network setting is active
     *  if a host (server or client) exists. */
    bool isNetworking() const { return m_network_type!=NETWORK_NONE; }
    // ------------------------------------------------------------------------
    /** Return true if it's a networked game with a LAN server. */
    bool isLAN() const { return m_network_type == NETWORK_LAN; }
    // ------------------------------------------------------------------------
    /** Return true if it's a networked game but with a WAN server. */
    bool isWAN() const { return m_network_type == NETWORK_WAN; }
    // ------------------------------------------------------------------------
    /** Set that this is a LAN networked game. */
    void setIsLAN() { m_network_type = NETWORK_LAN; }
    // ------------------------------------------------------------------------
    /** Set that this is a WAN networked game. */
    void setIsWAN() { m_network_type = NETWORK_WAN; }
    // ------------------------------------------------------------------------
    /** Set that this is not a networked game. */
    void unsetNetworking()
    {
        m_network_type = NETWORK_NONE;
        m_password = "";
    }
    // ------------------------------------------------------------------------
    const std::vector<std::tuple<InputDevice*, PlayerProfile*, bool> >&
                        getNetworkPlayers() const { return m_network_players; }
    // ------------------------------------------------------------------------
    bool isAddingNetworkPlayers() const
                                     { return !m_done_adding_network_players; }
    // ------------------------------------------------------------------------
    void doneAddingNetworkPlayers()   { m_done_adding_network_players = true; }
    // ------------------------------------------------------------------------
    bool addNetworkPlayer(InputDevice* device, PlayerProfile* profile, bool h)
    {
        for (auto& p : m_network_players)
        {
            if (std::get<0>(p) == device)
                return false;
            if (std::get<1>(p) == profile)
                return false;
        }
        m_network_players.emplace_back(device, profile, h);
        return true;
    }
    // ------------------------------------------------------------------------
    bool playerExists(PlayerProfile* profile) const
    {
        for (auto& p : m_network_players)
        {
            if (std::get<1>(p) == profile)
                return true;
        }
        return false;
    }
    // ------------------------------------------------------------------------
    void cleanNetworkPlayers()
    {
        m_network_players.clear();
        m_done_adding_network_players = false;
    }
    // ------------------------------------------------------------------------
    /** Sets the maximum number of players for this server. */
    void setMaxPlayers(unsigned n) { m_max_players = n; }
    // ------------------------------------------------------------------------
    /** Returns the maximum number of players for this server. */
    unsigned getMaxPlayers() const { return m_max_players; }
    // ------------------------------------------------------------------------
    /** Returns if this instance is a server. */
    bool isServer() const { return m_is_server;  }
    // ------------------------------------------------------------------------
    /** Returns if this instance is a client. */
    bool isClient() const { return !m_is_server; }
    // ------------------------------------------------------------------------
    /** Sets the name of this server. */
    void setServerName(const irr::core::stringw &name)
    {
        m_server_name = name;
    }   // setServerName
    // ------------------------------------------------------------------------
    /** Returns the server name. */
    const irr::core::stringw& getServerName() const
    {
        assert(isServer());
        return m_server_name;
    }   // getServerName
    // ------------------------------------------------------------------------
    /** Sets if a client should immediately connect to the first server. */
    void setAutoConnect(bool b) { m_auto_connect = b; }
    // ------------------------------------------------------------------------
    /** Returns if an immediate connection to the first server was
     *  requested. */
    bool isAutoConnect() const { return m_auto_connect; }
    // ------------------------------------------------------------------------
    /** Returns the minor and majar game mode from server database id. */
    std::pair<RaceManager::MinorRaceModeType, RaceManager::MajorRaceModeType>
        getLocalGameMode();
    // ------------------------------------------------------------------------
    void setCurrentUserId(uint32_t id) { m_cur_user_id = id ; }
    // ------------------------------------------------------------------------
    void setCurrentUserToken(const std::string& t) { m_cur_user_token = t; }
    // ------------------------------------------------------------------------
    uint32_t getCurrentUserId() const { return m_cur_user_id; }
    // ------------------------------------------------------------------------
    const std::string& getCurrentUserToken() const { return m_cur_user_token; }
    // ------------------------------------------------------------------------
    void setUserDetails(Online::XMLRequest* r, const std::string& name);
    // ------------------------------------------------------------------------
    void setServerIdFile(const std::string& id) { m_server_id_file = id; }
    // ------------------------------------------------------------------------
    const std::string& getServerIdFile() const { return m_server_id_file; }
    // ------------------------------------------------------------------------
    void setMOTD(const core::stringw& motd) { m_motd = motd; }
    // ------------------------------------------------------------------------
    const core::stringw& getMOTD() const { return m_motd; }
    // ------------------------------------------------------------------------
    void setServerMode(RaceManager::MinorRaceModeType mode,
                       RaceManager::MajorRaceModeType);
    // ------------------------------------------------------------------------
    void setServerMode(unsigned mode) { m_server_mode = mode; }
    // ------------------------------------------------------------------------
    unsigned getServerMode() const { return m_server_mode; }
    // ------------------------------------------------------------------------
    core::stringw getModeName(unsigned id);
    // ------------------------------------------------------------------------
    std::vector<GUIEngine::Screen*> getResetScreens(bool lobby = false) const;

};   // class NetworkConfig

#endif // HEADER_NETWORK_CONFIG
