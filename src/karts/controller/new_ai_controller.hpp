//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006-2007 Eduardo Hernandez Munoz
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

#ifndef HEADER_NEW_AI_CONTROLLER_HPP
#define HEADER_NEW_AI_CONTROLLER_HPP

#include "karts/controller/ai_base_controller.hpp"
#include "utils/vec3.hpp"

/* third coord won't be used */

class LinearWorld;
class QuadGraph;
class Track;

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
class NewAIController : public AIBaseController
{
private:
    /** How the AI uses nitro. */
    enum {NITRO_NONE, NITRO_SOME, NITRO_ALL} m_nitro_level;
    enum ItemTactic
    {
        IT_TEN_SECONDS, //Fire after 10 seconds have passed, since the item
                        //was grabbed.
        IT_CALCULATE //Aim carefully, check for enough space for boosters,
                     //and that other conditions are meet before firing.
    };

    class CrashTypes
    {
        public:

        bool m_road; //true if we are going to 'crash' with the bounds of the road
        int m_kart; //-1 if no crash, pos numbers are the kart it crashes with
        CrashTypes() : m_road(false), m_kart(-1) {};
        void clear() {m_road = false; m_kart = -1;}
    } m_crashes;

    /*Difficulty handling variables*/
    float m_max_start_delay; //Delay before accelerating at the start of each
                             //race
    int m_min_steps; //Minimum number of steps to check. If 0, the AI doesn't
                     //even has check around the kart, if 1, it checks around
                     //the kart always, and more than that will check the
                     //remaining number of steps in front of the kart, always
    bool  m_wait_for_players; //If true, the acceleration is decreased when
                              //the AI is in a better position than all the
                              //human players.
    float m_max_handicap_accel; //The allowed maximum speed, in percentage,
                                //from 0.0 to 1.0. Used only when
                                //m_wait_for_players == true.    
    ItemTactic m_item_tactic; //How are items going to be used?

    /** True if the kart should try to pass on a bomb to another kart. */

    bool m_handle_bomb;
    /*General purpose variables*/
    //The crash percentage is how much of the time the AI has been crashing,
    //if the AI has been crashing for some time, use the rescue.
    float m_crash_time;
    int   m_collided;           // true if the kart collided with the track

    /** Pointer to the closest kart ahead of this kart. NULL if this
     *  kart is first. */
    AbstractKart *m_kart_ahead;
    /** Distance to the kart ahead. */
    float m_distance_ahead;

    /** Pointer to the closest kart behind this kart. NULL if this kart
     *  is last. */
    AbstractKart *m_kart_behind;
    /** Distance to the kard behind. */
    float m_distance_behind;

    /** Time an item has been collected and not used. */
    float m_time_since_last_shot;

    float m_time_till_start; //Used to simulate a delay at the start of the
                             //race, since human players don't accelerate
                             //at the same time and rarely repeat the a
                             //previous timing.

    float m_curve_target_speed;
    float m_curve_angle;

    /** The point the kart was aiming at when it was on track last. */
    Vec3  m_last_target_point;

    float m_time_since_stuck;

    int m_start_kart_crash_direction; //-1 = left, 1 = right, 0 = no crash.

    /** For debugging purpose: a sphere indicating where the AI 
     *  is targeting at. */
    irr::scene::ISceneNode *m_debug_sphere;
    irr::scene::ISceneNode *m_debug_left, *m_debug_right;

    /*Functions called directly from update(). They all represent an action
     *that can be done, and end up setting their respective m_controls
     *variable, except handle_race_start() that isn't associated with any
     *specific action (more like, associated with inaction).
     */
    void  handleRaceStart();
    void  handleAcceleration(const float DELTA);
    void  handleSteering(float dt);
    void  handleItems(const float DELTA, const int STEPS);
    void  handleRescue(const float DELTA);
    void  handleBraking();
    void  handleNitroAndZipper();
    void  computeNearestKarts();
    void  checkCrashes(const int STEPS, const Vec3& pos);
    float findNonCrashingAngle();

    int   calcSteps();
    void  findCurve();
protected:
    virtual unsigned int getNextSector(unsigned int index);

public:
                 NewAIController(AbstractKart *kart);
    virtual     ~NewAIController();
    virtual void update      (float delta) ;
    virtual void reset       ();
    virtual void crashed     (AbstractKart *k) {if(k) m_collided = true;};
    virtual const irr::core::stringw& getN() const 
    {
        static irr::core::stringw name("(NewAI)");
        return name;
    }   // getName
};

#endif

/* EOF */
