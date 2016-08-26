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
#include "utils/log.hpp"

#include <algorithm>
#include <matrix4.h>
#include <S3DVertex.h>
#include <triangle3d.h>

#include "LinearMath/btTransform.h"

/** Constructor, takes 4 points. */
Quad::Quad(const Vec3 &p0, const Vec3 &p1, const Vec3 &p2, const Vec3 &p3,
           bool invisible, bool ai_ignore)
 {
         
     m_p[0]=p0; m_p[1]=p1; m_p[2]=p2; m_p[3]=p3;
     
     m_center = 0.25f*(p0+p1+p2+p3);
     m_min_height = std::min ( std::min(p0.getY(), p1.getY()),
                               std::min(p2.getY(), p3.getY())  );
     m_max_height = std::max ( std::max(p0.getY(), p1.getY()),
                               std::max(p2.getY(), p3.getY())  );
     m_invisible = invisible;
     m_ai_ignore = ai_ignore;

     findNormal();

     // Compute the quad bounding box used for pointInQuad
     Vec3 boxCorners[8];
     Vec3 normal = getNormal();
     float boxHigh = 5.0f;
     float boxLow = 1.0f;
     boxCorners[0] = m_p[0] + boxHigh*normal;
     boxCorners[1] = m_p[1] + boxHigh*normal;
     boxCorners[2] = m_p[2] + boxHigh*normal;
     boxCorners[3] = m_p[3] + boxHigh*normal;
     boxCorners[4] = m_p[0] - boxLow*normal;
     boxCorners[5] = m_p[1] - boxLow*normal;
     boxCorners[6] = m_p[2] - boxLow*normal;
     boxCorners[7] = m_p[3] - boxLow*normal;

     Vec3 boxFaces[6][4] = {
         { boxCorners[0], boxCorners[1], boxCorners[2], boxCorners[3] },
         { boxCorners[3], boxCorners[2], boxCorners[6], boxCorners[7] },
         { boxCorners[7], boxCorners[6], boxCorners[5], boxCorners[4] },
         { boxCorners[1], boxCorners[0], boxCorners[4], boxCorners[5] },
         { boxCorners[4], boxCorners[0], boxCorners[3], boxCorners[7] },
         { boxCorners[1], boxCorners[5], boxCorners[6], boxCorners[2] } 
     };

     for (unsigned int i = 0; i < 6 ; i++)
        for (unsigned int j = 0; j < 4; j++)
             m_box_faces[i][j] = boxFaces[i][j];
   
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

// ----------------------------------------------------------------------------
bool Quad::pointInQuad(const Vec3& p, bool ignore_vertical) const
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
       (p.getY() - m_max_height > 5.0f ||
       p.getY() - m_min_height < -1.0f   ))
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
}   // pointInQuad

// ----------------------------------------------------------------------------
/** Checks if a given point p lies in the bounding box of this quad */
bool Quad::pointInQuad3D(const Vec3& p) const
{
    float side = p.sideofPlane(m_box_faces[0][0], m_box_faces[0][1], m_box_faces[0][2]);
    for (int i = 1; i < 6; i++)
    {
        if (side*p.sideofPlane(m_box_faces[i][0], m_box_faces[i][1], m_box_faces[i][2]) < 0) return false;
    }
    return true;
}   // pointInQuad3D
    

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
    result->m_min_height = std::min ( std::min(result->m_p[0].getY(),
                                               result->m_p[1].getY()),
                                      std::min(result->m_p[2].getY(),
                                               result->m_p[3].getY())  );
}   // transform

// ----------------------------------------------------------------------------
/** Find the normal of this quad by computing the normal of two triangles and taking
 *  their average.
 */
void Quad::findNormal()
{
    core::triangle3df tri1(m_p[0].toIrrVector(), m_p[1].toIrrVector(), m_p[2].toIrrVector());
    core::triangle3df tri2(m_p[0].toIrrVector(), m_p[2].toIrrVector(), m_p[3].toIrrVector());
    Vec3 normal1 = tri1.getNormal();
    Vec3 normal2 = tri2.getNormal();
    m_normal = -0.5f*(normal1 + normal2);
    m_normal.normalize();

}   // findNormal

// ----------------------------------------------------------------------------
/** Return a flattened version of this quad. */
Quad Quad::getFlattenedQuad()
{
    core::CMatrix4<float> m;
    m.buildRotateFromTo(m_normal.toIrrVector(), core::vector3df(0, 1, 0));
    Vec3 m_p_flat[4];
    for (unsigned int i = 0; i < 4; i++)
    {
        m.rotateVect(m_p_flat[i], (m_p[i] - m_center).toIrrVector());

        m_p_flat[i].setY(0);
    }
     
    return Quad(m_p_flat[0], m_p_flat[1], m_p_flat[2], m_p_flat[3]);

}   // getFlattenedQuad
