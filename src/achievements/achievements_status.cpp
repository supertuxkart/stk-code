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
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
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
    for (unsigned int i=0; i<ACHIEVE_DATA_NUM; i++)
    {
        m_variables[i].counter = 0;
    }

    // Create one TrackStats instance for all existing tracks
    const unsigned int track_amount = track_manager->getNumberOfTracks();

    for (unsigned int n = 0; n < track_amount; n++)
    {
        Track* curr = track_manager->getTrack(n);
        if (curr->isArena() || curr->isSoccer()||curr->isInternal()) continue;

        TrackStats new_track;
        new_track.ident = curr->getIdent();
        new_track.race_started = new_track.race_finished = new_track.race_won = 0;
        new_track.race_finished_reverse = new_track.race_finished_alone = 0;
        new_track.egg_hunt_started = new_track.egg_hunt_finished = 0;

        m_track_stats.push_back(new_track);
    }   // for n<track_amount
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

    // Load achievement data
    int data_version = -1;
    const XMLNode *n = input->getNode("data");
    if (n!=NULL)
        n->get("version", &data_version);
    if (data_version == DATA_VERSION)
    {
        std::vector<XMLNode*> xml_achievement_data;
        input->getNodes("var", xml_achievement_data);
        for (unsigned int i = 0; i < xml_achievement_data.size(); i++)
        {
            if (i>=ACHIEVE_DATA_NUM)
            {
                Log::warn("AchievementsStatus",
                    "Found more saved achievement data "
                    "than there should be. Discarding.");
                continue;
            }
            xml_achievement_data[i]->get("counter",&m_variables[i].counter); 
        }
        // Load track usage data
        std::vector<XMLNode*> xml_achievement_tracks;
        input->getNodes("track_stats", xml_achievement_tracks);
        //FIXME : while this doesn't happen in a performance-critical part
        //        of the code, this double-loop is not very efficient.
        for (unsigned int i=0; i < xml_achievement_tracks.size(); i++)
        {
            bool track_found = false;
            std::string ident;
            xml_achievement_tracks[i]->get("ident",&ident);
            for (unsigned int j=0 ; j < m_track_stats.size(); j++)
            {
                if (ident == m_track_stats[j].ident)
                {
                    xml_achievement_tracks[i]->get("started",&m_track_stats[j].race_started);
                    xml_achievement_tracks[i]->get("finished",&m_track_stats[j].race_finished);
                    xml_achievement_tracks[i]->get("won",&m_track_stats[j].race_won);
                    xml_achievement_tracks[i]->get("finished_reverse",&m_track_stats[j].race_finished_reverse);
                    xml_achievement_tracks[i]->get("finished_alone",&m_track_stats[j].race_finished_alone);
                    xml_achievement_tracks[i]->get("egg_hunt_started",&m_track_stats[j].egg_hunt_started);
                    xml_achievement_tracks[i]->get("egg_hunt_finished",&m_track_stats[j].egg_hunt_finished);
                    track_found = true;
                    break;
                }
            }
            // Useful if, e.g. an addon track get deleted
            if (!track_found)
            {
                TrackStats new_track;
                new_track.ident = ident;
                xml_achievement_tracks[i]->get("started",&new_track.race_started);
                xml_achievement_tracks[i]->get("finished",&new_track.race_finished);
                xml_achievement_tracks[i]->get("won",&new_track.race_won);
                xml_achievement_tracks[i]->get("finished_reverse",&new_track.race_finished_reverse);
                xml_achievement_tracks[i]->get("finished_alone",&new_track.race_finished_alone);
                xml_achievement_tracks[i]->get("egg_hunt_started",&new_track.egg_hunt_started);
                xml_achievement_tracks[i]->get("egg_hunt_finished",&new_track.egg_hunt_finished);

                m_track_stats.push_back(new_track);
            }
        }
    }
    // If there is nothing valid to load ; we keep the init values

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
    out << "      <achievements online=\"" << m_online << "\"> \n";
    std::map<uint32_t, Achievement*>::const_iterator i;
    for(i = m_achievements.begin(); i != m_achievements.end();  i++)
    {
        if (i->second != NULL)
            i->second->save(out);
    }
    out << "          <data version=\"1\"/>\n";
    for(int i=0;i<ACHIEVE_DATA_NUM;i++)
    {
        out << "          <var counter=\"" << m_variables[i].counter << "\"/>\n";
    }
    for (unsigned int n = 0; n < m_track_stats.size(); n++)
    {
        out << "          <track_stats ident=\"" << m_track_stats[n].ident << "\"";
        out << " started=\"" << m_track_stats[n].race_started << "\"";
        out << " finished=\"" << m_track_stats[n].race_finished << "\"";
        out << " won=\"" << m_track_stats[n].race_won << "\"";
        out << " finished_reverse=\"" << m_track_stats[n].race_finished_reverse << "\"";
        out << " finished_alone=\"" << m_track_stats[n].race_finished_alone << "\"";
        out << " egg_hunt_started=\"" << m_track_stats[n].egg_hunt_started << "\"";
        out << " egg_hunt_finished=\"" << m_track_stats[n].egg_hunt_finished << "\"";
        out << "/>\n";
    }   // for n<m_track_stats.size()
    out << "      </achievements>\n";
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

/* This function checks over achievements to update their goals
   FIXME It is currently hard-coded to specific achievements,
   until it can entirely supersedes the previous system and
   removes its complications.
   FIXME : a data id don't cover some situations, and the function itself ignores it */
void AchievementsStatus::updateAchievementsProgress(unsigned int achieve_data_id)
{
    Achievement *beyond_luck = PlayerManager::getCurrentAchievementsStatus()->getAchievement(AchievementInfo::ACHIEVE_BEYOND_LUCK);
    if (!beyond_luck->isAchieved())
    {
        beyond_luck->reset();
        beyond_luck->increase("wins", "wins", m_variables[ACHIEVE_CONS_WON_RACES].counter);
    }

    Achievement *unstoppable = PlayerManager::getCurrentAchievementsStatus()->getAchievement(AchievementInfo::ACHIEVE_UNSTOPPABLE);
    if (!unstoppable->isAchieved())
    {
        unstoppable->reset();
        unstoppable->increase("wins", "wins", m_variables[ACHIEVE_CONS_WON_RACES_HARD].counter);
    }

    Achievement *gold_driver = PlayerManager::getCurrentAchievementsStatus()->getAchievement(AchievementInfo::ACHIEVE_GOLD_DRIVER);
    if (!gold_driver->isAchieved())
    {
        gold_driver->reset();
        gold_driver->increase("standard", "standard", m_variables[ACHIEVE_WON_NORMAL_RACES].counter);
        gold_driver->increase("std_timetrial", "std_timetrial", m_variables[ACHIEVE_WON_TT_RACES].counter);
        gold_driver->increase("follow_leader", "follow_leader", m_variables[ACHIEVE_WON_FTL_RACES].counter);
    }

    Achievement *powerup_lover = PlayerManager::getCurrentAchievementsStatus()->getAchievement(AchievementInfo::ACHIEVE_POWERUP_LOVER);
    if (!powerup_lover->isAchieved())
    {
        powerup_lover->reset();
        powerup_lover->increase("poweruplover", "poweruplover", m_variables[ACHIEVE_POWERUP_USED_1RACE].counter);
    }

    Achievement *banana_lover = PlayerManager::getCurrentAchievementsStatus()->getAchievement(AchievementInfo::ACHIEVE_BANANA);
    if (!banana_lover->isAchieved())
    {
        banana_lover->reset();
        banana_lover->increase("banana", "banana", m_variables[ACHIEVE_BANANA_1RACE].counter);
    }

    Achievement *skidding = PlayerManager::getCurrentAchievementsStatus()->getAchievement(AchievementInfo::ACHIEVE_SKIDDING);
    if (!skidding->isAchieved())
    {
        skidding->reset();
        skidding->increase("skidding", "skidding", m_variables[ACHIEVE_SKIDDING_1LAP].counter);
    }

    Achievement *strike = PlayerManager::getCurrentAchievementsStatus()->getAchievement(AchievementInfo::ACHIEVE_STRIKE);
    if (!strike->isAchieved())
    {
        strike->reset();
        strike->increase("ball", "ball", m_variables[BOWLING_HIT].counter);
    }

    Achievement *mosquito = PlayerManager::getCurrentAchievementsStatus()->getAchievement(AchievementInfo::ACHIEVE_MOSQUITO);
    if (!mosquito->isAchieved())
    {
        mosquito->reset();
        mosquito->increase("swatter", "swatter", m_variables[SWATTER_HIT_1RACE].counter);
    }

    Achievement *columbus = PlayerManager::getCurrentAchievementsStatus()->getAchievement(AchievementInfo::ACHIEVE_COLUMBUS);
    if (!columbus->isAchieved())
    {
        columbus->reset();
        for (unsigned int i=0;i<m_track_stats.size();i++)
        {
            // ignore addons tracks (compare returns 0 when the values are equal)
            if (m_track_stats[i].ident.compare(0 /*start of sub-string*/,5/*length*/,"addon") == 0)
                continue;

            columbus->increase(m_track_stats[i].ident, m_track_stats[i].ident, std::min<int>(m_track_stats[i].race_finished,1));
        }
    }

}

// ----------------------------------------------------------------------------
void AchievementsStatus::increaseDataVar(unsigned int achieve_data_id, int increase)
{
    if (achieve_data_id<ACHIEVE_DATA_NUM)
    {
        m_variables[achieve_data_id].counter += increase;
        if (m_variables[achieve_data_id].counter > 10000000)
            m_variables[achieve_data_id].counter = 10000000;

        updateAchievementsProgress(achieve_data_id);
    }
#ifdef DEBUG
    else
    {
        Log::error("Achievements", "Achievement data id %i don't match any variable.",
                  achieve_data_id);
    }
#endif
}   // increaseDataVar

// ----------------------------------------------------------------------------
void AchievementsStatus::resetDataVar(unsigned int achieve_data_id)
{
    if (achieve_data_id<ACHIEVE_DATA_NUM)
    {
        m_variables[achieve_data_id].counter = 0;
    }
#ifdef DEBUG
    else
    {
        Log::error("Achievements", "Achievement data id %i don't match any variable.",
                  achieve_data_id);
    }
#endif
}   // resetDataVar

// ----------------------------------------------------------------------------
void AchievementsStatus::onRaceEnd(bool aborted)
{
    updateAchievementsProgress(0);

    //reset all values that need to be reset
    std::map<uint32_t, Achievement *>::iterator iter;
    for ( iter = m_achievements.begin(); iter != m_achievements.end(); ++iter ) {
        iter->second->onRaceEnd();
    }

    m_variables[ACHIEVE_POWERUP_USED_1RACE].counter = 0;
    m_variables[ACHIEVE_BANANA_1RACE].counter = 0;
    m_variables[ACHIEVE_SKIDDING_1RACE].counter = 0;
    m_variables[BOWLING_HIT_1RACE].counter = 0;
    m_variables[SWATTER_HIT_1RACE].counter = 0;


    // Prevent restart from being abused to get consecutive wins achievement
    if (aborted)
    {
        m_variables[ACHIEVE_CONS_WON_RACES].counter = 0;
        m_variables[ACHIEVE_CONS_WON_RACES_HARD].counter = 0;
    }
}   // onRaceEnd

// ----------------------------------------------------------------------------
void AchievementsStatus::onLapEnd()
{
    updateAchievementsProgress(0);

    //reset all values that need to be reset
    std::map<uint32_t, Achievement *>::iterator iter;
    for (iter = m_achievements.begin(); iter != m_achievements.end(); ++iter) {
        iter->second->onLapEnd();
    }

    m_variables[ACHIEVE_SKIDDING_1LAP].counter = 0;
}   // onLapEnd

// ----------------------------------------------------------------------------
/** Use the event type to increment the correct track event counter
 *  \param track_ident - the internal name of the track
 *  \param event - the type of counter to increment */
void AchievementsStatus::trackEvent(std::string track_ident, AchievementsStatus::TrackData event)
{
    int track_id = -1;
    for (unsigned int i=0;i<m_track_stats.size();i++)
    {
        if (m_track_stats[i].ident == track_ident)
        {
            track_id = i;
            break;
        }
    }
    if (event==TR_STARTED)
        m_track_stats[track_id].race_started++;
    else if (event==TR_FINISHED)
        m_track_stats[track_id].race_finished++;
    else if (event==TR_WON)
        m_track_stats[track_id].race_won++;
    else if (event==TR_FINISHED_REVERSE)
        m_track_stats[track_id].race_finished_reverse++;
    else if (event==TR_FINISHED_ALONE)
        m_track_stats[track_id].race_finished_alone++;
    else if (event==TR_EGG_HUNT_STARTED)
        m_track_stats[track_id].egg_hunt_started++;
    else if (event==TR_EGG_HUNT_FINISHED)
        m_track_stats[track_id].egg_hunt_finished++;
} // trackEvent
