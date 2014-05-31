//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 Glenn De Jonghe
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

#include <string>
#include <irrString.h>
#include "io/xml_node.hpp"
#include "utils/types.hpp"

class XMLNode;

namespace Online{
    /**
      * \ingroup online
      */
    class Server
    {
    public:

        /** Set the sort order used in the comparison function. */
        enum SortOrder { SO_SCORE   = 1,    // Sorted on satisfaction score
                         SO_NAME    = 2,    // Sorted alphabetically by name
                         SO_PLAYERS = 4
        };

    protected:
        /** The name to be displayed. */
        irr::core::stringw m_name;
        std::string m_lower_case_name; //Used for comparison

        uint32_t m_server_id;
        uint32_t m_host_id;

        int m_max_players;

        int m_current_players;

        float m_satisfaction_score;

        /** The sort order to be used in the comparison. */
        static SortOrder m_sort_order;

        Server() {};

    public:

         /** Initialises the object from an XML node. */
         Server(const XMLNode & xml);
        // ------------------------------------------------------------------------
        /** Sets the sort order used in the comparison function. It is static, so
         *  that each instance can access the sort order. */
        static void setSortOrder(SortOrder so) { m_sort_order = so; }
        // ------------------------------------------------------------------------
        /** Returns the name of the server. */
        const irr::core::stringw& getName() const { return m_name; }
        const std::string & getLowerCaseName() const { return m_lower_case_name; }
        // ------------------------------------------------------------------------
        const float getScore() const { return m_satisfaction_score; }
        // ------------------------------------------------------------------------
        /** Returns the ID of this server. */
        const uint32_t getServerId() const { return m_server_id; }
        const uint32_t getHostId() const { return m_host_id; }
        const int getMaxPlayers() const { return m_max_players; }
        const int getCurrentPlayers() const { return m_current_players; }
        // ------------------------------------------------------------------------
        bool filterByWords(const irr::core::stringw words) const;
        // ------------------------------------------------------------------------

        /** Compares two servers according to the sort order currently defined.
         *  \param a The addon to compare this addon to.
         */
        bool operator<(const Server &server) const
        {
            switch(m_sort_order)
            {
                case SO_SCORE:
                    return m_satisfaction_score < server.getScore();
                    break;
                case SO_NAME:
                    // m_id is the lower case name
                    return m_lower_case_name < server.m_lower_case_name;
                    break;
                case SO_PLAYERS:
                    return m_current_players < server.getCurrentPlayers();
                    break;
            }   // switch
            return true;
        }   // operator<

    };   // Server
} // namespace Online

#endif
