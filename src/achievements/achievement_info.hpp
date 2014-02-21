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

#ifndef HEADER_ACHIEVEMENT_INFO_HPP
#define HEADER_ACHIEVEMENT_INFO_HPP

#include "utils/types.hpp"

#include "io/xml_node.hpp"
#include "achievements/achievement.hpp"

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
protected:
    /** The id of this Achievement. */
    uint32_t           m_id;

    /** The title of this achievement. */
    irr::core::stringw m_title;

    /** The description of this achievement. */
    irr::core::stringw m_description;

    /** True if the achievement needs to be reset after each race. */
    bool m_reset_after_race;

public:
    AchievementInfo                     (const XMLNode * input);
    virtual ~AchievementInfo            () {};
    virtual Achievement::AchievementType getType() const = 0;
    virtual irr::core::stringw toString() const = 0;
    virtual bool checkCompletion(Achievement * achievement) const = 0;

    // ------------------------------------------------------------------------
    /** Returns the id of this achievement. */
    uint32_t getID() const { return m_id; }
    // ------------------------------------------------------------------------
    /** Returns the description of this achievement. */
    irr::core::stringw getDescription() const { return m_description; }
    // ------------------------------------------------------------------------
    /** Returns the title of this achievement. */
    irr::core::stringw getTitle() const { return m_title; }
    // ------------------------------------------------------------------------
    bool needsResetAfterRace() const { return m_reset_after_race; }
};   // class AchievementInfo


// ============================================================================
/** This class stores the information about an achievement that count a 
 *  single value.
 */
class SingleAchievementInfo : public AchievementInfo
{
private:
    /** Which value must be reached in order to achieve this achievement. */
    int m_goal_value;

public:
             SingleAchievementInfo(const XMLNode * input);
    virtual ~SingleAchievementInfo() {};
    virtual irr::core::stringw toString() const;
    virtual bool checkCompletion(Achievement * achievement) const;
    // ------------------------------------------------------------------------
    int getGoalValue() const { return m_goal_value; }
    // ------------------------------------------------------------------------
    virtual Achievement::AchievementType getType() const
    {
        return Achievement::AT_SINGLE; 
    }   // getType
};   // class SingleAchievementInfo


// ============================================================================
/** This class stores a set of key-value pairs.
 */
class MapAchievementInfo : public AchievementInfo
{
protected:

    /** The target values needed to be reached. */
    std::map<std::string, int> m_goal_values;

public:
             MapAchievementInfo(const XMLNode * input);
    virtual ~MapAchievementInfo() {};
    virtual bool checkCompletion(Achievement * achievement) const;
    virtual irr::core::stringw toString() const;
    // ------------------------------------------------------------------------
    int getGoalValue(const std::string & key) { return m_goal_values[key]; }
    // ------------------------------------------------------------------------
    const std::map<std::string, int> & getGoalValues() const 
    {
        return m_goal_values; 
    }   // getGoalValues
    // ------------------------------------------------------------------------
    virtual Achievement::AchievementType  getType() const
    {
        return Achievement::AT_MAP;
    }   // getType

};   // class MapAchievementInfo

#endif

/*EOF*/
