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

#include "tracks/arena_node.hpp"

// ----------------------------------------------------------------------------
ArenaNode::ArenaNode(const Vec3 &p0, const Vec3 &p1, const Vec3 &p2,
                     const Vec3 &p3, const Vec3 &normal,
                     unsigned int node_index)
         : Quad(p0, p1, p2, p3, normal, node_index)
{
    Vec3 lower_center = (p0 + p1) * 0.5f;
    Vec3 upper_center = (p2 + p3) * 0.5f;

    m_line = core::line3df(lower_center.toIrrVector(),
        upper_center.toIrrVector());
}   // ArenaNode

// ----------------------------------------------------------------------------
/** Returns the square of the distance between the given point and any point
 *  on the 'centre' line, i.e. the finite line from the middle point of the
 *  lower end of the node to the middle point of the upper end of the node
 *  which belongs to this node.
 *  \param xyz The point for which the distance to the line is computed.
 */
float ArenaNode::getDistance2FromPoint(const Vec3 &xyz) const
{
    core::vector3df closest = m_line.getClosestPoint(xyz.toIrrVector());
    return (closest-xyz.toIrrVector()).getLengthSQ();
}   // getDistance2FromPoint
