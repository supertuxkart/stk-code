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

#include "tracks/node_3d.hpp"

// ----------------------------------------------------------------------------
Node3D::Node3D(const Vec3 &p0, const Vec3 &p1, const Vec3 &p2, const Vec3 &p3,
               const Vec3 &normal, unsigned int node_index, bool invisible,
               bool ai_ignore)
       : GraphNode(p0, p1, p2, p3, normal, node_index, invisible, ai_ignore)
{
    // Compute the node bounding box used for pointInNode
    Vec3 box_corners[8];
    float box_high = 5.0f;
    float box_low = 1.0f;
    box_corners[0] = m_p[0] + box_high * normal;
    box_corners[1] = m_p[1] + box_high * normal;
    box_corners[2] = m_p[2] + box_high * normal;
    box_corners[3] = m_p[3] + box_high * normal;
    box_corners[4] = m_p[0] - box_low * normal;
    box_corners[5] = m_p[1] - box_low * normal;
    box_corners[6] = m_p[2] - box_low * normal;
    box_corners[7] = m_p[3] - box_low * normal;

    Vec3 box_faces[6][4] =
    {
        { box_corners[0], box_corners[1], box_corners[2], box_corners[3] },
        { box_corners[3], box_corners[2], box_corners[6], box_corners[7] },
        { box_corners[7], box_corners[6], box_corners[5], box_corners[4] },
        { box_corners[1], box_corners[0], box_corners[4], box_corners[5] },
        { box_corners[4], box_corners[0], box_corners[3], box_corners[7] },
        { box_corners[1], box_corners[5], box_corners[6], box_corners[2] }
    };

    for (unsigned int i = 0; i < 6 ; i++)
    {
        for (unsigned int j = 0; j < 4; j++)
            m_box_faces[i][j] = box_faces[i][j];
    }

    m_line = core::line3df(m_lower_center.toIrrVector(),
        m_upper_center.toIrrVector());
}   // Node3D

// ----------------------------------------------------------------------------
/** Returns the distance a point has from this node in forward and sidewards
 *  direction, i.e. how far forwards the point is from the beginning of the
 *  node, and how far to the side from the line connecting the center points
 *  is it.
 *  \param xyz The coordinates of the point.
 *  \param result The X coordinate contains the sidewards distance, the
 *                Z coordinate the forward distance.
 */
void Node3D::getDistances(const Vec3 &xyz, Vec3 *result) const
{
    core::vector3df xyz_irr = xyz.toIrrVector();
    core::vector3df closest = m_line.getClosestPoint(xyz.toIrrVector());
    core::vector3df normal = getNormal().toIrrVector();

    if (xyz.sideofPlane(closest, closest + normal, m_line.end) < 0)
        result->setX( (closest-xyz_irr).getLength());   // to the right
    else
        result->setX(-(closest-xyz_irr).getLength());   // to the left
    result->setZ(m_distance_from_start +
        (closest-m_lower_center.toIrrVector()).getLength());
}   // getDistances

// ----------------------------------------------------------------------------
/** Returns the square of the distance between the given point and any point
 *  on the 'centre' line, i.e. the finite line from the middle point of the
 *  lower end of the node to the middle point of the upper end of the node
 *  which belongs to this node.
 *  \param xyz The point for which the distance to the line is computed.
 */
float Node3D::getDistance2FromPoint(const Vec3 &xyz) const
{
    core::vector3df closest = m_line.getClosestPoint(xyz.toIrrVector());
    return (closest-xyz.toIrrVector()).getLengthSQ();
}   // getDistance2FromPoint
