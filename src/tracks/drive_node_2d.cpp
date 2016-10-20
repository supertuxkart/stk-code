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

#include "tracks/drive_node_2d.hpp"

// ----------------------------------------------------------------------------
DriveNode2D::DriveNode2D(const Vec3 &p0, const Vec3 &p1, const Vec3 &p2,
                         const Vec3 &p3, const Vec3 &normal,
                         unsigned int node_index, bool invisible,
                         bool ai_ignore, bool ignored)
           : DriveNode(p0, p1, p2, p3, normal, node_index, invisible,
                       ai_ignore, ignored)
{
    m_line = core::line2df(m_upper_center.getX(), m_upper_center.getZ(),
        m_lower_center.getX(), m_lower_center.getZ());

    // Only this 2d point is needed later
    m_lower_center_2d = core::vector2df(m_lower_center.getX(),
        m_lower_center.getZ());
}   // DriveNode2D

// ----------------------------------------------------------------------------
/** Returns the distance a point has from this node in forward and sidewards
 *  direction, i.e. how far forwards the point is from the beginning of the
 *  node, and how far to the side from the line connecting the center points
 *  is it. All these computations are done in 2D only.
 *  \param xyz The coordinates of the point.
 *  \param result The X coordinate contains the sidewards distance, the
 *                Z coordinate the forward distance.
 */
void DriveNode2D::getDistances(const Vec3 &xyz, Vec3 *result) const
{
    core::vector2df xyz2d(xyz.getX(), xyz.getZ());
    core::vector2df closest = m_line.getClosestPoint(xyz2d);
    if (m_line.getPointOrientation(xyz2d) > 0)
        result->setX( (closest-xyz2d).getLength());   // to the right
    else
        result->setX(-(closest-xyz2d).getLength());   // to the left

    result->setZ(m_distance_from_start +
        (closest-m_lower_center_2d).getLength());
}   // getDistances

// ----------------------------------------------------------------------------
/** Returns the square of the distance between the given point and any point
 *  on the 'centre' line, i.e. the finite line from the middle point of the
 *  lower end of the node to the middle point of the upper end of the node
 *  which belongs to this graph node. The value is computed in 2d only!
 *  \param xyz The point for which the distance to the line is computed.
 */
float DriveNode2D::getDistance2FromPoint(const Vec3 &xyz) const
{
    core::vector2df xyz2d(xyz.getX(), xyz.getZ());
    core::vector2df closest = m_line.getClosestPoint(xyz2d);
    return (closest-xyz2d).getLengthSQ();
}   // getDistance2FromPoint
