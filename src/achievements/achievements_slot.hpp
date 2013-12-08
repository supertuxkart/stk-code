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

#ifndef HEADER_ACHIEVEMENTS_SLOT_HPP
#define HEADER_ACHIEVEMENTS_SLOT_HPP

#include "utils/types.hpp"
#include "achievements/achievement.hpp"
#include "online/http_manager.hpp"

#include <irrString.h>
#include <string>
#include "io/xml_node.hpp"


class AchievementsSlot
{
private:
    std::map<uint32_t, Achievement *> m_achievements;
    bool m_online;
    bool m_valid;
    std::string m_id;

    void createFreshSlot();
    void deleteAchievements();

    class SyncAchievementsRequest : public Online::XMLRequest {
        virtual void callback ();
    public:
        SyncAchievementsRequest() : Online::XMLRequest(true) {}
    };

public :
    AchievementsSlot(const XMLNode * input);
    AchievementsSlot(std::string id, bool online);
    ~AchievementsSlot();
    bool isValid() const { return m_valid;}
    void save(std::ofstream & out);
    bool isOnline() const {return m_online;}
    void sync(const std::vector<uint32_t> & achieved_ids);
    void onRaceEnd();
    const std::string & getID() const {return m_id;}
    const std::map<uint32_t, Achievement *> & getAllAchievements() {return m_achievements;}
    Achievement * getAchievement(uint32_t id);
};

#endif

/*EOF*/
