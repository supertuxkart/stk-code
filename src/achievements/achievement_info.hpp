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

#include "io/xml_node.hpp"
#include "utils/translation.hpp"
#include "utils/types.hpp"

#include <irrString.h>
#include <string>

// ============================================================================

class Achievement;

/** This class stores an achievement definition from the xml file, including
 *  title, description, but also how to achieve this achievement.
 *  Constrat with the Achievement class, which is a player-specific instance
 *  tracking the progress of the achievement.
 * \ingroup achievements
 */
class AchievementInfo
{
public:
    //FIXME : try to get rid of this list
    /** Some handy names for the various achievements. */
    enum { ACHIEVE_COLUMBUS      = 1,
           ACHIEVE_FIRST         = ACHIEVE_COLUMBUS,
           ACHIEVE_STRIKE        = 2,
           ACHIEVE_ARCH_ENEMY    = 3,
           ACHIEVE_MARATHONER    = 4,
           ACHIEVE_SKIDDING      = 5,
           ACHIEVE_GOLD_DRIVER   = 6,
           ACHIEVE_POWERUP_LOVER = 7,
           ACHIEVE_BEYOND_LUCK   = 8,
           ACHIEVE_BANANA        = 9,
           ACHIEVE_MOSQUITO      = 11,
           ACHIEVE_UNSTOPPABLE   = 12
    };

private:
    /** The id of this Achievement. */
    uint32_t           m_id;

    /** The title of this achievement. */
    irr::core::stringw m_name;

    /** The description of this achievement. */
    irr::core::stringw m_description;

    /** The target values needed to be reached. */
    std::map<std::string, int> m_goal_values;

    /** A secret achievement has its progress not shown. */
    bool m_is_secret;

public:
             AchievementInfo(const XMLNode * input);
    virtual ~AchievementInfo() {};

    virtual irr::core::stringw toString() const;
    virtual bool checkCompletion(Achievement * achievement) const;
    int getGoalValue(const std::string &key) const;

    uint32_t           getID()          const { return m_id; }
    irr::core::stringw getDescription() const { return _(m_description.c_str()); }
    irr::core::stringw getName()        const { return _LTR(m_name.c_str()); }
    bool               isSecret()       const { return m_is_secret; }
};   // class AchievementInfo


#endif

/*EOF*/
