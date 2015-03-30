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

#ifndef HEADER_ACHIEVEMENTS_SLOT_HPP
#define HEADER_ACHIEVEMENTS_SLOT_HPP

#include "achievements/achievement.hpp"
#include "online/request_manager.hpp"
#include "online/xml_request.hpp"
#include "utils/types.hpp"

#include <irrString.h>
#include <string>

class UTFWriter;
class XMLNode;

/** This class keeps tracks of all achievements of one player. One instance
 *  of this class is stored in each PlayerProfile. It stores a map of
 *  achievements ids to instances of Achievement. Each achievement in
 *  turn stores either fulfilled achievements, or the current state of
 *  an achievement (e.g. an achievement to race every track in STK needs
 *  to keep information about which tracks have already been used.)
*/
class AchievementsStatus
{
private:
    std::map<uint32_t, Achievement *> m_achievements;
    bool         m_online;
    bool         m_valid;

    class SyncAchievementsRequest : public Online::XMLRequest {
        virtual void callback ();
    public:
        SyncAchievementsRequest() : Online::XMLRequest(true) {}
    };

public :
    AchievementsStatus();
    ~AchievementsStatus();
    Achievement * getAchievement(uint32_t id);
    void load(const XMLNode * input);
    void save(UTFWriter &out);
    void add(Achievement *achievement);
    void sync(const std::vector<uint32_t> & achieved_ids);
    void onRaceEnd();
    void onLapEnd();
    // ------------------------------------------------------------------------
    const std::map<uint32_t, Achievement *>& getAllAchievements()
    {
        return m_achievements;
    }
    // ------------------------------------------------------------------------
    bool isOnline() const { return m_online; }
    // ------------------------------------------------------------------------
    bool isValid() const { return m_valid; }
    // ------------------------------------------------------------------------
};   // class AchievementsStatus

#endif

/*EOF*/
