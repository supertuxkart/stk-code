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

#include "achievements/achievements_manager.hpp"

#include "achievements/achievement.hpp"
#include "utils/log.hpp"
#include "utils/translation.hpp"
#include "io/file_manager.hpp"
#include "io/xml_writer.hpp"

#include <sstream>
#include <stdlib.h>
#include <assert.h>

static AchievementsManager* achievements_manager_singleton(NULL);

AchievementsManager* AchievementsManager::get()
{
    if (achievements_manager_singleton == NULL)
        achievements_manager_singleton = new AchievementsManager();
    return achievements_manager_singleton;
}

void AchievementsManager::deallocate()
{
    delete achievements_manager_singleton;
    achievements_manager_singleton = NULL;
}   // deallocate

// ============================================================================
AchievementsManager::AchievementsManager()
{
    parseAchievements();
}

// ============================================================================
AchievementsManager::~AchievementsManager()
{

}
// ============================================================================
void AchievementsManager::parseAchievements()
{
    const std::string file_name = file_manager->getDataFile("items.xml");
    const XMLNode *root         = file_manager->createXMLTree(file_name);
    unsigned int num_nodes = root->getNumNodes();
    for(unsigned int i = 0; i < num_nodes; i++)
    {
        const XMLNode *node = root->getNode(i);
        std::string type("");
        node->get("type", &type);
        Achievement * achievement;
        if(type == "single")
        {
            achievement = new SingleAchievement(node);
        }
        else if(type == "map")
        {
            achievement = new MapAchievement(node);
        }
        else
        {
            Log::error("AchievementsManager::parseAchievements","Non-existent achievement type. Skipping - definitely results in unwanted behaviour.");
            continue;
        }
        m_achievements[achievement->getID()] = achievement;
    }
    if(num_nodes != m_achievements.count())
        Log::error("AchievementsManager::parseAchievements","Multiple achievements with the same id!");
}   // parseAchievements
