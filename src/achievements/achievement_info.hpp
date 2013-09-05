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

#include <irrString.h>
#include <string>
#include "io/xml_node.hpp"
#include "achievements/achievement.hpp"


// ============================================================================

class Achievement;

/**
  * \brief
  * \ingroup
  */
class AchievementInfo
{
protected:
    uint32_t m_id;
    irr::core::stringw m_title;
    irr::core::stringw m_description;
    bool m_reset_after_race;

public:
    AchievementInfo                     (const XMLNode * input);
    virtual ~AchievementInfo            () {};
    uint32_t getID                      () const { return m_id; }
    irr::core::stringw getDescription   () const { return m_description; }
    virtual Achievement::AchievementType getType () const = 0;
    virtual bool checkCompletion        (Achievement * achievement) const = 0;
    bool needsResetAfterRace() const {return m_reset_after_race; }
};   // class AchievementInfo

class SingleAchievementInfo : public AchievementInfo
{
protected:
    int m_goal_value;

public:
    SingleAchievementInfo               (const XMLNode * input);
    virtual ~SingleAchievementInfo      () {};
    int getGoalValue                    () const { return m_goal_value; }
    virtual bool checkCompletion        (Achievement * achievement) const;

    virtual Achievement::AchievementType getType() const { return Achievement::AT_SINGLE; };
};   // class SingleAchievementInfo

class MapAchievementInfo : public AchievementInfo
{
protected:
    std::map<std::string, int> m_goal_values;

public:
    MapAchievementInfo                  (const XMLNode * input);
    virtual ~MapAchievementInfo         () {};
    int getGoalValue                    (const std::string & key) { return m_goal_values[key];}
    virtual bool checkCompletion        (Achievement * achievement) const;
    virtual Achievement::AchievementType  getType() const { return Achievement::AT_MAP; };
};   // class MapAchievementInfo

#endif

/*EOF*/
