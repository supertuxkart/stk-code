//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006-2015 Eduardo Hernandez Munoz
//  Copyright (C) 2010-2015 Joerg Henrichs
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

#ifndef HEADER_END_CONTROLLER_HPP
#define HEADER_END_CONTROLLER_HPP

#include "karts/controller/ai_base_lap_controller.hpp"

class Camera;
class LinearWorld;
class Track;
class Vec3;

namespace irr
{
    namespace scene
    {
        class ISceneNode;
    }
}

/**
  * \ingroup controller
  */
class EndController : public AIBaseLapController
{
private:
    /** Stores the type of the previous controller. This is necessary so that
     *  after the end of race ths kart (and its results) can still be
     *  identified to be from a player kart. */
    bool  m_was_player_controller;

    int   m_min_steps; //Minimum number of steps to check. If 0, the AI doesn't
                       //even has check around the kart, if 1, it checks around
                       //the kart always, and more than that will check the
                       //remaining number of steps in front of the kart, always
    float m_max_handicap_accel; //The allowed maximum speed, in percentage,
                                //from 0.0 to 1.0. Used only when
                                //m_wait_for_players == true.
    
    /*General purpose variables*/
    //The crash percentage is how much of the time the AI has been crashing,
    //if the AI has been crashing for some time, use the rescue.
    float m_crash_time;

    float m_time_since_stuck;

    /** Stores a pointer to the original controller. */
    Controller *m_previous_controller;

    /** For debugging purpose: a sphere indicating where the AI
     *  is targeting at. */
    irr::scene::ISceneNode *m_debug_sphere;

    /*Functions called directly from update(). They all represent an action
     *that can be done, and end up setting their respective m_controls
     *variable.
     */
    void          handleSteering(float dt);
    void          handleRescue(const float DELTA);

    void          checkCrashes(const int STEPS, const Vec3& pos);
    void          findNonCrashingPoint(Vec3 *result);
    int           calcSteps();
    virtual bool  canSkid(float steer_fraction) { return false; }
public:
                 EndController(AbstractKart *kart,
                               Controller *prev_controller);
                ~EndController();
    virtual void update      (float delta) ;
    virtual void reset       ();
    virtual void action      (PlayerAction action, int value);
    virtual void newLap      (int lap);
    // ------------------------------------------------------------------------
    virtual bool canGetAchievements() const
    {
        return m_previous_controller->canGetAchievements();
    }   // canGetAchievements
    // ------------------------------------------------------------------------
    /** Returns if the original controller of the kart was a player
     *  controller. This way e.g. highscores can still be assigned
     *  to the right player. */
    virtual bool isPlayerController () const
    {
        return m_previous_controller->isPlayerController();
    }   // isPlayerController
    // ------------------------------------------------------------------------
    /** Returns if the original controller of the kart was a local player
    *  controller. This way e.g. highscores can still be assigned
    *  to the right player. */
    virtual bool isLocalPlayerController () const
    {
        return m_previous_controller->isLocalPlayerController();
    }   // isLocalPlayerController
    // ------------------------------------------------------------------------
    /** Returns the name of the previous controller (which has the right
     *  player name associated). */
    core::stringw getName() const { return m_previous_controller->getName(); }

};   // EndKart

#endif

/* EOF */
