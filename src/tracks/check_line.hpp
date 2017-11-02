//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015  Joerg Henrichs
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

#ifndef HEADER_CHECK_LINE_HPP
#define HEADER_CHECK_LINE_HPP

#include <IMeshSceneNode.h>
#include <line2d.h>
#include <vector2d.h>
using namespace irr;

#include "tracks/check_structure.hpp"

class XMLNode;
class CheckManager;

/**
 *  \brief Implements a simple checkline.
 *  It's a finite line with 2 endpoints in 2d
 *  and a minimum height (i.e. the minimum Y coordinate of the two points).
 *  If a kart crosses the line (in the projection on the 2d plane) and has an
 *  appropriate height, the checkline will be triggered. This allows for very
 *  easy checking of checklines, and should be sufficient for most check
 *  structure.
 *
 * \ingroup tracks
 */
class CheckLine : public CheckStructure
{
private:
    /** The line that is tested for being crossed. */
    core::line2df   m_line;

    /** True if this line should ignore the height test. This is required
     *  e.g. for basketball cannons, since the ball can be too height to
     *  otherwise trigger the cannon. */
    bool            m_ignore_height;

    /** The minimum height of the checkline. */
    float           m_min_height;

    /** The actual (or estimated) left and right end points in 3d. This is
     *  used by the cannon. If the xml file stores only the min_height, those
     *  points are set from the 2d points and the min height. */
    Vec3            m_left_point, m_right_point;

    /** Stores the sign (i.e. side) of the previous line to save some
     *  computations. True if the value is >=0, i.e. the point is on
     *  or to the right of the line. */
    std::vector<bool> m_previous_sign;

    /** Used to display debug information about checklines. */
    scene::IMeshSceneNode *m_debug_node;

    /** How much a kart is allowed to be under the minimum height of a
     *  quad and still considered to be able to cross it. */
    static const int m_under_min_height = 1;

    /** How much a kart is allowed to be over the minimum height of a
     *  quad and still considered to be able to cross it. */
    static const int m_over_min_height  = 4;
protected:
    const Vec3 &getLeftPoint() const { return m_left_point;  }
    const Vec3 &getRightPoint() const { return m_right_point; }

public:
                 CheckLine(const XMLNode &node, unsigned int index);
    virtual     ~CheckLine();
    virtual bool isTriggered(const Vec3 &old_pos, const Vec3 &new_pos,
                             int indx);
    virtual void reset(const Track &track);
    virtual void resetAfterKartMove(unsigned int kart_index);
    virtual void changeDebugColor(bool is_active);
    virtual bool triggeringCheckline() const { return true; }
    // ------------------------------------------------------------------------
    /** Returns the actual line data for this checkpoint. */
    const core::line2df &getLine2D() const {return m_line;}
    // ------------------------------------------------------------------------
    /** Sets if this check line should not do a height test for testing
     *  if a line is crossed. Used for basket calls in cannon (the ball can
     *  be too heigh to otherwise trigger he cannon). */
    void setIgnoreHeight(bool b) { m_ignore_height = b;  }
};   // CheckLine

#endif

