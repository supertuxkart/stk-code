//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Joerg Henrichs
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

#ifndef HEADER_NAV_POLY_HPP
#define HEADER_NAV_POLY_HPP

#include "tracks/navmesh.hpp"

#include <vector>
#include <string>

#include "utils/vec3.hpp"


class NavPoly
{
private:

    /** Holds the index of vertices for a polygon **/
    std::vector<int> m_vertices;

    /** Center of this polygon. **/
    Vec3 m_center;

    /** Holds the index of adjacent polyogns **/
    std::vector<int> m_adjacents;

public:
    NavPoly(const  std::vector<int> &polygonVertIndices,  
            const std::vector<int> &adjacentPolygonIndices);
    
    const Vec3&        getCenter() const   {return m_center;}
    const std::vector<int>&     getAdjacents() const {return m_adjacents;}
    const std::vector<Vec3>     getVertices();
    bool                        pointInPoly(const Vec3& p) const;
};


#endif