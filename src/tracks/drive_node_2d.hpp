//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2016 SuperTuxKart-Team
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

#ifndef HEADER_DRIVE_NODE_2D_HPP
#define HEADER_DRIVE_NODE_2D_HPP

#include "tracks/drive_node.hpp"
#include "utils/cpp2011.hpp"

#include <line2d.h>

/**
  * \ingroup tracks
  */
class DriveNode2D : public DriveNode
{
private:
    /** The center point of the lower two points (e.g. points 0 and 1).
     *  This saves some computations in getDistances later. Only the
     *  start point is needed, and only in 2d. */
    core::vector2df m_lower_center_2d;

    /** Line between lower and upper center, saves computation in
     *  getDistance() later. The line is 2d only since otherwise taller karts
     *  would have a larger distance from the center. It also saves
     *  computation, and it is only needed to determine the distance from the
     *  center of the drivelines anyway. */
    core::line2df m_line;

public:
    DriveNode2D(const Vec3 &p0, const Vec3 &p1, const Vec3 &p2, const Vec3 &p3,
                const Vec3 &normal, unsigned int node_index, bool invisible,
                bool ai_ignore, bool ignored);
    // ------------------------------------------------------------------------
    virtual void getDistances(const Vec3 &xyz, Vec3 *result) const OVERRIDE;
    // ------------------------------------------------------------------------
    virtual float getDistance2FromPoint(const Vec3 &xyz) const OVERRIDE;

};
#endif
