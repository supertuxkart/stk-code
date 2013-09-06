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


#include "achievements/achievements_slot.hpp"

#include "achievements/achievement_info.hpp"
#include "achievements/achievements_manager.hpp"
#include "utils/log.hpp"
#include "utils/ptr_vector.hpp"
#include "utils/translation.hpp"
#include "io/xml_writer.hpp"
#include "online/current_user.hpp"

#include <sstream>
#include <stdlib.h>
#include <assert.h>
// ============================================================================
AchievementsSlot::AchievementsSlot(const XMLNode * input,  const PtrVector<AchievementInfo> & info)
{
    int fetched_user_id = input->get("user_id", &m_id);
    std::string online;
    int fetched_online = input->get("online", &online);
    if(!fetched_user_id || !fetched_online || !(online == "true" || online == "false"))
    {
        m_valid = false;
    }
    m_valid = true;
    m_online = online == "true";

    createFreshSlot(info);

    std::vector<XMLNode*> xml_achievements;
    input->getNodes("achievement", xml_achievements);
    for( unsigned int i=0; i<xml_achievements.size(); i++)
    {
        uint32_t achievement_id(0);
        xml_achievements[i]->get("id", &achievement_id);
        Achievement * achievement = getAchievement(achievement_id);
        if(achievement == NULL)
        {
            Log::warn("AchievementsSlot", "Found saved achievement data for a non-existent achievement. Discarding.");
            continue;
        }
        achievement->load(xml_achievements[i]);
    }
}

// ============================================================================
AchievementsSlot::AchievementsSlot(std::string id, bool online, const PtrVector<AchievementInfo> & info)
{
    m_valid = true;
    m_online = online;
    m_id = id;

    createFreshSlot(info);
}

// ============================================================================
void AchievementsSlot::createFreshSlot( const PtrVector<AchievementInfo> & all_info)
{
    m_achievements.clear();
    for(int i=0; i < all_info.size(); i++)
    {
        const AchievementInfo * info = all_info.get(i);
        Achievement::AchievementType achievement_type = info->getType();
        Achievement * achievement;
        if(achievement_type == Achievement::AT_SINGLE)
        {
            achievement = new SingleAchievement(info);
        }
        else if(achievement_type == Achievement::AT_MAP)
        {
            achievement = new MapAchievement(info);
        }
        m_achievements[achievement->getID()] = achievement;
    }
}

// ============================================================================
void AchievementsSlot::save(std::ofstream & out)
{
    out << "    <slot user_id=\"" << m_id.c_str()
        << "\" online=\""           << StringUtils::boolstr(m_online)
        << "\"> \n";
    std::map<uint32_t, Achievement*>::const_iterator i;
    for(i = m_achievements.begin(); i != m_achievements.end();  i++)
    {
        if (i->second != NULL)
            i->second->save(out);
    }
    out << "    </slot>\n";
}

// ============================================================================
Achievement * AchievementsSlot::getAchievement(uint32_t id)
{
    if ( m_achievements.find(id) != m_achievements.end())
        return m_achievements[id];
    return NULL;
}

// ============================================================================
void AchievementsSlot::sync(const std::string & achieved_string)
{
    std::vector<std::string> parts = StringUtils::split(achieved_string, ' ');
    std::vector<uint32_t> achieved_ids;
    for(unsigned int i = 0; i < parts.size(); ++i)
    {
       achieved_ids.push_back(atoi(parts[i].c_str()));
    }
    for(unsigned int i =0; i < achieved_ids.size(); ++i)
    {
        Achievement * achievement = getAchievement(achieved_ids[i]);
        if(achievement != NULL)
            achievement->setAchieved();
    }
}

// ============================================================================
void AchievementsSlot::onRaceEnd()
{
    //reset all values that need to be reset
    std::map<uint32_t, Achievement *>::iterator iter;
    for ( iter = m_achievements.begin(); iter != m_achievements.end(); ++iter ) {
        iter->second->onRaceEnd();
    }
}
