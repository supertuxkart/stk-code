//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015 SuperTuxKart-Team
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

#include <cstring>
#include <limits>
#include <map>
#include <memory>
#include <vector>
#include <stdexcept>

#include "graphics/weather.hpp"
#include "modes/world_status.hpp"
#include "race/highscores.hpp"
#include "states_screens/race_gui_base.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/random_generator.hpp"
#include "utils/stk_process.hpp"

#include "LinearMath/btTransform.h"

class AbstractKart;
class BareNetworkString;
class btRigidBody;
class Controller;
class ItemState;
class PhysicalObject;
class STKPeer;

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
    typedef std::vector<std::shared_ptr<AbstractKart> > KartList;
private:
    /** A pointer to the global world object for a race. */
    static World *m_world[PT_COUNT];
    // ------------------------------------------------------------------------
    void setAITeam();
    // ------------------------------------------------------------------------
    std::shared_ptr<AbstractKart> createKartWithTeam
        (const std::string &kart_ident, int index, int local_player_id,
        int global_player_id, RaceManager::KartType type,
        HandicapLevel handicap);

protected:

#ifdef DEBUG
    unsigned int m_magic_number;
#endif

    /* Team related variables. */
    int m_red_ai;
    int m_blue_ai;
    std::map<int, KartTeam> m_kart_team_map;
    std::map<int, unsigned int> m_kart_position_map;

    /** The list of all karts. */
    KartList                  m_karts;
    RandomGenerator           m_random;

    AbstractKart* m_fastest_kart;
    /** Number of eliminated karts. */
    int         m_eliminated_karts;
    /** Number of eliminated players. */
    int         m_eliminated_players;
    /** OVerall number of players. */
    int         m_num_players;

    bool        m_faster_music_active; // true if faster music was activated

    bool        m_stop_music_when_dialog_open;

    bool        m_unfair_team;

    /** Whether highscores should be used for this kind of race.
     *  True by default, change to false in a child class to disable.
    */
    bool        m_use_highscores;

    void  updateHighscores  (int* best_highscore_rank);
    void  resetAllKarts     ();
    Controller*
          loadAIController  (AbstractKart *kart);

    virtual std::shared_ptr<AbstractKart> createKart
        (const std::string &kart_ident, int index, int local_player_id,
        int global_player_id, RaceManager::KartType type,
        HandicapLevel handicap);

    /** Pointer to the race GUI. The race GUI is handled by world. */
    RaceGUIBase *m_race_gui;

    /** The actual race gui needs to be saved when the race result gui is
        displayed since it is still needed in case of a restart, and it
        can't simply be created again (since it assumes that it can render
        to texture without having any scene nodes, but in case of a restart
        there are scene nodes). */
    RaceGUIBase *m_saved_race_gui;

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

    bool m_ended_early;

    virtual void  onGo() OVERRIDE;
    /** Returns true if the race is over. Must be defined by all modes. */
    virtual bool  isRaceOver() = 0;
    virtual void  update(int ticks) OVERRIDE;
    virtual void  createRaceGUI();
            void  updateTrack(int ticks);
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
    void updateAchievementDataEndRace();
    void updateAchievementModeCounters(bool start);

public:
                    World();
    virtual        ~World();
    // Static functions to access world:
    // =================================
    // ------------------------------------------------------------------------
    /** Returns a pointer to the (singleton) world object. */
    static World*   getWorld()
    {
        ProcessType type = STKProcess::getType();
        return m_world[type];
    }
    // ------------------------------------------------------------------------
    /** Delete the )singleton) world object, if it exists, and sets the
      * singleton pointer to NULL. It's harmless to call this if the world
      *  has been deleted already. */
    static void     deleteWorld()
    {
        ProcessType type = STKProcess::getType();
        delete m_world[type];
        m_world[type] = NULL;
    }
    // ------------------------------------------------------------------------
    /** Sets the pointer to the world object. This is only used by
     *  the race_manager.*/
    static void     setWorld(World *world)
    {
        ProcessType type = STKProcess::getType();
        m_world[type] = world;
    }
    // ------------------------------------------------------------------------
    static void     clear() { memset(m_world, 0, sizeof(m_world)); }
    // ------------------------------------------------------------------------

    // Pure virtual functions
    // ======================

    /** Each game mode should have a unique identifier. Override
      * this method in child classes to provide it. */
    virtual const std::string& getIdent() const = 0;
    // ------------------------------------------------------------------------
    /** Returns the number of rescue positions on a given track and game
     *  mode. */
    virtual unsigned int getNumberOfRescuePositions() const;
    // ------------------------------------------------------------------------
    /** Determines the rescue position index of the specified kart. */
    virtual unsigned int getRescuePositionIndex(AbstractKart *kart) = 0;
    // ------------------------------------------------------------------------
    /** Returns the bullet transformation for the specified rescue index. */
    virtual btTransform getRescueTransform(unsigned int index) const;
    // ------------------------------------------------------------------------
    virtual void moveKartAfterRescue(AbstractKart* kart);
    // ------------------------------------------------------------------------
    /** Called when it is needed to know whether this kind of race involves
     *  counting laps. */
    virtual bool raceHasLaps() = 0;
    // ------------------------------------------------------------------------
    /** If true lap counter shows lap count in format: 4/20 or if false then in format: 4 */
    virtual bool showLapsTarget() { return false; }
    // ------------------------------------------------------------------------
    /** Returns the number of laps for a given kart. Only valid when
     *  raceHasLaps() - otherwise STK will abort. */
    virtual int getFinishedLapsOfKart(unsigned int kart_index) const
    {
        assert(false); return -1; // remove compiler warning
    }   // getFinishedLapsOfKart
    // ------------------------------------------------------------------------
    /** Called by the code that draws the list of karts on the race GUI
      * to know what needs to be drawn in the current mode. */
    virtual void getKartsDisplayInfo(
                       std::vector<RaceGUIBase::KartIconDisplayInfo> *info)= 0;
    // ------------------------------------------------------------------------

    // Virtual functions
    // =================
    virtual void    init();
    virtual void    updateGraphics(float dt);
    virtual void    terminateRace() OVERRIDE;
    virtual void    reset(bool restart=false) OVERRIDE;
    virtual void    pause(Phase phase) OVERRIDE;
    virtual void    unpause() OVERRIDE;
    virtual void    getDefaultCollectibles(int *collectible_type,
                                           int *amount );
    // ------------------------------------------------------------------------
    /** Receives notification if an item is collected. Used for easter eggs. */
    virtual void collectedItem(const AbstractKart *kart, 
                               const ItemState *item    ) {}
    // ------------------------------------------------------------------------
    virtual void endRaceEarly() { return; }
    // ------------------------------------------------------------------------
    virtual bool hasRaceEndedEarly() const { return m_ended_early; }
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
    virtual bool kartHit(int kart_id, int hitter = -1) { return false; }
    // ------------------------------------------------------------------------
    virtual void onMouseClick(int x, int y) {};

    // Other functions
    // ===============
    Highscores     *getHighscores() const;
    Highscores     *getGPHighscores() const; 
    void            schedulePause(Phase phase);
    void            scheduleUnpause();
    void            scheduleExitRace() { m_schedule_exit_race = true; }
    void            scheduleTutorial();
    void            updateWorld(int ticks);
    void            handleExplosion(const Vec3 &xyz, AbstractKart *kart_hit,
                                    PhysicalObject *object);
    AbstractKart*   getPlayerKart(unsigned int player) const;
    AbstractKart*   getLocalPlayerKart(unsigned int n) const;
    virtual const btTransform &getStartTransform(int index);
    void moveKartTo(AbstractKart* kart, const btTransform &t);
    void updateTimeTargetSound();
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
                        return m_karts[kartId].get();                              }
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
    void resetElimination()
    {
        m_eliminated_karts = 0;
        m_eliminated_players = 0;
    }
    // ------------------------------------------------------------------------
    virtual void addReservedKart(int kart_id)
    {
        if (m_eliminated_karts > 0)
            m_eliminated_karts--;
    }
    // ------------------------------------------------------------------------
    virtual void saveCompleteState(BareNetworkString* bns, STKPeer* peer) {}
    // ------------------------------------------------------------------------
    virtual void restoreCompleteState(const BareNetworkString& buffer) {}
    // ------------------------------------------------------------------------
    /** The code that draws the timer should call this first to know
     *  whether the game mode wants a timer drawn. */
    virtual bool shouldDrawTimer() const
                    { return isActiveRacePhase() && getClockMode() != CLOCK_NONE; }
    // ------------------------------------------------------------------------
    /** \return whether this world can generate/have highscores */
    bool useHighScores() const { return m_use_highscores; }
    // ------------------------------------------------------------------------
    /** Override if you want to know when a kart presses fire */
    virtual void onFirePressed(Controller* who) {}
    // ------------------------------------------------------------------------
    /** Whether to compute checkline requirements for each world on the
      * quadgraph. Override to change value. */
    virtual bool useChecklineRequirements() const { return false; }
    // ------------------------------------------------------------------------
    virtual void escapePressed();
    // ------------------------------------------------------------------------
    virtual void loadCustomModels() {}
    // ------------------------------------------------------------------------
    void eliminateKart(int kart_number, bool notify_of_elimination = true);
    // ------------------------------------------------------------------------
    void setUnfairTeam(bool val)                       { m_unfair_team = val; }
    // ------------------------------------------------------------------------
    virtual bool hasTeam() const                              { return false; }
    // ------------------------------------------------------------------------
    /** Get the team of kart in world (including AIs) */
    KartTeam getKartTeam(unsigned int kart_id) const;
    // ------------------------------------------------------------------------
    int getTeamNum(KartTeam team) const;
    // ------------------------------------------------------------------------
    /** Set the network mode (true if networked) */
    void setNetworkWorld(bool is_networked) { m_is_network_world = is_networked; }
    // ------------------------------------------------------------------------
    bool isNetworkWorld() const { return m_is_network_world; }
    // ------------------------------------------------------------------------
    /** Set the team arrow on karts if necessary*/
    void initTeamArrows(AbstractKart* k);
    // ------------------------------------------------------------------------
    /** Used by server to get the current started game progress in either or
     *  both remaining time or progress in percent. uint32_t max for either or
     *  both if not available.  */
    virtual std::pair<uint32_t, uint32_t> getGameStartedProgress() const
    {
        return std::make_pair(std::numeric_limits<uint32_t>::max(),
            std::numeric_limits<uint32_t>::max());
    }
    // ------------------------------------------------------------------------
    virtual bool isGoalPhase() const { return false; }
};   // World

#endif

/* EOF */
