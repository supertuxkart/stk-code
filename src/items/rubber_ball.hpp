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

class Kart;
class QuadGraph;

/**
  * \ingroup items
  */
class RubberBall: public Flyable
{
private:
    /** A pointer to the target kart. */
    const Kart  *m_target;

    /** The current quad this ball is on. */
    int          m_current_graph_node;

    /** The current quad this ball is aiming at. */
    int          m_aimed_graph_node;

    /** The previous distance to the graph node we are aiming
     *  at atm. If the distance increases, we have passed the
     *  point we aimed at and have to aim at the next point. */
    float        m_distance_along_track;

    /** Since the distance between target and ball can vary a bit,
     *  using it to determine height on a formula is not smooth (e.g.
     *  ball can actually go up a bit during one frame while actually 
     *  going down). Therefore we use a weighted average of previous
     *  height newly determined height. */
    float        m_previous_height;

    /** True if the ball just crossed the start line, i.e. its
     *  distance changed from close to length of track in the
     *  previous time step to a bit over zero now. */
    bool         m_wrapped_around;

    /** Once the ball is close enough, it will aim for the kart. If the
     *  kart should be able to then increase the distance to the ball,
     *  the ball will be removed and the kart escapes. This boolean is
     *  used to keep track of the state of this ball. */
    bool         m_aiming_at_target;

    /** For convenience keep a pointer to the quad graph. */
    QuadGraph   *m_quad_graph;

    void         computeTarget();
    void         determineTargetCoordinates(float dt, Vec3 *aim_xyz);
public:
                 RubberBall  (Kart* kart);
    static  void init(const XMLNode &node, scene::IMesh *bowling);

    virtual void update      (float dt);
};   // RubberBall

#endif
