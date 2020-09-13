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


#include "achievements/web_achievements_status.hpp"

#include "achievements/achievement.hpp"
#include "achievements/achievement_info.hpp"
#include "achievements/achievements_status.hpp"
#include "io/file_manager.hpp"

#ifdef GAMERZILLA
#include "gamerzilla.h"
#endif

/** Constructur, initialises this object with the data from the
 *  corresponding AchievementInfo.
 */
WebAchievementsStatus::WebAchievementsStatus(int version, std::map<uint32_t, AchievementInfo *> &info)
{
#ifdef GAMERZILLA
    GamerzillaStart(false, (file_manager->getUserConfigDir() + "gamerzilla/").c_str());
    Gamerzilla g;
    std::string path = file_manager->getAsset("gamerzilla/");
    std::string main_image = path + "supertuxkart.png";
    std::string true_image = path + "achievement1.png";
    std::string false_image = path + "achievement0.png";
    GamerzillaInitGame(&g);
    g.version = version;
    g.short_name = strdup("supertuxkart");
    g.name = strdup("SuperTuxKart");
    g.image = strdup(main_image.c_str());
    for (auto const &i : info)
    {
        GamerzillaGameAddTrophy(&g, i.second->getRawName().c_str(), i.second->getRawDescription().c_str(), 0, true_image.c_str(), false_image.c_str());
    }
    m_game_id = GamerzillaSetGame(&g);
    GamerzillaClearGame(&g);
#endif
}   // WebAchievementStatus

// ----------------------------------------------------------------------------
WebAchievementsStatus::~WebAchievementsStatus()
{
#ifdef GAMERZILLA
    GamerzillaQuit();
#endif
}   // ~WebAchievementStatus

// ----------------------------------------------------------------------------
/** Checks for an updates to achievements progress
 *  \param Current status
 */
void WebAchievementsStatus::updateAchievementsProgress(AchievementsStatus *status)
{
#ifdef GAMERZILLA
    std::map<uint32_t, Achievement *> &a = status->getAllAchievements();
    for (auto &current : a)
    {
        if (current.second->isAchieved())
        {
            GamerzillaSetTrophy(m_game_id, current.second->getInfo()->getRawName().c_str());
        }
    }
#endif
}   // updateAchievementsProgress

// ----------------------------------------------------------------------------
/** Sets an achievement
 *  \param Achievement acquired
 */
void WebAchievementsStatus::achieved(AchievementInfo *info)
{
#ifdef GAMERZILLA
    GamerzillaSetTrophy(m_game_id, info->getRawName().c_str());
#endif
}
