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

#include "achievements/achievement_info.hpp"

#include "utils/log.hpp"
#include "utils/translation.hpp"

#include <sstream>
#include <stdlib.h>
#include <assert.h>

// ============================================================================
AchievementInfo::AchievementInfo(const XMLNode * input)
{
    input->get("id", &m_id);
    input->get("title", &m_title);
    input->get("description", &m_description);

    std::string reset_after_race("");
    input->get("reset_after_race", &reset_after_race);
    m_reset_after_race = reset_after_race == "true";

}

// ============================================================================
SingleAchievementInfo::SingleAchievementInfo(const XMLNode * input)
    : AchievementInfo(input)
{
    input->get("goal", &m_goal_value);
}

// ============================================================================
bool SingleAchievementInfo::checkCompletion(Achievement * achievement) const
{
    SingleAchievement * single_achievement = (SingleAchievement *) achievement;
    if(single_achievement->getValue() >= m_goal_value)
        return true;
    return false;
}

// ============================================================================
MapAchievementInfo::MapAchievementInfo(const XMLNode * input)
    : AchievementInfo(input)
{
    std::vector<XMLNode*> xml_entries;
    input->getNodes("entry", xml_entries);
    for (unsigned int n=0; n < xml_entries.size(); n++)
    {
        std::string key("");
        xml_entries[n]->get("key", &key);
        int goal(0);
        xml_entries[n]->get("goal", &goal);
        m_goal_values[key] = goal;
    }
    if(m_goal_values.size() != xml_entries.size())
        Log::error("MapAchievementInfo","Duplicate keys for the entries of a MapAchievement found.");
}

// ============================================================================
bool MapAchievementInfo::checkCompletion(Achievement * achievement) const
{
    MapAchievement * map_achievement = (MapAchievement *) achievement;
    std::map<std::string, int>::const_iterator iter;
    for ( iter = m_goal_values.begin(); iter != m_goal_values.end(); iter++ ) {
        if(map_achievement->getValue(iter->first) < iter->second)
            return false;
    }
    return true;
}
