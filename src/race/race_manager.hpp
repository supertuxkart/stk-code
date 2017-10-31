//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 SuperTuxKart-Team
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
#include "utils/vec3.hpp"

class AbstractKart;
class SavedGrandPrix;
class Track;

static const std::string IDENT_STD      ("STANDARD"        );
static const std::string IDENT_TTRIAL   ("STD_TIMETRIAL"   );
static const std::string IDENT_FTL      ("FOLLOW_LEADER"   );
static const std::string IDENT_STRIKES  ("BATTLE_3_STRIKES");
static const std::string IDENT_EASTER   ("EASTER_EGG_HUNT" );
static const std::string IDENT_SOCCER   ("SOCCER"          );
static const std::string IDENT_GHOST    ("GHOST"           );
static const std::string IDENT_OVERWORLD("OVERWORLD"       );
static const std::string IDENT_CUTSCENE ("CUTSCENE"        );

/**
 * The race manager has two functions:
 *  1) it stores information about the race the user selected (e.g. number
 *     of karts, track, race mode etc.). Most of the values are just stored
 *     from the menus, and just read back, except for GP mode (the race
 *     manager stores the GP information, but World queries only track
 *     and number of laps, so in case of GP this information is taken from
 *     the GrandPrix object), and local player information (number of local
 *     players, and selected karts). 
 *     Information about player karts (which player selected which kart,
 *     player ids) is stored in a RemoteKartInfo structure and used later
 *     to initialise the KartStatus array (startNew()). The KartStatus array
 *     stores information about all karts (player and AI), and is used to
 *     determine the order in which karts are started (see startNextRace()).
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
    // think of it like a bitmask, but done in decimal to avoid endianness
    // issues
#define LINEAR_RACE(ID, COUNT_LAPSES) (1000+ID+100*COUNT_LAPSES)
#define BATTLE_ARENA(ID) (2000+ID)
#define EASTER_EGG(ID)   (3000+ID)

    // ------------------------------------------------------------------------
    /** Minor variants to the major types of race.
     *  Make sure to use the 'LINEAR_RACE/BATTLE_ARENA' macros. */
    enum MinorRaceModeType
    {
        MINOR_MODE_NONE          = -1,

        MINOR_MODE_NORMAL_RACE   = LINEAR_RACE(0, true),
        MINOR_MODE_TIME_TRIAL    = LINEAR_RACE(1, true),
        MINOR_MODE_FOLLOW_LEADER = LINEAR_RACE(2, false),
        MINOR_MODE_OVERWORLD     = LINEAR_RACE(3, false),
        MINOR_MODE_TUTORIAL      = LINEAR_RACE(4, false),

        MINOR_MODE_3_STRIKES     = BATTLE_ARENA(0),
        MINOR_MODE_SOCCER        = BATTLE_ARENA(1),
        MINOR_MODE_CUTSCENE      = BATTLE_ARENA(2),
        MINOR_MODE_EASTER_EGG    = EASTER_EGG(0)
    };

    // ------------------------------------------------------------------------
    /** True if the AI should have additional abbilities, e.g.
     *  nolok will get special bubble gums in the final challenge. */
    enum AISuperPower
    {
        SUPERPOWER_NONE       = 0,
        SUPERPOWER_NOLOK_BOSS = 1
    };

    // ------------------------------------------------------------------------
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
            case MINOR_MODE_EASTER_EGG:     return IDENT_EASTER;
            case MINOR_MODE_SOCCER:         return IDENT_SOCCER;
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
            case MINOR_MODE_EASTER_EGG:     return "/gui/mode_easter.png";
            case MINOR_MODE_SOCCER:         return "/gui/mode_soccer.png";
            default: assert(false); return NULL;
        }
    }   // getIconOf

    // ------------------------------------------------------------------------
    /** Returns a (translated) name of a minor race mode.
     *  \param mode Minor race mode.
     */
    static const core::stringw getNameOf(const MinorRaceModeType mode)
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
            //I18N: Game mode
            case MINOR_MODE_EASTER_EGG:     return _("Egg Hunt");
            //I18N: Game mode
            case MINOR_MODE_SOCCER:         return _("Soccer");
            default: assert(false); return L"";
        }
    }   // getNameOf

    // ------------------------------------------------------------------------
    /** Returns if the currently set minor game mode can be used by the AI. */
    bool hasAI()
    {
        switch (m_minor_mode)
        {
            case MINOR_MODE_NORMAL_RACE:    return true;
            case MINOR_MODE_TIME_TRIAL:     return true;
            case MINOR_MODE_FOLLOW_LEADER:  return true;
            case MINOR_MODE_3_STRIKES:      return true;
            case MINOR_MODE_EASTER_EGG:     return false;
            case MINOR_MODE_SOCCER:         return true;
            default: assert(false);         return false;
        }
    }   // hasAI


    // ------------------------------------------------------------------------
    /** Returns the minor mode id from a string identifier. This function is
     *  used from challenge_data, which reads the mode from a challenge file.
     *  \param name The name of the minor mode.
     */
    static const MinorRaceModeType getModeIDFromInternalName(
                                                       const std::string &name)
    {
        if      (name==IDENT_STD    ) return MINOR_MODE_NORMAL_RACE;
        else if (name==IDENT_TTRIAL ) return MINOR_MODE_TIME_TRIAL;
        else if (name==IDENT_FTL    ) return MINOR_MODE_FOLLOW_LEADER;
        else if (name==IDENT_STRIKES) return MINOR_MODE_3_STRIKES;
        else if (name==IDENT_EASTER ) return MINOR_MODE_EASTER_EGG;
        else if (name==IDENT_SOCCER)  return MINOR_MODE_SOCCER;

        assert(0);
        return MINOR_MODE_NONE;
    }

#undef LINEAR_RACE
#undef BATTLE_ARENA

    /** Game difficulty. */
    enum Difficulty     { DIFFICULTY_EASY,
                          DIFFICULTY_FIRST = DIFFICULTY_EASY,
                          DIFFICULTY_MEDIUM,
                          DIFFICULTY_HARD,
                          DIFFICULTY_BEST,
                          DIFFICULTY_LAST = DIFFICULTY_BEST,
                          DIFFICULTY_COUNT};

    /** Different kart types: A local player, a player connected via network,
     *  an AI kart, the leader kart (currently not used), a ghost kart and
     *  spare tire karts which allow gain life in battle mode */
    enum KartType       { KT_PLAYER, KT_NETWORK_PLAYER, KT_AI, KT_LEADER,
                          KT_GHOST, KT_SPARE_TIRE };
private:

    bool m_started_from_overworld;

public:

    /** This data structure accumulates kart data and race result data from
     *  each race. */
    struct KartStatus
    {
        /** The kart identifier. */
        std::string m_ident;
        /** For networked karts. */
        std::string m_player_name;
        // Score for this kart. */
        int         m_score;
        /** Needed for restart race, and for race results GUI. */
        int         m_last_score;
        /** Sum of times of all races. */
        float       m_overall_time;
        /** Needed for restart. */
        float       m_last_time;
        /** Kart type: AI, player, network player etc. */
        KartType    m_kart_type;
        /** Player controling the kart, for AI: -1 */
        int         m_local_player_id;
        /** Global ID of player. */
        int         m_global_player_id;
        /** In GPs, at the end, will hold the overall rank of this kart
         *  (0<=m_gp_rank < num_karts-1). */
        int         m_gp_rank;
        /** The difficulty for this player. */
        PerPlayerDifficulty m_difficulty;

        KartStatus(const std::string& ident, const int& prev_finish_pos,
                   int local_player_id, int global_player_id,
                   int init_gp_rank, KartType kt,
                   PerPlayerDifficulty difficulty) :
                   m_ident(ident), m_score(0), m_last_score(0),
                   m_overall_time(0.0f), m_last_time(0.0f),
                   m_kart_type(kt),
                   m_local_player_id(local_player_id),
                   m_global_player_id(global_player_id),
                   m_gp_rank(init_gp_rank), m_difficulty(difficulty)
                {}

    };   // KartStatus
private:

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
    std::vector<std::string>         m_tracks;
    std::vector<int>                 m_host_ids;

    /** Number of local players. */
    unsigned int m_num_local_players;

    /** The number of laps for each track of a GP (only one element
     *  is used if only a single track is used. */
    std::vector<int>                 m_num_laps;

    /** Whether a track should be reversed */
    std::vector<bool>                m_reverse_track;

    /** The list of default AI karts to use. This is from the command line. */
    std::vector<std::string>         m_default_ai_list;

    /** If set, specifies which kart to use for AI(s) */
    std::string                      m_ai_kart_override;

    AISuperPower                     m_ai_superpower;

    /** The list of AI karts to use. This is stored here so that the
     *  same list of AIs is used for all tracks of a GP. */
    std::vector<std::string>         m_ai_kart_list;
    int                              m_track_number;
    GrandPrixData                    m_grand_prix;
    SavedGrandPrix*                  m_saved_gp;
    int                              m_num_karts;
    unsigned int                     m_num_spare_tire_karts;
    unsigned int                     m_num_finished_karts;
    unsigned int                     m_num_finished_players;
    int                              m_coin_target;
    float                            m_time_target;
    int                              m_goal_target;

    void startNextRace();    // start a next race

    friend bool operator< (const KartStatus& left, const KartStatus& right)
    {
        return (left.m_score < right.m_score) ||
            (left.m_score == right.m_score &&
             left.m_overall_time > right.m_overall_time);
    }

    bool m_have_kart_last_position_on_overworld;
    Vec3 m_kart_last_position_on_overworld;

    /** Determines if saved GP should be continued or not*/
    bool m_continue_saved_gp;

    bool m_is_recording_race;

    bool m_has_ghost_karts;

    bool m_watching_replay;
public:
         RaceManager();
        ~RaceManager();

    void reset();
    void setPlayerKart(unsigned int player_id, const std::string &kart_name);
    void setPlayerKart(unsigned int player_id,
                       const RemoteKartInfo& ki);

    /** Sets additional information for a player to indicate which soccer team it belong to
    */
    void setKartSoccerTeam(unsigned int player_id, SoccerTeam team);

    /** Sets the per-player difficulty for a player.
     */
    void setPlayerDifficulty(unsigned int player_id, PerPlayerDifficulty difficulty);

    /** In case of non GP mode set the track to use.
     *  \param track Pointer to the track to use.
     */
    void setTrack(const std::string& track);

    /** \brief Returns the kart with a given GP rank (or NULL if no such
     *  kart exists).
     *  \param n Rank (0<=n<num_karts) to look for.
     */
    const AbstractKart* getKartWithGPRank(unsigned int n);

    /** \return the GP rank of a local player, or -1 if the given player ID
     *  doesn't exist */
    int getLocalPlayerGPRank(const int playerID) const;


    /** Sort karts and update the m_gp_rank KartStatus member, in preparation
      * for future calls to RaceManager::getKartGPRank or
      *  RaceManager::getKartWithGPRank
      */
    void computeGPRanks();

    /** \brief Sets the difficulty.
      * \param diff Difficulty.
      */
    void setDifficulty(Difficulty diff);
    static Difficulty convertDifficulty(const std::string &difficulty);
    void startNew(bool from_overworld);
    void next();
    void rerunRace();
    void exitRace(bool delete_world=true);
    void startGP(const GrandPrixData &gp, bool from_overworld,
                 bool continue_saved_gp);
    void saveGP();
    void startSingleRace(const std::string &track_ident, const int num_laps,
                          bool from_overworld);
    void startWatchingReplay(const std::string &track_ident, const int num_laps);
    void setupPlayerKartInfo();
    void kartFinishedRace(const AbstractKart* kart, float time);
    void setNumPlayers(int players, int local_players=-1);
    void setDefaultAIKartList(const std::vector<std::string> &ai_list);
    void computeRandomKartList();

    // ------------------------------------------------------------------------
    bool hasTimeTarget() const { return m_time_target > 0.0f; }
    // ------------------------------------------------------------------------
    void setMaxGoal(int max_goal)
    {
        m_time_target = 0.0f;
        m_goal_target = max_goal;
    }   // setMaxGoal
    // ------------------------------------------------------------------------
    int getMaxGoal(){ return m_goal_target; }
    // ------------------------------------------------------------------------
    void setCoinTarget(int num)   { m_coin_target = num; }
    // ------------------------------------------------------------------------
    void setGrandPrix(const GrandPrixData &gp)
    {
        m_grand_prix = gp;
        setCoinTarget(0);
    }   // setGrandPrix
    // ------------------------------------------------------------------------
    void setAIKartOverride(const std::string& kart)
    {
        m_ai_kart_override = kart;
    }   // setAIKartOverride
    // ------------------------------------------------------------------------
    void setAISuperPower(AISuperPower superpower)
    {
        m_ai_superpower = superpower;
    }   // setAISuperPower
    // ------------------------------------------------------------------------
    AISuperPower getAISuperPower() const { return m_ai_superpower; }
    // ------------------------------------------------------------------------
    void setNumLaps(int num)
    {
        m_num_laps.clear();
        m_num_laps.push_back(num);
    }   // setNumLaps
    // ------------------------------------------------------------------------
    void setReverseTrack(bool r_t)
    {
        m_reverse_track.clear();
        m_reverse_track.push_back(r_t);
    }   // setReverseTrack
    // ------------------------------------------------------------------------
    void setMajorMode(MajorRaceModeType mode) { m_major_mode = mode; }
    // ------------------------------------------------------------------------
    void setMinorMode(MinorRaceModeType mode)
    {
        m_minor_mode = mode;
    }   // setMinorMode
    // ------------------------------------------------------------------------
    void setNumKarts(int num)
    {
        m_num_karts = num;
        m_ai_kart_override = "";
        m_ai_superpower = SUPERPOWER_NONE;
    }   // setNumKarts
    // ------------------------------------------------------------------------
    void setTimeTarget(float time)
    {
        m_goal_target = 0;
        m_time_target = time;
    }   // setTimeTarget
    // ------------------------------------------------------------------------
    const RemoteKartInfo& getKartInfo(unsigned int n) const
    {
        return m_player_karts[n];
    }   // getKartInfo
    // ------------------------------------------------------------------------
    unsigned int getNumLocalPlayers() const
    {
        return m_num_local_players;
    }   // getNumLocalPlayers
    // ------------------------------------------------------------------------
    /** Returns the selected number of karts (selected number of players and
     *  AI karts. */
    unsigned int getNumberOfKarts() const {return m_num_karts; }
    // ------------------------------------------------------------------------
    MajorRaceModeType getMajorMode() const { return m_major_mode; }
    // ------------------------------------------------------------------------
    MinorRaceModeType getMinorMode() const { return m_minor_mode; }
    // ------------------------------------------------------------------------
    unsigned int getNumPlayers() const 
    {
        return (unsigned int) m_player_karts.size(); 
    }   // getNumPlayers
    // ------------------------------------------------------------------------
    /** \brief Returns the number lf laps.
     *  In case of FTL or battle mode always return 9999, since they don't
     *  have laps. This avoids problems in FTL GP, since in this case no laps
     *  would be set (otherwise we would need many more tests in calls to
     *  getNumLaps).
     */
    int getNumLaps() const
    {
        if(m_minor_mode==MINOR_MODE_3_STRIKES     ||
            m_minor_mode==MINOR_MODE_FOLLOW_LEADER    )
            return 9999;
        return m_num_laps[m_track_number];
    }   // getNumLaps
    // ------------------------------------------------------------------------
    /** \return whether the track should be reversed */
    bool getReverseTrack() const { return m_reverse_track[m_track_number]; }
    // ------------------------------------------------------------------------
    /** Returns the difficulty. */
    Difficulty getDifficulty() const { return m_difficulty; }
    // ------------------------------------------------------------------------
    /** Returns the specified difficulty as a string. */
    std::string getDifficultyAsString(Difficulty diff) const
    {
        switch(diff)
        {
        case RaceManager::DIFFICULTY_EASY:   return "easy";   break;
        case RaceManager::DIFFICULTY_MEDIUM: return "medium"; break;
        case RaceManager::DIFFICULTY_HARD:   return "hard";   break;
        case RaceManager::DIFFICULTY_BEST:   return "best";   break;
        default:  assert(false);
        }
        return "";
    }   // getDifficultyAsString

    // ------------------------------------------------------------------------
    /** Returns the specified difficulty as a string. */
    core::stringw getDifficultyName(Difficulty diff) const
    {
        switch (diff)
        {
        case RaceManager::DIFFICULTY_EASY:   return _("Novice");   break;
        case RaceManager::DIFFICULTY_MEDIUM: return _("Intermediate"); break;
        case RaceManager::DIFFICULTY_HARD:   return _("Expert");   break;
        case RaceManager::DIFFICULTY_BEST:   return _("SuperTux");   break;
        default:  assert(false);
        }
        return "";
    }   // getDifficultyName
    // ------------------------------------------------------------------------
    const std::string& getTrackName() const { return m_tracks[m_track_number];}
    // ------------------------------------------------------------------------
    const GrandPrixData& getGrandPrix() const { return m_grand_prix; }
    // ------------------------------------------------------------------------
    unsigned int getFinishedKarts() const { return m_num_finished_karts; }
    // ------------------------------------------------------------------------
    unsigned int getFinishedPlayers() const { return m_num_finished_players; }
    // ------------------------------------------------------------------------
    int getKartGPRank(const int kart_id)const
    {
        return m_kart_status[kart_id].m_gp_rank;
    }   // getKartGPRank
    // ------------------------------------------------------------------------
    const std::string& getKartIdent(int kart) const
    {
        return m_kart_status[kart].m_ident;
    }   // getKartIdent
    // ------------------------------------------------------------------------
    int getKartScore(int krt) const { return m_kart_status[krt].m_score; }
    // ------------------------------------------------------------------------
    int getKartPrevScore(int krt) const
    {
        return m_kart_status[krt].m_last_score;
    }   // getKartPrevScore
    // ------------------------------------------------------------------------
    int getKartLocalPlayerId(int k) const
    {
        return m_kart_status[k].m_local_player_id;
    }   // getKartLocalPlayerId
    // ------------------------------------------------------------------------
    int getKartGlobalPlayerId(int k) const
    {
        return m_kart_status[k].m_global_player_id;
    }   // getKartGlobalPlayerId
    // ------------------------------------------------------------------------
    float getOverallTime(int kart) const
    {
        return m_kart_status[kart].m_overall_time;
    }   // getOverallTime
    // ------------------------------------------------------------------------
    float getKartRaceTime(int kart) const
    {
        return m_kart_status[kart].m_last_time;
    }   // getKartRaceTime
    // ------------------------------------------------------------------------
    KartType getKartType(int kart) const
    {
        return m_kart_status[kart].m_kart_type;
    }   // getKartType
    // ------------------------------------------------------------------------
    PerPlayerDifficulty getPlayerDifficulty(int kart) const
    {
        return m_kart_status[kart].m_difficulty;
    }   // getPlayerDifficulty
    // ------------------------------------------------------------------------
    int getCoinTarget() const { return m_coin_target; }
    // ------------------------------------------------------------------------
    float getTimeTarget() const { return m_time_target; }
    // ------------------------------------------------------------------------
    int getTrackNumber() const { return m_track_number; }
    // ------------------------------------------------------------------------
    int getNumOfTracks() const { return (int)m_tracks.size(); }
    // ------------------------------------------------------------------------
    /** Returns the list of AI karts to use. Used for networking, and for
    *  the --ai= command line option. */
    const std::vector<std::string>& getAIKartList() const
    {
        return m_ai_kart_list;
    }   // getAIKartList
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
    }   // isLinearRaceMode
    // ------------------------------------------------------------------------
    /** \brief Returns true if the current mode is a battle mode. */
    bool isBattleMode()
    {
        const int id = (int)m_minor_mode;
        // This uses the  numerical id of the mode, see the macros
        // LINEAR_RACE and BATTLE_ARENA above for exact meaning.
        if (id >= 2000 && id != 2001) return true;
        else            return false;
    }   // isBattleMode

    // ------------------------------------------------------------------------
    /** \brief Returns true if the current mode is a soccer mode. */
    bool isSoccerMode()
    {
        const int id = (int)m_minor_mode;
        // This uses the  numerical id of the mode, see the macros
        // LINEAR_RACE and BATTLE_ARENA above for exact meaning.
        if (id == 2001) return true;
        else            return false;
    }   // isSoccerMode

    // ------------------------------------------------------------------------
    bool isTutorialMode()
    {
        return m_minor_mode == MINOR_MODE_TUTORIAL;
    }   // isTutorialMode
    // ------------------------------------------------------------------------
    /** \brief Returns true if the current mode has laps. */
    bool modeHasLaps()
    {
        if (isBattleMode() || isSoccerMode()) return false;
        const int id = (int)m_minor_mode;
        // See meaning of IDs above
        const int answer = (id-1000)/100;
        return answer!=0;
    }   // modeHasLaps
    // ------------------------------------------------------------------------
    /** Returns true if the currently selected minor mode has highscores. */
    bool modeHasHighscores()
    {
        //FIXME: this information is duplicated. RaceManager knows about it,
        //       and each World may set m_use_highscores to true or false.
        //       The reason for this duplication is that we might want to know
        //       whether to display highscores without creating a World.
        return m_minor_mode != MINOR_MODE_3_STRIKES &&
               m_minor_mode != MINOR_MODE_SOCCER &&
               m_minor_mode != MINOR_MODE_FOLLOW_LEADER;
    }   // modeHasHighscore
    // ------------------------------------------------------------------------
    bool raceWasStartedFromOverworld() const
    {
        return m_started_from_overworld;
    }   // raceWasStartedFromOverworld

    // ------------------------------------------------------------------------
    /** \name Callbacks from the race classes
     * These methods are to be used by the classes that manage the various
     *  races, to let the race manager know about current status
     */
    bool allPlayerFinished() const
    {
        return m_num_finished_players == m_player_karts.size();
    }   // allPlayerFinished
    // ------------------------------------------------------------------------
    /** Sets the AI to use. This is used in networking mode to set the karts
     *  that will be used by the server to the client. It will take precedence
     *  over the random selection. */
    void setAIKartList(const std::vector<std::string>& rkl)
    {
        m_ai_kart_list = rkl;
    }   // setAIKartList
    // ------------------------------------------------------------------------
    bool haveKartLastPositionOnOverworld()
    {
        return m_have_kart_last_position_on_overworld;
    }   // haveKartLastPositionOnOverworld
    // ------------------------------------------------------------------------
    void setKartLastPositionOnOverworld(const Vec3 &pos)
    {
        m_have_kart_last_position_on_overworld = true;
        m_kart_last_position_on_overworld = pos;
    }   // setKartLastPositionOnOverworld
    // ------------------------------------------------------------------------
    void clearKartLastPositionOnOverworld()
    {
        m_have_kart_last_position_on_overworld = false;
    }   // clearKartLastPositionOnOverworld
    // ------------------------------------------------------------------------
    Vec3 getKartLastPositionOnOverworld()
    {
        return m_kart_last_position_on_overworld;
    }   // getKartLastPositionOnOverworld
    // ------------------------------------------------------------------------
    void setRecordRace(bool record)
    {
        m_is_recording_race = record;
    }   // setRecordRace
    // ------------------------------------------------------------------------
    void setRaceGhostKarts(bool ghost)
    {
        m_has_ghost_karts = ghost;
    }   // setRaceGhostKarts
    // ------------------------------------------------------------------------
    void setWatchingReplay(bool watch)
    {
        m_watching_replay = watch;
    }   // setWatchingReplay
    // ------------------------------------------------------------------------
    bool isRecordingRace() const
    {
        return m_is_recording_race;
    }   // isRecordingRace
    // ------------------------------------------------------------------------
    bool hasGhostKarts() const
    {
        return m_has_ghost_karts;
    }   // hasGhostKarts
    // ------------------------------------------------------------------------
    bool isWatchingReplay() const
    {
        return m_watching_replay;
    }   // isWatchingReplay
    // ------------------------------------------------------------------------
    void addSpareTireKart(const std::string& name)
    {
        m_kart_status.push_back(KartStatus(name, 0, -1, -1,
            -1, KT_SPARE_TIRE, PLAYER_DIFFICULTY_NORMAL));
        m_num_spare_tire_karts++;
        m_num_karts++;
    }   // addSpareTireKart
    // ------------------------------------------------------------------------
    void setSpareTireKartNum(unsigned int i)
    {
        m_num_spare_tire_karts = i;
    }   // setSpareTireKartNum
    // ------------------------------------------------------------------------
    unsigned int getNumSpareTireKarts() const
    {
        return m_num_spare_tire_karts;
    }   // getNumSpareTireKarts

};   // RaceManager

extern RaceManager *race_manager;
#endif

/* EOF */
