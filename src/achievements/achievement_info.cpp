//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2014 Glenn De Jonghe
//                     2014 Joerg Henrichs
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


AchievementInfo::AchievementInfo(const XMLNode * input)
{
    m_reset_after_race = false;
    m_id               = 0;
    m_title            = "";
    m_description      = "";
    bool all;
    all = input->get("id",               &m_id              ) &&
          input->get("title",            &m_title           ) &&
          input->get("description",      &m_description     );
    if (!all)
    {
        Log::error("AchievementInfo", 
                   "Not all necessary values for achievement defined.");
        Log::error("AchievementInfo",
                   "ID %d title '%s' description '%s'", m_id, m_title.c_str(),
                                                        m_description.c_str());
    }
    input->get("reset-after-race", &m_reset_after_race);

    // Now load the goal nodes
    for (unsigned int n = 0; n < input->getNumNodes(); n++)
    {
        const XMLNode *node = input->getNode(n);
        std::string key = node->getName();
        int goal = 0;
        node->get("goal", &goal);
        m_goal_values[key] = goal;
    }
    if (m_goal_values.size() != input->getNumNodes())
        Log::error("MapAchievementInfo", 
                  "Duplicate keys for the entries of a MapAchievement found.");
}   // AchievementInfo

// ----------------------------------------------------------------------------
irr::core::stringw AchievementInfo::toString() const
{
    int count = 0;
    std::map<std::string, int>::const_iterator iter;
    for (iter = m_goal_values.begin(); iter != m_goal_values.end(); iter++)
    {
        count += iter->second;
    }
    return StringUtils::toWString(count);

}   // toString

// ----------------------------------------------------------------------------
bool AchievementInfo::checkCompletion(Achievement * achievement) const
{
    std::map<std::string, int>::const_iterator iter;
    for ( iter = m_goal_values.begin(); iter != m_goal_values.end(); iter++ )
    {
        if(achievement->getValue(iter->first) < iter->second)
            return false;
    }
    return true;
}
// ----------------------------------------------------------------------------
