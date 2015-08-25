//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 Glenn De Jonghe
//            (C) 2014-2015 Joerg Henrichs
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

#include "achievements/achievements_manager.hpp"

#include "achievements/achievement_info.hpp"
#include "achievements/achievements_status.hpp"
#include "config/player_manager.hpp"
#include "config/player_profile.hpp"
#include "config/user_config.hpp"
#include "io/file_manager.hpp"
#include "utils/log.hpp"
#include "utils/translation.hpp"

#include <sstream>
#include <stdlib.h>
#include <assert.h>

AchievementsManager* AchievementsManager::m_achievements_manager = NULL;

// ----------------------------------------------------------------------------
/** Constructor, which reads data/achievements.xml and stores the information
 *  in AchievementInfo objects.
 */
AchievementsManager::AchievementsManager()
{
    const std::string file_name = file_manager->getAsset("achievements.xml");
    const XMLNode *root = file_manager->createXMLTree(file_name);
    unsigned int num_nodes = root->getNumNodes();
    for (unsigned int i = 0; i < num_nodes; i++)
    {
        const XMLNode *node = root->getNode(i);
        AchievementInfo * achievement_info = new AchievementInfo(node);
        m_achievements_info[achievement_info->getID()] = achievement_info;
    }
    if (num_nodes != m_achievements_info.size())
        Log::error("AchievementsManager",
                   "Multiple achievements with the same id!");

    delete root;
}   // AchievementsManager

// ----------------------------------------------------------------------------
AchievementsManager::~AchievementsManager()
{
    std::map<uint32_t, AchievementInfo *>::iterator it;
    for ( it = m_achievements_info.begin(); it != m_achievements_info.end(); ++it ) {
        delete it->second;
    }
    m_achievements_info.clear();
}   // ~AchievementsManager

// ----------------------------------------------------------------------------
/** Create a new AchievementStatus object that stores all achievement status
 *  information for a single player.
 *  \param node The XML of saved data, or NULL if no saved data exists.
 */
AchievementsStatus*
             AchievementsManager::createAchievementsStatus(const XMLNode *node)
{
    AchievementsStatus *status = new AchievementsStatus();

    // First add all achievements, before restoring the saved data.
    std::map<uint32_t, AchievementInfo *>::const_iterator it;
    for (it  = m_achievements_info.begin();
         it != m_achievements_info.end(); ++it)
    {
        Achievement * achievement;
        achievement = new Achievement(it->second);
        status->add(achievement);
    }

    if (node)
        status->load(node);

    return status;
}   // createAchievementStatus

// ----------------------------------------------------------------------------
AchievementInfo * AchievementsManager::getAchievementInfo(uint32_t id) const
{
    std::map<uint32_t, AchievementInfo*>::const_iterator info =
        m_achievements_info.find(id);
    if (info != m_achievements_info.end())
        return info->second;
    return NULL;
}
