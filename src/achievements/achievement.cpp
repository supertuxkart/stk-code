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

#include "achievements/achievements_manager.hpp"
#include "achievements/achievement_info.hpp"
#include "config/player_manager.hpp"
#include "guiengine/message_queue.hpp"
#include "io/utf_writer.hpp"
#include "io/xml_node.hpp"
#include "online/xml_request.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <stdlib.h>

/** Constructur, initialises this object with the data from the
 *  corresponding AchievementInfo.
 */
Achievement::Achievement(AchievementInfo * info)
           : m_achievement_info(info)
{
    m_achieved = false;
    m_id = m_achievement_info->getID();
    m_achievement_info->copyGoalTree(m_progress_goal_tree, m_achievement_info->m_goal_tree, true /*set values to 0*/);
}   // Achievement

// ----------------------------------------------------------------------------
Achievement::~Achievement()
{
}   // ~Achievement

// ----------------------------------------------------------------------------
/** Loads the value from an XML node.
 *  \param input*/
void Achievement::loadProgress(const XMLNode *node)
{
    node->get("achieved", &m_achieved);
}   // load

// ----------------------------------------------------------------------------
/** Saves the achievement status to a file.
 *  \param Output stream.
 */
void Achievement::saveProgress(UTFWriter &out)
{
    out << "        <achievement id=\"" << getID() << "\" "
        << "achieved=\"" << m_achieved << "\"";
 
    out << "/>\n";
}   // save

// ----------------------------------------------------------------------------
/** Returns how many goals of an achievement have been achieved,
 *  in the form n/m.
 */
irr::core::stringw Achievement::getGoalProgressAsString()
{
    irr::core::stringw target = getInfo()->goalString();

    return StringUtils::toWString(getFullfiledGoals()) + "/" + target;
} // getGoalProgressAsString

// ----------------------------------------------------------------------------
/** Returns how many goals of an achievement have been achieved.
 */
int Achievement::getFullfiledGoals()
{
    int fullfiled_goals = (m_achieved) ? getInfo()->getGoalCount() :
                          computeFullfiledGoals(m_progress_goal_tree, m_achievement_info->m_goal_tree);
    return fullfiled_goals;
} // getFullfiledGoals

// ----------------------------------------------------------------------------
int Achievement::computeFullfiledGoals(AchievementInfo::goalTree &progress, AchievementInfo::goalTree &reference)
{
    if (progress.children.size() != 1)
    {
        // This always returns 0 if the achievement has not been completed
        if (progress.children.size() != 0 && progress.children[0].type == "OR")
        {
            bool completed = false;
            for (unsigned int i=0;i<progress.children.size();i++)
            {
                if (recursiveCompletionCheck(progress.children[i], reference.children[i]))
                {
                    completed = true;
                    break;
                }
            }
            return (completed) ? 1 : 0;
        }
        else
        {
            int goals_completed = 0;
            for (unsigned int i=0;i<progress.children.size();i++)
            {
                if (recursiveCompletionCheck(progress.children[i], reference.children[i]))
                    goals_completed++;
            }
            return goals_completed;
        }
    }
    else if (progress.children.size() == 1 &&
             (progress.children[0].type == "AND" ||
              progress.children[0].type == "AND-AT-ONCE" || 
              progress.children[0].type == "OR"))
    {
        return computeFullfiledGoals(progress.children[0], reference.children[0]);
    }
    else
    {
        return (recursiveCompletionCheck(progress.children[0], reference.children[0])) ? 1 : 0;
    }
} // computeFullfiledGoals

// ----------------------------------------------------------------------------
/** Returns how much of an achievement has been achieved in the form n/m.
 *  ONLY applicable for single goal achievements. Returns a string with a single
 *  space if there are multiple goals.
 */
irr::core::stringw Achievement::getProgressAsString()
{
    irr::core::stringw target = getInfo()->progressString();
    irr::core::stringw empty = " ";// See issue #3081

    if (target == "-1")
        return empty;

    return StringUtils::toWString(getProgress()) + "/" + target;
}   // getProgressAsString

// ----------------------------------------------------------------------------
/** Returns how many goals of an achievement have been achieved.
 */
int Achievement::getProgress()
{
    int progress = (m_achieved) ? getInfo()->getProgressTarget() :
                                  computeGoalProgress(m_progress_goal_tree, m_achievement_info->m_goal_tree);
    return progress;
} // getProgress


// ----------------------------------------------------------------------------
/** Should ONLY be called if the achievement has one goal (a sum counts as one goal).
  * Returning an error code with a number is not full-proof because a sum goal can
  * legitimately be negative (a counter can be chosen to count against the
  * achievement's fullfilment). */
int Achievement::computeGoalProgress(AchievementInfo::goalTree &progress, AchievementInfo::goalTree &reference, bool same_tree)
{
    if (progress.children.size() >= 2)
    {
        // This should NOT happen
        assert(false);
        return 0;
    }
    // Can happen when showing the progress status of all parts of the goal tree
    else if (progress.children.size() == 0)
    {
        //TODO : find a more automatic way ; clean up repetition
        if (progress.type == "race-started-all" ||
            progress.type == "race-finished-all" ||
            progress.type == "race-won-all" ||
            progress.type == "race-finished-reverse-all" ||
            progress.type == "race-finished-alone-all" ||
            progress.type == "less-laps-all" ||
            progress.type == "more-laps-all" ||
            progress.type == "twice-laps-all" ||
            progress.type == "egg-hunt-started-all" ||
            progress.type == "egg-hunt-finished-all")
        {
            if (same_tree)
            {
                return PlayerManager::getCurrentAchievementsStatus()
                    ->getNumTracksAboveValue(0, reference.type);
            }
            // Compare against the target value (in the reference tree) !
            // Progress is only shown for the current local accuont, so we can use the current achievements status
            return PlayerManager::getCurrentAchievementsStatus()
                       ->getNumTracksAboveValue(reference.value, reference.type);
        }
        return progress.value;
    }
    else if (progress.children.size() == 1 &&
             (progress.children[0].type == "AND" ||
              progress.children[0].type == "AND-AT-ONCE" || 
              progress.children[0].type == "OR"))
    {
        return computeGoalProgress(progress.children[0], reference.children[0]);
    }
    else
    {
        //TODO : find a more automatic way
        if (progress.children[0].type == "race-started-all" ||
            progress.children[0].type == "race-finished-all" ||
            progress.children[0].type == "race-won-all" ||
            progress.children[0].type == "race-finished-reverse-all" ||
            progress.children[0].type == "race-finished-alone-all" ||
            progress.children[0].type == "less-laps-all" ||
            progress.children[0].type == "more-laps-all" ||
            progress.children[0].type == "twice-laps-all" ||
            progress.children[0].type == "egg-hunt-started-all" ||
            progress.children[0].type == "egg-hunt-finished-all")
        {
            // Compare against the target value (in the reference tree) !
            // Progress is only shown for the current local accuont, so we can use the current achievements status
            return PlayerManager::getCurrentAchievementsStatus()
                       ->getNumTracksAboveValue(reference.children[0].value, reference.children[0].type);
        }
        else
        {
            return progress.children[0].value;
        }
    }
} // computeGoalProgress

// ----------------------------------------------------------------------------
/** Set any leaf of the progress goal tree whose type matches the
 *  goal_string to the value passed as parameter.
 *  The goal string can contain a logical prefix.
 *  If it is LOGC- ; the update is for the current value of a
 *  resetable counter. It is applied if the parent node is of type
 *  SUM or AND-AT-ONCE, ignored otherwise.
 *  If it is LOGM- ; the update is for the highest achieved value of a
 *  resetable counter. It is appliedif the parent node is of type
 *  AND or OR, ignored otherwise.
 *  If there is no logical prefix, the new value is set in all cases.
 *
 *  If the leaf has an operator defined, this will trigger an update of the
 *  relevant values.
 *  If the leaf's new value match or exceed its target goal value,
 *  a check for the achievement's completions is triggered.
 */
void Achievement::setGoalValue(std::string &goal_string, int value)
{
    if(m_achieved) // This should not happen, but it costs little to double-check
        return;

    bool and_or = true;
    bool sum_andatonce = true;
    if (goal_string.compare(0 /*start of sub-string*/,5/*length*/,"LOGC-") == 0)
    {
        and_or = false;
        goal_string = goal_string.substr(5,999);
    }
    else if (goal_string.compare(0 /*start of sub-string*/,5/*length*/,"LOGM-") == 0)
    {
        sum_andatonce = false;
        goal_string = goal_string.substr(5,999);
    }

    bool found = recursiveSetGoalValue(m_progress_goal_tree, goal_string, value, and_or, sum_andatonce);

    // If a value has been updated, check for completion
    if (found && recursiveCompletionCheck(m_progress_goal_tree, m_achievement_info->m_goal_tree))
    {
        setAchieved();
        onCompletion();
    }
} // setGoalValue

bool Achievement::recursiveSetGoalValue(AchievementInfo::goalTree &tree, const std::string &goal_string, int value,
                                        bool and_or, bool sum_andatonce)
{
    if (tree.type == goal_string)
    {
        // We don't update here, because it may yet be cancelled
        // depending on the parent tree logical type.
        return true;
    }

    bool can_set_child = ((and_or && (tree.type == "AND" || tree.type == "OR")) ||
                          (sum_andatonce && (tree.type == "SUM" || tree.type == "AND-AT-ONCE")));

    bool value_set = false;
    for (unsigned int i=0;i<tree.children.size();i++)
    {
        if(recursiveSetGoalValue(tree.children[i],goal_string,value, and_or, sum_andatonce))
        {
            // The value has already been set, pass on the information
            if (tree.children[i].type == "AND" ||
                tree.children[i].type == "OR"  ||
                tree.children[i].type == "SUM" ||
                tree.children[i].type == "AND-AT-ONCE")
            {

                value_set = true;
            }
            // The child has the good type and we can increment the goal;
            else if (can_set_child)
            {
                tree.children[i].value = value;
                value_set = true;
            }
        }
    }
    // Recompute the sum
    if (tree.type == "SUM" && value_set)
    {
        int new_value = 0;
        for (unsigned int i=0;i<tree.children.size();i++)
        {
            if(tree.children[i].operation == AchievementInfo::OP_ADD)
                new_value += tree.children[i].value;
            else if(tree.children[i].operation == AchievementInfo::OP_SUBSTRACT)
                new_value -= tree.children[i].value;
        }
        tree.value = new_value;
    }

    return value_set;
} // recursiveSetGoalValue

// ----------------------------------------------------------------------------
/** Checks if this achievement has been achieved.
 */
bool Achievement::recursiveCompletionCheck(AchievementInfo::goalTree &progress, AchievementInfo::goalTree &reference)
{
    bool completed = false;
    if (progress.type == "AND" || progress.type == "AND-AT-ONCE")
    {
        completed = true;
        for (unsigned int i=0;i<progress.children.size();i++)
        {
            if (!recursiveCompletionCheck(progress.children[i], reference.children[i]))
            {
                completed = false;
                break;
            }
        }
    }
    else if (progress.type == "OR")
    {
        completed = false;
        for (unsigned int i=0;i<progress.children.size();i++)
        {
            if (recursiveCompletionCheck(progress.children[i], reference.children[i]))
            {
                completed = true;
                break;
            }
        }
    }
    // Whether a sum or a leaf node, it has a value.
    // The value for sums are updated when the underlying values are,
    // we don't need to do it again
    else if (progress.value >= reference.value)
    {
        completed = true;
    }
    return completed;
} // recursiveCompletionCheck

// ----------------------------------------------------------------------------
/** Manages what needs to happen once the achievement is completed,
 *  like displaying the completion message to the player or synching
 *  with the server.
 */
void Achievement::onCompletion()
{
    //show achievement
    // Note: the "name" variable is required, see issue #2068
    // calling _("...", info->getName()) is invalid because getName also calls
    // _() and thus the string it returns is mapped to a temporary buffer
    // in theory, it should return a copy of the string, but clang tries to
    // optimise away the copy
    core::stringw name = m_achievement_info->getName();
    core::stringw s = _("Completed achievement \"%s\".", name);
    MessageQueue::add(MessageQueue::MT_ACHIEVEMENT, s);

    // Sends a confirmation to the server that an achievement has been
    // completed, if a user is signed in.
    if (PlayerManager::isCurrentLoggedIn())
    {
        auto request = std::make_shared<Online::HTTPRequest>();
        PlayerManager::setUserDetails(request, "achieving");
        request->addParameter("achievementid", getID());
        request->queue();
    }
    if (PlayerManager::getCurrentPlayer()->getUniqueID() == 1)
    {
        AchievementsManager::get()->getWebAchievementStatus()->achieved(m_achievement_info);
    }
}   // onCompletion
