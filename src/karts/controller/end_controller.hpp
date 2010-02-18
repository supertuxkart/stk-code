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

#ifndef HEADER_END_KART_HPP
#define HEADER_END_KART_HPP

#include "karts/controller/controller.hpp"
#include "modes/profile_world.hpp"
#include "utils/vec3.hpp"

class Track;
class LinearWorld;
class QuadGraph;

namespace irr
{
    namespace scene
    {
        class ISceneNode;
    }
}

class EndController : public Controller
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

    float m_time_till_start; //Used to simulate a delay at the start of the
                             //race, since human players don't accelerate
                             //at the same time and rarely repeat the a
                             //previous timing.

    float m_curve_target_speed;
    float m_curve_angle;

    /** Keep a pointer to the track to reduce calls */
    Track       *m_track;

    /** Keep a pointer to world. */
    LinearWorld *m_world;
    /** The current node the kart is on. This can be different from the value
     *  in LinearWorld, since it takes the chosen path of the AI into account
     *  (e.g. the closest point in LinearWorld might be on a branch not
     *  chosen by the AI). */
    int   m_track_node;
    /** The graph of qudas of this track. */
    const QuadGraph *m_quad_graph;
    
    /** Which of the successors of a node was selected by the AI. */
    std::vector<int> m_successor_index;
    /** For each node in the graph this list contains the chosen next node.
     *  For normal lap track without branches we always have 
     *  m_next_node_index[i] = (i+1) % size;
     *  but if a branch is possible, the AI will select one option here. 
     *  If the node is not used, m_next_node_index will be -1. */
    std::vector<int> m_next_node_index;
    /** For each graph node this list contains a list of the next X
     *  graph nodes. */
    std::vector<std::vector<int> > m_all_look_aheads;

    float m_time_since_stuck;

    int m_start_kart_crash_direction; //-1 = left, 1 = right, 0 = no crash.

    /** Length of the kart, storing it here saves many function calls. */
    float m_kart_length;

    /** Cache width of kart. */
    float m_kart_width;

    /** For debugging purpose: a sphere indicating where the AI 
     *  is targeting at. */
    irr::scene::ISceneNode *m_debug_sphere;

    /** The minimum steering angle at which the AI adds skidding. Lower values
     *  tend to improve the line the AI is driving. This is used to adjust for
     *  different AI levels.
     */
    float m_skidding_threshold;

    /*Functions called directly from update(). They all represent an action
     *that can be done, and end up setting their respective m_controls
     *variable, except handle_race_start() that isn't associated with any
     *specific action (more like, associated with inaction).
     */
    void         handleAcceleration(const float DELTA);
    void         handleSteering(float dt);
    void         handleRescue(const float DELTA);
    void         handleBraking();
    /*Lower level functions not called directly from update()*/
    float        steerToAngle(const size_t SECTOR, const float ANGLE);
    float        steerToPoint(const Vec3 &point, float dt);

    void         checkCrashes(const int STEPS, const Vec3& pos);
    void         findNonCrashingPoint(Vec3 *result);
    float        normalizeAngle(float angle);
    int          calcSteps();
    void         setSteering(float angle, float dt);
    void         findCurve();
public:
                 EndController(Kart *kart, StateManager::ActivePlayer* player);
                ~EndController();
    virtual void update      (float delta) ;
    virtual void reset       ();
    /** Returns if the original controller of the kart was a player 
     *  controller. This way e.g. highscores can still be assigned
     *  to the right player. */
    virtual bool isPlayerController () const {return m_player!=NULL;}

};   // EndKart

#endif

/* EOF */
