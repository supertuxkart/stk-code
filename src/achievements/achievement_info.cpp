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

#include "achievements/achievement.hpp"
#include "achievements/achievement_info.hpp"
#include "utils/log.hpp"

#include <sstream>
#include <stdlib.h>
#include <assert.h>

/** The constructor reads the dat from the xml information.
 *  \param input XML node for this achievement info.
 */
AchievementInfo::AchievementInfo(const XMLNode * input)
{
    m_id               = 0;
    m_name            = "";
    m_description      = "";
    m_is_secret        = false;
    bool all;
    all = input->get("id",               &m_id              ) &&
          input->get("name",             &m_name            ) &&
          input->get("description",      &m_description     );
    if (!all)
    {
        Log::error("AchievementInfo",
                   "Not all necessary values for achievement defined.");
        Log::error("AchievementInfo",
                   "ID %d name '%s' description '%s'", m_id, m_name.c_str(),
                                                        m_description.c_str());
    }

    input->get("secret", &m_is_secret);

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
        Log::fatal("AchievementInfo",
                  "Duplicate keys for the entries of a MapAchievement found.");
}   // AchievementInfo

// ----------------------------------------------------------------------------
/** Returns a string with a numerical value to display the progress of
 *  this achievement. It adds up all the goal values
 */
irr::core::stringw AchievementInfo::toString() const
{
    int count = 0;
    std::map<std::string, int>::const_iterator iter;

    // If all values need to be reached, add up all goal values
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

    for (iter = m_goal_values.begin(); iter != m_goal_values.end(); iter++)
    {
        if (achievement->getValue(iter->first) < iter->second)
            return false;
    }
    return true;
}
// ----------------------------------------------------------------------------
int AchievementInfo::getGoalValue(const std::string &key) const
{
    std::map<std::string, int>::const_iterator it;
    it = m_goal_values.find(key);
    if (it != m_goal_values.end())
        return it->second;
    else
        return 0;
}   // getGoalValue
// ----------------------------------------------------------------------------
