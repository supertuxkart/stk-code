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

#ifndef HEADER_ARENA_NODE_3D_HPP
#define HEADER_ARENA_NODE_3D_HPP

#include "tracks/arena_node.hpp"
#include "tracks/bounding_box_3d.hpp"

/**
  * \ingroup tracks
  */
class ArenaNode3D : public ArenaNode,
                    public BoundingBox3D
{
public:
    ArenaNode3D(const Vec3 &p0, const Vec3 &p1, const Vec3 &p2, const Vec3 &p3,
                const Vec3 &normal, unsigned int node_index)
    : ArenaNode(p0, p1, p2, p3, normal, node_index),
      BoundingBox3D(p0, p1, p2, p3, normal) {}
    // ------------------------------------------------------------------------
    virtual bool pointInside(const Vec3& p,
                             bool ignore_vertical = false) const OVERRIDE
    {
        return BoundingBox3D::pointInside(p);
    }
    // ------------------------------------------------------------------------
    virtual bool is3DQuad() const OVERRIDE                     { return true; }

};

#endif
