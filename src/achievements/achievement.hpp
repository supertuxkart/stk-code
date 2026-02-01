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

#ifndef HEADER_ACHIEVEMENT_HPP
#define HEADER_ACHIEVEMENT_HPP

#include "achievements/achievement_info.hpp"
#include "utils/types.hpp"

#include <irrString.h>
#include <map>
#include <string>

class UTFWriter;
class XMLNode;

// ============================================================================
/** This class tracks the progress of an achievement for a player, whose
 *  definition is stored by an associated AchievementInfo. It allows achievement
 *  status to be saved, and detects when an achievement is fulfilled.
 * \ingroup achievements
 */
class AchievementInfo;

class Achievement
{
private:
    /** True if this achievement has been achieved. */
    bool                      m_achieved;

    /* When quitting the game, the achievement info is deleted before
     * the achievement's status is saved. We need to store the id here 
     * to prevent saving junk data.
     * FIXME: an achievement info should not be removed until all references
     *        to it have been too.*/
    int                       m_id;

    void onCompletion();
    bool recursiveSetGoalValue(AchievementInfo::goalTree &tree, const std::string &goal_string, int value,
                               bool and_or, bool sum_andatonce);
    bool recursiveCompletionCheck(AchievementInfo::goalTree &progress, AchievementInfo::goalTree &reference);
protected:
    friend class AchievementProgressDialog;


    /** The tree of goals. It is identical to the
      * goal tree of the matching AchievementInfo,
      * except that the stored values represent the
      * achieved values instead of the values to meet.  */
    AchievementInfo::goalTree m_progress_goal_tree;

    /** A pointer to the corresponding AchievementInfo instance. */
    AchievementInfo    *m_achievement_info;

    int computeFullfiledGoals(AchievementInfo::goalTree &progress, AchievementInfo::goalTree &reference);
    int computeGoalProgress(AchievementInfo::goalTree &progress, AchievementInfo::goalTree &reference, bool same_tree=false);
public:

             Achievement(AchievementInfo * info);
    virtual ~Achievement();
    virtual void loadProgress(const XMLNode *node);
    virtual void saveProgress(UTFWriter &out);

    virtual irr::core::stringw getProgressAsString();
    virtual irr::core::stringw getGoalProgressAsString();

    uint32_t          getID()   const { return m_id; }
    AchievementInfo * getInfo() { return m_achievement_info; }
    int               getFullfiledGoals();
    int               getProgress();

    void setAchieved() { m_achieved = true; };
    bool isAchieved() const { return m_achieved;  }

    void setGoalValue(std::string &goal_string, int value);
};   // class Achievement
#endif
