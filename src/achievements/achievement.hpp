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

#ifndef HEADER_ACHIEVEMENT_HPP
#define HEADER_ACHIEVEMENT_HPP

#include "utils/types.hpp"

#include <irrString.h>
#include <string>
#include "io/xml_node.hpp"

class AchievementInfo;

// ============================================================================

/**
  * \brief
  * \ingroup
  */
class AchievementInfo;

class Achievement
{
protected:
    uint32_t                            m_id;
    bool                                m_achieved;
    AchievementInfo *                   m_achievement_info;
    void                check           ();

public:
    Achievement                         (AchievementInfo * info);
    virtual ~Achievement                ();
    uint32_t getID                      () const { return m_id; }
    virtual void load                   (XMLNode * input) = 0;
    virtual void save                   (std::ofstream & out) = 0;

    enum AchievementType
    {
        AT_SINGLE,
        AT_MAP
    };

};   // class Achievement

class SingleAchievement : public Achievement
{
protected:
    int m_progress;

public:
    SingleAchievement                   (AchievementInfo * info);
    virtual ~SingleAchievement          () {};

    void load                           (XMLNode * input);
    int getValue                        () const { return m_progress; }
    void save                           (std::ofstream & out);
};   // class SingleAchievement

class MapAchievement : public Achievement
{
protected:
    std::map<std::string, int> m_progress_map;

public:
    MapAchievement                      (AchievementInfo * info);
    virtual ~MapAchievement             () {};

    void load                           (XMLNode * input);
    int getValue                        (const std::string & key);
    void save                           (std::ofstream & out);
};   // class MapAchievement

#endif

/*EOF*/
