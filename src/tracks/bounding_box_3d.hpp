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

#ifndef HEADER_NODE_BOUNDING_BOX_3D_HPP
#define HEADER_NODE_BOUNDING_BOX_3D_HPP

#include "utils/vec3.hpp"

/**
  * \ingroup tracks
  */
class BoundingBox3D
{
private:
    /** A 3D box using to check if a point lies inside a quad.
     */
    Vec3 m_box_faces[6][4];

public:
    // ------------------------------------------------------------------------
    BoundingBox3D(const Vec3& p0, const Vec3& p1, const Vec3& p2,
                  const Vec3& p3, const Vec3& normal)
    {
        // Compute the node bounding box used by pointInside
        Vec3 box_corners[8];
        float box_high = 5.0f;
        float box_low = 1.0f;
        box_corners[0] = p0 + box_high * normal;
        box_corners[1] = p1 + box_high * normal;
        box_corners[2] = p2 + box_high * normal;
        box_corners[3] = p3 + box_high * normal;
        box_corners[4] = p0 - box_low * normal;
        box_corners[5] = p1 - box_low * normal;
        box_corners[6] = p2 - box_low * normal;
        box_corners[7] = p3 - box_low * normal;

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
    }
    // ------------------------------------------------------------------------
    bool pointInside(const Vec3& p, bool ignore_vertical = false) const
    {
        float side = p.sideofPlane(m_box_faces[0][0], m_box_faces[0][1],
            m_box_faces[0][2]);
        for (int i = 1; i < 6; i++)
        {
            if (side * p.sideofPlane(m_box_faces[i][0], m_box_faces[i][1],
                m_box_faces[i][2]) < 0)
                return false;
        }
        return true;
    }

};

#endif