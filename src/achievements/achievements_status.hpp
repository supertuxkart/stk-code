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

#undef SYNC_ACHIEVEMENTS

#include "utils/types.hpp"

#include <irrString.h>
#include <map>
#include <string>
#include <vector>

class Achievement;
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
    enum AchievementData {
           // Won races values share the following properties :
           // 1. Only races with at least 3 AI count unless otherwise specified.
           WON_RACES                     = 0, // Normal, time-trial and FTL
           WON_NORMAL_RACES              = 1, // Normal race only
           WON_TT_RACES                  = 2, // Time-trial race only
           WON_FTL_RACES                 = 3, // Follow-the-leader race only
           // Consecutive won race values :
           // 1. Ignore races with not enough AIs for incrementation
           // 2. Reset the counter in case of loss against any number of AIs
           CONS_WON_RACES                = 4,
           CONS_WON_RACES_MAX            = 5,
           // Won races in (at least) hard requires at least 5 AI opponents
           CONS_WON_RACES_HARD           = 6,
           CONS_WON_RACES_HARD_MAX       = 7,
           // Count how many normal, TT & FTL races were started and finished by difficulty
           EASY_STARTED                  = 8,
           EASY_FINISHED                 = 9,
           CASUAL_STARTED                = 10,
           CASUAL_FINISHED               = 11,
           MEDIUM_STARTED                = 12,
           MEDIUM_FINISHED               = 13,
           HARD_STARTED                  = 14,
           HARD_FINISHED                 = 15,
           BEST_STARTED                  = 16,
           BEST_FINISHED                 = 17,
           // Count how many time a race/match was started and finished by game mode.
           // Races with ghost replays technically belong to TT or egg hunt race mode,
           // they increment both the with_ghost counter and the relevant mode counter.
           NORMAL_STARTED                = 18,
           NORMAL_FINISHED               = 19,
           TT_STARTED                    = 20,
           TT_FINISHED                   = 21,
           FTL_STARTED                   = 22,
           FTL_FINISHED                  = 23,
           THREE_STRIKES_STARTED         = 24,
           THREE_STRIKES_FINISHED        = 25,
           SOCCER_STARTED                = 26,
           SOCCER_FINISHED               = 27,
           EGG_HUNT_STARTED              = 28,
           EGG_HUNT_FINISHED             = 29,
           EGG_HUNT_STARTED_HARD         = 30,
           EGG_HUNT_FINISHED_HARD        = 31,
           WITH_GHOST_STARTED            = 32,
           WITH_GHOST_FINISHED           = 33,
           CTF_STARTED                   = 34,
           CTF_FINISHED                  = 35,
           FFA_STARTED                   = 36,
           FFA_FINISHED                  = 37,

           // Count the number of powerups used by the player.
           POWERUP_USED                  = 38,
           POWERUP_USED_1RACE            = 39,
           POWERUP_USED_1RACE_MAX        = 40,
           // Count how many times a bowling ball from the player hit a kart
           BOWLING_HIT                   = 41,
           BOWLING_HIT_1RACE             = 42,
           BOWLING_HIT_1RACE_MAX         = 43,
           // Count how many times a swatter from the player hit a kart
           SWATTER_HIT                   = 44,
           SWATTER_HIT_1RACE             = 45,
           SWATTER_HIT_1RACE_MAX         = 46,
           // Count how many times a swatter, bowling ball or cake from
           // the player hit a kart (excluding the player's own kart)
           ALL_HITS                      = 47,
           ALL_HITS_1RACE                = 48,
           ALL_HITS_1RACE_MAX            = 49,
           // Count the number of bananas hit
           BANANA                        = 50,
           BANANA_1RACE                  = 51,
           BANANA_1RACE_MAX              = 52,
           // Count how many times the player skidded
           SKIDDING                      = 53,
           SKIDDING_1RACE                = 54,
           SKIDDING_1RACE_MAX            = 55,
           SKIDDING_1LAP                 = 56,
           SKIDDING_1LAP_MAX             = 57,


           ACHIEVE_DATA_NUM              = 58
    };

private:
    std::map<uint32_t, Achievement *> m_achievements;

    // Variables used to track achievements progress,
    // one variable may be used by several achievements.
    // TODO
    // Currently this only uses an int counter.
    // Evaluate if additional data keeping (max achieved ?) can be useful,
    // and either expand the struct or remove it.
    struct AchievementVariable
    {
        int counter;
    };

    const int DATA_VERSION = 4;

    // The tracked values are defined at compile time
    AchievementVariable m_variables[ACHIEVE_DATA_NUM];

    // We store the enum name and matching goalTree type
    // in this table for faster lookup.
    std::string m_ach_enum_to_xml[ACHIEVE_DATA_NUM];

// Switching a few times from public to private
// helps here to keep related things next to each other
public:
    enum TrackData {
        // counters for standard, TT & FTL races
        TR_STARTED           = 0,
        TR_FINISHED          = 1,
        // doesn't count race without any other AI/player
        TR_WON               = 2,
        TR_FINISHED_REVERSE  = 3,
        // races against replays are counted, too
        TR_FINISHED_ALONE    = 4,
        // counters for standard & TT races, apply to finished races only,
        // lap number compared to track default.
        TR_LESS_LAPS         = 5,
        TR_MORE_LAPS         = 6,
        TR_MIN_TWICE_LAPS    = 7, // at least twice the track's default lap count
        // counters for egg hunts
        TR_EGG_HUNT_FIRST    = 8,
        TR_EGG_HUNT_STARTED  = TR_EGG_HUNT_FIRST,
        TR_EGG_HUNT_FINISHED = 9,
        TR_EGG_HUNT_STARTED_HARD  = 10,
        TR_EGG_HUNT_FINISHED_HARD = 11,
        TR_EGG_HUNT_LAST = TR_EGG_HUNT_FINISHED_HARD,

        TR_DATA_NUM          = 12,
    };    

private:
    // To keep track of track-specific data without hardcoding
    // a list of tracks, we use a special structure.
    struct TrackStats
    {
        std::string ident;
        int track_data[TR_DATA_NUM];
    };

    std::vector<TrackStats> m_track_stats;

    // We store the enum name and matching goalTree type
    // in this table for faster lookup.
    // Each track data value matches 2 xml command
    std::string m_tr_enum_to_xml[2*TR_DATA_NUM];

    // TODO : keep track of battle/soccer arenas

    // Keeps track of hits inflicted to other karts,
    // identified by their world id.
    // Reset at the beginning of a race
    std::vector<int> m_kart_hits;

    // To avoid updating achievement progress being
    // too computationally wasteful, we restrain
    // what is checked on an update
    enum UpdateType
    {
        UP_ACHIEVEMENT_DATA = 0,
        UP_TRACK_DATA = 1,
        UP_KART_HITS = 2,
    };

    bool                m_online;
    bool                m_valid;

    void setEnumToString();
    void updateAchievementsProgress(UpdateType type, unsigned int enum_id);

public :
    AchievementsStatus();
    ~AchievementsStatus();
    Achievement * getAchievement(uint32_t id);
    void load(const XMLNode * input);
    void save(UTFWriter &out);
    void add(Achievement *achievement);
    void sync(const std::vector<uint32_t> & achieved_ids);
    void increaseDataVar(unsigned int achieve_data_id, int increase);
    void resetDataVar(unsigned int achieve_data_id);
    void onRaceEnd(bool aborted=false);
    void onLapEnd();
    void trackEvent(std::string track_ident, AchievementsStatus::TrackData event);
    void resetKartHits(int num_karts);
    void addKartHit(int kart_id);
    void updateAllAchievementsProgress();
    int getNumTracksAboveValue(int value, std::string goal_string, bool is_egg_hunt);
    int getNumAchieveTracks(bool is_egg_hunt);
    int getAllTrackStatus(std::string type);
    // ------------------------------------------------------------------------
    std::map<uint32_t, Achievement *>& getAllAchievements()
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
