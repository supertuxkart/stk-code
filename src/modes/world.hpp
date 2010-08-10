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

/** \defgroup modes */

#include <vector>

#include "modes/world_status.hpp"
#include "network/network_kart.hpp"
#include "physics/physics.hpp"
#include "race/highscores.hpp"
#include "states_screens/race_gui_base.hpp"
#include "utils/random_generator.hpp"

class btRigidBody;
class Kart;
class Track;

/** 
 *  \brief base class for all game modes
 *  This class is responsible for running the actual race. A world is created
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
 * \ingroup modes
 */

class World : public WorldStatus
{
public:
    typedef std::vector<Kart*> KartList;
private:
    /** A pointer to the global world object for a race. */
    static World *m_world;

    /** Counts the karts that have 'started', i.e. pressed acceleration
     *  after 'ready-set-go'. The first two karts will get a speed boost. */
    unsigned int  m_num_started_karts;

protected:
    /** The list of all karts. */
    KartList                  m_karts;
    RandomGenerator           m_random;

    Physics*    m_physics;
    float       m_fastest_lap;
    Kart*       m_fastest_kart;
    /** Number of eliminated karts. */
    int         m_eliminated_karts;
    /** Number of eliminated players. */
    int         m_eliminated_players;
    /** OVerall number of players. */
    int         m_num_players;

    bool        m_faster_music_active; // true if faster music was activated

    /** Whether highscores should be used for this kind of race.
     *  True by default, change to false in a child class to disable.
    */
    bool        m_use_highscores;
    
    void  updateHighscores  ();
    void  resetAllKarts     ();
    void  removeKart        (int kart_number, bool notifyOfElimination=true);
    Controller* 
          loadAIController  (Kart *kart);

    virtual Kart *createKart(const std::string &kart_ident, int index, 
                             int local_player_id, int global_player_id,
                             const btTransform &init_pos);
    /** Pointer to the track. The track is managed by world. */
    Track* m_track;

    /** Pointer to the race GUI. The race GUI is handled by world. */
    RaceGUIBase *m_race_gui;

    /** The actual race gui needs to be saved when the race result gui is 
        displayed since it is still needed in case of a restart, and it
        can't simply be created again (since it assumes that it can render
        to texture without having any scene nodes, but in case of a restart
        there are scene nodes). */
    RaceGUIBase *m_saved_race_gui;

    bool     m_clear_back_buffer;
    
    irr::video::SColor m_clear_color;
    
    virtual void    onGo();
    /** Returns true if the race is over. Must be defined by all modes. */
    virtual bool    isRaceOver() = 0;
    virtual void    update(float dt);
    /** Used for AI karts that are still racing when all player kart finished.
     *  Generally it should estimate the arrival time for those karts, but as
     *  a default (useful for battle mode and ftl races) we just use the 
     *  current time for this (since this is a good value for karts still 
     *  around at the end of a race, and other criteria (number of lives,
     *  race position) will be used to determine the final order.
     */
    virtual float   estimateFinishTimeForKart(Kart* kart) {return getTime(); }

public:
                    World();
    virtual        ~World();
    /** Returns a pointer to the (singleton) world object. */
    static World*   getWorld() { return m_world; }

    /** Delete the )singleton) world object, if it exists, and sets the singleton pointer to NULL.
      * It's harmless to call this if the world has been deleted already. */
    static void     deleteWorld() { delete m_world; m_world = NULL; }
    
    /** Sets the pointer to the world object. This is only used by
     *  the race_manager.*/
    static void     setWorld(World *world) {m_world = world; }
    /** call just after instanciating. can't be moved to the contructor as child
        classes must be instanciated, otherwise polymorphism will fail and the
        results will be incorrect */
    virtual void    init();
    
    void            updateWorld(float dt);
    virtual void    restartRace();
    
    /** Put race into limbo phase */
    void            disableRace();
    
    Kart           *getPlayerKart(unsigned int player) const;
    Kart           *getLocalPlayerKart(unsigned int n) const;
    /** Returns a pointer to the race gui. */
    RaceGUIBase    *getRaceGUI()                const { return m_race_gui;                  }
    unsigned int    getNumKarts()               const { return m_karts.size();              }
    Kart           *getKart(int kartId)         const { assert(kartId >= 0 &&
                                                            kartId < int(m_karts.size()));
                                                        return m_karts[kartId];             }
    /** Returns the number of currently active (i.e.non-elikminated) karts. */
    unsigned int    getCurrentNumKarts()        const { return (int)m_karts.size() -
                                                            m_eliminated_karts;              }
    /** Returns the number of currently active (i.e. non-eliminated) players. */
    unsigned int    getCurrentNumPlayers()      const { return m_num_players -
                                                            m_eliminated_players;            }
    
    Physics        *getPhysics()                const { return m_physics;                   }
    Track          *getTrack()                  const { return m_track;                     }
    Kart           *getFastestKart()            const { return m_fastest_kart;              }
    float           getFastestLapTime()         const { return m_fastest_lap;               }
    void            setFastestLap(Kart *k, float time){ m_fastest_kart = k;
                                                        m_fastest_lap  = time;              }
    Highscores *getHighscores() const;

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
    
    virtual void  pause(Phase phase);
    virtual void  unpause();
    
    /**
      * The code that draws the timer should call this first to know
      * whether the game mode wants a timer drawn
      */
    bool shouldDrawTimer() const    { return isRacePhase() &&
                                             getClockMode() != CLOCK_NONE; }
    
    /** \return whether this world can generate/have highscores */
    bool useHighScores() const      { return m_use_highscores; }
        
    /** Called by the code that draws the list of karts on the race GUI
      * to know what needs to be drawn in the current mode
      */
    virtual RaceGUIBase::KartIconDisplayInfo* getKartsDisplayInfo() = 0;
    
    /** Since each mode will have a different way of deciding where a rescued
      * kart is dropped, this method will be called and each mode can implement it.
      */
    virtual void moveKartAfterRescue(Kart* kart) = 0;
    
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
      * \param[out] order returns the order of karts. order[0] will contain the ID of
      *                   the first kart, order[1] the ID of the second kart, etc...
      *                   Array dimension must be the number of karts.
      */
    virtual void raceResultOrder(std::vector<int> *order ) = 0;
    
    /** Returns the number of started karts, used to determine which karts
     *  receive a speed boost. */
    unsigned int getNumStartedKarts() const { return m_num_started_karts; }

    /** Increases the number of karts that have accelerated. */
    void         incNumStartedKarts()       { m_num_started_karts++;      }
    bool         clearBackBuffer() const { return m_clear_back_buffer; }
    
    const irr::video::SColor& getClearColor() const { return m_clear_color; }
    void         setClearBackBuffer(bool enabled) { m_clear_back_buffer = enabled; }
    void         setClearbackBufferColor(irr::video::SColor color) { m_clear_color = color; }
    
};

#endif

/* EOF */
