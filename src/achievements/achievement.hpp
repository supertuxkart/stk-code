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

#include "utils/types.hpp"

#include <irrString.h>
#include <map>
#include <string>

class UTFWriter;
class XMLNode;

// ============================================================================
/** This is the base class for any achievement. It allows achievement status
 *  to be saved, and detects when an achievement is fulfilled. It provides
 *  storage for state information by a generic key-value mapping. The values
 *  are stored as strings, but can be used to store numerical values. E.g.
 *  you can call increase("key", 10) for an achievement, which will convert
 *  the string to int, add 10, then convert the result back to string for
 *  storage.
 * \ingroup achievements
 */
class AchievementInfo;

class Achievement
{
private:
    /** The id of this achievement. */
    uint32_t               m_id;

    /** True if this achievement has been achieved. */
    bool                   m_achieved;

    /** The map of key-value pairs. */
    std::map<std::string, int> m_progress_map;

    /** A pointer to the corresponding AchievementInfo instance. */
    const AchievementInfo *m_achievement_info;

    void check();

public:

             Achievement(const AchievementInfo * info);
    virtual ~Achievement();
    virtual void load(const XMLNode *node);
    virtual void save(UTFWriter &out);
    virtual int getValue(const std::string & key);
    void increase(const std::string & key, const std::string &goal_key,
                  int increase = 1);

    virtual void reset();
    virtual irr::core::stringw getProgressAsString() const;
    void onRaceEnd();
    void onLapEnd();
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
    // ------------------------------------------------------------------------
    const std::map<std::string, int>& getProgress() const
    {
        return m_progress_map;
    }   // getProgress
};   // class Achievement
#endif
