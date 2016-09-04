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

#ifndef HEADER_QUAD_HPP
#define HEADER_QUAD_HPP

#include <SColor.h>

#include "utils/leak_check.hpp"
#include "utils/no_copy.hpp"
#include "utils/vec3.hpp"

namespace irr
{
    namespace video { struct S3DVertex; }
}
using namespace irr;

class btTransform;

/**
  * \ingroup tracks
  */
class Quad : public NoCopy
{
protected:
    /** The four points of a quad. */
    Vec3 m_p[4];

    /** The center of all four points, which is used by the AI.
     *  This saves some computations at runtime. */
    Vec3 m_center;

private:
    /** The minimum height of the quad, used in case that several quads
     *  are on top of each other when determining the sector a kart is on. */
    float m_min_height;

    /** The maximum height of the quad, used together with m_min_height
     *  to distinguish between quads which are on top of each other. */
    float m_max_height;

public:
    LEAK_CHECK()
    // ------------------------------------------------------------------------
         Quad(const Vec3 &p0, const Vec3 &p1, const Vec3 &p2, const Vec3 &p3);
    // ------------------------------------------------------------------------
         virtual ~Quad() {}
    // ------------------------------------------------------------------------
    void getVertices(video::S3DVertex *v, const video::SColor &color) const;
    // ------------------------------------------------------------------------
    void transform(const btTransform &t, Quad *result) const;
    // ------------------------------------------------------------------------
    /** Returns the i-th. point of a quad. */
    const Vec3& operator[](int i) const                      { return m_p[i]; }
    // ------------------------------------------------------------------------
    /** Returns the center of a quad. */
    const Vec3& getCenter ()      const                    { return m_center; }
    // ------------------------------------------------------------------------
    /** Returns the minimum height of a quad. */
    float       getMinHeight() const                   { return m_min_height; }
    // ------------------------------------------------------------------------
    virtual bool pointInside(const Vec3& p,
                             bool ignore_vertical = false) const;

};   // class Quad
#endif
