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

#ifndef HEADER_NODE_3D_HPP
#define HEADER_NODE_3D_HPP

#include "tracks/graph_node.hpp"
#include "utils/cpp2011.hpp"

/**
  * \ingroup tracks
  */
class Node3D : public GraphNode
{
private:
    /** For each node, construct a 3D box to check if a point lies inside it.
     */
    Vec3 m_box_faces[6][4];

    /** Line between lower and upper center, saves computation in
     *  getDistance() later.
     */
    core::line3df m_line;

public:
    Node3D(const Vec3 &p0, const Vec3 &p1, const Vec3 &p2, const Vec3 &p3,
           const Vec3 &normal, unsigned int node_index, bool invisible,
           bool ai_ignore);
    // ------------------------------------------------------------------------
    virtual bool pointInside(const Vec3& p,
                             bool ignore_vertical = false) const OVERRIDE
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
    // ------------------------------------------------------------------------
    virtual void getDistances(const Vec3 &xyz, Vec3 *result) OVERRIDE;
    // ------------------------------------------------------------------------
    virtual float getDistance2FromPoint(const Vec3 &xyz) OVERRIDE;

};
#endif
