//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 Glenn De Jonghe
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

#include "network/server.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "io/xml_node.hpp"
#include "online/online_player_profile.hpp"
#include "online/online_profile.hpp"
#include "online/profile_manager.hpp"
#include "network/network_config.hpp"
#include "network/socket_address.hpp"
#include "states_screens/online/online_screen.hpp"
#include "tracks/track_manager.hpp"
#include "utils/constants.hpp"
#include "utils/string_utils.hpp"

#include <algorithm>

/** Constructor based on XML data received from the stk server.
 *  \param xml The data for one server as received as part of the
 *         get-all stk-server request.
 */
Server::Server(const XMLNode& server_info) : m_supports_encrytion(true)
{
    const XMLNode& xml = *server_info.getNode("server-info");
    m_address.reset(new SocketAddress());

    m_reconnect_when_quit_lobby = false;
    m_ipv6_connection = false;
    m_name = "";
    m_server_id = 0;
    m_current_players = 0;
    m_current_ai = 0;
    m_bookmark_id = 0;
    m_max_players = 0;
    m_distance = 0.0f;
    m_server_mode = 0;
    xml.get("game_mode", &m_server_mode);
    unsigned server_data = 0;
    xml.get("difficulty", &server_data);
    m_difficulty = (RaceManager::Difficulty)server_data;

    xml.get("name", &m_lower_case_name);
    m_name = StringUtils::xmlDecode(m_lower_case_name);
    m_lower_case_name = StringUtils::toLowerCase(
        StringUtils::wideToUtf8(m_name));

    xml.get("id", &m_server_id);
    xml.get("host_id", &m_server_owner);
    xml.get("max_players", &m_max_players);
    xml.get("current_players", &m_current_players);
    xml.get("current_ai", &m_current_ai);
    xml.get("current_track", &m_current_track);
    uint32_t ip;
    xml.get("ip", &ip);
    m_address->setIP(ip);
    uint16_t port;
    xml.get("port", &port);
    m_address->setPort(port);
    std::string ipv6_address;
    xml.get("ipv6", &ipv6_address);
    if (!ipv6_address.empty())
        m_ipv6_address.reset(new SocketAddress(ipv6_address, port));
    xml.get("private_port", &m_private_port);
    xml.get("password", &m_password_protected);
    xml.get("game_started", &m_game_started);
    xml.get("distance", &m_distance);
    xml.get("country_code", &m_country_code);
    m_server_owner_name = L"-";
    m_server_owner_lower_case_name = "-";

    const XMLNode* players = server_info.getNode("players");
    assert(players);
    for (unsigned int i = 0; i < players->getNumNodes(); i++)
    {
        const XMLNode* player_info = players->getNode(i);
        assert(player_info->getName() == "player-info");
        std::string username;
        std::tuple<int, core::stringw, double, float> t;
        // Default rank and scores if none
        std::get<0>(t) = -1;
        std::get<2>(t) = 2000.0;
        player_info->get("rank", &std::get<0>(t));
        player_info->get("username", &username);
        std::get<1>(t) = StringUtils::utf8ToWide(username);
        std::string country;
        player_info->get("country-code", &country);
        const core::stringw& flag = StringUtils::getCountryFlag(country);
        if (!flag.empty())
        {
            std::get<1>(t) += L" ";
            std::get<1>(t) += flag;
        }
        m_lower_case_player_names += StringUtils::toLowerCase(username);
        player_info->get("scores", &std::get<2>(t));
        float time_played = 0.0f;
        player_info->get("time-played", &time_played);
        std::get<3>(t) = time_played;
        m_players.push_back(t);
    }
    // Sort by rank
    std::sort(m_players.begin(), m_players.end(),
        [](const std::tuple<int, core::stringw, double, float>& a,
        const std::tuple<int, core::stringw, double, float>& b)->bool
        {
            return std::get<0>(a) < std::get<0>(b);
        });

    std::string key = getBookmarkKey();
    std::map<std::string, uint32_t>& bookmarks =
        UserConfigParams::m_server_bookmarks;
    if (bookmarks.find(key) != bookmarks.end())
    {
        std::map<std::string, uint32_t>& bookmarks_order =
            UserConfigParams::m_server_bookmarks_order;
        auto it = bookmarks_order.find(key);
        if (it == bookmarks_order.end())
        {
            uint32_t max_id = 0;
            for (auto& order : bookmarks_order)
            {
                if (order.second > max_id)
                    max_id = order.second;
            }
            max_id += 1;
            bookmarks_order[key] = max_id;
            m_bookmark_id = max_id;
        }
        else
            m_bookmark_id = it->second;
    }

    // Show owner name as Official right now if official server hoster account
    m_official = false;
    xml.get("official", &m_official);
    if (m_official)
    {
        m_server_owner_name = L"\u2606\u2605STK\u2605\u2606";
        m_server_owner_lower_case_name = "stk";
        return;
    }

    // Display server owner name if they're your friend or localhost
    Online::OnlineProfile* opp =
        PlayerManager::getCurrentPlayer()->getProfile();
    // Check localhost owner
    if (opp && opp->getID() == m_server_owner)
    {
        m_server_owner_name = opp->getUserName();
        m_server_owner_lower_case_name = StringUtils::toLowerCase
            (StringUtils::wideToUtf8(m_server_owner_name));
    }
    else if (opp && opp->hasFetchedFriends())
    {
        // Check friend(s)
        for (uint32_t user_id : opp->getFriends())
        {
            if (user_id == m_server_owner)
            {
                Online::OnlineProfile* friend_profile =
                    Online::ProfileManager::get()->getProfileByID(user_id);
                if (friend_profile)
                {
                    m_server_owner_name = friend_profile->getUserName();
                    m_server_owner_lower_case_name = StringUtils::toLowerCase
                        (StringUtils::wideToUtf8(m_server_owner_name));
                }
            }
        }
    }

} // Server(const XML&)

// ----------------------------------------------------------------------------
/** Manual server creation, based on data received from a LAN server discovery
 *  (see ServersManager::getLANRefresh) or local graphics server creation
 *  where the server info is known already.
 *  \param server_id ID of server.
 *  \param name Name of the server.
 *  \param max_players Maximum number of players allowed on this server.
 *  \param current_players The currently connected number of players.
 *  \param difficulty The difficulty of the server.
 *  \param server_mode The game modes of the server (including minor and major).
 *  \param address IP and port of the server.
 *  \param password_protected True if can only be joined with a password.
 *  \param game_started True if a game has already begun in the server.
 *  \param current_track If server is in game, store the track ident
 */
Server::Server(unsigned server_id, const core::stringw &name, int max_players,
               int current_players, unsigned difficulty, unsigned server_mode,
               const SocketAddress &address, bool password_protected,
               bool game_started, const std::string& current_track)
      : m_supports_encrytion(false)
{
    m_reconnect_when_quit_lobby = false;
    m_ipv6_connection = false;
    m_name               = name;
    m_lower_case_name    = StringUtils::toLowerCase(StringUtils::wideToUtf8(name));
    m_server_id          = server_id;
    m_server_owner       = 0;
    m_current_players    = current_players;
    m_max_players        = max_players;
    m_address.reset(new SocketAddress(address));

    // In case of LAN server, public and private port are the same.
    m_private_port       = m_address->getPort();
    m_difficulty         = (RaceManager::Difficulty)difficulty;
    m_server_mode        = server_mode;
    m_password_protected = password_protected;
    m_distance = 0.0f;
    m_official = false;
    m_game_started = game_started;
    m_current_track = current_track;
    m_current_ai = m_bookmark_id = 0;
}   // server(server_id, ...)

// ----------------------------------------------------------------------------
Server::~Server()
{
}   // ~Server

// ----------------------------------------------------------------------------
Track* Server::getCurrentTrack() const
{
    if (!m_current_track.empty())
        return track_manager->getTrack(m_current_track);
    return NULL;
}   // getCurrentTrack

// ----------------------------------------------------------------------------
bool Server::searchByName(const std::string& lower_case_word)
{
    auto list = StringUtils::split(lower_case_word, ' ', false);
    bool server_name_found = true;
    for (auto& word : list)
    {
        const std::string& for_search = m_lower_case_name +
            m_lower_case_player_names;
        server_name_found = server_name_found &&
            for_search.find(word) != std::string::npos;
    }
    return server_name_found;
}   // searchByName

// ----------------------------------------------------------------------------
void Server::setIPV6Address(const SocketAddress& addr)
{
    m_ipv6_address.reset(new SocketAddress(addr));
}   // setIPV6Address

// ----------------------------------------------------------------------------
void Server::setAddress(const SocketAddress& addr)
{
    m_address.reset(new SocketAddress(addr));
}   // setAddress

// ----------------------------------------------------------------------------
std::string Server::getBookmarkKey() const
{
    core::stringw name = m_name;
    return StringUtils::xmlEncode(name.trim().removeChars(L"\n\r\t")) +
        StringUtils::toString(m_server_owner);
}   // getBookmarkKey

// ----------------------------------------------------------------------------
void UserDefinedServer::saveServer() const
{
    OnlineScreen::getInstance()->setEnteredServerName(m_name);
}   // saveServer
