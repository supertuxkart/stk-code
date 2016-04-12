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


#include "achievements/achievements_status.hpp"

#include "achievements/achievement_info.hpp"
#include "achievements/achievements_manager.hpp"
#include "config/player_manager.hpp"
#include "io/utf_writer.hpp"
#include "utils/log.hpp"
#include "utils/ptr_vector.hpp"
#include "utils/translation.hpp"

#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <assert.h>


// ----------------------------------------------------------------------------
/** Constructor for an Achievement.
 */
AchievementsStatus::AchievementsStatus()
{
    m_valid  = true;
    m_online = true;
}   // AchievementsStatus

// ----------------------------------------------------------------------------
/** Removes all achievements.
 */
AchievementsStatus::~AchievementsStatus()
{
    std::map<uint32_t, Achievement *>::iterator it;
    for (it = m_achievements.begin(); it != m_achievements.end(); ++it) {
        delete it->second;
    }
    m_achievements.clear();
}   // ~AchievementsStatus

// ----------------------------------------------------------------------------
/** Loads the saved state of all achievements from an XML file.
 *  \param input The XML node to load the data from.
 */
void AchievementsStatus::load(const XMLNode * input)
{
    std::vector<XMLNode*> xml_achievements;
    input->getNodes("achievement", xml_achievements);
    for (unsigned int i = 0; i < xml_achievements.size(); i++)
    {
        uint32_t achievement_id(0);
        xml_achievements[i]->get("id", &achievement_id);
        Achievement * achievement = getAchievement(achievement_id);
        if (achievement == NULL)
        {
            Log::warn("AchievementsStatus",
                "Found saved achievement data for a non-existent "
                "achievement. Discarding.");
            continue;
        }
        achievement->load(xml_achievements[i]);
    }   // for i in xml_achievements

}   // load

// ----------------------------------------------------------------------------
void AchievementsStatus::add(Achievement *achievement)
{
    m_achievements[achievement->getID()] = achievement;
}    // add


// ----------------------------------------------------------------------------
/** Saves the achievement status to a file. Achievements are stored as part
 *  of the player data file players.xml.
 *  \param out File to write to.
 */
void AchievementsStatus::save(UTFWriter &out)
{
    out << L"      <achievements online=\"" << m_online << L"\"> \n";
    std::map<uint32_t, Achievement*>::const_iterator i;
    for(i = m_achievements.begin(); i != m_achievements.end();  i++)
    {
        if (i->second != NULL)
            i->second->save(out);
    }
    out << L"      </achievements>\n";
}   // save

// ----------------------------------------------------------------------------
Achievement * AchievementsStatus::getAchievement(uint32_t id)
{
    if ( m_achievements.find(id) != m_achievements.end())
        return m_achievements[id];
    return NULL;
}   // getAchievement

// ----------------------------------------------------------------------------
/** Synchronises the achievements between local and online usage. It takes
 *  the list of online achievements, and marks them all to be achieved
 *  locally. Then it issues 'achieved' requests to the server for all local
 *  achievements that are not set online.
*/
void AchievementsStatus::sync(const std::vector<uint32_t> & achieved_ids)
{
    std::vector<bool> done;
    for(unsigned int i =0; i < achieved_ids.size(); ++i)
    {
        if(done.size()< achieved_ids[i]+1)
            done.resize(achieved_ids[i]+1);
        done[achieved_ids[i]] = true;
        Achievement * achievement = getAchievement(achieved_ids[i]);
        if(achievement != NULL)
            achievement->setAchieved();
    }

    std::map<uint32_t, Achievement*>::iterator i;

    // String to collect all local ids that are not synched
    // to the online account
    std::string ids;
    for(i=m_achievements.begin(); i!=m_achievements.end(); i++)
    {
        unsigned int id = i->second->getID();
        if(i->second->isAchieved() && (id>=done.size() || !done[id]) )
        {
            ids=ids+StringUtils::toString(id)+",";
        }
    }

    if(ids.size()>0)
    {
        ids = ids.substr(0, ids.size() - 1); // delete the last "," in the string
        Log::info("Achievements", "Synching achievement %s to server.",
                  ids.c_str());
        Online::HTTPRequest * request = new Online::HTTPRequest(true, 2);
        PlayerManager::setUserDetails(request, "achieving");
        request->addParameter("achievementid", ids);
        request->queue();
    }
}   // sync

// ----------------------------------------------------------------------------
void AchievementsStatus::onRaceEnd()
{
    //reset all values that need to be reset
    std::map<uint32_t, Achievement *>::iterator iter;
    for ( iter = m_achievements.begin(); iter != m_achievements.end(); ++iter ) {
        iter->second->onRaceEnd();
    }
}   // onRaceEnd

// ----------------------------------------------------------------------------
void AchievementsStatus::onLapEnd()
{
    //reset all values that need to be reset
    std::map<uint32_t, Achievement *>::iterator iter;
    for (iter = m_achievements.begin(); iter != m_achievements.end(); ++iter) {
        iter->second->onLapEnd();
    }
}   // onLapEnd
