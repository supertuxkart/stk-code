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

#ifndef HEADER_DRIVE_NODE_3D_HPP
#define HEADER_DRIVE_NODE_3D_HPP

#include "tracks/bounding_box_3d.hpp"
#include "tracks/drive_node.hpp"
#include "utils/cpp2011.hpp"

#include <line3d.h>

/**
  * \ingroup tracks
  */
class DriveNode3D : public DriveNode,
                    public BoundingBox3D
{
private:
    /** Line between lower and upper center, saves computation in
     *  getDistance() later.
     */
    core::line3df m_line;

public:
    DriveNode3D(const Vec3 &p0, const Vec3 &p1, const Vec3 &p2, const Vec3 &p3,
                const Vec3 &normal, unsigned int node_index, bool invisible,
                bool ai_ignore, bool ignored);
    // ------------------------------------------------------------------------
    virtual bool pointInside(const Vec3& p,
                             bool ignore_vertical = false) const OVERRIDE
    {
        return BoundingBox3D::pointInside(p);
    }
    // ------------------------------------------------------------------------
    virtual void getDistances(const Vec3 &xyz, Vec3 *result) const OVERRIDE;
    // ------------------------------------------------------------------------
    virtual float getDistance2FromPoint(const Vec3 &xyz) const OVERRIDE;
    // ------------------------------------------------------------------------
    virtual bool is3DQuad() const OVERRIDE                     { return true; }

};
#endif
