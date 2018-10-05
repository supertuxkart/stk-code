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
#include "guiengine/message_queue.hpp"
#include "io/utf_writer.hpp"
#include "config/player_manager.hpp"
#include "utils/log.hpp"
#include "utils/translation.hpp"

#include <stdlib.h>

/** Constructur, initialises this object with the data from the
 *  corresponding AchievementInfo.
 */
Achievement::Achievement(AchievementInfo * info)
           : m_achievement_info(info)
{
    m_achieved = false;
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
/** Returns how much of an achievement has been achieved in the form n/m.
 *  The AchievementInfo adds up the number of goals if there are several,
 *  or take the target value of the goal if there is only one.
 *  This do the same, but with achieved value or fullfilled goals.
 */
irr::core::stringw Achievement::getProgressAsString()
{
    //TODO : add a progress computation function.
    int progress = 0;
    irr::core::stringw target = getInfo()->toString();

    // For now return N/N in case of an achieved achievement.
    if (m_achieved)
        return target + "/" + target;

    return StringUtils::toWString(progress) + "/" + target;
}   // getProgressAsString

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

    bool AO = true;
    bool SAAO = true;
    if (goal_string.compare(0 /*start of sub-string*/,5/*length*/,"LOGC-") == 0)
    {
        AO = false;
        goal_string = goal_string.substr(5,999);
    }
    else if (goal_string.compare(0 /*start of sub-string*/,5/*length*/,"LOGM-") == 0)
    {
        SAAO = false;
        goal_string = goal_string.substr(5,999);
    }

    bool found = recursiveSetGoalValue(m_progress_goal_tree, goal_string, value, AO, SAAO);

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
        Online::HTTPRequest * request = new Online::HTTPRequest(true);
        PlayerManager::setUserDetails(request, "achieving");
        request->addParameter("achievementid", getID());
        request->queue();
    }
}   // onCompletion
