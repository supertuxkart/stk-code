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

#ifndef HEADER_ACHIEVEMENT_INFO_HPP
#define HEADER_ACHIEVEMENT_INFO_HPP

#include "utils/types.hpp"

#include <irrString.h>
#include <string>
#include <vector>

// ============================================================================

class Achievement;
class XMLNode;

/** This class stores an achievement definition from the xml file, including
 *  title, description, but also how to achieve this achievement.
 *  Contrast with the Achievement class, which is a player-specific instance
 *  tracking the progress of the achievement.
 * \ingroup achievements
 */
class AchievementInfo
{
public:
    // The operations supported for a goal
    enum operationType {
        OP_NONE      = 0,
        OP_ADD       = 1,
        OP_SUBSTRACT = 2,
    };

    // We store goals in a recursive tree.
    // This structure matching the algorithms
    // we use to manipulate it simplify code.
    struct goalTree {
        std::string           type;
        int                   value;
        operationType         operation;
        std::vector<goalTree> children;      
    };

private:
    /** The id of this Achievement. */
    uint32_t           m_id;

    /** The title of this achievement. */
    std::string m_name;

    /** The description of this achievement. */
    std::string m_description;

    /** A secret achievement has its progress not shown. */
    bool m_is_secret;

    void parseGoals(const XMLNode * input, goalTree &parent);
    int  recursiveGoalCount(goalTree &parent);
    int  recursiveProgressCount(goalTree &parent);
    int  getRecursiveDepth(goalTree &parent);
protected:
    friend class Achievement;
    friend class AchievementProgressDialog;
    /** The tree storing all goals */
    goalTree           m_goal_tree;

public:
             AchievementInfo(const XMLNode * input);
    virtual ~AchievementInfo() {};

    virtual irr::core::stringw goalString();
    virtual irr::core::stringw progressString();

    int                getProgressTarget()    { return recursiveProgressCount(m_goal_tree); }
    int                getGoalCount()         { return recursiveGoalCount(m_goal_tree); }
    int                getDepth()             { return getRecursiveDepth(m_goal_tree); }
    uint32_t           getID()          const { return m_id; }
    irr::core::stringw getDescription() const;
    irr::core::stringw getName()        const;
    std::string        getRawName()     const { return m_name; }
    std::string        getRawDescription() const { return m_description; }
    bool               isSecret()       const { return m_is_secret; }

    // This function should not be called if copy already has children
    void copyGoalTree(goalTree &copy, goalTree &model, bool set_values_to_zero);
};   // class AchievementInfo


#endif

/*EOF*/
