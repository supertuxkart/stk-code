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
    
    /** A pointer to the target kart. */
    const Kart  *m_target;

    /** The current quad this ball is on. */
    int          m_current_graph_node;

    /** The current quad this ball is aiming at. */
    int          m_aimed_graph_node;

    /** Keep the last two, current, and next aiming points 
      * for interpolation. */
    Vec3         m_aiming_points[4];

    /** The parameter for the spline, m_t in [0,1]. */
    float        m_t;

    /** How much m_tt must increase per second in order to maintain a
     *  constant speed. */
    float        m_t_increase;


    /** The previous distance to the graph node we are aiming
     *  at atm. If the distance increases, we have passed the
     *  point we aimed at and have to aim at the next point. */
    float        m_distance_along_track;

    /** A class variable to store the default squash slowdown. */
    static float m_st_squash_slowdown;

    /** How long it takes from one bounce of the ball to the next. */
    float        m_interval;

    /** This timer is used to determine the height depending on the time.
     *  It is always between 0 and m_interval. */
    float        m_timer;

    /** The maximum height of the ball. This value will be reduced if the
     *  ball gets closer to the target. */
    float        m_height;

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
    unsigned int getSuccessorToHitTarget(unsigned int node_index);
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
