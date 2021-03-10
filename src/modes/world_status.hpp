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

#ifndef HEADER_WORLD_STATUS_HPP
#define HEADER_WORLD_STATUS_HPP

#include "utils/cpp2011.hpp"
#include <atomic>

enum ProcessType : unsigned int;
class SFXBase;

/**
 * \brief A class that manages the clock (countdown, chrono, etc.)
 * Also manages stuff like the 'ready/set/go' text at the beginning or the delay at the end of a race.
 * \ingroup modes
 */
class WorldStatus
{
public:
    /** Different clock types for a race. */
    enum ClockType
    {
        CLOCK_NONE,
        CLOCK_CHRONO, // counts up
        CLOCK_COUNTDOWN
    };

    enum Phase {
        // Time for a camera movement, and introductory song
        TRACK_INTRO_PHASE,

        // Game setup, e.g. track loading
        SETUP_PHASE,

        // Used in network games only: wait for the server to broadcast
        // 'start'. This happens on a network client only
        WAIT_FOR_SERVER_PHASE,

        // Used in network games only: server is ready
        SERVER_READY_PHASE,

        // 'Ready' is displayed
        READY_PHASE,

        // 'Set' is displayed
        SET_PHASE,

        // 'Go' is displayed, but this is already race phase
        GO_PHASE,

        // Race is started, 'go' is gone, but music name is still there
        MUSIC_PHASE,

        // the actual race has started, no ready/set/go is displayed anymore
        RACE_PHASE,

        // All players have finished, now wait a certain amount of time for AI
        // karts to finish. If they do not finish in that time, finish the race
        // and estimate their arrival time.
        DELAY_FINISH_PHASE,

        // Display the results, while world is still being updated to
        // show the end animation
        RESULT_DISPLAY_PHASE,

        // The player crossed the finishing line and his and the time of
        // the other players is displayed, controll is automatic
        FINISH_PHASE,

        // Display the in-game menu, but no update of world or anything
        IN_GAME_MENU_PHASE,

        // Undefined, used in asserts to catch incorrect states.
        UNDEFINED_PHASE,
    };

protected:
    /** Elasped/remaining time in seconds. */
    double          m_time;

    /** Time in number of ticks (in terms of physics time steps). */
    int             m_time_ticks;

    /** If the start race should be played, disabled in cutscenes. */
    bool            m_play_racestart_sounds;

    /** Process type of this world (main or child). */
    const ProcessType m_process_type;

private:
    /** Sound to play at the beginning of a race, during which a
     *  a camera intro of the track can be shown. */
    SFXBase    *m_track_intro_sound;
    /** Sound used for the first two 'beeps' in ready, set, go. */
    SFXBase    *m_prestart_sound;
    /** The third sound to be played in ready, set, go. */
    SFXBase    *m_start_sound;

    /** (Unix) time when we started */
    uint64_t m_started_at;

    /** The clock mode: normal counting forwards, or countdown */ 
    ClockType       m_clock_mode;
protected:
    bool            m_play_track_intro_sound;
    bool            m_play_ready_set_go_sounds;
    std::atomic<Phase> m_phase;

private:

    /**
      * Remember previous phase e.g. on pause
      */
    Phase           m_previous_phase;

    /**
     * Counts time during the initial 'ready/set/go' phase, or at the end of a race.
     * This timer basically kicks in when we need to calculate non-race time like labels.
     */
    int             m_auxiliary_ticks;

    int             m_start_music_ticks;

    int             m_race_ticks;

    int             m_live_join_ticks;

    /** Special counter to count ticks since start (in terms of physics
     *  timestep size). */
    int             m_count_up_ticks;

    bool            m_engines_started;

    bool            m_live_join_world;

    void startEngines();

public:
             WorldStatus();
    virtual ~WorldStatus();

    virtual void reset(bool restart);
    virtual void updateTime(int ticks);
    virtual void update(int ticks);
    void         startReadySetGo();
    virtual void pause(Phase phase);
    virtual void unpause();
    virtual void enterRaceOverState();
    virtual void terminateRace();
    void         setTime(const float time);
    void         setTicks(int ticks);
    void         setTicksForRewind(int ticks);
    // ------------------------------------------------------------------------
    // Note: GO_PHASE is both: start phase and race phase
    bool     isStartPhase() const  { return m_phase<GO_PHASE;               }
    // ------------------------------------------------------------------------
    bool     isRacePhase()  const  { return m_phase>=GO_PHASE &&
                                            m_phase<FINISH_PHASE;           }
    // ------------------------------------------------------------------------
    bool     isActiveRacePhase() const { return m_phase>=GO_PHASE &&
                                                m_phase<DELAY_FINISH_PHASE; }
    // ------------------------------------------------------------------------
    /** While the race menu is being displayed, m_phase is limbo, and
     *  m_previous_phase is finish. So we have to test this case, too.  */
    bool     isFinishPhase() const { return m_phase==FINISH_PHASE ||
                                           (m_phase==IN_GAME_MENU_PHASE &&
                                            m_previous_phase==FINISH_PHASE);}
    // ------------------------------------------------------------------------
    /** Returns the current race phase. */
    const Phase getPhase() const  { return m_phase;                        }

    // ------------------------------------------------------------------------
    /** Sets the current race phase. Canbe used to e.g. avoid the count down
     *  etc. */
    void setPhase(Phase phase) { m_phase = phase; }

    // ------------------------------------------------------------------------
    /** Call to specify what kind of clock you want. The second argument
     *  can be used to specify the initial time value (especially useful
     *  for countdowns). */
    void    setClockMode(const ClockType mode, const float initial_time=0.0f);

    // ------------------------------------------------------------------------
    /** Returns the current clock mode. */
    int     getClockMode() const { return m_clock_mode; }

    // ------------------------------------------------------------------------
    /** Returns the current race time. */
    float   getTime() const { return (float)m_time; }

    // ------------------------------------------------------------------------
    /** Returns the start time. */
    uint64_t   getStart() const { return m_started_at; }

    // ------------------------------------------------------------------------
    /** Returns the current race time in time ticks (i.e. based on the physics
     *  time step size). */
    int getTimeTicks() const { return m_time_ticks; }

    // ------------------------------------------------------------------------
    /** Will be called to notify your derived class that the clock,
     *  which is in COUNTDOWN mode, has reached zero. */
    virtual void countdownReachedZero() {};

    // ------------------------------------------------------------------------
    /** Called when the race actually starts. */
    virtual void onGo() {};

    // ------------------------------------------------------------------------
    /** Get the ticks since start regardless of which way the clock counts */
    int getTicksSinceStart() const { return m_count_up_ticks; }
    // ------------------------------------------------------------------------
    int getAuxiliaryTicks() const { return m_auxiliary_ticks; }
    // ------------------------------------------------------------------------
    bool isLiveJoinWorld() const { return m_live_join_world; }
    // ------------------------------------------------------------------------
    void setLiveJoinWorld(bool val) { m_live_join_world = val; }
    // ------------------------------------------------------------------------
    int getMusicDescriptionTicks() const
    {
        return m_live_join_ticks == -1 ?
            m_count_up_ticks : m_count_up_ticks - m_live_join_ticks;
    }
    // ------------------------------------------------------------------------
    void endLiveJoinWorld(int ticks_now);
};   // WorldStatus


#endif
