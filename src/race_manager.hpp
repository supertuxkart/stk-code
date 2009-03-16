//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
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

#ifndef HEADER_RACEMANAGER_H
#define HEADER_RACEMANAGER_H

#include <vector>
#include <algorithm>
#include <string>

#include "grand_prix_data.hpp"
#include "network/remote_kart_info.hpp"

class World;
class Track;
class Kart;

/** The race manager has two functions:
 *  1) it stores information about the race the user selected (e.g. number
 *     of karts, track, race mode etc.). Most of the values are just stored
 *     from the menus, and just read back, except for GP mode (the race
 *     manager stores the GP information, but World queries only track
 *     and number of laps, so in case of GP this information is taken from
 *     the GrandPrix object), and local player information (number of local
 *     players, and selected karts). The local player information is read
 *     from the NetworkManager, gathered on the server (from all clients and
 *     the server, see NetworkManager::setupPlayerKartInfo), and then the 
 *     combined information distributed to all RaceManagers in all clients 
 *     and server. Even in no networking mode, the data flow is the same:
 *     information about local players is stored here, then processed by
 *     NetworkManager::setupPlayerKartInfo and the 'global' information about
 *     player karts is set in the RaceManager, to be used by World later on.
 *  2) when a race is started, it creates the world, and keeps track of
 *     score during the race. When a race is finished, it deletes the world,
 *     and (depending on race mode) starts the next race by creating a new
 *     world.
 *  Information in the RaceManager is considered to be 'more static', sometimes
 *  the world has similar functions showing the current state. E.g.: the
 *  race manager keeps track of the number of karts with which the race was
 *  started, while world keeps track of the number of karts currently in the
 *  race (consider a race mode like follow the leader where karts can get
 *  eliminated, but still the RaceManager has to accumulate points for those
 *  karts).
 *  The race manager handles all race types as a kind of grand prix. E.g.:
 *  a quick race is basically a GP with only one track (so the race manager
 *  keeps track of scores even though no scores are used in a quick race).
 */
class RaceManager
{
public:
    /** The major types or races supported in STK
    */
    enum MajorRaceModeType
    {
        MAJOR_MODE_GRAND_PRIX,
        MAJOR_MODE_SINGLE
    };
    
    // quick method to tell the difference between battle modes and race modes
    // think of it like a bitmask, but done in decimal to avoid endianness issues
#define LINEAR_RACE(ID, COUNT_LAPSES) (1000+ID+100*COUNT_LAPSES)
#define BATTLE_ARENA(ID) (2000+ID)
    /** Minor variants to the major types of race.
      * Make sure to use the 'LINEAR_RACE/BATTLE_ARENA' macros
      */
    enum MinorRaceModeType
    {
        MINOR_MODE_QUICK_RACE    = LINEAR_RACE(0, true),
        MINOR_MODE_TIME_TRIAL    = LINEAR_RACE(1, true),
        MINOR_MODE_FOLLOW_LEADER = LINEAR_RACE(2, false),
        
        MINOR_MODE_3_STRIKES     = BATTLE_ARENA(0)
    };
#undef LINEAR_RACE
#undef BATTLE_ARENA
    
    /** Difficulty. */
    enum Difficulty     { RD_EASY, RD_MEDIUM, RD_HARD };

    /** Different kart types: A local player, a player connected via network,
     *  an AI kart, the leader kart (currently not used), a ghost kart 
     *  (currently not used). */
    enum KartType       { KT_PLAYER, KT_NETWORK_PLAYER, KT_AI, KT_LEADER, KT_GHOST };
private:
    struct KartStatus
    {
        std::string m_ident;            // The .tkkf filename without the .tkkf
        std::string m_player_name;      // for networked karts
        int         m_score;            // score for this kart
        int         m_last_score;       // needed for restart race, and for race results GUI.
        double      m_overall_time;     // sum of times of all races
        double      m_last_time;        // needed for restart
        int         m_prev_finish_pos;  // previous finished position
        KartType    m_kart_type;        // Kart type: AI, player, network player etc.
        int         m_local_player_id;  // player controling the kart, for AI: -1
        int         m_global_player_id; // global ID of player
        int         m_gp_final_rank;    // In GPs, at the end, will hold the overall rank of this kart.
        
        KartStatus(const std::string& ident, const int& prev_finish_pos, 
                   int local_player_id, int global_player_id, KartType kt) :
                   m_ident(ident), m_score(0), m_last_score(0), 
                   m_overall_time(0.0f), m_last_time(0.0f),
                   m_prev_finish_pos(prev_finish_pos), m_kart_type(kt),
                   m_local_player_id(local_player_id),
                   m_global_player_id(global_player_id)
                {}
        
    };   // KartStatus

    std::vector<KartStatus>          m_kart_status;
    Difficulty                       m_difficulty;
    MajorRaceModeType                m_major_mode;
    MinorRaceModeType                m_minor_mode;
    typedef std::vector<std::string> PlayerKarts;
    /** Stores remote kart information about all player karts. */
    std::vector<RemoteKartInfo>      m_player_karts;
    std::vector<RemoteKartInfo>      m_local_kart_info;
    std::vector<std::string>         m_tracks;
    std::vector<int>                 m_host_ids;
    std::vector<int>                 m_num_laps;
    std::vector<int>                 m_score_for_position;
    std::vector<std::string>         m_random_kart_list;
    int                              m_track_number;
    GrandPrixData                    m_grand_prix;
    int                              m_num_karts;
    unsigned int                     m_num_finished_karts;
    unsigned int                     m_num_finished_players;
    int                              m_coin_target;
    
    void startNextRace();    // start a next race

    friend bool operator< (const KartStatus& left, const KartStatus& right)
    {
        return (left.m_score < right.m_score);
    }

public:
    bool   m_active_race; //True if there is a race

    static World* getWorld();
    static void setWorld(World* world);
    static Track* getTrack();
    static Kart* getPlayerKart(const unsigned int n);
    static Kart* getKart(const unsigned int n);
    
public:
                 RaceManager();
                ~RaceManager();
    void         reset();
    void         setLocalKartInfo(unsigned int player_id, const std::string& kart);
    const RemoteKartInfo& 
                 getLocalKartInfo(unsigned int n) const {return m_local_kart_info[n];}
    void         setNumLocalPlayers(unsigned int n);
    unsigned int getNumLocalPlayers() const  {return m_local_kart_info.size(); };

    void         setNumPlayers(int num);
    void         setPlayerKart(unsigned int player_id, const RemoteKartInfo& ki);
    void         RaceFinished(const Kart* kart, float time);
    void         setTrack(const std::string& track);
    void         setGrandPrix(const GrandPrixData &gp){ m_grand_prix = gp;          }
    void         setDifficulty(Difficulty diff);
    void         setNumLaps(int num)            { m_num_laps.clear();
                                                  m_num_laps.push_back(num);        }
    void         setMajorMode(MajorRaceModeType mode)
                                                { m_major_mode = mode;        }
    void         setMinorMode(MinorRaceModeType mode)
                                                { m_minor_mode = mode;       }
    void         setNumKarts(int num)           { m_num_karts = num;                }
    void         setCoinTarget(int num)         { m_coin_target = num;              }
    MajorRaceModeType
                 getMajorMode()           const { return m_major_mode;              }
    MinorRaceModeType
                 getMinorMode()           const { return m_minor_mode;              }
    unsigned int getNumKarts()            const { return m_num_karts;               }
    unsigned int getNumPlayers()          const { return m_player_karts.size();     }
    int          getNumLaps()             const { return m_num_laps[m_track_number];}
    Difficulty   getDifficulty()          const { return m_difficulty;              }
    const std::string& getTrackName()     const { return m_tracks[m_track_number];  }
    const GrandPrixData  *getGrandPrix()  const { return &m_grand_prix;             }
    unsigned int getFinishedKarts()       const { return m_num_finished_karts;      }
    unsigned int getFinishedPlayers()     const { return m_num_finished_players;    }
    const std::string& getItemStyle()     const { return m_grand_prix.getItemStyle(); }
    
    int          getKartFinalGPRank(const int kart_id)
                                          const { return m_kart_status[kart_id].m_gp_final_rank; }
    
    const std::string&  
                 getKartName(int kart)    const { return m_kart_status[kart].m_ident;}
    int          getKartScore(int krt)    const { return m_kart_status[krt].m_score;     }
    int          getKartPrevScore(int krt)const { return m_kart_status[krt].m_last_score;}
    int          getKartLocalPlayerId(int k)
                                          const { return m_kart_status[k].m_local_player_id; }
    int          getKartGlobalPlayerId(int k)
                                          const { return m_kart_status[k].m_global_player_id; }
    double       getOverallTime(int kart) const { return m_kart_status[kart].m_overall_time;}
    KartType     getKartType(int kart)    const { return m_kart_status[kart].m_kart_type;}
    int          getCoinTarget()          const { return m_coin_target;                  }
    int          getPositionScore(int p)  const { return m_score_for_position[p-1];      }
    int          allPlayerFinished()      const {return 
                                           m_num_finished_players==m_player_karts.size();}
    int          raceIsActive()           const { return m_active_race;                  }
    const std::vector<std::string>&
                 getRandomKartList()      const { return m_random_kart_list;             }
    void         setRandomKartList(const std::vector<std::string>& rkl)
                                                { m_random_kart_list = rkl;              }
    void         computeRandomKartList();

    void         setMirror() {/*FIXME*/}
    void         setReverse(){/*FIXME*/}
    void startNew();         // start new race/GP/...
    void next();             // start the next race or go back to the start screen
    void rerunRace();        // Rerun the same race again
    void exit_race();        // exit a race (and don't start the next one)
    
    /** get information about given mode (returns true if 'mode' is of linear races type)
        info is stored in its ID for conveniance, see the macros above for exact meaning
        */
    static bool isLinearRaceMode(const MinorRaceModeType type)
    {
        const int id = (int)type;
        if(id > 999 && id < 2000) return true;
        else return false;
    }
    /** get information about given mode (returns true if 'mode' is of battle type)
        info is stored in its ID for conveniance, see the macros above for exact meaning
        */
    static bool isBattleMode(const MinorRaceModeType type)
    {
        const int id = (int)type;
        if(id >= 2000) return true;
        else return false;
    }
    /** get information about given mode (returns true if 'mode' requires lap counting)
        info is stored in its ID for conveniance, see the macros above for exact meaning
        */
    static bool modeHasLaps(const MinorRaceModeType type)
    {
        if(isBattleMode(type)) return false;
        const int id = (int)type;
        const int answer = (id-1000)/100;
        return answer!=0;
    }
};

extern RaceManager *race_manager;
#endif

/* EOF */
