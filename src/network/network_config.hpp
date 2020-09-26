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

/*! \file network_config.hpp
 *  \brief Defines network configuration for server and client.
 */
#ifndef HEADER_NETWORK_CONFIG
#define HEADER_NETWORK_CONFIG

#include "race/race_manager.hpp"
#include "utils/stk_process.hpp"
#include "utils/no_copy.hpp"

#include "irrString.h"
#include <array>
#include <atomic>
#include <cstring>
#include <memory>
#include <set>
#include <tuple>
#include <vector>
#include <utility>

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
public:
    enum IPType : int
    {
        IP_NONE, IP_V4, IP_V6, IP_V6_NAT64, IP_DUAL_STACK
    };
private:
    /** The singleton instance. */
    static NetworkConfig *m_network_config[PT_COUNT];

    static bool m_system_ipv4;

    static bool m_system_ipv6;

    enum NetworkType : int
    {
        NETWORK_NONE, NETWORK_WAN, NETWORK_LAN
    };

    std::atomic<IPType> m_ip_type;

    /** Keeps the type of network connection: none (yet), LAN or WAN. */
    std::atomic<NetworkType> m_network_type;

    /** If set it allows clients to connect directly to this server without
     *  using the stk server in between. It requires obviously that this
     *  server is accessible (through the firewall) from the outside. */
    bool m_is_public_server;

    /** True if this host is a server, false otherwise. */
    std::atomic_bool m_is_server;

    /** True if a client should connect to the first server it finds and
     *  immediately start a race. */
    bool m_auto_connect;

    bool m_done_adding_network_players;

    /** True if this STK instance is an AI instance which is used for server
     *  AI. (usually used together with ai-handling in server config) */
    bool m_network_ai_instance;

    /** When live join is disabled addon kart will use their real hitbox */
    bool m_tux_hitbox_addon;

    /** No. of fixed AI in all-in-one graphical client server, the player
     *  connecting with 127.* or ::1/128 will be in charged of controlling the
     *  AI. */
    unsigned m_num_fixed_ai;

    /** The LAN port on which a client is waiting for a server connection. */
    uint16_t m_client_port;

    /** Used by wan server. */
    uint32_t m_cur_user_id;
    std::string m_cur_user_token;

    /** Used by client server to determine if the child server is created. */
    std::string m_server_id_file;

    std::vector<std::tuple<InputDevice*, PlayerProfile*, HandicapLevel> > m_network_players;

    NetworkConfig();

    uint32_t m_joined_server_version;

    /** Set by client or server which is required to be the same. */
    int m_state_frequency;

    /** List of server capabilities set when joining it, to determine features
     *  available in same version. */
    std::set<std::string> m_server_capabilities;

    /** For IPv6 only network we try to detect the NAT64 prefix so we can
     *  use it to connect to ipv4 only servers. STK assumes that for all ipv4
     *  addresses they use the same prefix for each initIPTest. */
    std::string m_nat64_prefix;
    std::array<uint32_t, 8> m_nat64_prefix_data;
    // ------------------------------------------------------------------------
    static void fillStunList(std::vector<std::pair<std::string, int> >* l,
                             const std::string& dns);

public:
    static void initSystemIP();
    // ------------------------------------------------------------------------
    /** Singleton get, which creates this object if necessary. */
    static NetworkConfig *get()
    {
        ProcessType type = STKProcess::getType();
        if (!m_network_config[type])
            m_network_config[type] = new NetworkConfig();
        return m_network_config[type];
    }   // get
    // ------------------------------------------------------------------------
    static NetworkConfig* getByType(ProcessType type)
    {
        return m_network_config[type];
    }   // get
    // ------------------------------------------------------------------------
    static void clearDetectIPThread(bool quit_stk);
    // ------------------------------------------------------------------------
    static void queueIPDetection();
    // ------------------------------------------------------------------------
    static void destroy()
    {
        ProcessType type = STKProcess::getType();
        delete m_network_config[type];   // It's ok to delete NULL
        m_network_config[type] = NULL;
        if (type == PT_MAIN)
            clearDetectIPThread(true/*quit_stk*/);
    }   // destroy
    // ------------------------------------------------------------------------
    static void clear()
    {
        memset(m_network_config, 0, sizeof(m_network_config));
    }   // clear
    // ------------------------------------------------------------------------
    /** Sets if this instance is a server or client. */
    void setIsServer(bool b)
    {
        m_is_server = b;
    }   // setIsServer
    // ------------------------------------------------------------------------
    /** Sets the port on which a client listens for server connection. */
    void setClientPort(uint16_t port) { m_client_port = port; }
    // ------------------------------------------------------------------------
    /** Returns the port on which a client listens for server connections. */
    uint16_t getClientPort() const { return m_client_port; }
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
    void unsetNetworking();
    // ------------------------------------------------------------------------
    std::vector<std::tuple<InputDevice*, PlayerProfile*, HandicapLevel> >&
                        getNetworkPlayers()       { return m_network_players; }
    // ------------------------------------------------------------------------
    bool isAddingNetworkPlayers() const
                                     { return !m_done_adding_network_players; }
    // ------------------------------------------------------------------------
    void doneAddingNetworkPlayers()   { m_done_adding_network_players = true; }
    // ------------------------------------------------------------------------
    bool addNetworkPlayer(InputDevice* device, PlayerProfile* profile,
                          HandicapLevel h)
    {
        for (auto& p : m_network_players)
        {
            if (std::get<0>(p) == device && !m_network_ai_instance)
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
    /** Returns if this instance is a server. */
    bool isServer() const { return m_is_server;  }
    // ------------------------------------------------------------------------
    /** Returns if this instance is a client. */
    bool isClient() const { return !m_is_server; }
    // ------------------------------------------------------------------------
    /** Sets if a client should immediately connect to the first server. */
    void setAutoConnect(bool b) { m_auto_connect = b; }
    // ------------------------------------------------------------------------
    /** Returns if an immediate connection to the first server was
     *  requested. */
    bool isAutoConnect() const { return m_auto_connect; }
    // ------------------------------------------------------------------------
    void setNetworkAIInstance(bool b) { m_network_ai_instance = b; }
    // ------------------------------------------------------------------------
    bool isNetworkAIInstance() const { return m_network_ai_instance; }
    // ------------------------------------------------------------------------
    void setCurrentUserId(uint32_t id) { m_cur_user_id = id ; }
    // ------------------------------------------------------------------------
    void setCurrentUserToken(const std::string& t) { m_cur_user_token = t; }
    // ------------------------------------------------------------------------
    uint32_t getCurrentUserId() const { return m_cur_user_id; }
    // ------------------------------------------------------------------------
    const std::string& getCurrentUserToken() const { return m_cur_user_token; }
    // ------------------------------------------------------------------------
    void setUserDetails(std::shared_ptr<Online::XMLRequest> r,
                        const std::string& name);
    // ------------------------------------------------------------------------
    void setServerDetails(std::shared_ptr<Online::XMLRequest> r,
                          const std::string& name);
    // ------------------------------------------------------------------------
    void setServerIdFile(const std::string& id) { m_server_id_file = id; }
    // ------------------------------------------------------------------------
    const std::string& getServerIdFile() const { return m_server_id_file; }
    // ------------------------------------------------------------------------
    std::vector<GUIEngine::Screen*> getResetScreens(bool lobby = false) const;
    // ------------------------------------------------------------------------
    void setJoinedServerVersion(uint32_t v)    { m_joined_server_version = v; }
    // ------------------------------------------------------------------------
    uint32_t getJoinedServerVersion() const { return m_joined_server_version; }
    // ------------------------------------------------------------------------
    void clearActivePlayersForClient() const;
    // ------------------------------------------------------------------------
    void setStateFrequency(int frequency)    { m_state_frequency = frequency; }
    // ------------------------------------------------------------------------
    int getStateFrequency() const                 { return m_state_frequency; }
    // ------------------------------------------------------------------------
    bool roundValuesNow() const;
    // ------------------------------------------------------------------------
    void setServerCapabilities(std::set<std::string>& caps)
                                   { m_server_capabilities = std::move(caps); }
    // ------------------------------------------------------------------------
    void clearServerCapabilities()           { m_server_capabilities.clear(); }
    // ------------------------------------------------------------------------
    const std::set<std::string>& getServerCapabilities() const
                                              { return m_server_capabilities; }
    // ------------------------------------------------------------------------
    void getIPDetectionResult(uint64_t timeout);
    // ------------------------------------------------------------------------
    IPType getIPType() const                       { return m_ip_type.load(); }
    // ------------------------------------------------------------------------
    void setIPType(IPType ip_type)                { m_ip_type.store(ip_type); }
    // ------------------------------------------------------------------------
    const std::string& getNAT64Prefix() const        { return m_nat64_prefix; }
    // ------------------------------------------------------------------------
    const std::array<uint32_t, 8>& getNAT64PrefixData() const
                                                { return m_nat64_prefix_data; }
    // ------------------------------------------------------------------------
    void initClientPort();
    // ------------------------------------------------------------------------
    void setNumFixedAI(unsigned num)                  { m_num_fixed_ai = num; }
    // ------------------------------------------------------------------------
    unsigned getNumFixedAI() const                   { return m_num_fixed_ai; }
    // ------------------------------------------------------------------------
    static const std::vector<std::pair<std::string, int> >&
                                                        getStunList(bool ipv4);
    // ------------------------------------------------------------------------
    void setTuxHitboxAddon(bool val)              { m_tux_hitbox_addon = val; }
    // ------------------------------------------------------------------------
    bool useTuxHitboxAddon() const               { return m_tux_hitbox_addon; }
};   // class NetworkConfig

#endif // HEADER_NETWORK_CONFIG
