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

#ifndef HEADER_WEB_ACHIEVEMENTS_STATUS_HPP
#define HEADER_WEB_ACHIEVEMENTS_STATUS_HPP

#include "achievements/achievement_info.hpp"
#include <map>

class AchievementsStatus;

// ============================================================================
/** This class updates achievements on an online site. It detects when
 * achievements haven't been updated and sends the changes.
 * \ingroup achievements
 */
class WebAchievementsStatus
{
private:

#ifdef GAMERZILLA
    int m_game_id;
#endif
public:

             WebAchievementsStatus(int version, std::map<uint32_t, AchievementInfo *> &info);
    virtual ~WebAchievementsStatus();
    
    void updateAchievementsProgress(AchievementsStatus *status);
    void achieved(AchievementInfo *info);

#ifdef GAMERZILLA
    int getGameID() const { return m_game_id; }
#endif
};   // class Achievement
#endif
