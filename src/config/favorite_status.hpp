//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2024 SuperTuxKart-Team
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

#ifndef HEADER_FAVORITE_STATUS_HPP
#define HEADER_FAVORITE_STATUS_HPP

#include "utils/leak_check.hpp"

#include <irrString.h>

#include <set>
#include <string>
#include <unordered_map>

using namespace irr;

class KartPropertiesManager;
class TrackManager;
class UTFWriter;
class XMLNode;

/** Class for managing player profiles (name, usage frequency,
 *  etc.). All PlayerProfiles are managed by the PlayerManager.
 *  A PlayerProfile keeps track of the story mode progress using an instance
 *  of StoryModeStatus, and achievements with AchievementsStatus. All data
 *  is saved in the players.xml file.
 *  This class also defines the interface for handling online data. All of
 *  the online handling is done in the derived class OnlinePlayerProfile,
 *  where the interface is fully implemented.
 * \ingroup config
 */
class FavoriteStatus
{
private:
    LEAK_CHECK()

    std::string m_parse_type;

    /** unordered_map<Group Name, set<Track Name> > .*/
    std::unordered_map<std::string, std::set<std::string> > m_favorite;

public:
    friend class KartPropertiesManager;
    friend class TrackManager;

    static const std::string DEFAULT_FAVORITE_GROUP_NAME;

    /** Parse all <(parse_type)/> in <favorite> in xml node */
    FavoriteStatus(const XMLNode *node, std::string parse_type);

    virtual ~FavoriteStatus();

    void save(UTFWriter &out);

    bool isFavorite(std::string ident);

    void addFavorite(std::string ident, std::string group = DEFAULT_FAVORITE_GROUP_NAME);

    void removeFavorite(std::string ident, std::string group = DEFAULT_FAVORITE_GROUP_NAME);
};   // class PlayerProfile

#endif

/*EOF*/
