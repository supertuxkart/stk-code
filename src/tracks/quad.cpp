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

#include "tracks/quad.hpp"
#include "tracks/graph.hpp"
#include "mini_glm.hpp"
#include "utils/log.hpp"

#include <algorithm>
#include <S3DVertex.h>
#include <triangle3d.h>

/** Constructor, takes 4 points. */
Quad::Quad(const Vec3 &p0, const Vec3 &p1, const Vec3 &p2, const Vec3 &p3,
           const Vec3 &normal, int index, bool invisible, bool ignored)
     : m_index(index), m_normal(normal), m_invisible(invisible)
{
    m_is_ignored = ignored;
    m_p[0]=p0; m_p[1]=p1; m_p[2]=p2; m_p[3]=p3;

    m_center = 0.25f*(p0+p1+p2+p3);
    m_min_height = std::min ( std::min(p0.getY(), p1.getY()),
                              std::min(p2.getY(), p3.getY())  );
    m_max_height = std::max ( std::max(p0.getY(), p1.getY()),
                              std::max(p2.getY(), p3.getY())  );
    m_min_height_testing = Graph::MIN_HEIGHT_TESTING;
    m_max_height_testing = Graph::MAX_HEIGHT_TESTING;
}   // Quad

/** Takes 4 points. */
void Quad::setQuad(const Vec3 &p0, const Vec3 &p1, const Vec3 &p2, const Vec3 &p3)
{
    m_p[0]=p0; m_p[1]=p1; m_p[2]=p2; m_p[3]=p3;

    m_center = 0.25f*(p0+p1+p2+p3);
    m_min_height = std::min ( std::min(p0.getY(), p1.getY()),
                              std::min(p2.getY(), p3.getY())  );
    m_max_height = std::max ( std::max(p0.getY(), p1.getY()),
                              std::max(p2.getY(), p3.getY())  );
    m_min_height_testing = Graph::MIN_HEIGHT_TESTING;
    m_max_height_testing = Graph::MAX_HEIGHT_TESTING;
}   // setQuad

// ----------------------------------------------------------------------------
/** Sets the vertices in a irrlicht vertex array to the 4 points of this quad.
 *  \param v The vertex array in which to set the vertices.
 *  \param color The color to use for this quad.
 */
void Quad::getVertices(video::S3DVertex *v, const video::SColor &color) const
{
    // Eps is used to raise the track debug quads a little bit higher than
    // the ground, so that they are actually visible.
    core::vector3df normal = getNormal().toIrrVector();
    core::vector3df eps = normal * 0.1f;
    v[0].Pos = m_p[0].toIrrVector()+eps;
    v[1].Pos = m_p[1].toIrrVector()+eps;
    v[2].Pos = m_p[2].toIrrVector()+eps;
    v[3].Pos = m_p[3].toIrrVector()+eps;

    v[0].Normal = normal;
    v[1].Normal = normal;
    v[2].Normal = normal;
    v[3].Normal = normal;

    v[0].Color  = color;
    v[1].Color  = color;
    v[2].Color  = color;
    v[3].Color  = color;
}   // setVertices

// ----------------------------------------------------------------------------
/** Sets the vertices in an spm vertex array to the 4 points of this quad.
 *  \param v The vertex array in which to set the vertices.
 *  \param color The color to use for this quad.
 */
void Quad::getSPMVertices(video::S3DVertexSkinnedMesh *v,
                          const video::SColor &color) const
{
    // Eps is used to raise the track debug quads a little bit higher than
    // the ground, so that they are actually visible.
    core::vector3df normal = getNormal().toIrrVector();
    core::vector3df eps = normal * 0.1f;
    v[0].m_position = m_p[0].toIrrVector()+eps;
    v[1].m_position = m_p[1].toIrrVector()+eps;
    v[2].m_position = m_p[2].toIrrVector()+eps;
    v[3].m_position = m_p[3].toIrrVector()+eps;

    v[0].m_normal = MiniGLM::compressVector3(normal);
    v[1].m_normal = MiniGLM::compressVector3(normal);
    v[2].m_normal = MiniGLM::compressVector3(normal);
    v[3].m_normal = MiniGLM::compressVector3(normal);

    v[0].m_color  = color;
    v[1].m_color  = color;
    v[2].m_color  = color;
    v[3].m_color  = color;
}   // setVertices

// ----------------------------------------------------------------------------
bool Quad::pointInside(const Vec3& p, bool ignore_vertical) const
{
    // In case that a kart can validly run too high over one driveline
    // and it should not be considered to be on that driveline. Example:
    // a kart is driving over a bridge, slightly off the bridge
    // driveline. It must be avoided that the kart is then considered
    // to be on the driveline under the brige. So the vertical distance
    // is taken into account, too. to simplify this test we only compare
    // with the minimum height of the quad (and not with the actual
    // height of the quad at the point where the kart is).
    if(!ignore_vertical                &&
       (p.getY() - m_max_height > m_max_height_testing ||
       p.getY() - m_min_height < m_min_height_testing   ))
       return false;

    // If a point is exactly on the line of two quads (e.g. between points
    // 0,1 on one quad, and 3,2 of the previous quad), assign this point
    // to be on the 'later' quad, i.e. on the line between points 0 and 1.
    if(p.sideOfLine2D(m_p[0], m_p[2])<0)
    {
        return p.sideOfLine2D(m_p[0], m_p[1]) >= 0.0 &&
               p.sideOfLine2D(m_p[1], m_p[2]) >= 0.0;
    }
    else
    {
        return p.sideOfLine2D(m_p[2], m_p[3]) >  0.0 &&
               p.sideOfLine2D(m_p[3], m_p[0]) >= 0.0;
    }
}   // pointInside
