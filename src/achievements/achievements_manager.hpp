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
#include "achievements/achievement.hpp"


#include <irrString.h>
#include <string>


// ============================================================================

/**
  * \brief Class that takes care of online profiles
  * \ingroup online
  */
class AchievementsManager
{

    private:
        AchievementsManager      ();
        ~AchievementsManager     ();
        void parseAchievements();
        std::map<uint32_t, * Achievement> m_achievements;

    public:
        /**Singleton */
        static AchievementsManager *            get();
        static void                             deallocate();
};   // class AchievementsManager

#endif

/*EOF*/
