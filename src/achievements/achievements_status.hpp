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
public :
    // Warning : changing what an existing id does breaks
    // save-game compatibility. Bump version number if doing so.
    enum { // Won races values share the following properties :
           // 1. Only races with at least 3 AI count unless otherwise specified.
           ACHIEVE_WON_RACES             = 0, // Normal, time-trial and FTL
           ACHIEVE_WON_NORMAL_RACES      = 1, // Normal race only
           ACHIEVE_WON_TT_RACES          = 2, // Time-trial race only
           ACHIEVE_WON_FTL_RACES         = 3, // Follow-the-leader race only
           // Consecutive won race values :
           // 1. Ignore races with not enough AIs for incrementation
           // 2. Reset the counter in case of loss against any number of AIs
           ACHIEVE_CONS_WON_RACES        = 4,
           // Won races in (at least) hard requires at least 5 AI opponents
           ACHIEVE_CONS_WON_RACES_HARD   = 5,
           // Count the number of powerups used by the player.
           ACHIEVE_POWERUP_USED          = 6,
           ACHIEVE_POWERUP_USED_1RACE    = 7,
           // Count the number of bananas hit
           ACHIEVE_BANANA                = 8,
           ACHIEVE_BANANA_1RACE          = 9,
           // Count how many times the player skidded
           ACHIEVE_SKIDDING_1LAP         = 10,
           ACHIEVE_SKIDDING_1RACE        = 11,
           ACHIEVE_SKIDDING              = 12,

           ACHIEVE_DATA_NUM              = 13
    };

private:
    std::map<uint32_t, Achievement *> m_achievements;

    // Variables used to track achievements progress,
    // one variable may be used by several achievements.
    // TODO
    // Currently this only uses an int counter.
    // Evaluate if additional data keeping (max achived ?) can be useful,
    // and either expand the struct or remove it.
    struct AchievementVariable
    {
        int counter;
    };

    const int DATA_VERSION = 1;

    // The tracked values are defined at compile time
    AchievementVariable m_variables[ACHIEVE_DATA_NUM];

    // To keep track of track-specific data without hardcoding
    // a list of tracks, we use a special structure.
    struct TrackStats
    {
        std::string ident;
        // counters for standard, TT & FTL races
        int race_started; 
        int race_finished;
        int race_won;
        int race_finished_reverse;
        // TODO : Add data to keep track of egg hunts, battle, soccer
    };

    std::vector<TrackStats> m_track_stats;

    bool                m_online;
    bool                m_valid;

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
    void updateAchievementsProgress(unsigned int achieve_data_id);
    void increaseDataVar(unsigned int achieve_data_id, int increase);
    void resetDataVar(unsigned int achieve_data_id);
    void onRaceEnd(bool aborted=false);
    void onLapEnd();
    void raceStarted(std::string track_ident);
    void raceFinished(std::string track_ident);
    void raceWon(std::string track_ident);
    void raceFinishedReverse(std::string track_ident);
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
