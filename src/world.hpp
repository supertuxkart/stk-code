//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 SuperTuxKart-Team
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

#ifndef HEADER_WORLD_H
#define HEADER_WORLD_H

#include <vector>
#define _WINSOCKAPI_
#include <plib/ssg.h>
#include "track.hpp"
#include "player_kart.hpp"
#include "physics.hpp"
#include "kart.hpp"
#include "highscores.hpp"
#include "network/network_kart.hpp"
#include "utils/random_generator.hpp"

#ifdef HAVE_GHOST_REPLAY
#  include "replay_recorder.hpp"
   class ReplayPlayer;
#endif

/** This class is responsible for running the actual race. A world is created
 *  by the race manager on the start of each race (so a new world is created
 *  for each race of a Grand Prix). It creates the 
 *  physics, loads the track, creates all karts, and initialises the race
 *  specific managers (HerringManager, ProjectilManager, highscores, ...).
 *  It uses the information from the race manager to get information like
 *  what and how many karts, track to load etc. This class does not really
 *  know about Grand Prixs, a GP is created
 *  It maintains the race clock, and updates all karts, herings etc. during
 *  the race.
 *  Special game modes (e.g. follow the leader) are currently integrated in
 *  this world, see e.g. updateRaceStatus where the world clock behaviour
 *  is handled differently to create the count down.
 *  \todo The different game modes should be implemented more cleanly (instead
 *        of being handled by if statements all over the place). E.g. world
 *        could become a base class, from which the following classes are
 *        created:
 *        - LapCountingWorld:World           Adds lap counting to the world
 *        - TimeTrialWorld:LapCountingWorld  Creates a world for time trial
 *                                           mode.
 *        - CrazyRaceWorld:LapCountingWorld  Creates a world for crazy race
 *                                           (currently called quick race).
 *        - FollowLeaderWorld:World          Creates the follow the leader mode.
 *        - TagWorld:World                   Multi-player 'tag' like game.
 *  Of course, other designs are possible (e.g. a mode-specific object could
 *  be passed into the world constructor, and world just calls functions in
 *  this object.
 *  \todo The handling of the FollowTheLeader mode should probably be changed
 *        as well. Currently the leader kart is anormal AI kart (so it is 
 *        included in the number of karts selected), and this causes some 
 *        unnecessary handling of special cases everywhere (e.g. score 
 *        counting will subtract one from the rank, since the kart on position
 *        1 is the leader and does not score points, and the kart on position 
 *        2 should get the points for a kart on position 1 etc). With a 
 *        special world for the FTL mode this could be handled better: 
 *        creating a special kart type (see RaceManager::KartType), scoring
 *        would be done in the mode specific world (instead of in the
 *        RaceManager).
 */

class World
{
public:
    typedef std::vector<Kart*> Karts;

    /** resources, this should be put in a separate class or replaced by a smart
     * resource manager
     */

    enum Phase {
        // Game setup, e.g. track loading
        SETUP_PHASE,
        // 'Ready' is displayed
        READY_PHASE,
        // 'Set' is displayed
        SET_PHASE,
        // 'Go' is displayed, but this is already race phase
        GO_PHASE,
        // the actual race has started, no ready/set/go is displayed anymore
        RACE_PHASE,
        // All players have finished, now wait a certain amount of time for AI
        // karts to finish. If they do not finish in that time, finish the race
        DELAY_FINISH_PHASE,
        // The player crossed the finishing line and his and the time of
        // the other players is displayed, controll is automatic
        FINISH_PHASE,
        // The state after finish where no calculations are done.
        LIMBO_PHASE,
    };

    Track* m_track;
    
        /** debug text that will be overlaid to the screen */
    std::string m_debug_text[10];

                World();
    virtual    ~World();
    void        update(float delta);
    // Note: GO_PHASE is both: start phase and race phase
    bool        isStartPhase() const  {return m_phase<GO_PHASE;}
    bool        isRacePhase() const   {return m_phase>=GO_PHASE && m_phase<LIMBO_PHASE;}
    void        restartRace();
    void        disableRace(); // Put race into limbo phase

    PlayerKart *getPlayerKart(int player) const { return m_player_karts[player];   }
    unsigned int getCurrentNumLocalPlayers() const {return m_local_player_karts.size();}
    PlayerKart *getLocalPlayerKart(int n) const { return m_local_player_karts[n];  }
    NetworkKart*getNetworkKart(int n) const   {return m_network_karts[n];          }
    Kart       *getKart(int kartId) const     { assert(kartId >= 0 &&
                                                      kartId < int(m_kart.size()));
                                                return m_kart[kartId];             }
    unsigned int getCurrentNumKarts()  const  { return (int)m_kart.size()-
                                                          m_eliminated_karts;      }
    unsigned int getCurrentNumPlayers() const { return (int)m_player_karts.size()-
                                                        m_eliminated_players;      }

    /** Returns the phase of the game */
    Phase getPhase() const                    { return m_phase;                    }
    Physics *getPhysics() const               { return m_physics;                  }
    Track *getTrack() const                   { return m_track;                    }
    Kart* getFastestKart() const              { return m_fastest_kart;             }
    float getFastestLapTime() const           { return m_fastest_lap;              }
    void  setFastestLap(Kart *k, float time)  {m_fastest_kart=k;m_fastest_lap=time;}
    const Highscores* getHighscores() const   { return m_highscores;               }
    float getTime() const                     { return m_clock;                    }

    void  pause();
    void  unpause();

private:
    std::vector<PlayerKart*>  m_player_karts;
    std::vector<PlayerKart*>  m_local_player_karts;
    std::vector<NetworkKart*> m_network_karts; 
    RandomGenerator           m_random;

    Karts       m_kart;
    Physics*    m_physics;
    float       m_fastest_lap;
    Kart*       m_fastest_kart;
    Highscores* m_highscores;
    Phase       m_phase;
    Phase       m_previous_phase;      // used during the race popup menu
    float       m_clock;
    float       m_finish_delay_start_time;
    int         m_eliminated_karts;    // number of eliminated karts
    int         m_eliminated_players;  // number of eliminated players
    std::vector<float>
                m_leader_intervals;    // time till elimination in follow leader
    bool        m_faster_music_active; // true if faster music was activated

    void  updateRacePosition(int k);
    void  updateHighscores  ();
    void  loadTrack         ();
    void  updateRaceStatus  (float dt);
    void  resetAllKarts     ();
    void  removeKart        (int kart_number);
    Kart* loadRobot         (const std::string& kart_name, int position,
                             const btTransform& init_pos);
    void  updateLeaderMode  (float dt);
    void  printProfileResultAndExit();
    void  estimateFinishTimes();
#ifdef HAVE_GHOST_REPLAY
private:
    bool    saveReplayHumanReadable( std::string const &filename ) const;
    bool    loadReplayHumanReadable( std::string const &filename );

    ReplayRecorder  m_replay_recorder;
    ReplayPlayer    *m_p_replay_player;
#endif

};

extern World* world;

#endif

/* EOF */
