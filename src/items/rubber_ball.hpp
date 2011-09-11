//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011 Joerg Henrichs
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

#ifndef HEADER_RUBBER_BALL_HPP
#define HEADER_RUBBER_BALL_HPP

#include <irrString.h>

#include "items/flyable.hpp"
#include "tracks/track_sector.hpp"

class Kart;
class QuadGraph;
class SFXBase;

/**
  * \ingroup items
  */
class RubberBall: public Flyable, public TrackSector
{
private:
    /** Used in case of flyable debugging so that each output line gets
     *  a unique number for each ball. */
    int m_id;

    /** A class variable which stores the next id number to use. */
    static int m_next_id;

    /** A class variable to store the default interval size. */
    static float m_st_interval;
    
    /** A class variable to store the default squash duration. */
    static float m_st_squash_duration;
    
    /** A class variable to store the default squash slowdown. */
    static float m_st_squash_slowdown;

    /** If the ball is closer than this distance to the target, it will
     *  start to aim directly at the target (and not interpolate anymore). */
    static float m_st_target_distance;

    /** Each control point chosen must be at least this far away from
     *  the previous one. This gives smooth 'overall' interpolation
     *  even if the quads should be close to each other. */
    static float m_st_min_interpolation_distance;

    /** A pointer to the target kart. */
    const Kart  *m_target;

    /** The last graph node who's coordinates are stored in
     *  m_control_points[3]. */
    int          m_last_aimed_graph_node;

    /** Keep the last two, current, and next aiming points 
     *  for interpolation. */
    Vec3         m_control_points[4];

    /** Saves the previous location of the ball. This is needed if a ball
     *  should lose it target, and has to reinitialise the control points
     *  for the interpolation. */
    Vec3         m_previous_xyz;

    /** Estimated length of the spline between the control points
     *  1 and 2. */
    float        m_length_cp_1_2;

    /** Estimated length of the spline between the control points
     *  2 and 3. */
    float        m_length_cp_2_3;

    /** The parameter for the spline, m_t in [0,1]. This is not directly
     *  related to the time, since depending on the distance between 
     *  the two control points different increments must be used for m_t. 
     *  For example, if the distance is 10 m, and assuming a speed of
     *  10 m/s for the ball, then each second must add '1' to m_t. If
     *  the distance on the other hand is 200 m, then 10/200 = 1/20 per
     *  second must be added (so that it takes 20 seconds to move from 0
     *  to 1). */
    float        m_t;

    /** How much m_t must increase per second in order to maintain a
     *  constant speed, i.e. the speed of the ball divided by the
     *  distance between the control points. See m_t for more details. */
    float        m_t_increase;

    /** How long it takes from one bounce of the ball to the next. */
    float        m_interval;

    /** This timer is used to determine the height depending on the time.
     *  It is always between 0 and m_interval. */
    float        m_timer;

    /** The current maximum height of the ball. This value will be 
     *  reduced if the ball gets closer to the target. */
    float        m_current_max_height;

    /** Once the ball is close enough, it will aim for the kart. If the
     *  kart should be able to then increase the distance to the ball,
     *  the ball will be removed and the kart escapes. This boolean is
     *  used to keep track of the state of this ball. */
    bool         m_aiming_at_target;

    /** A 'ping' sound effect to be played when the ball hits the ground. */
    SFXBase     *m_ping_sfx;

    /** Sound effect to be played when a ball hits a kart. */
    SFXBase     *m_hit_sfx;
    
    Track       *m_track;

    void         computeTarget();
    void         checkDistanceToTarget();
    unsigned int getSuccessorToHitTarget(unsigned int node_index, 
                                         float *f=NULL);
    void         getNextControlPoint();
    float        updateHeight();
    void         interpolate(Vec3 *next_xyz, float dt);
    void         initializeControlPoints(const Vec3 &xyz);
	float        getMaxTerrainHeight() const;
public:
                 RubberBall  (Kart* kart, Track* track);
    virtual     ~RubberBall();
    static  void init(const XMLNode &node, scene::IMesh *bowling);
    virtual bool updateAndDelete(float dt);
    virtual bool hit(Kart* kart, PhysicalObject* obj=NULL);
    virtual const core::stringw getHitString(const Kart *kart) const;
    // ------------------------------------------------------------------------
    /** This object does not create an explosion, all affects on
     *  karts are handled by this hit() function. */
    virtual HitEffect *getHitEffect() const {return NULL; }

};   // RubberBall

#endif
