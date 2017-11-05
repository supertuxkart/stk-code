//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2015  Joerg Henrichs
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

#ifndef HEADER_CHECK_GOAL_HPP
#define HEADER_CHECK_GOAL_HPP

#include "tracks/check_structure.hpp"
#include "utils/cpp2011.hpp"
#include <line2d.h>
using namespace irr;

class XMLNode;
class Track;
class Vec3;

/**
 *  \brief Implements a simple checkline that will score a point when the
 *         soccer ball crosses it.
 *
 * \ingroup tracks
 */
class CheckGoal : public CheckStructure
{
public:
    /** Used by AIs to test whether the ball is likely to goal. */
    enum PointLocation
    {
        POINT_FIRST,
        POINT_CENTER,
        POINT_LAST
    };
private:
    /** Previois ball position. */
    Vec3            m_previous_ball_position;

    /** Which team is this goal for? */
    bool            m_first_goal;

    /** The line that is tested for being crossed. */
    core::line2df   m_line;

    /** Used by AIs to test whether the ball is likely to goal. */
    Vec3            m_p1;
    Vec3            m_p2;
    Vec3            m_p3;

public:
             CheckGoal(const XMLNode &node, unsigned int index);
    virtual ~CheckGoal() {}
    virtual void update(float dt) OVERRIDE;
    virtual void trigger(unsigned int kart_index) OVERRIDE;
    virtual bool isTriggered(const Vec3 &old_pos, const Vec3 &new_pos,
                             int indx) OVERRIDE;
    virtual void reset(const Track &track) OVERRIDE;

    // ------------------------------------------------------------------------
    bool getTeam() const                             { return m_first_goal; }
    // ------------------------------------------------------------------------
    const Vec3& getPoint(PointLocation point) const
    {
        return (point == POINT_LAST ? m_p3 :
            (point == POINT_CENTER ? m_p2 : m_p1));
    }
};   // CheckGoal

#endif
