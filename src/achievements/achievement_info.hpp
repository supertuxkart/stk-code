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

#include "achievements/achievement.hpp"
#include "io/xml_node.hpp"
#include "utils/translation.hpp"
#include "utils/types.hpp"

#include <irrString.h>
#include <string>

// ============================================================================

class Achievement;

/** This is the base class for storing the definition of an achievement, e.g.
 *  title, description (which is common for all achievements), but also how
 *  to achieve this achievement.
 * \ingroup achievements
 */
class AchievementInfo
{
public:
    /** Some handy names for the various achievements. */
    enum { ACHIEVE_COLUMBUS      = 1,
           ACHIEVE_FIRST         = ACHIEVE_COLUMBUS,
           ACHIEVE_STRIKE        = 2,
           ACHIEVE_ARCH_ENEMY    = 3,
           ACHIEVE_MARATHONER    = 4,
           ACHIEVE_SKIDDING      = 5,
           ACHIEVE_GOLD_DRIVER   = 6,
           ACHIEVE_POWERUP_LOVER = 7,
           ACHIEVE_UNSTOPPABLE   = 8,
           ACHIEVE_BANANA        = 9,
           ACHIEVE_MOSQUITO      = 11
    };

    /** Achievement check type:
     *  ALL_AT_LEAST: All goal values must be reached (or exceeded).
     *  ONE_AT_LEAST: At least one current value reaches or exceedes the goal.
     */
    enum AchievementCheckType
    {
        AC_ALL_AT_LEAST,
        AC_ONE_AT_LEAST
    };
    /** Achievement reset type:
     *  AFTER_LAP:  Achievement needs to be reset after each lap.
     *  AFTER_RACE: Achievement needs to be reset after each race.
     *  NEVER:      Achievement does not need to be reset
     */
    enum ResetType
    {
        AFTER_LAP  = 1,
        AFTER_RACE = 2,
        NEVER      = 3
    };

private:
    /** The id of this Achievement. */
    uint32_t           m_id;

    /** The title of this achievement. */
    irr::core::stringw m_name;

    /** The description of this achievement. */
    irr::core::stringw m_description;

    /** Determines how this achievement is checked if it is successful. */
    AchievementCheckType  m_check_type;

    /** The target values needed to be reached. */
    std::map<std::string, int> m_goal_values;

    /** Determines when the achievement needs to be reset */
    ResetType m_reset_type;

    /** A secret achievement has its progress not shown. */
    bool m_is_secret;

public:
             AchievementInfo(const XMLNode * input);
    virtual ~AchievementInfo() {};

    virtual irr::core::stringw toString() const;
    virtual bool checkCompletion(Achievement * achievement) const;
    int getGoalValue(const std::string &key) const;

    // ------------------------------------------------------------------------
    /** Returns the id of this achievement. */
    uint32_t getID() const { return m_id; }
    // ------------------------------------------------------------------------
    /** Returns the description of this achievement. */
    irr::core::stringw getDescription() const { return _(m_description.c_str()); }
    // ------------------------------------------------------------------------
    /** Returns the name of this achievement. */
    irr::core::stringw getName() const { return _LTR(m_name.c_str()); }
    // ------------------------------------------------------------------------
    bool needsResetAfterRace() const { return m_reset_type == AFTER_RACE; }
    // ------------------------------------------------------------------------
    bool needsResetAfterLap() const { return m_reset_type == AFTER_LAP; }
    // ------------------------------------------------------------------------
    /** Returns the check type for this achievement. */
    AchievementCheckType getCheckType() const { return m_check_type; }
    // ------------------------------------------------------------------------
    /** Returns if this achievement is a secret achievement. */
    bool isSecret() const { return m_is_secret; }
    // ------------------------------------------------------------------------
};   // class AchievementInfo


#endif

/*EOF*/
