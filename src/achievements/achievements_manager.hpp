//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 Glenn De Jonghe
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

#ifndef HEADER_ACHIEVEMENTS_MANAGER_HPP
#define HEADER_ACHIEVEMENTS_MANAGER_HPP

#include "achievements/achievements_status.hpp"
#include "achievements/web_achievements_status.hpp"

#include "utils/types.hpp"
#include "utils/ptr_vector.hpp"

class AchievementInfo;
class AchievementsStatus;

#include <string>
#include <map>


/** This class manages the list of all achievements. It reads the
 *  data/achievements.xml file, which contains the conditions for
 *  each achievement.
  * \ingroup online
  */
class AchievementsManager
{
private:
    /** Pointer to the single instance. */
    static AchievementsManager* m_achievements_manager;

    std::map<uint32_t, AchievementInfo *> m_achievements_info;
    WebAchievementsStatus *m_web;

    AchievementsManager      ();
    ~AchievementsManager     ();
    AchievementsStatus * createNewSlot(unsigned int id, bool online);

public:
    /** Static function to create the instance of the achievement manager. */
    static void create()
    {
        assert(!m_achievements_manager);
        m_achievements_manager = new AchievementsManager();
    }   // create
    // ------------------------------------------------------------------------
    /** Static function to get the achievement manager. */
    static AchievementsManager* get()
    {
        assert(m_achievements_manager);
        return m_achievements_manager;
    }   // get
    // ------------------------------------------------------------------------
    static void destroy()
    {
        delete m_achievements_manager;
        m_achievements_manager = NULL;
    }   // destroy
    // ========================================================================

    AchievementInfo* getAchievementInfo(uint32_t id) const;
    AchievementsStatus* createAchievementsStatus(const XMLNode *node=NULL, bool updateWeb = false);
    // ------------------------------------------------------------------------
    const std::map<uint32_t, AchievementInfo *> & getAllInfo()
    {
        return m_achievements_info;
    }  // getAllInfo

    WebAchievementsStatus* getWebAchievementStatus() { return m_web; }
};   // class AchievementsManager

#endif

/*EOF*/
