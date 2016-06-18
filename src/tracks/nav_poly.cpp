//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Joerg Henrichs
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

#include "tracks/nav_poly.hpp"
#include "tracks/navmesh.hpp"

#include <algorithm>
#include <iostream>

/** Constructor that takes a vector of points and a vector of adjacent polygons */
NavPoly::NavPoly(const  std::vector<int> &polygonVertIndices,
                 const std::vector<int> &adjacentPolygonIndices)
{
    m_vertices = polygonVertIndices;

    m_adjacents = adjacentPolygonIndices;

    std::vector<Vec3> xyz_points = getVertices();

    Vec3 temp(0.0f,0.0f,0.0f);
    for(unsigned int i=0; i<xyz_points.size(); i++)
        temp = temp + xyz_points[i];

    m_center = (temp)*( 1.0f / xyz_points.size());

}

//-----------------------------------------------------------------------------

const std::vector<Vec3>  NavPoly::getVertices()
{
    std::vector<Vec3> points;
    for(unsigned int i=0; i<m_vertices.size(); i++)
        points.push_back(NavMesh::get()->getVertex(m_vertices[i]));
    return points;
}

//-----------------------------------------------------------------------------

bool NavPoly::pointInPoly(const Vec3& p, bool ignore_vertical) const
{
    std::vector<Vec3> points;
    for(unsigned int i=0; i<m_vertices.size(); i++)
        points.push_back(NavMesh::get()->getVertex(m_vertices[i]));

    // The point is on which side of the first edge
    float side = p.sideOfLine2D(points[0],points[1]);

    // The point is inside the polygon if it is on the same side for all edges
    for(unsigned int i=1; i<points.size(); i++)
    {
        // If it is on different side then product is < 0 , return false
        if(p.sideOfLine2D(points[i % points.size()],
                            points[(i+1)% points.size()]) * side < 0)
            return false;
    }

    if (ignore_vertical) return true;

    // Check for vertical distance too
    const float dist = p.getY() - m_center.getY();
    if (fabsf(dist) > 1.0f)
        return false;
    return true;
}

//-----------------------------------------------------------------------------

const Vec3& NavPoly::operator[](int i) const
{
    return NavMesh::get()->getVertex(m_vertices[i]);
}
