//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2015 Joerg Henrichs
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

#ifndef HEADER_DEMO_WORLD_HPP
#define HEADER_DEMO_WORLD_HPP

#include "modes/profile_world.hpp"

class Kart;

/** \brief This class is used to show a demo of STK (e.g. after being
 *  idle for a certain amount of time, or on certain command line options.
 *  This allows STK to be used as a demo in a shop.
 *  \ingroup modes
 */
class DemoWorld : public ProfileWorld
{
private:
    /** True if demo mode should be aborted, e.g. because a key was
     *  pressed. */
    bool   m_abort;

    /** The list of tracks to use for demo mode. They will be picked randomly,
     *  but each track will be picked once before one track is repeated. */
    static std::vector<std::string> m_demo_tracks;

    /** Number of karts to use in demo mode. */
    static int m_num_karts;

    /** Idle time after which demo mode should be started. */
    static float m_max_idle_time;

    /** Used by the menu system to measure idle time. */
    static float m_current_idle_time;

    /** True if the next race is to be a demo race. */
    static bool m_do_demo;
public:
                          DemoWorld();
    virtual              ~DemoWorld();
    /** Returns identifier for this world. */
    virtual  std::string getInternalCode() const OVERRIDE { return "DEMO"; }
    virtual  void        update(float dt) OVERRIDE {ProfileWorld::update(dt);};
    virtual  bool        isRaceOver() OVERRIDE;
    virtual  void        enterRaceOverState() OVERRIDE;
    // ------------------------------------------------------------------------
    static   bool        updateIdleTimeAndStartDemo(float dt);
    // ------------------------------------------------------------------------
    /** Sets the number of laps to use in demo mode. m_num_laps is from
     *  ProfileWorld. */
    static void setNumLaps(unsigned int num_laps) { m_num_laps = num_laps; }
    // ------------------------------------------------------------------------
    /** Sets the number of karts to use in demo mode. */
    static void setNumKarts(unsigned int num_karts) { m_num_karts = num_karts;}
    // ------------------------------------------------------------------------
    static void setTracks(const std::vector<std::string> &tracks);
    // ------------------------------------------------------------------------
    /** Enables demo mode after the specified amount of time (default 1
     *  second).
     *  \param time The idle time after which demo mode should be started. */
    static void enableDemoMode(float time=1.0f) { m_max_idle_time = time; }
    // ------------------------------------------------------------------------
    /** Returns true if the menu detected enough idle time to run a demo. */
    static bool isDemoMode() { return m_do_demo; }
    // ------------------------------------------------------------------------
    /** Resets the idle time to 0. */
    static void resetIdleTime() { m_current_idle_time = 0; }
    // ------------------------------------------------------------------------
    /** Signals that the demo should be aborted. */
    void abortDemo() {m_abort = true; }
};   // DemoWorld

#endif
