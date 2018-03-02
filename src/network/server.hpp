//
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

#ifndef HEADER_SERVER_HPP
#define HEADER_SERVER_HPP

/**
  * \defgroup onlinegroup Online
  * Represents a server that is joinable
  */

#include "network/transport_address.hpp"
#include "race/race_manager.hpp"
#include "utils/types.hpp"

#include <irrString.h>

#include <string>

class XMLNode;

/**
 * \ingroup online
 */
class Server
{
public:

    /** Set the sort order used in the comparison function. */
    enum SortOrder
    {
        SO_NAME = 0,     // Sorted alphabetically by name
        SO_PLAYERS = 1,
        SO_DIFFICULTY = 2,
        SO_GAME_MODE = 3,
        SO_SCORE = 4    // Sorted on satisfaction score (unused)

    };

protected:
    /** The server name to be displayed. */
    irr::core::stringw m_name;

    /** Name in lower case for comparisons. */
    std::string m_lower_case_name;

    uint32_t m_server_id;
    uint32_t m_host_id;

    /** The maximum number of players that the server supports */
    int m_max_players;

    /** The number of players currently on the server */
    int m_current_players;

    /** The score/rating given */
    float m_satisfaction_score;

    /** True if this server is on the LAN, false otherwise. */
    bool m_is_lan;

    /** The public ip address and port of this server. */
    TransportAddress m_address;

    /** This is the private port of the server. This is used if a WAN game
     *  is started, but one client is discovered on the same LAN, so a direct
     *  connection using the private port is possible. */
    uint16_t m_private_port;

    RaceManager::MinorRaceModeType m_minor_mode;

    RaceManager::MajorRaceModeType m_major_mode;

    RaceManager::Difficulty m_difficulty;

    /** The sort order to be used in the comparison. */
    static SortOrder m_sort_order;

public:

         /** Initialises the object from an XML node. */
         Server(const XMLNode &xml);
         Server(unsigned server_id, const irr::core::stringw &name,
                int max_players, int current_players, unsigned difficulty,
                unsigned server_mode, const TransportAddress &address);
    bool filterByWords(const irr::core::stringw words) const;
    // ------------------------------------------------------------------------
    /** Returns ip address and port of this server. */
    const TransportAddress& getAddress() const { return m_address; }
    // ------------------------------------------------------------------------
    /** Sets the sort order used in the comparison function. It is static, so
    *  that each instance can access the sort order. */
    static void setSortOrder(SortOrder so) { m_sort_order = so; }

    // ------------------------------------------------------------------------
    /** Returns the name of the server. */
    const irr::core::stringw& getName() const { return m_name; }
    // ------------------------------------------------------------------------
    /** Returns the ID of this server. */
    const uint32_t getServerId() const { return m_server_id; }
    // ------------------------------------------------------------------------
    /** Returns the unique host id of this server. */
    const uint32_t getHostId() const { return m_host_id; }
    // ------------------------------------------------------------------------
    /** Returns the maximum number of players allowed on this server. */
    const int getMaxPlayers() const { return m_max_players; }
    // ------------------------------------------------------------------------
    /** Returns the number of currently connected players. */
    const int getCurrentPlayers() const { return m_current_players; }
    // ------------------------------------------------------------------------
    RaceManager::MinorRaceModeType getRaceMinorMode() const
                                                       { return m_minor_mode; }
    // ------------------------------------------------------------------------
    RaceManager::MajorRaceModeType getRaceMajorMode() const
                                                       { return m_major_mode; }
    // ------------------------------------------------------------------------
    RaceManager::Difficulty getDifficulty() const      { return m_difficulty; }
    // ------------------------------------------------------------------------
    /** Compares two servers according to the sort order currently defined.
     *  \param a The addon to compare this addon to.
     */
    bool operator<(const Server &server) const
    {
        switch (m_sort_order)
        {
        case SO_SCORE:
            return m_satisfaction_score < server.m_satisfaction_score;
            break;
        case SO_NAME:
            // m_id is the lower case name
            return m_lower_case_name < server.m_lower_case_name;
            break;
        case SO_PLAYERS:
            return m_current_players < server.m_current_players;
            break;
        case SO_DIFFICULTY:
            return m_difficulty < server.m_difficulty;
            break;
        case SO_GAME_MODE:
            return m_minor_mode < server.m_minor_mode;
            break;
        }   // switch

        return true;
    }   // operator<

};   // Server
#endif // HEADER_SERVER_HPP
