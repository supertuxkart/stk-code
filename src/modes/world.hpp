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

#ifndef HEADER_WORLD_HPP
#define HEADER_WORLD_HPP

#include <vector>

#include "karts/kart.hpp"
#include "karts/player_kart.hpp"
#include "modes/clock.hpp"
#include "network/network_kart.hpp"
#include "physics/physics.hpp"
#include "race/highscores.hpp"
#include "states_screens/race_gui.hpp"
#include "utils/random_generator.hpp"

class SFXBase;
class btRigidBody;
class Track;
class RaceGUI;

/** This class is responsible for running the actual race. A world is created
 *  by the race manager on the start of each race (so a new world is created
 *  for each race of a Grand Prix). It creates the 
 *  physics, loads the track, creates all karts, and initialises the race
 *  specific managers (ItemManager, ProjectilManager, highscores, ...).
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
 *        as well. Currently the leader kart is a normal AI kart (so it is 
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

class World : public TimedRace
{
public:
    typedef std::vector<Kart*> Karts;
protected:
    
    std::vector<PlayerKart*>  m_player_karts;
    std::vector<PlayerKart*>  m_local_player_karts;
    std::vector<NetworkKart*> m_network_karts; 
    RandomGenerator           m_random;

    Karts       m_kart;
    Physics*    m_physics;
    float       m_fastest_lap;
    Kart*       m_fastest_kart;
    Phase       m_previous_phase;      // used during the race popup menu
    int         m_eliminated_karts;    // number of eliminated karts
    int         m_eliminated_players;  // number of eliminated players

    bool        m_faster_music_active; // true if faster music was activated

    /** Whether highscores should be used for this kind of race.
     *  True by default, change to false in a child class to disable.
    */
    bool        m_use_highscores;
    
    void  updateHighscores  ();
    void  resetAllKarts     ();
    void  removeKart        (int kart_number);
    Kart* loadRobot         (const std::string& kart_name, int position,
                             const btTransform& init_pos);
    void  estimateFinishTimes();

    virtual Kart *createKart(const std::string &kart_ident, int index, 
                             int local_player_id, int global_player_id,
                             const btTransform &init_pos);
protected:
    /** Pointer to the track. The track is managed by world. */
    Track* m_track;
    
    /** Pointer to the race GUI. The race GUI is handedl by world. */
    RaceGUI *m_race_gui;
    
public:
    World();
    /** call just after instanciating. can't be moved to the contructor as child
        classes must be instanciated, otherwise polymorphism will fail and the
        results will be incorrect */
    virtual void init();
    
    virtual         ~World();
    virtual void    update(float delta);
    /** Returns true if the race is over. Must be defined by all modes. */
    virtual bool    isRaceOver() = 0;
    virtual void    restartRace();
    void            disableRace(); // Put race into limbo phase
    /** Returns a pointer to the race gui. */
    RaceGUI        *getRaceGUI()                const { return m_race_gui;                  }
    PlayerKart     *getPlayerKart(int player)   const { return m_player_karts[player];      }
    unsigned int    getCurrentNumLocalPlayers() const { return m_local_player_karts.size(); }
    PlayerKart     *getLocalPlayerKart(int n)   const { return m_local_player_karts[n];     }
    NetworkKart    *getNetworkKart(int n)       const { return m_network_karts[n];          }
    Kart           *getKart(int kartId)         const { assert(kartId >= 0 &&
                                                            kartId < int(m_kart.size()));
                                                        return m_kart[kartId];               }
    unsigned int    getCurrentNumKarts()        const { return (int)m_kart.size() -
                                                            m_eliminated_karts;              }
    unsigned int    getCurrentNumPlayers()      const { return (int)m_player_karts.size()-
                                                            m_eliminated_players;            }
    
    Physics *getPhysics() const               { return m_physics;                   }
    Track *getTrack() const                   { return m_track;                     }
    Kart* getFastestKart() const              { return m_fastest_kart;              }
    float getFastestLapTime() const           { return m_fastest_lap;               }
    void  setFastestLap(Kart *k, float time)  {m_fastest_kart=k;m_fastest_lap=time; }
    HighscoreEntry* getHighscores() const;
    float getTime() const                     { return TimedRace::getTime();           }
    Phase getPhase() const                    { return TimedRace::getPhase();          }

    virtual void terminateRace();
    
    /** Called to determine the default collectibles to give each player for this
      * kind of race. Both parameters are of 'out' type.
      */
    virtual void getDefaultCollectibles(int& collectible_type, int& amount );
    
    /** Called to determine whether this race mode uses bonus boxes.
      */
    virtual bool haveBonusBoxes(){ return true; }
    
    /** Each game mode should have a unique identifier. Override
      * this method in child classes to provide it.
      */
    virtual std::string getIdent() const = 0;
        
    virtual bool useFastMusicNearEnd() const { return true; }
    
    void  pause();
    void  unpause();
    
    /**
      * The code that draws the timer should call this first to know
      * whether the game mode wants a timer drawn
      */
    bool shouldDrawTimer() const    { return TimedRace::isRacePhase() &&
                                             TimedRace::getClockMode() != CLOCK_NONE; }
    
    /** called when a bonus box is hit, to determine which types of powerups are allowed
        in each game mode. By default all are accepted, override in child classes to get
        a different behaviour  */
    virtual bool acceptPowerup(const int type) const { return true; }
    
    /** Called by the code that draws the list of karts on the race GUI
      * to know what needs to be drawn in the current mode
      */
    virtual RaceGUI::KartIconDisplayInfo* getKartsDisplayInfo() = 0;
    
    /** Since each mode will have a different way of deciding where a rescued
      * kart is dropped, this method will be called and each mode can implement it.
      */
    virtual void moveKartAfterRescue(Kart* kart, btRigidBody* body) = 0;
    
    /** Called when it is needed to know whether this kind of race involves counting laps.
      */
    virtual bool raceHasLaps() = 0;

    /** Called whenever a kart starts a new lap. Meaningless (and won't be 
     *  called) in non-laped races.
     */
    virtual void newLap(unsigned int kart_index) {}
    
    /** Called when a kart was hit by a projectile
     */
    virtual void kartHit(const int kart_id) {};
    
    /** Called by the race result GUI at the end of the race to know the final order
        (fill in the 'order' array) */
    virtual void raceResultOrder( int* order ) = 0;
};

#endif

/* EOF */
