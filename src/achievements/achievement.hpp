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

#ifndef HEADER_ACHIEVEMENT_HPP
#define HEADER_ACHIEVEMENT_HPP

#include "utils/types.hpp"

#include <irrString.h>
#include <map>
#include <string>

class UTFWriter;
class XMLNode;

// ============================================================================

/** This is the base class for any achievement. It allows achievement status
 *  to be saved, and detects when an achievement is fulfilled.
 * \ingroup achievements
 */
class AchievementInfo;

class Achievement
{
protected:
    /** The id of this achievement. */
    uint32_t               m_id;

    /** True if this achievement has been achieved. */
    bool                   m_achieved;

    /** A pointer to the corresponding AchievementInfo instance. */
    const AchievementInfo *m_achievement_info;

    void check();

public:
    enum AchievementType
    {
        AT_SINGLE,
        AT_MAP
    };

    Achievement(const AchievementInfo * info);
    virtual ~Achievement       ();
    virtual void load          (const XMLNode *node) ;
    virtual void save          (UTFWriter &out) ;
    virtual void reset         () = 0;
    virtual irr::core::stringw getProgressAsString() = 0;
    void onRaceEnd();
    // ------------------------------------------------------------------------
    /** Returns the id of this achievement. */
    uint32_t getID() const { return m_id; }
    // ------------------------------------------------------------------------
    /** Returns the AchievementInfo for this achievement. */
    const AchievementInfo * getInfo() const { return m_achievement_info; }
    // ------------------------------------------------------------------------
    /** Sets this achievement to be fulfilled. */
    void setAchieved() { m_achieved = true; };
    // ------------------------------------------------------------------------
    /** Returns if this achievement has been fulfilled. */
    bool isAchieved() const { return m_achieved;  }

};   // class Achievement

// ============================================================================
/** This is a base class for an achievement that counts how often an event
 *  happened, and triggers the achievement to be fulfilled when a certain
 *  goal value is reached.
 * \ingroup achievements
 */
class SingleAchievement : public Achievement
{
protected:
    int m_progress;

public:
    SingleAchievement                   (const AchievementInfo * info);
    virtual ~SingleAchievement          () {};

    void load                           (const XMLNode *node);
    int getValue                        () const { return m_progress; }
    void save                           (UTFWriter &out);
    void increase                       (int increase = 1);
    void reset                          ();
    virtual irr::core::stringw          getProgressAsString ();
};   // class SingleAchievement

// ============================================================================
/** This achievement can keep track of a set of key-value pairs. Fulfillment is
 *  triggered when all values defined in the data/achievements.xml file have
 *  been reached.
* \ingroup achievements
*/
class MapAchievement : public Achievement
{
protected:
    /** The map of key-value pairs. */
    std::map<std::string, int> m_progress_map;

public:
    MapAchievement                      (const AchievementInfo * info);
    virtual ~MapAchievement             () {};

    void load                           (const XMLNode *node);
    int getValue                        (const std::string & key);
    void increase                       (const std::string & key, int increase = 1);
    void save                           (UTFWriter &out);
    void reset                          ();
    virtual irr::core::stringw          getProgressAsString ();
};   // class MapAchievement

#endif

/*EOF*/
