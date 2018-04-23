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
#include "online/online_player_profile.hpp"
#include "online/online_profile.hpp"
#include "online/profile_manager.hpp"
#include "network/network_config.hpp"
#include "io/xml_node.hpp"
#include "utils/constants.hpp"
#include "utils/string_utils.hpp"

/** Constructor based on XML data received from the stk server.
 *  \param xml The data for one server as received as part of the
 *         get-all stk-server request.
 */
Server::Server(const XMLNode& xml)
{
    assert(xml.getName() == "server");

    m_name = "";
    m_satisfaction_score = 0;
    m_server_id = 0;
    m_current_players = 0;
    m_max_players = 0;
    m_distance = 0.0f;
    m_server_mode = 0;
    xml.get("game_mode", &m_server_mode);
    unsigned server_data = 0;
    xml.get("difficulty", &server_data);
    m_difficulty = (RaceManager::Difficulty)server_data;

    xml.get("name", &m_lower_case_name);
    m_name = StringUtils::xmlDecode(m_lower_case_name);
    m_lower_case_name = StringUtils::toLowerCase(m_lower_case_name);

    xml.get("id", &m_server_id);
    xml.get("host_id", &m_server_owner);
    xml.get("max_players", &m_max_players);
    xml.get("current_players", &m_current_players);
    uint32_t ip;
    xml.get("ip", &ip);
    m_address.setIP(ip);
    uint16_t port;
    xml.get("port", &port);
    m_address.setPort(port);
    xml.get("private_port", &m_private_port);
    xml.get("password", &m_password_protected);
    xml.get("distance", &m_distance);
    m_server_owner_name = "-";

    // Display server owner name if he's your friend or localhost
    Online::OnlineProfile* opp = PlayerManager::getCurrentPlayer()->getProfile();
    // Check localhost owner
    if (opp && opp->getID() == m_server_owner)
    {
        m_server_owner_name =
            StringUtils::wideToUtf8(opp->getUserName());
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
                    m_server_owner_name =
                        StringUtils::wideToUtf8(friend_profile->getUserName());
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
 *  \param difficulty The difficulty of server.
 *  \param server_mode The game modes of server (including minor and major).
 *  \param address IP and port of the server.
 *  \param password_protected True if can only be joined with a password.
 */
Server::Server(unsigned server_id, const core::stringw &name, int max_players,
               int current_players, unsigned difficulty, unsigned server_mode,
               const TransportAddress &address, bool password_protected)
{
    m_name               = name;
    m_lower_case_name    = StringUtils::toLowerCase(StringUtils::wideToUtf8(name));
    m_satisfaction_score = 0;
    m_server_id          = server_id;
    m_server_owner       = 0;
    m_current_players    = current_players;
    m_max_players        = max_players;
    m_address            = address;
    // In case of LAN server, public and private port are the same.
    m_private_port       = m_address.getPort();
    m_difficulty         = (RaceManager::Difficulty)difficulty;
    m_server_mode        = server_mode;
    m_password_protected = password_protected;
    m_distance = 0.0f;
}   // server(server_id, ...)

// ----------------------------------------------------------------------------
/** \brief Filter the add-on with a list of words.
 *  \param words A list of words separated by ' '.
 *  \return true if the add-on contains one of the words, otherwise false.
 */
bool Server::filterByWords(const core::stringw words) const
{
    if (words == NULL || words.empty())
        return true;

    std::vector<core::stringw> list = StringUtils::split(words, ' ', false);

    for (unsigned int i = 0; i < list.size(); i++)
    {
        list[i].make_lower();

        if ((core::stringw(m_name).make_lower()).find(list[i].c_str()) != -1)
        {
            return true;
        }
    }

    return false;
} // filterByWords
