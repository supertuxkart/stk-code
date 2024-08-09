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

#include "config/favorite_track_status.hpp"

#include "config/player_manager.hpp"
#include "io/utf_writer.hpp"
#include "io/xml_node.hpp"
#include "utils/string_utils.hpp"

const std::string FavoriteTrackStatus::DEFAULT_FAVORITE_GROUP_NAME = "Favorites";

//------------------------------------------------------------------------------
FavoriteTrackStatus::FavoriteTrackStatus(const XMLNode* node)
{
    std::vector<XMLNode*> xml_favorite_tracks;
    std::vector<XMLNode*> xml_favorite_groups;

    if (node)
    {
        node->getNodes("track", xml_favorite_tracks);
        node->getNodes("group", xml_favorite_groups);
    }
    for (unsigned int i = 0; i < xml_favorite_tracks.size(); i++)
    {
        std::string temp_string;
        xml_favorite_tracks[i]->get("ident", &temp_string);
        m_favorite_tracks[DEFAULT_FAVORITE_GROUP_NAME].insert(temp_string);
    }
    for (unsigned int i = 0; i < xml_favorite_groups.size(); i++)
    {
        std::string temp_group_string;
        std::vector<XMLNode*> temp_group;

        xml_favorite_groups[i]->get("name", &temp_group_string);
        xml_favorite_groups[i]->getNodes("track", temp_group);

        for (unsigned int j = 0; j < temp_group.size(); j++)
        {
            std::string temp_string;
            temp_group[j]->get("ident", &temp_string);
            m_favorite_tracks[temp_group_string].insert(temp_string);
        }
    }
}   // FavoriteTrackStatus

//------------------------------------------------------------------------------
FavoriteTrackStatus::~FavoriteTrackStatus()
{

}   // ~FavoriteTrackStatus

//------------------------------------------------------------------------------
/** Adds a new favorite track to this player profile and to the group
 * of favorite tracks of the Track Manager.
 * To be used only if this player profile is the current player.
 */
bool FavoriteTrackStatus::isFavoriteTrack(std::string ident)
{
    return m_favorite_tracks[DEFAULT_FAVORITE_GROUP_NAME].find(ident)
        != m_favorite_tracks[DEFAULT_FAVORITE_GROUP_NAME].end();
} // addFavoriteTrack

//------------------------------------------------------------------------------
/** Adds a new favorite track to this player profile and to the group
 * of favorite tracks of the Track Manager.
 */
void FavoriteTrackStatus::addFavoriteTrack(std::string ident, std::string group)
{
    m_favorite_tracks[group].insert(ident);
} // addFavoriteTrack

//------------------------------------------------------------------------------
/** Removes a favorite track from this player profile and from the group
 * of favorite tracks of the Track Manager.
 */
void FavoriteTrackStatus::removeFavoriteTrack(std::string ident, std::string group)
{
    if (m_favorite_tracks[group].find(ident) != m_favorite_tracks[group].end())
    {
        m_favorite_tracks[group].erase(ident);
    }
} // removeFavoriteTrack

//------------------------------------------------------------------------------
/** Writes the data for this player to the specified UTFWriter.
 *  \param out The utf writer to write the data to.
 */
void FavoriteTrackStatus::save(UTFWriter &out)
{
    out << "      <favorites>\n";
    for (auto it_group = m_favorite_tracks.begin(); it_group != m_favorite_tracks.end(); it_group++)
    {
        std::string group_name = it_group->first;

        if (group_name == DEFAULT_FAVORITE_GROUP_NAME)
        {
            for (auto it_track = it_group->second.begin(); it_track != it_group->second.end(); it_track++)
            {
                out << "        <track ident=\"" << *it_track << "\"/>\n";
            }
        }
        else
        {
            out << "        <group name=\"" << group_name << "\">\n";
            for (auto it_track = it_group->second.begin(); it_track != it_group->second.end(); it_track++)
            {
                out << "          <track ident=\"" << *it_track << "\"/>\n";
            }
            out << "        </group>\n";
        }
    }
    out << "      </favorites>\n";
}   // save
