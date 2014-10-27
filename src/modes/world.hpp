//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2013 SuperTuxKart-Team
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

/**
  * \defgroup modes
  * Contains the logic for the various game modes (race, follow the leader,
  * battle, etc.)
  */

#include <vector>
#include <stdexcept>

#include "graphics/weather.hpp"
#include "modes/world_status.hpp"
#include "race/highscores.hpp"
#include "states_screens/race_gui_base.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/random_generator.hpp"

#include "LinearMath/btTransform.h"

class AbstractKart;
class btRigidBody;
class Controller;
class PhysicalObject;
class Physics;
class Track;

namespace Scripting
{
    class ScriptEngine;
}

namespace irr
{
    namespace scene { class ISceneNode; }
}

class AbortWorldUpdateException : public std::runtime_error
{
public:
    AbortWorldUpdateException() : std::runtime_error("race abort") { };
};

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
 * \ingroup modes
 */

class World : public WorldStatus
{
public:
    typedef std::vector<AbstractKart*> KartList;
private:
    /** A pointer to the global world object for a race. */
    static World *m_world;

protected:

#ifdef DEBUG
    unsigned int m_magic_number;
#endif

    /** The list of all karts. */
    KartList                  m_karts;
    RandomGenerator           m_random;

    Physics*      m_physics;
    bool          m_force_disable_fog;
    AbstractKart* m_fastest_kart;
    /** Number of eliminated karts. */
    int         m_eliminated_karts;
    /** Number of eliminated players. */
    int         m_eliminated_players;
    /** OVerall number of players. */
    int         m_num_players;

    bool        m_faster_music_active; // true if faster music was activated

    bool        m_stop_music_when_dialog_open;

    /** Whether highscores should be used for this kind of race.
     *  True by default, change to false in a child class to disable.
    */
    bool        m_use_highscores;

    void  updateHighscores  (int* best_highscore_rank, int* best_finish_time,
                             std::string* highscore_who,
                             StateManager::ActivePlayer** best_player);
    void  resetAllKarts     ();
    void  eliminateKart     (int kart_number, bool notifyOfElimination=true);
    Controller*
          loadAIController  (AbstractKart *kart);

    virtual AbstractKart *createKart(const std::string &kart_ident, int index,
                             int local_player_id, int global_player_id,
                             RaceManager::KartType type);
    /** Pointer to the track. The track is managed by world. */
    Track* m_track;

    /**Pointer to scripting engine  */
    Scripting::ScriptEngine* m_script_engine;

    /** Pointer to the race GUI. The race GUI is handled by world. */
    RaceGUIBase *m_race_gui;

    /** The actual race gui needs to be saved when the race result gui is
        displayed since it is still needed in case of a restart, and it
        can't simply be created again (since it assumes that it can render
        to texture without having any scene nodes, but in case of a restart
        there are scene nodes). */
    RaceGUIBase *m_saved_race_gui;

    irr::video::SColor m_clear_color;

    /** Pausing/unpausing are not done immediately, but at next udpdate. The
     *  use of this is when switching between screens : if we leave a screen
     *  that paused the game, only to go to another screen that pauses back
     *  the game, this mechanism prevents the game from moving on between
     *  the switch. */
    bool m_schedule_pause;

    /** Pausing/unpausing are not done immediately, but at next udpdate. The
     *  use of this is when switching between screens : if we leave a screen
     *  that paused the game, only to go to another screen that pauses back
     *  the game, this mechanism prevents the game from moving on between the
     *  switch. */
    bool m_schedule_unpause;

    bool m_schedule_exit_race;

    bool m_schedule_tutorial;

    Phase m_scheduled_pause_phase;

    /** Set when the world needs to be deleted but you can't do it immediately
     * because you are e.g. within World::update()
     */
    bool m_self_destruct;

    /** Set when the world is online and counts network players. */
    bool m_is_network_world;
    
    /** Used to show weather graphical effects. */
    Weather* m_weather;


    virtual void  onGo();
    /** Returns true if the race is over. Must be defined by all modes. */
    virtual bool  isRaceOver() = 0;
    virtual void  update(float dt);
    virtual void  createRaceGUI();
            void  updateTrack(float dt);
    // ------------------------------------------------------------------------
    /** Used for AI karts that are still racing when all player kart finished.
     *  Generally it should estimate the arrival time for those karts, but as
     *  a default (useful for battle mode and ftl races) we just use the
     *  current time for this (since this is a good value for karts still
     *  around at the end of a race, and other criteria (number of lives,
     *  race position) will be used to determine the final order.
     */
    virtual float estimateFinishTimeForKart(AbstractKart* kart)
                                        {return getTime(); }


public:
                    World();
    virtual        ~World();
    // Static functions to access world:
    // =================================
    // ------------------------------------------------------------------------
    /** Returns a pointer to the (singleton) world object. */
    static World*   getWorld() { return m_world; }
    // ------------------------------------------------------------------------
    /** Delete the )singleton) world object, if it exists, and sets the
      * singleton pointer to NULL. It's harmless to call this if the world
      *  has been deleted already. */
    static void     deleteWorld() { delete m_world; m_world = NULL; }
    // ------------------------------------------------------------------------
    /** Sets the pointer to the world object. This is only used by
     *  the race_manager.*/
    static void     setWorld(World *world) {m_world = world; }
    // ------------------------------------------------------------------------

    // Pure virtual functions
    // ======================

    /** Each game mode should have a unique identifier. Override
      * this method in child classes to provide it. */
    virtual const std::string& getIdent() const = 0;
    // ------------------------------------------------------------------------
    /** Returns the number of rescue positions on a given track and game
     *  mode. */
    virtual unsigned int getNumberOfRescuePositions() const = 0;
    // ------------------------------------------------------------------------
    /** Determines the rescue position index of the specified kart. */
    virtual unsigned int getRescuePositionIndex(AbstractKart *kart) = 0;
    // ------------------------------------------------------------------------
    /** Returns the bullet transformation for the specified rescue index. */
    virtual btTransform getRescueTransform(unsigned int index) const = 0;
    // ------------------------------------------------------------------------
    void moveKartAfterRescue(AbstractKart* kart);
    // ------------------------------------------------------------------------
    /** Called when it is needed to know whether this kind of race involves
     *  counting laps. */
    virtual bool raceHasLaps() = 0;
    // ------------------------------------------------------------------------
    /** Returns the number of laps for a given kart. Only valid when
     *  raceHasLaps() - otherwise STK will abort. */
    virtual int getKartLaps(unsigned int kart_index) const
    {
        assert(false); return -1; // remove compiler warning
    }   // getKartLaps
    // ------------------------------------------------------------------------
    /** Called by the code that draws the list of karts on the race GUI
      * to know what needs to be drawn in the current mode. */
    virtual void getKartsDisplayInfo(
                       std::vector<RaceGUIBase::KartIconDisplayInfo> *info)= 0;
    // ------------------------------------------------------------------------

    // Virtual functions
    // =================
    virtual void    init();
    virtual void    terminateRace() OVERRIDE;
    virtual void    reset();
    virtual void    pause(Phase phase) OVERRIDE;
    virtual void    unpause() OVERRIDE;
    virtual void    getDefaultCollectibles(int *collectible_type,
                                           int *amount );
    virtual void    endRaceEarly() { return; }

    // ------------------------------------------------------------------------
    /** Called to determine whether this race mode uses bonus boxes. */
    virtual bool haveBonusBoxes() { return true; }
    // ------------------------------------------------------------------------
    /** Returns if this mode should use fast music (if available). */
    virtual bool useFastMusicNearEnd() const { return true; }
    // ------------------------------------------------------------------------
    /** If you want to do something to karts or their graphics at the start
     *  of the race, override this. */
    virtual void kartAdded(AbstractKart* kart, scene::ISceneNode* node) {}
    // ------------------------------------------------------------------------
    /** Called whenever a kart starts a new lap. Meaningless (and won't be
     *  called) in non-laped races.
     */
    virtual void newLap(unsigned int kart_index) {}
    // ------------------------------------------------------------------------
    /** Called when a kart was hit by a projectile. */
    virtual void kartHit(const unsigned int kart_id) {};
    // ------------------------------------------------------------------------
    virtual void onMouseClick(int x, int y) {};

    // Other functions
    // ===============
    Highscores     *getHighscores() const;
    void            schedulePause(Phase phase);
    void            scheduleUnpause();
    void            scheduleExitRace() { m_schedule_exit_race = true; }
    void            scheduleTutorial();
    void            updateWorld(float dt);
    void            handleExplosion(const Vec3 &xyz, AbstractKart *kart_hit,
                                    PhysicalObject *object);
    AbstractKart*   getPlayerKart(unsigned int player) const;
    AbstractKart*   getLocalPlayerKart(unsigned int n) const;
    // ------------------------------------------------------------------------
    /** Returns a pointer to the race gui. */
    RaceGUIBase    *getRaceGUI() const { return m_race_gui;}
    // ------------------------------------------------------------------------
    /** Returns the number of karts in the race. */
    unsigned int    getNumKarts() const { return (unsigned int) m_karts.size(); }
    // ------------------------------------------------------------------------
    /** Returns the kart with a given world id. */
    AbstractKart       *getKart(int kartId) const {
                        assert(kartId >= 0 && kartId < int(m_karts.size()));
                        return m_karts[kartId];                              }
    // ------------------------------------------------------------------------
    /** Returns all karts. */
    const KartList & getKarts() const { return m_karts; }
    // ------------------------------------------------------------------------
    /** Returns the number of currently active (i.e.non-elikminated) karts. */
    unsigned int    getCurrentNumKarts() const { return (int)m_karts.size() -
                                                         m_eliminated_karts; }
    // ------------------------------------------------------------------------
    /** Returns the number of currently active (i.e. non-eliminated) players.*/
    unsigned int    getCurrentNumPlayers() const { return m_num_players -
                                                         m_eliminated_players;}
    // ------------------------------------------------------------------------
    /** Returns a pointer to the physics. */
    Physics        *getPhysics() const { return m_physics; }
    // ------------------------------------------------------------------------
    /** Returns a pointer to the track. */
    Track          *getTrack() const { return m_track; }
    // ------------------------------------------------------------------------
    /** Returns a pointer to the Scripting Engine. */
    Scripting::ScriptEngine   *getScriptEngine() 
                               const { return m_script_engine; }
    //-------------------------------------------------------------------------
    bool            isFogEnabled() const;
    // ------------------------------------------------------------------------
    void moveKartTo(AbstractKart* kart, const btTransform &t);
    // ------------------------------------------------------------------------
    /** The code that draws the timer should call this first to know
     *  whether the game mode wants a timer drawn. */
    virtual bool shouldDrawTimer() const
                    { return isRacePhase() && getClockMode() != CLOCK_NONE; }
    // ------------------------------------------------------------------------
    /** \return whether this world can generate/have highscores */
    bool useHighScores() const { return m_use_highscores; }
    // ------------------------------------------------------------------------
    /** Returns the color to clear the back buffer. */
    const irr::video::SColor& getClearColor() const { return m_clear_color; }
    // ------------------------------------------------------------------------
    /** Sets the color to use when clearing the back buffer. */
    void setClearbackBufferColor(irr::video::SColor color)
    {
        m_clear_color       = color;
    }
    /** Override track fog value to force disabled */
    void forceFogDisabled(bool v) { m_force_disable_fog = v; }
    // ------------------------------------------------------------------------
    /** Override if you want to know when a kart presses fire */
    virtual void onFirePressed(Controller* who) {}
    // ------------------------------------------------------------------------
    /** Whether to compute checkline requirements for each world on the
      * quadgraph. Override to change value. */
    virtual bool useChecklineRequirements() const { return false; }
    // ------------------------------------------------------------------------
    void delayedSelfDestruct();
    // ------------------------------------------------------------------------
    virtual void escapePressed();

    /** Set the network mode (true if networked) */
    void setNetworkWorld(bool is_networked) { m_is_network_world = is_networked; }

    bool isNetworkWorld() const { return m_is_network_world; }
    
    /** Returns a pointer to the weather. */
    Weather* getWeather() {return m_weather;}
};   // World

#endif

/* EOF */
