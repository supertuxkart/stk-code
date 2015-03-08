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
/**
  \page online Online
  */

#include "online/server.hpp"

#include "io/xml_node.hpp"
#include "utils/constants.hpp"
#include "utils/string_utils.hpp"

namespace Online
{
    Server::SortOrder Server::m_sort_order = Server::SO_NAME;

    Server::Server(const XMLNode & xml)
    {
        assert(xml.getName() == "server");

        m_name                      = "";
        m_satisfaction_score        = 0;
        m_server_id                 = 0;
        m_current_players           = 0;
        m_max_players               = 0;

        xml.get("name", &m_lower_case_name);
        m_name = StringUtils::xmlDecode(m_lower_case_name);
        m_lower_case_name = StringUtils::toLowerCase(m_lower_case_name);

        xml.get("id",               &m_server_id);
        xml.get("hostid",           &m_host_id);
        xml.get("max_players",      &m_max_players);
        xml.get("current_players",  &m_current_players);

    } // Server(const XML&)

    // ----------------------------------------------------------------------------
    /**
     * \brief Filter the add-on with a list of words.
     * \param words A list of words separated by ' '.
     * \return true if the add-on contains one of the words, otherwise false.
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
} // namespace Online
