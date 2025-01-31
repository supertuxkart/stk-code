//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2015 SuperTuxKart-Team
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

#include "config/favorite_status.hpp"

#include "config/player_manager.hpp"
#include "io/utf_writer.hpp"
#include "io/xml_node.hpp"
#include "utils/string_utils.hpp"

const std::string FavoriteStatus::DEFAULT_FAVORITE_GROUP_NAME = "Favorites";

//------------------------------------------------------------------------------
FavoriteStatus::FavoriteStatus(const XMLNode* node, std::string parse_type)
{
    m_parse_type = parse_type;

    std::vector<XMLNode*> xml_favorite_tracks;
    std::vector<XMLNode*> xml_favorite_groups;

    if (node)
    {
        node->getNodes(parse_type.c_str(), xml_favorite_tracks);
        node->getNodes("group", xml_favorite_groups);
    }
    for (unsigned int i = 0; i < xml_favorite_tracks.size(); i++)
    {
        std::string temp_string;
        xml_favorite_tracks[i]->get("ident", &temp_string);
        m_favorite[DEFAULT_FAVORITE_GROUP_NAME].insert(temp_string);
    }
    for (unsigned int i = 0; i < xml_favorite_groups.size(); i++)
    {
        std::string temp_group_string;
        std::vector<XMLNode*> temp_group;

        xml_favorite_groups[i]->get("name", &temp_group_string);
        xml_favorite_groups[i]->getNodes(parse_type.c_str(), temp_group);

        for (unsigned int j = 0; j < temp_group.size(); j++)
        {
            std::string temp_string;
            temp_group[j]->get("ident", &temp_string);
            m_favorite[temp_group_string].insert(temp_string);
        }
    }
}   // FavoriteStatus

//------------------------------------------------------------------------------
/** Adds a new favorite track to this player profile and to the group
 * of favorite tracks of the Track Manager.
 * To be used only if this player profile is the current player.
 */
bool FavoriteStatus::isFavorite(std::string ident)
{
    return m_favorite[DEFAULT_FAVORITE_GROUP_NAME].find(ident)
        != m_favorite[DEFAULT_FAVORITE_GROUP_NAME].end();
} // addFavorite

//------------------------------------------------------------------------------
/** Adds a new favorite track to this player profile and to the group
 * of favorite tracks of the Track Manager.
 */
void FavoriteStatus::addFavorite(std::string ident, std::string group)
{
    m_favorite[group].insert(ident);
} // addFavorite

//------------------------------------------------------------------------------
/** Removes a favorite track from this player profile and from the group
 * of favorite tracks of the Track Manager.
 */
void FavoriteStatus::removeFavorite(std::string ident, std::string group)
{
    if (m_favorite[group].find(ident) != m_favorite[group].end())
    {
        m_favorite[group].erase(ident);
    }
} // removeFavorite

//------------------------------------------------------------------------------
/** Writes the data for this player to the specified UTFWriter.
 *  \param out The utf writer to write the data to.
 */
void FavoriteStatus::save(UTFWriter &out)
{
    for (auto it_group = m_favorite.begin(); it_group != m_favorite.end(); it_group++)
    {
        std::string group_name = it_group->first;

        if (group_name == DEFAULT_FAVORITE_GROUP_NAME)
        {
            for (auto it_track = it_group->second.begin(); it_track != it_group->second.end(); it_track++)
            {
                out << "        <" << m_parse_type.c_str() << " ident=\"" << *it_track << "\"/>\n";
            }
        }
        else
        {
            out << "        <group name=\"" << group_name << "\">\n";
            for (auto it_track = it_group->second.begin(); it_track != it_group->second.end(); it_track++)
            {
                out << "          <" << m_parse_type.c_str() << " ident=\"" << *it_track << "\"/>\n";
            }
            out << "        </group>\n";
        }
    }
}   // save
