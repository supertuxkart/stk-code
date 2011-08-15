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

#include "items/flyable.hpp"
#include "tracks/track_sector.hpp"

class Kart;
class QuadGraph;

/**
  * \ingroup items
  */
class RubberBall: public Flyable, public TrackSector
{
private:
    /** A class variable to store the default interval size. */
    static float m_st_interval;
    
    /** A class variable to store the default squash duration. */
    static float m_st_squash_duration;
    
    /** A class variable to store the default squash slowdown. */
    static float m_st_squash_slowdown;

    /** A pointer to the target kart. */
    const Kart  *m_target;

    /** The last graph node who's coordinates are stored in
     *  m_control_points[3]. */
    int          m_last_aimed_graph_node;

    /** Keep the last two, current, and next aiming points 
      * for interpolation. */
    Vec3         m_control_points[4];

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

    /** How much m_tt must increase per second in order to maintain a
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

    /** True if the ball just crossed the start line, i.e. its
     *  distance changed from close to length of track in the
     *  previous time step to a bit over zero now. */
    bool         m_wrapped_around;

    /** Once the ball is close enough, it will aim for the kart. If the
     *  kart should be able to then increase the distance to the ball,
     *  the ball will be removed and the kart escapes. This boolean is
     *  used to keep track of the state of this ball. */
    bool         m_aiming_at_target;

    void         computeTarget();
    void         determineTargetCoordinates(float dt, Vec3 *aim_xyz);
    unsigned int getSuccessorToHitTarget(unsigned int node_index, 
                                         float *f=NULL);
    void         getNextControlPoint();
    float        updateHeight();
public:
                 RubberBall  (Kart* kart);
    static  void init(const XMLNode &node, scene::IMesh *bowling);
    virtual void update      (float dt);
    virtual void hit         (Kart* kart, PhysicalObject* obj=NULL);
    /** This object does not create an explosion, all affects on
     *  karts are handled by this hit() function. */
    virtual bool needsExplosion() const {return false;}

};   // RubberBall

#endif
