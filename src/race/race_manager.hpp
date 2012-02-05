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

#ifndef HEADER_RACEMANAGER_HPP
#define HEADER_RACEMANAGER_HPP

/**
  * \defgroup race
  * Contains the race information that is conceptually above what you can find
  * in group Modes. Handles highscores, grands prix, number of karts, which
  * track was selected, etc.
  */

#include <vector>
#include <algorithm>
#include <string>

#include "network/remote_kart_info.hpp"
#include "race/grand_prix_data.hpp"
#include "utils/translation.hpp"

class Kart;
class Track;

static const std::string IDENT_STD      ("STANDARD"        );
static const std::string IDENT_TTRIAL   ("STD_TIMETRIAL"   );
static const std::string IDENT_FTL      ("FOLLOW_LEADER"   );
static const std::string IDENT_STRIKES  ("BATTLE_3_STRIKES");
static const std::string IDENT_OVERWORLD("OVERWORLD"   );

/** 
 * The race manager has two functions:
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
 *
 * \ingroup race
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
     *  Make sure to use the 'LINEAR_RACE/BATTLE_ARENA' macros. */
    enum MinorRaceModeType
    {
        MINOR_MODE_NONE          = -1,
        
        MINOR_MODE_NORMAL_RACE   = LINEAR_RACE(0, true),
        MINOR_MODE_TIME_TRIAL    = LINEAR_RACE(1, true),
        MINOR_MODE_FOLLOW_LEADER = LINEAR_RACE(2, false),
        MINOR_MODE_OVERWORLD     = LINEAR_RACE(3, false),
        
        MINOR_MODE_3_STRIKES     = BATTLE_ARENA(0)
    };

    /** Returns a string identifier for each minor race mode.
     *  \param mode Minor race mode.
     */
    static const std::string& getIdentOf(const MinorRaceModeType mode)
    {
        switch (mode)
        {
            case MINOR_MODE_NORMAL_RACE:    return IDENT_STD;
            case MINOR_MODE_TIME_TRIAL:     return IDENT_TTRIAL;
            case MINOR_MODE_FOLLOW_LEADER:  return IDENT_FTL;
            case MINOR_MODE_3_STRIKES:      return IDENT_STRIKES;
            default: assert(false); 
                     return IDENT_STD;  // stop compiler warning
        }
    }   // getIdentOf
    
    // ------------------------------------------------------------------------
    /** Returns the icon for a minor race mode. 
     *  \param mode Minor race mode. 
     */
    static const char* getIconOf(const MinorRaceModeType mode)
    {
        switch (mode)
        {
            case MINOR_MODE_NORMAL_RACE:    return "/gui/mode_normal.png";
            case MINOR_MODE_TIME_TRIAL:     return "/gui/mode_tt.png";
            case MINOR_MODE_FOLLOW_LEADER:  return "/gui/mode_ftl.png";
            case MINOR_MODE_3_STRIKES:      return "/gui/mode_3strikes.png";
            default: assert(false); return NULL;
        }
    }   // getIconOf

    // ------------------------------------------------------------------------
    /** Returns a (translated) name of a minor race mode.
     *  \param mode Minor race mode.
     */
    static const wchar_t* getNameOf(const MinorRaceModeType mode)
    {
        switch (mode)
        {
            //I18N: Game mode
            case MINOR_MODE_NORMAL_RACE:    return _("Normal Race");
            //I18N: Game mode
            case MINOR_MODE_TIME_TRIAL:     return _("Time Trial");
            //I18N: Game mode
            case MINOR_MODE_FOLLOW_LEADER:  return _("Follow the Leader");
            //I18N: Game mode
            case MINOR_MODE_3_STRIKES:      return _("3 Strikes Battle");
            default: assert(false); return NULL;
        }
    }
    
    static bool hasAI(const MinorRaceModeType mode)
    {
        switch (mode)
        {
            case MINOR_MODE_NORMAL_RACE:    return true;
            case MINOR_MODE_TIME_TRIAL:     return true;
            case MINOR_MODE_FOLLOW_LEADER:  return true;
            case MINOR_MODE_3_STRIKES:      return false;
            default: assert(false); return NULL;
        }
    }
                          
    
    // ------------------------------------------------------------------------
    /** Returns the minor mode id from a string identifier. This function is
     *  used from challenge_data, which reads the mode from a challenge file.
     *  \param name The name of the minor mode.
     */
    static const MinorRaceModeType getModeIDFromInternalName(const std::string &name)
    {
        if      (name==IDENT_STD    ) return MINOR_MODE_NORMAL_RACE;
        else if (name==IDENT_TTRIAL ) return MINOR_MODE_TIME_TRIAL;
        else if (name==IDENT_FTL    ) return MINOR_MODE_FOLLOW_LEADER;
        else if (name==IDENT_STRIKES) return MINOR_MODE_3_STRIKES;

        assert(0);
        return MINOR_MODE_NONE;
    }
    
#undef LINEAR_RACE
#undef BATTLE_ARENA
    
    /** Game difficulty. */
    enum Difficulty     { RD_EASY, RD_MEDIUM, RD_HARD };

    static const int DIFFICULTY_COUNT = 3;
    
    /** Different kart types: A local player, a player connected via network,
     *  an AI kart, the leader kart (currently not used), a ghost kart 
     *  (currently not used). */
    enum KartType       { KT_PLAYER, KT_NETWORK_PLAYER, KT_AI, KT_LEADER, KT_GHOST };
private:

    /** This data structure accumulates kart data and race result data from
     *  each race. */
    struct KartStatus
    {
        std::string m_ident;            // The .tkkf filename without the .tkkf
        std::string m_player_name;      // for networked karts
        int         m_score;            // score for this kart
        int         m_last_score;       // needed for restart race, and for race results GUI.
        float       m_overall_time;     // sum of times of all races
        float       m_last_time;        // needed for restart
        int         m_prev_finish_pos;  // previous finished position
        KartType    m_kart_type;        // Kart type: AI, player, network player etc.
        int         m_local_player_id;  // player controling the kart, for AI: -1
        int         m_global_player_id; // global ID of player
        int         m_gp_rank;    // In GPs, at the end, will hold the overall
                                  // rank of this kart (0<=m_gp_rank < num_karts-1)
        
        KartStatus(const std::string& ident, const int& prev_finish_pos, 
                   int local_player_id, int global_player_id, 
                   int init_gp_rank, KartType kt) :
                   m_ident(ident), m_score(0), m_last_score(0), 
                   m_overall_time(0.0f), m_last_time(0.0f),
                   m_prev_finish_pos(prev_finish_pos), m_kart_type(kt),
                   m_local_player_id(local_player_id),
                   m_global_player_id(global_player_id),
                   m_gp_rank(init_gp_rank)
                {}
        
    };   // KartStatus

    /** The kart status data for each kart. */
    std::vector<KartStatus>          m_kart_status;

    /** The selected difficulty. */
    Difficulty                       m_difficulty;

    /** The major mode (single race, GP). */
    MajorRaceModeType                m_major_mode;

    /** The minor mode (race, time trial, ftl, battle mode). */
    MinorRaceModeType                m_minor_mode;
    /** Stores remote kart information about all player karts. */
    std::vector<RemoteKartInfo>      m_player_karts;
    std::vector<RemoteKartInfo>      m_local_player_karts;
    std::vector<std::string>         m_tracks;
    std::vector<int>                 m_host_ids;

    /** The number of laps for each track of a GP (only one element
     *  is used if only a single track is used. */
    std::vector<int>                 m_num_laps;
    
    /** Whether a track should be reversed */
    std::vector<bool>                m_reverse_track;

    /** The points given to a kart on a given position (index is
     *  0 based, so using race-position - 1. */
    std::vector<int>                 m_score_for_position;

    /** The list of default AI karts to use. This is from the command line. */
    std::vector<std::string>         m_default_ai_list;

    /** The list of AI karts to use. This is stored here so that the
     *  same list of AIs is used for all tracks of a GP. */
    std::vector<std::string>         m_ai_kart_list;
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
                 RaceManager();
                ~RaceManager();
    
    /** Resets the race manager. It is called by world when restarting a race. */
    void         reset();


    /**
      * \{
      * \name Setting race parameters
      */
    
    /** \brief Stores the information which local players uses which karts.
      * \param player_id  Id of the local player for which the kart is set.
      * \param kart       Kart name this player is using.
      */
    void         setLocalKartInfo(unsigned int player_id, const std::string& kart);
    
    /** Sets the number of local players playing on this computer (including
      * split screen).
      * \param n Number of local players.
      */
    void         setNumLocalPlayers(unsigned int n);
    
    /** In case of non GP mode set the track to use.
     *  \param track Pointer to the track to use.
     */
    void         setTrack(const std::string& track);
    
    void         setGrandPrix(const GrandPrixData &gp){ m_grand_prix = gp; m_coin_target = 0; }
    
    /** \brief Sets the difficulty.
      * \param diff Difficulty.
      */
    void         setDifficulty(Difficulty diff);
    
    void         setNumLaps(int num)            { m_num_laps.clear();
                                                  m_num_laps.push_back(num);        }
    
    void         setReverseTrack(bool r_t)      { m_reverse_track.clear();
                                                  m_reverse_track.push_back(r_t);   }
                                                  
    void         setMajorMode(MajorRaceModeType mode)
                                                { m_major_mode = mode;              }
    void         setMinorMode(MinorRaceModeType mode)
                                                { m_minor_mode = mode;              }
    void         setNumKarts(int num)           { m_num_karts = num;                }
    void         setCoinTarget(int num)         { m_coin_target = num;              }
    
    /** \} */
    
    /** Sort karts and update the m_gp_rank KartStatus member, in preparation
      * for future calls to RaceManager::getKartGPRank or RaceManager::getKartWithGPRank
      */
    void         computeGPRanks();
    
    /** \{
      * \name Getters
      * Get current race manager state and settings
      */
    const RemoteKartInfo& 
         getLocalKartInfo(unsigned int n) const { return m_local_player_karts[n];     }
    unsigned int getNumLocalPlayers()     const { return m_local_player_karts.size(); }
    
    /** Returns the selected number of karts (selected number of players and 
     *  AI karts. */
    unsigned int getNumberOfKarts()       const {return m_num_karts; }
    
    MajorRaceModeType
                 getMajorMode()           const { return m_major_mode;              }
    MinorRaceModeType
                 getMinorMode()           const { return m_minor_mode;              }
    unsigned int getNumPlayers()          const { return m_player_karts.size();     }
    
    /** \brief Returns the number lf laps.
      * In case of FTL or battle mode always return 9999, since they don't have laps.
      * This avoids problems in FTL GP, since in this case no laps would be set
      * (otherwise we would need many more tests in calls to getNumLaps).
      */
    int          getNumLaps()             const 
    {
        if(m_minor_mode==MINOR_MODE_3_STRIKES || m_minor_mode==MINOR_MODE_FOLLOW_LEADER)
            return 9999;
        return m_num_laps[m_track_number];
    }   // getNumLaps
    // ------------------------------------------------------------------------    
    /** \return whether the track should be reversed */
    bool getReverseTrack() const { return m_reverse_track[m_track_number]; }
    
    Difficulty   getDifficulty()          const { return m_difficulty;              }
    const std::string& getTrackName()     const { return m_tracks[m_track_number];  }
    const GrandPrixData  *getGrandPrix()  const { return &m_grand_prix;             }
    unsigned int getFinishedKarts()       const { return m_num_finished_karts;      }
    unsigned int getFinishedPlayers()     const { return m_num_finished_players;    }
    
    int          getKartGPRank(const int kart_id)
                                          const { return m_kart_status[kart_id].m_gp_rank; }
    
    /** \brief Returns the kart with a given GP rank (or NULL if no such kart exists).
     *  \param n Rank (0<=n<num_karts) to look for.
     */
    const Kart*  getKartWithGPRank(unsigned int n);
    
    /** \return the GP rank of a local player, or -1 if the given player ID doesn't exist */
    int          getLocalPlayerGPRank(const int playerID) const;
    
    const std::string&  
                 getKartIdent(int kart)   const { return m_kart_status[kart].m_ident;}
    int          getKartScore(int krt)    const { return m_kart_status[krt].m_score;     }
    int          getKartPrevScore(int krt)const { return m_kart_status[krt].m_last_score;}
    int          getKartLocalPlayerId(int k)
                                          const { return m_kart_status[k].m_local_player_id; }
    int          getKartGlobalPlayerId(int k)
                                          const { return m_kart_status[k].m_global_player_id; }
    float        getOverallTime(int kart) const { return m_kart_status[kart].m_overall_time;}
    KartType     getKartType(int kart)    const { return m_kart_status[kart].m_kart_type;}
    int          getCoinTarget()          const { return m_coin_target;                  }
    int          getPositionScore(int p)  const { return m_score_for_position[p-1];      }
    int          getTrackNumber()         const { return m_track_number;                 } 

    /** Returns the list of AI karts to use. Used for networking, and for
     *  the --ai= command line option. */
    const std::vector<std::string>&
                 getAIKartList()          const { return m_ai_kart_list;                  }
    // ------------------------------------------------------------------------
    /** \brief get information about given mode (returns true if 'mode' is of
     *  linear races type) */
    bool isLinearRaceMode()
    {
        const int id = (int)m_minor_mode;
        // info is stored in its ID for conveniance, see the macros LINEAR_RACE
        // and BATTLE_ARENA above for exact meaning.
        if(id > 999 && id < 2000) return true;
        else return false;
    }
    
    // ------------------------------------------------------------------------
    /** \brief Returns true if the current mode is a battle mode. */
    bool isBattleMode()
    {
        const int id = (int)m_minor_mode;
        // This uses the  numerical id of the mode, see the macros LINEAR_RACE and
        // BATTLE_ARENA above for exact meaning.
        if (id >= 2000) return true;
        else            return false;
    }
    
    // ------------------------------------------------------------------------
    /** \brief Returns true if the current mode has laps. */
    bool modeHasLaps()
    {
        if (isBattleMode()) return false;
        const int id = (int)m_minor_mode;
        // See meaning of IDs above
        const int answer = (id-1000)/100;
        return answer!=0;
    }
    // ------------------------------------------------------------------------
    /** Returns true if the currently selected minor mode has highscores. */
    bool modeHasHighscores()
    {
        //FIXME: this information is duplicated. RaceManager knows about it, and
        //       each World may set m_use_highscores to true or false. The reason
        //       for this duplication is that we might want to know whether to
        //       display highscores without creating a World.
        return m_minor_mode != MINOR_MODE_3_STRIKES && 
               m_minor_mode != MINOR_MODE_FOLLOW_LEADER;
    }

    /** \} */
    
    /**
      * \{
      * \name Controlling race
      * Start, stop, continue, restart races
      */
    
    /**
     *  \brief Starts a new race or GP (or other mode).
     *  It sets up the list of player karts, AI karts, GP tracks if relevant etc.
     *  \pre The list of AI karts to use must be set up first. This is usually being
     *       done by a call to computeRandomKartList() from 
     *       NetworkManager::setupPlayerKartInfo, but could be done differently 
     *       (e.g. depending on user command line options to test certain AIs)
     */
    void         startNew();
    
    /** \brief Start the next race or go back to the start screen
      * If there are more races to do, starts the next race, otherwise
      * calls exitRace to finish the race.
      */
    void         next();
    
    /** \brief Rerun the same race again
      * This is called after a race is finished, and it will adjust
      * the number of points and the overall time before restarting the race.
      */
    void         rerunRace();
    
    /** \brief Exit a race (and don't start the next one)
      * \note In GP, displays the GP result screen first
      * \note Deletes the world.
      */
    void         exitRace();
    
    /**
      * \brief Higher-level method to start a GP without having to care about the exact startup sequence
      */
    void         startGP(const GrandPrixData* gp);

    /**
      * \brief Higher-level method to start a GP without having to care about 
      *  the exact startup sequence.
      * \param trackIdent Internal name of the track to race on
      * \param num_laps   Number of laps to race, or -1 if number of laps is 
      *        not relevant in current mode
      */
    void         startSingleRace(const std::string trackIdent, const int num_laps);

    /** \} */
    
    /**
      * \{
      * \name Callbacks from the race classes
      * These methods are to be used by the classes that manage the various races, to let
      * the race manager know about current status
      */
    bool         allPlayerFinished()      const {return m_num_finished_players==m_player_karts.size();}
    void         kartFinishedRace(const Kart* kart, float time);

    /** \} */
    
    /**
     * \{
     * \name For internal use
     * Functions for internal use by RaceManager or its close friends;
     * You shouldn't need to call any of those from higher-level code.
     */
    void         setNumPlayers(int num);
    
    void         setPlayerKart(unsigned int player_id, 
                               const RemoteKartInfo& ki);
    void         setDefaultAIKartList(const std::vector<std::string> &ai_list);
    void         computeRandomKartList();
    /** Sets the AI to use. This is used in networking mode to set the karts
     *  that will be used by the server to the client. It will take precedence
     *  over the random selection. */
    void         setAIKartList(const std::vector<std::string>& rkl)
                    { m_ai_kart_list = rkl; }

    /** \} */
    
};

extern RaceManager *race_manager;
#endif

/* EOF */
