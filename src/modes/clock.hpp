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

#ifndef CLOCK_H
#define CLOCK_H

class SFXBase;

enum ClockType
{
    CLOCK_NONE,
    CHRONO, // counts up
    COUNTDOWN
};

enum Phase {
    // Game setup, e.g. track loading
    SETUP_PHASE,
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
    DELAY_FINISH_PHASE,
    // The player crossed the finishing line and his and the time of
    // the other players is displayed, controll is automatic
    FINISH_PHASE,
    // The state after finish where no calculations are done.
    LIMBO_PHASE,
};

/**
 * A class that manages the clock (countdown, chrono, etc.) Also manages stuff
 * like the 'ready/set/go' text at the beginning or the delay at the end of a race.
 */
class TimedRace
{
protected:
    SFXBase    *m_prestart_sound;
    SFXBase    *m_start_sound;
    
    /**
        * Elasped/remaining time in seconds
     */
    float           m_time;
    ClockType       m_mode;
    

    Phase           m_phase;

    /**
        * Remember previous phase e.g. on pause
     */
    Phase          m_previous_phase;
public:
    TimedRace();
    virtual ~TimedRace();
    
    void reset();
    
    // Note: GO_PHASE is both: start phase and race phase
    bool    isStartPhase() const  { return m_phase<GO_PHASE;             }
    bool    isRacePhase()  const  { return m_phase>=GO_PHASE && 
                                           m_phase<FINISH_PHASE;          }
    /** While the race menu is being displayed, m_phase is limbo, and
     *  m_previous_phase is finish. So we have to test this case, too.  */
    bool    isFinishPhase() const { return m_phase==FINISH_PHASE ||
                                          (m_phase==LIMBO_PHASE &&
                                           m_previous_phase==FINISH_PHASE);}
    const Phase getPhase() const  { return m_phase;                      }
    
    /**
     * Counts time during the initial 'ready/set/go' phase, or at the end of a race.
     * This timer basically kicks in when we need to calculate non-race time like labels.
     */
    float           m_auxiliary_timer;
    
    /**
     * Call to specify what kind of clock you want. The second argument
     * can be used to specify the initial time value (especially useful
                                                      * for countdowns)
     */
    void    setClockMode(const ClockType mode, const float initial_time=0.0f);
    int     getClockMode() const { return m_mode; }
    /**
        * Call each frame, with the elapsed time as argument.
     */
    void    update(const float dt);
    
    float   getTime() const                 { return m_time; }
    void    setTime(const float time);
    
    void    pause();
    void    unpause();
    
    /** Gets called when the race is about to finish (but with the option of adding
        * some delay to watch the end of the race. */
    virtual void enterRaceOverState(const bool delay=false);

    /** Called when it's really over (delay over if any)
        */
    virtual void terminateRace() = 0;
    
    /*
     * Will be called to notify your derived class that the clock,
     * which is in COUNTDOWN mode, has reached zero.
     */
    virtual void countdownReachedZero() {};
    
    /*
     * Called when the race actually starts.
     */
    virtual void onGo() {};
    
    
};


#endif
