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

    /** Index of this quad, used only with graph. */
    int m_index;

    /** Normal of the quad */
    Vec3 m_normal;

private:
    /** Set to true if this quad should not be shown in the minimap. */
    bool m_invisible;

    bool m_is_ignored;

    /** The minimum height of the quad, used in case that several quads
     *  are on top of each other when determining the sector a kart is on. */
    float m_min_height, m_min_height_testing;

    /** The maximum height of the quad, used together with m_min_height
     *  to distinguish between quads which are on top of each other. */
    float m_max_height, m_max_height_testing;

public:
    LEAK_CHECK()
    // ------------------------------------------------------------------------
    Quad(const Vec3 &p0, const Vec3 &p1, const Vec3 &p2, const Vec3 &p3,
         const Vec3 & normal = Vec3(0, 1, 0), int index = -1,
         bool invisible = false, bool ignored = false);
    // ------------------------------------------------------------------------
    virtual ~Quad() {}
    // ------------------------------------------------------------------------
    void getVertices(video::S3DVertex *v, const video::SColor &color) const;
    // ------------------------------------------------------------------------
    /** Returns the i-th. point of a quad. */
    const Vec3& operator[](int i) const                      { return m_p[i]; }
    // ------------------------------------------------------------------------
    /** Returns the center of a quad. */
    const Vec3& getCenter ()      const                    { return m_center; }
    // ------------------------------------------------------------------------
    void setHeightTesting(float min, float max)
    {
        m_min_height_testing = min;
        m_max_height_testing = max;
    }
    // ------------------------------------------------------------------------
    /** Returns the minimum height of a quad. */
    float getMinHeight() const                         { return m_min_height; }
    // ------------------------------------------------------------------------
    /** Returns the index of this quad. */
    int getIndex() const
    {
        // You should not call this if it has default value (like slipstream)
        assert(m_index != -1);
        return m_index;
    }
    // ------------------------------------------------------------------------
    /** Returns true of this quad is invisible, i.e. not to be shown in
     *  the minimap. */
    bool isInvisible() const                            { return m_invisible; }
	// ------------------------------------------------------------------------
    bool isIgnored() const                             { return m_is_ignored; }
    // ------------------------------------------------------------------------
    /** Returns the normal of this quad. */
    const Vec3& getNormal() const                          { return m_normal; }
    // ------------------------------------------------------------------------
    /** Returns true if a point is inside this quad. */
    virtual bool pointInside(const Vec3& p,
                             bool ignore_vertical = false) const;
    // ------------------------------------------------------------------------
    /** Returns true if this quad is 3D, which additional 3D testing is used in
     *  pointInside. */
    virtual bool is3DQuad() const                             { return false; }
    // ------------------------------------------------------------------------
    virtual float getDistance2FromPoint(const Vec3 &xyz) const
    {
        // You should not call this in a bare quad
        assert(false);
        return 0.0f;
    }

};   // class Quad
#endif
