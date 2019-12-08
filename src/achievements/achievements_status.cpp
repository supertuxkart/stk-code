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
#include "io/xml_node.hpp"
#include "online/xml_request.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/log.hpp"
#include "utils/ptr_vector.hpp"
#include "utils/string_utils.hpp"

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
    const unsigned int track_amount =
        (unsigned int)track_manager->getNumberOfTracks();

    for (unsigned int n = 0; n < track_amount; n++)
    {
        Track* curr = track_manager->getTrack(n);
        if (curr->isArena() || curr->isSoccer()||curr->isInternal()) continue;

        TrackStats new_track;
        new_track.ident = curr->getIdent();
        for (unsigned int i=0;i<TR_DATA_NUM;i++)
        {
            new_track.track_data[i] = 0;
        }

        m_track_stats.push_back(new_track);
    }   // for n<track_amount

    setEnumToString();
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

/** This function loads a table associating an enum identifier
  * with the matching command in achievements.xml.
  * counters with anassociated max version are prefixed to allow
  * the achievement progress update to do the correct action. */
void AchievementsStatus::setEnumToString()
{
    m_ach_enum_to_xml[(int)WON_RACES] = "won-races";
    m_ach_enum_to_xml[(int)WON_NORMAL_RACES] = "won-normal-races";
    m_ach_enum_to_xml[(int)WON_TT_RACES] = "won-tt-races";
    m_ach_enum_to_xml[(int)WON_FTL_RACES] = "won-ftl-races";
    m_ach_enum_to_xml[(int)CONS_WON_RACES] = "LOGC-cons-won-races";
    m_ach_enum_to_xml[(int)CONS_WON_RACES_MAX] = "LOGM-cons-won-races";
    m_ach_enum_to_xml[(int)CONS_WON_RACES_HARD] = "LOGC-cons-won-races-hard";
    m_ach_enum_to_xml[(int)CONS_WON_RACES_HARD_MAX] = "LOGM-cons-won-races-hard";
    m_ach_enum_to_xml[(int)EASY_STARTED] = "easy-started";
    m_ach_enum_to_xml[(int)EASY_FINISHED] = "easy-finished";
    m_ach_enum_to_xml[(int)MEDIUM_STARTED] = "medium-started";
    m_ach_enum_to_xml[(int)MEDIUM_FINISHED] = "medium-finished";
    m_ach_enum_to_xml[(int)HARD_STARTED] = "hard-started";
    m_ach_enum_to_xml[(int)HARD_FINISHED] = "hard-finished";
    m_ach_enum_to_xml[(int)BEST_STARTED] = "best-started";
    m_ach_enum_to_xml[(int)BEST_FINISHED] = "best-finished";
    m_ach_enum_to_xml[(int)NORMAL_STARTED] = "normal-started";
    m_ach_enum_to_xml[(int)NORMAL_FINISHED] = "normal-finished";
    m_ach_enum_to_xml[(int)TT_STARTED] = "tt-started";
    m_ach_enum_to_xml[(int)TT_FINISHED] = "tt-finished";
    m_ach_enum_to_xml[(int)FTL_STARTED] = "ftl-started";
    m_ach_enum_to_xml[(int)FTL_FINISHED] = "ftl-finished";
    m_ach_enum_to_xml[(int)THREE_STRIKES_STARTED] = "three-strikes-started";
    m_ach_enum_to_xml[(int)THREE_STRIKES_FINISHED] = "three-strikes-finished";
    m_ach_enum_to_xml[(int)SOCCER_STARTED] = "soccer-started";
    m_ach_enum_to_xml[(int)SOCCER_FINISHED] = "soccer-finished";
    m_ach_enum_to_xml[(int)EGG_HUNT_STARTED] = "egg-hunt-started";
    m_ach_enum_to_xml[(int)EGG_HUNT_FINISHED] = "egg-hunt-finished";
    m_ach_enum_to_xml[(int)WITH_GHOST_STARTED] = "with-ghost-started";
    m_ach_enum_to_xml[(int)WITH_GHOST_FINISHED] = "with-ghost-finished";
    m_ach_enum_to_xml[(int)CTF_STARTED] = "ctf-started";
    m_ach_enum_to_xml[(int)CTF_FINISHED] = "ctf-finished";
    m_ach_enum_to_xml[(int)FFA_STARTED] = "ffa-started";
    m_ach_enum_to_xml[(int)FFA_FINISHED] = "ffa-finished";
    m_ach_enum_to_xml[(int)POWERUP_USED] = "powerup-used";
    m_ach_enum_to_xml[(int)POWERUP_USED_1RACE] = "LOGC-powerup-used-1race";
    m_ach_enum_to_xml[(int)POWERUP_USED_1RACE_MAX] = "LOGM-powerup-used-1race";
    m_ach_enum_to_xml[(int)BOWLING_HIT] = "bowling-hit";
    m_ach_enum_to_xml[(int)BOWLING_HIT_1RACE] = "LOGC-bowling-hit-1race";
    m_ach_enum_to_xml[(int)BOWLING_HIT_1RACE_MAX] = "LOGM-bowling-hit-1race";
    m_ach_enum_to_xml[(int)SWATTER_HIT] = "swatter-hit";
    m_ach_enum_to_xml[(int)SWATTER_HIT_1RACE] = "LOGC-swatter-hit-1race";
    m_ach_enum_to_xml[(int)SWATTER_HIT_1RACE_MAX] = "LOGM-swatter-hit-1race";
    m_ach_enum_to_xml[(int)ALL_HITS] = "all-hits";
    m_ach_enum_to_xml[(int)ALL_HITS_1RACE] = "LOGC-all-hits-1race";
    m_ach_enum_to_xml[(int)ALL_HITS_1RACE_MAX] = "LOGM-all-hits-1race";
    m_ach_enum_to_xml[(int)BANANA] = "banana";
    m_ach_enum_to_xml[(int)BANANA_1RACE] = "LOGC-banana-1race";
    m_ach_enum_to_xml[(int)BANANA_1RACE_MAX] = "LOGM-banana-1race";
    m_ach_enum_to_xml[(int)SKIDDING] = "skidding";
    m_ach_enum_to_xml[(int)SKIDDING_1RACE] = "LOGC-skidding-1race";
    m_ach_enum_to_xml[(int)SKIDDING_1RACE_MAX] = "LOGM-skidding-1race";
    m_ach_enum_to_xml[(int)SKIDDING_1LAP] = "LOGC-skidding-1lap";
    m_ach_enum_to_xml[(int)SKIDDING_1LAP_MAX] = "LOGM-skidding-1lap";

    m_tr_enum_to_xml[(int)TR_STARTED] = "race-started";
    m_tr_enum_to_xml[(int)TR_FINISHED] = "race-finished";
    m_tr_enum_to_xml[(int)TR_WON] = "race-won";
    m_tr_enum_to_xml[(int)TR_FINISHED_REVERSE] = "race-finished-reverse";
    m_tr_enum_to_xml[(int)TR_LESS_LAPS] = "less-laps";
    m_tr_enum_to_xml[(int)TR_MORE_LAPS] = "more-laps";
    m_tr_enum_to_xml[(int)TR_MIN_TWICE_LAPS] = "twice-laps";
    m_tr_enum_to_xml[(int)TR_FINISHED_ALONE] = "race-finished-alone";
    m_tr_enum_to_xml[(int)TR_EGG_HUNT_STARTED] = "egg-hunt-started";
    m_tr_enum_to_xml[(int)TR_EGG_HUNT_FINISHED] = "egg-hunt-started";

    m_tr_enum_to_xml[(int)TR_STARTED + (int)TR_DATA_NUM] = "race-started-all";
    m_tr_enum_to_xml[(int)TR_FINISHED + (int)TR_DATA_NUM] = "race-finished-all";
    m_tr_enum_to_xml[(int)TR_WON + (int)TR_DATA_NUM] = "race-won-all";
    m_tr_enum_to_xml[(int)TR_FINISHED_REVERSE + (int)TR_DATA_NUM] = "race-finished-reverse-all";
    m_tr_enum_to_xml[(int)TR_LESS_LAPS + (int)TR_DATA_NUM] = "less-laps-all";
    m_tr_enum_to_xml[(int)TR_MORE_LAPS + (int)TR_DATA_NUM] = "more-laps-all";
    m_tr_enum_to_xml[(int)TR_MIN_TWICE_LAPS + (int)TR_DATA_NUM] = "twice-laps-all";
    m_tr_enum_to_xml[(int)TR_FINISHED_ALONE + (int)TR_DATA_NUM] = "race-finished-alone-all";
    m_tr_enum_to_xml[(int)TR_EGG_HUNT_STARTED + (int)TR_DATA_NUM] = "egg-hunt-started-all";
    m_tr_enum_to_xml[(int)TR_EGG_HUNT_FINISHED + (int)TR_DATA_NUM] = "egg-hunt-started-all";
} // setEnumToString

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
        achievement->loadProgress(xml_achievements[i]);
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
            // We go over already loaded tracks to avoid duplicates
            for (unsigned int j=0 ; j < m_track_stats.size(); j++)
            {
                if (ident == m_track_stats[j].ident)
                {
                    xml_achievement_tracks[i]->get("sta",&m_track_stats[j].track_data[(int)TR_STARTED]);
                    xml_achievement_tracks[i]->get("fin",&m_track_stats[j].track_data[(int)TR_FINISHED]);
                    xml_achievement_tracks[i]->get("won",&m_track_stats[j].track_data[(int)TR_WON]);
                    xml_achievement_tracks[i]->get("fin_rev",&m_track_stats[j].track_data[(int)TR_FINISHED_REVERSE]);
                    xml_achievement_tracks[i]->get("fin_al",&m_track_stats[j].track_data[(int)TR_FINISHED_ALONE]);
                    xml_achievement_tracks[i]->get("l_laps",&m_track_stats[j].track_data[(int)TR_LESS_LAPS]);
                    xml_achievement_tracks[i]->get("m_laps",&m_track_stats[j].track_data[(int)TR_MORE_LAPS]);
                    xml_achievement_tracks[i]->get("t_laps",&m_track_stats[j].track_data[(int)TR_MIN_TWICE_LAPS]);
                    xml_achievement_tracks[i]->get("eh_sta",&m_track_stats[j].track_data[(int)TR_EGG_HUNT_STARTED]);
                    xml_achievement_tracks[i]->get("eh_fin",&m_track_stats[j].track_data[(int)TR_EGG_HUNT_FINISHED]);
                    track_found = true;
                    break;
                }
            }
            // Useful if, e.g. an addon track gets deleted
            if (!track_found)
            {
                TrackStats new_track;
                new_track.ident = ident;
                xml_achievement_tracks[i]->get("sta",&new_track.track_data[(int)TR_STARTED]);
                xml_achievement_tracks[i]->get("fin",&new_track.track_data[(int)TR_FINISHED]);
                xml_achievement_tracks[i]->get("won",&new_track.track_data[(int)TR_WON]);
                xml_achievement_tracks[i]->get("fin_rev",&new_track.track_data[(int)TR_FINISHED_REVERSE]);
                xml_achievement_tracks[i]->get("fin_al",&new_track.track_data[(int)TR_FINISHED_ALONE]);
                xml_achievement_tracks[i]->get("l_laps",&new_track.track_data[(int)TR_LESS_LAPS]);
                xml_achievement_tracks[i]->get("m_laps",&new_track.track_data[(int)TR_MORE_LAPS]);
                xml_achievement_tracks[i]->get("t_laps",&new_track.track_data[(int)TR_MIN_TWICE_LAPS]);
                xml_achievement_tracks[i]->get("eh_sta",&new_track.track_data[(int)TR_EGG_HUNT_STARTED]);
                xml_achievement_tracks[i]->get("eh_fin",&new_track.track_data[(int)TR_EGG_HUNT_FINISHED]);

                m_track_stats.push_back(new_track);
            }
        }
    }
    // If there is nothing valid to load ; we keep the init values

    // Now that we have retrieved the counters data, we can set
    // the progress of the achievements - it isn't saved directly.
    updateAllAchievementsProgress();
}   // load

void AchievementsStatus::updateAllAchievementsProgress()
{
    for (int i=0;i<ACHIEVE_DATA_NUM;i++)
    {
        updateAchievementsProgress(UP_ACHIEVEMENT_DATA, i);
    }
    for (int i=0;i<TR_DATA_NUM;i++)
    {
        updateAchievementsProgress(UP_TRACK_DATA, i);
    }
    updateAchievementsProgress(UP_KART_HITS, 0);
}

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
            i->second->saveProgress(out);
    }
    out << "          <data version=\"" << DATA_VERSION << "\"/>\n";
    for(int i=0;i<ACHIEVE_DATA_NUM;i++)
    {
        out << "          <var counter=\"" << m_variables[i].counter << "\"/>\n";
    }
    for (unsigned int n = 0; n < m_track_stats.size(); n++)
    {
        out << "          <track_stats ident=\"" << m_track_stats[n].ident << "\"";
        out << " sta=\"" << m_track_stats[n].track_data[(int)TR_STARTED] << "\"";
        out << " fin=\"" << m_track_stats[n].track_data[(int)TR_FINISHED] << "\"";
        out << " won=\"" << m_track_stats[n].track_data[(int)TR_WON] << "\"";
        out << " fin_rev=\"" << m_track_stats[n].track_data[(int)TR_FINISHED_REVERSE] << "\"";
        out << " fin_al=\"" << m_track_stats[n].track_data[(int)TR_FINISHED_ALONE] << "\"";
        out << " l_laps=\"" << m_track_stats[n].track_data[(int)TR_LESS_LAPS] << "\"";
        out << " m_laps=\"" << m_track_stats[n].track_data[(int)TR_MORE_LAPS] << "\"";
        out << " t_laps=\"" << m_track_stats[n].track_data[(int)TR_MIN_TWICE_LAPS] << "\"";
        out << " eh_sta=\"" << m_track_stats[n].track_data[(int)TR_EGG_HUNT_STARTED] << "\"";
        out << " eh_fin=\"" << m_track_stats[n].track_data[(int)TR_EGG_HUNT_FINISHED] << "\"";
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
        auto request = std::make_shared<Online::HTTPRequest>(/*priority*/2);
        PlayerManager::setUserDetails(request, "achieving");
        request->addParameter("achievementid", ids);
        request->queue();
    }
}   // sync

// ----------------------------------------------------------------------------
/* This function returns for how many tracks the requested goal type
 * is matched or exceeded. Addons tracks are ignored.
 * It returns -1 if the goal type is invalid.
 * \param value - the value to match or exceed
 * \param goal_string - the identifier of the value to check. */
int AchievementsStatus::getNumTracksAboveValue(int value, std::string goal_string)
{
    int counter = 0;
    int enum_id = -1;

    for (int i=0;i<2*(int)TR_DATA_NUM;i++)
    {
        if (m_tr_enum_to_xml[i] == goal_string)
            enum_id = i%(int)TR_DATA_NUM;
    }

    if (enum_id == -1)
    {
        Log::warn("AchievementsStatus",
            "Number of matching tracks requested for a non-existent "
            "data value.");
        return -1;
    }

    for (unsigned int i=0;i<m_track_stats.size();i++)
    {
        // ignore addons tracks (compare returns 0 when the values are equal)
        // Note: non-official tracks installed directly in the tracks folder
        // are considered as officials by this method.
        if (m_track_stats[i].ident.compare(0 /*start of sub-string*/,5/*length*/,"addon") == 0)
            continue;

        if (m_track_stats[i].track_data[enum_id] >= value)
            counter++;
    }
    return counter;
} // getNumTracksAboveValue

// ----------------------------------------------------------------------------
/* This function returns the number of tracks valid for by-track achievements. */
int AchievementsStatus::getNumAchieveTracks()
{
    int num_tracks = 0;
    for (unsigned int i=0;i<m_track_stats.size();i++)
    {
        // TODO : have a generic function to call instead
        // ignore addons tracks (compare returns 0 when the values are equal)
        if (m_track_stats[i].ident.compare(0 /*start of sub-string*/,5/*length*/,"addon") == 0)
            continue;

        num_tracks++;
    }
    return num_tracks;
} //getNumAchieveTracks

// ----------------------------------------------------------------------------
/* This function checks over achievements to update their goals
 * \param type - the data type triggering the update, used to know
 *               to what enum the enum_id refers to.
 * \param enum_id - the id of the enum identifying the change triggering
 *                  the update. */
void AchievementsStatus::updateAchievementsProgress(UpdateType type, unsigned int enum_id)
{
    std::string goal_string[2];
    int max_across_tracks = -1;
    int min_across_tracks = INT_MAX;
    int max_kart_hits = 0;
    if (type == UP_ACHIEVEMENT_DATA)
    {
        goal_string[0] = m_ach_enum_to_xml[(int)enum_id];
    } // if type == UP_ACHIEVEMENT_DATA)
    else if (type ==  UP_TRACK_DATA)
    {
        goal_string[0] = m_tr_enum_to_xml[(int)enum_id]; // The "one-track at least" goal
        goal_string[1] = m_tr_enum_to_xml[(int)enum_id+(int)TR_DATA_NUM]; // The "all tracks" goal

        for (unsigned int i=0;i<m_track_stats.size();i++)
        {
            // ignore addons tracks (compare returns 0 when the values are equal)
            // Note: non-official tracks installed directly in the tracks folder
            // are considered as officials by this method.
            if (m_track_stats[i].ident.compare(0 /*start of sub-string*/,5/*length*/,"addon") == 0)
                continue;

            if (m_track_stats[i].track_data[enum_id] > max_across_tracks)
                max_across_tracks = m_track_stats[i].track_data[enum_id];
            if (m_track_stats[i].track_data[enum_id] < min_across_tracks)
                min_across_tracks = m_track_stats[i].track_data[enum_id];
        }
    } // if type ==  UP_TRACK_DATA
    else if (type == UP_KART_HITS)
    {
        goal_string[0] = "hit-same-kart-1race";

        for (unsigned int i=0;i<m_kart_hits.size();i++)
        {
            if (m_kart_hits[i] > max_kart_hits)
                max_kart_hits = m_kart_hits[i];
        }
    } // if type == UP_KART_HITS

    // Now that we know what string to look for, call an Achievement function
    // which will look throughout the progress goalTree to update it
    std::map<uint32_t, Achievement*>::const_iterator i;
    for(i=m_achievements.begin(); i!=m_achievements.end(); i++)
    {
        // Don't bother checking again already completed achievements
        if (i->second->isAchieved())
            continue;

        if (type == UP_ACHIEVEMENT_DATA)
        {
            i->second->setGoalValue(goal_string[0],m_variables[enum_id].counter);
        }
        else if (type ==  UP_TRACK_DATA)
        {
            i->second->setGoalValue(goal_string[0],max_across_tracks);
            i->second->setGoalValue(goal_string[1],min_across_tracks);
        }
        else if (type == UP_KART_HITS)
        {
            i->second->setGoalValue(goal_string[0],max_kart_hits);
        }
    }
}

// ----------------------------------------------------------------------------
void AchievementsStatus::increaseDataVar(unsigned int achieve_data_id, int increase)
{
    if (achieve_data_id<ACHIEVE_DATA_NUM)
    {
        m_variables[achieve_data_id].counter += increase;

        updateAchievementsProgress(UP_ACHIEVEMENT_DATA, achieve_data_id);
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
    onLapEnd();

    if (m_variables[POWERUP_USED_1RACE].counter > m_variables[POWERUP_USED_1RACE_MAX].counter)
        m_variables[POWERUP_USED_1RACE_MAX].counter = m_variables[POWERUP_USED_1RACE].counter;

    if (m_variables[BANANA_1RACE].counter > m_variables[BANANA_1RACE_MAX].counter)
        m_variables[BANANA_1RACE_MAX].counter = m_variables[BANANA_1RACE].counter;

    if (m_variables[SKIDDING_1RACE].counter > m_variables[SKIDDING_1RACE_MAX].counter)
        m_variables[SKIDDING_1RACE_MAX].counter = m_variables[SKIDDING_1RACE].counter;

    if (m_variables[BOWLING_HIT_1RACE].counter > m_variables[BOWLING_HIT_1RACE_MAX].counter)
        m_variables[BOWLING_HIT_1RACE_MAX].counter = m_variables[BOWLING_HIT_1RACE].counter;

    if (m_variables[SWATTER_HIT_1RACE].counter > m_variables[SWATTER_HIT_1RACE_MAX].counter)
        m_variables[SWATTER_HIT_1RACE_MAX].counter = m_variables[SWATTER_HIT_1RACE].counter;

    if (m_variables[ALL_HITS_1RACE].counter > m_variables[ALL_HITS_1RACE_MAX].counter)
        m_variables[ALL_HITS_1RACE_MAX].counter = m_variables[ALL_HITS_1RACE].counter;

    m_variables[POWERUP_USED_1RACE].counter = 0;
    m_variables[BANANA_1RACE].counter = 0;
    m_variables[SKIDDING_1RACE].counter = 0;
    m_variables[BOWLING_HIT_1RACE].counter = 0;
    m_variables[SWATTER_HIT_1RACE].counter = 0;
    m_variables[ALL_HITS_1RACE].counter = 0;

    if (m_variables[CONS_WON_RACES].counter > m_variables[CONS_WON_RACES_MAX].counter)
        m_variables[CONS_WON_RACES_MAX].counter = m_variables[CONS_WON_RACES].counter;

    if (m_variables[CONS_WON_RACES_HARD].counter > m_variables[CONS_WON_RACES_HARD_MAX].counter)
        m_variables[CONS_WON_RACES_HARD_MAX].counter = m_variables[CONS_WON_RACES_HARD].counter;

    // Prevent restart from being abused to get consecutive wins achievement
    if (aborted)
    {
        m_variables[CONS_WON_RACES].counter = 0;
        m_variables[CONS_WON_RACES_HARD].counter = 0;
    }

    updateAllAchievementsProgress();
}   // onRaceEnd

// ----------------------------------------------------------------------------
void AchievementsStatus::onLapEnd()
{
    if (m_variables[SKIDDING_1LAP].counter > m_variables[SKIDDING_1LAP_MAX].counter)
        m_variables[SKIDDING_1LAP_MAX].counter = m_variables[SKIDDING_1LAP].counter;

    m_variables[SKIDDING_1LAP].counter = 0;
    updateAchievementsProgress(UP_ACHIEVEMENT_DATA, (int)SKIDDING_1LAP);
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

    // This can happen for e.g. cutscenes.
    // In this case, don't try to update the track data
    // TODO : Update the track list upon addon installation
    if (track_id == -1)
        return;

    m_track_stats[track_id].track_data[(int)event]++;
    updateAchievementsProgress(UP_TRACK_DATA, (int)event);
} // trackEvent

// ----------------------------------------------------------------------------
void AchievementsStatus::resetKartHits(int num_karts)
{
    m_kart_hits.clear();
    m_kart_hits.resize(num_karts);
} // resetKartHits

// ----------------------------------------------------------------------------
void AchievementsStatus::addKartHit(int kart_id)
{
    m_kart_hits[kart_id]++;
    updateAchievementsProgress(UP_KART_HITS, 0);
} // addKartHit
