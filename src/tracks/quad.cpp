//  $Id$
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

#include "tracks/quad.hpp"

#include "irrlicht.h"

/** Constructor, takes 4 points. */
Quad::Quad(const Vec3 &p0, const Vec3 &p1, const Vec3 &p2, const Vec3 &p3)
 {
     m_p[0]=p0; m_p[1]=p1; m_p[2]=p2; m_p[3]=p3;
     m_center = 0.25f*(p0+p1+p2+p3);
     m_min_height = std::min ( std::min(p0.getZ(), p1.getZ()),
                               std::min(p0.getZ(), p1.getZ())  );
}   // Quad

// ----------------------------------------------------------------------------
/** Sets the vertices in a irrlicht vertex array to the 4 points of this quad.
 *  \param v The vertex array in which to set the vertices. 
 *  \param color The color to use for this quad.
 */
void Quad::setVertices(video::S3DVertex *v, const video::SColor &color) const
{
    // Eps is used to raise the track debug quads a little bit higher than
    // the ground, so that they are actually visible.
    core::vector3df eps(0, 0.1f, 0);
    v[0].Pos = m_p[0].toIrrVector()+eps;
    v[1].Pos = m_p[1].toIrrVector()+eps;
    v[2].Pos = m_p[2].toIrrVector()+eps;
    v[3].Pos = m_p[3].toIrrVector()+eps;

    core::triangle3df tri(m_p[0].toIrrVector(), m_p[1].toIrrVector(), 
                          m_p[2].toIrrVector());
    core::vector3df normal = tri.getNormal();
    normal.normalize();
    v[0].Normal = normal;
    v[1].Normal = normal;
    v[2].Normal = normal;

    core::triangle3df tri1(m_p[0].toIrrVector(), m_p[2].toIrrVector(), 
                           m_p[3].toIrrVector());
    core::vector3df normal1 = tri1.getNormal();
    normal1.normalize();
    v[3].Normal = normal1;

    v[0].Color  = color;
    v[1].Color  = color;
    v[2].Color  = color;
    v[3].Color  = color;
}   // setVertices

// -----------------------------------------------------------------------------
/** Returns wether a point is to the left or to the right of a line.
 *  While all arguments are 3d, only the x and y coordinates are actually used.
*/
float Quad::sideOfLine2D(const Vec3& l1, const Vec3& l2, const Vec3& p) const
{
    return (l2.getX()-l1.getX())*(p.getY()-l1.getY()) -
           (l2.getY()-l1.getY())*(p.getX()-l1.getX());
}   // sideOfLine

// ----------------------------------------------------------------------------
bool Quad::pointInQuad(const Vec3& p) const 
{
    if(sideOfLine2D(m_p[0], m_p[2], p)<0) {
        return sideOfLine2D(m_p[0], m_p[1], p) >  0.0 &&
               sideOfLine2D(m_p[1], m_p[2], p) >= 0.0;
    } else {
        return sideOfLine2D(m_p[2], m_p[3], p) >  0.0 &&
               sideOfLine2D(m_p[3], m_p[0], p) >= 0.0;
    }
}   // pointInQuad

