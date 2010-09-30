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

#include "LinearMath/btTransform.h"

/** Constructor, takes 4 points. */
Quad::Quad(const Vec3 &p0, const Vec3 &p1, const Vec3 &p2, const Vec3 &p3,
           bool invisible)
 {
     if(sideOfLine2D(p0, p2, p1)>0 ||
         sideOfLine2D(p0, p2, p3)<0)
     {
         printf("Warning: quad has wrong orientation: p0=%f %f %f p1=%f %f %f\n",
                p0.getX(), p0.getY(), p0.getZ(),p1.getX(), p1.getY(), p1.getZ());
         printf("The quad will be swapped, nevertheless test for correctness -\n");
         printf("quads must be counter-clockwise oriented.\n");
         m_p[0]=p1; m_p[1]=p0; m_p[2]=p3; m_p[3]=p2;
     }
     else
     {
        m_p[0]=p0; m_p[1]=p1; m_p[2]=p2; m_p[3]=p3;
     }
     m_center = 0.25f*(p0+p1+p2+p3);
     m_min_height = std::min ( std::min(p0.getY(), p1.getY()),
                               std::min(p0.getY(), p1.getY())  );
     m_invisible = invisible;

}   // Quad

// ----------------------------------------------------------------------------
/** Sets the vertices in a irrlicht vertex array to the 4 points of this quad.
 *  \param v The vertex array in which to set the vertices. 
 *  \param color The color to use for this quad.
 */
void Quad::getVertices(video::S3DVertex *v, const video::SColor &color) const
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
    return (l2.getX()-l1.getX())*(p.getZ()-l1.getZ()) -
           (l2.getZ()-l1.getZ())*(p.getX()-l1.getX());
}   // sideOfLine

// ----------------------------------------------------------------------------
bool Quad::pointInQuad(const Vec3& p) const 
{
    // In case that a kart can validly run too high over one driveline
    // and it should not be considered to be on that driveline. Example:
    // a kart is driving over a bridge, slightly off the bridge 
    // driveline. It must be avoided that the kart is then considered
    // to be on the driveline under the brige. So the vertical distance
    // is taken into account, too. to simplify this test we only compare 
    // with the minimum height of the quad (and not with the actual
    // height of the quad at the point where the kart is).
    if(p.getY() - m_min_height > 5.0f ||
       p.getY() - m_min_height < -1.0f    )
       return false;

    // If a point is exactly on the line of two quads (e.g. between points
    // 0,1 on one quad, and 3,2 of the previous quad), assign this point
    // to be on the 'later' quad, i.e. on the line between points 0 and 1.
    if(sideOfLine2D(m_p[0], m_p[2], p)<0) {
        return sideOfLine2D(m_p[0], m_p[1], p) >= 0.0 &&
               sideOfLine2D(m_p[1], m_p[2], p) >= 0.0;
    } else {
        return sideOfLine2D(m_p[2], m_p[3], p) >  0.0 &&
               sideOfLine2D(m_p[3], m_p[0], p) >= 0.0;
    }
}   // pointInQuad

// ----------------------------------------------------------------------------
/** Transforms a quad by a given transform (i.e. translation+rotation). This
 *  function does not modify this quad, the results are stored in the quad
 *  specified as parameter. These functions are used for slipstreaming to
 *  determine the slipstream area from the original value (kart at 0,0,0 and
 *  no rotation) to the current value.
 *  \param t The transform to apply.
 *  \param result The quad which stores the result.
 */
void Quad::transform(const btTransform &t, Quad *result) const
{
    result->m_p[0] = t(m_p[0]);
    result->m_p[1] = t(m_p[1]);
    result->m_p[2] = t(m_p[2]);
    result->m_p[3] = t(m_p[3]);
}   // transform
