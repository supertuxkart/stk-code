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

#ifndef HEADER_ACHIEVEMENTS_MANAGER_HPP
#define HEADER_ACHIEVEMENTS_MANAGER_HPP

#include "utils/types.hpp"
#include "utils/ptr_vector.hpp"
#include "achievements/achievement_info.hpp"
#include "achievements/achievements_slot.hpp"


#include <irrString.h>
#include <string>
#include <vector>
#include <map>


// ============================================================================

/**
  * \brief Class that takes care of online profiles
  * \ingroup online
  */
class AchievementsManager
{
private :
    AchievementsSlot * m_active_slot;
    PtrVector<AchievementsSlot> m_slots;
    PtrVector<AchievementInfo> m_achievements_info;
    AchievementsManager      ();
    ~AchievementsManager     ();
    AchievementsSlot * createNewSlot(std::string id, bool online);

public:
    /**Singleton */
    static AchievementsManager *            get();
    static void                             deallocate();

    void parseDataFile();
    void parseConfigFile();
    void save();
    void onRaceEnd();
    void updateCurrentPlayer();
    AchievementsSlot * getActive() const { return m_active_slot; }
    AchievementsSlot * getSlot(const std::string & id, bool online);
    void createSlotsIfNeeded();
};   // class AchievementsManager

#endif

/*EOF*/
