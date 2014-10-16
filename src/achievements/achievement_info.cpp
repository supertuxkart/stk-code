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

/** The constructor reads the dat from the xml information.
 *  \param input XML node for this achievement info.
 */
AchievementInfo::AchievementInfo(const XMLNode * input)
{
    m_reset_type       = NEVER;
    m_id               = 0;
    m_title            = "";
    m_description      = "";
    m_is_secret        = false;
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

    // Load the reset-type
    std::string s;
    input->get("reset-type", &s);
    if (s == "race")
        m_reset_type = AFTER_RACE;
    else if (s == "lap")
        m_reset_type = AFTER_LAP;
    else if (s != "never")
        Log::warn("AchievementInfo", "Achievement check type '%s' unknown.",
            s.c_str());

    // Load check-type
    m_check_type = AC_ALL_AT_LEAST;
    input->get("check-type", &s);
    if (s == "all-at-least")
        m_check_type = AC_ALL_AT_LEAST;
    else if (s == "one-at-least")
        m_check_type = AC_ONE_AT_LEAST;
    else
        Log::warn("AchievementInfo", "Achievement check type '%s' unknown.",
                  s.c_str());
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

    if (m_check_type == AC_ONE_AT_LEAST)
    {
        if (m_goal_values.size() != 1)
            Log::fatal("AchievementInfo",
                     "A one-at-least achievement must have exactly one goal.");
    }
}   // AchievementInfo

// ----------------------------------------------------------------------------
/** Returns a string with a numerical value to display the progress of
 *  this achievement. It adds up all the goal values
 */
irr::core::stringw AchievementInfo::toString() const
{
    int count = 0;
    std::map<std::string, int>::const_iterator iter;
    switch (m_check_type)
    {
    case AC_ALL_AT_LEAST:
        // If all values need to be reached, add up all goal values
        for (iter = m_goal_values.begin(); iter != m_goal_values.end(); iter++)
        {
            count += iter->second;
        }
        break;
    case AC_ONE_AT_LEAST:
        // Only one goal is defined for a one-at-least
        count = m_goal_values.begin()->second;
        break;
    default:
        Log::fatal("AchievementInfo", "Missing toString for type %d.",
                   m_check_type);
    }
    return StringUtils::toWString(count);

}   // toString

// ----------------------------------------------------------------------------
bool AchievementInfo::checkCompletion(Achievement * achievement) const
{
    std::map<std::string, int>::const_iterator iter;

    switch (m_check_type)
    {
    case AC_ALL_AT_LEAST:
        for (iter = m_goal_values.begin(); iter != m_goal_values.end(); iter++)
        {
            if (achievement->getValue(iter->first) < iter->second)
                return false;
        }
        return true;
    case AC_ONE_AT_LEAST:
    {
        // Test all progress values the kart has.
        const std::map<std::string, int> &progress = achievement->getProgress();
        for (iter = progress.begin(); iter != progress.end(); iter++)
        {
            // A one-at-least achievement has only one goal, so use it
            if (iter->second >= m_goal_values.begin()->second)
                return true;
        }
        return false;
    }
    default:
        Log::fatal("AchievementInfo", "Missing check for type %d.",
                   m_check_type);
    }   // switch

    // Avoid compiler warning
    return false;
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
