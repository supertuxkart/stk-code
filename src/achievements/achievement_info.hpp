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
public:
    /** Achievement types:
     *  SINGLE_AT_LEAST: a single value, which must at least be the 
     *                   goal value.
     */
    enum AchievementType { AT_SINGLE_AT_LEAST};
private:
    /** The id of this Achievement. */
    uint32_t           m_id;

    /** The title of this achievement. */
    irr::core::stringw m_title;

    /** The description of this achievement. */
    irr::core::stringw m_description;

    AchievementType    m_type;

    /** The target values needed to be reached. */
    std::map<std::string, int> m_goal_values;

    /** True if the achievement needs to be reset after each race. */
    bool m_reset_after_race;

public:
    AchievementInfo                     (const XMLNode * input);
    virtual ~AchievementInfo            () {};
    virtual AchievementType getType() const { return m_type; }
    virtual irr::core::stringw toString() const;
    virtual bool checkCompletion(Achievement * achievement) const;

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


#endif

/*EOF*/
