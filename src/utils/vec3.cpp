//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008-2015 Joerg Henrichs
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

#include "utils/vec3.hpp"

void Vec3::setHPR(const btQuaternion& q)
{
    float W = q.getW();
    float X = q.getX();
    float Y = q.getY();
    float Z = q.getZ();
    float WSquared = W * W;
    float XSquared = X * X;
    float YSquared = Y * Y;
    float ZSquared = Z * Z;

    setX(atan2f(2.0f * (Y * Z + X * W), -XSquared - YSquared + ZSquared + WSquared));
    setY(btAsin(-2.0f * (X * Z - Y * W)));
    setZ(atan2f(2.0f * (X * Y + Z * W), XSquared - YSquared - ZSquared + WSquared));
}   // setHPR(btQuaternion)

// ----------------------------------------------------------------------------
/** Sets the pitch and the roll of this vector to follow the normal given. The
 *  heading is taken from this vector.
 *  \param normal The normal vector to which pitch and roll should be aligned.
 */
void Vec3::setPitchRoll(const Vec3 &normal)
{
    const float X = sinf(getHeading());
    const float Z = cosf(getHeading());
    // Compute the angle between the normal of the plane and the line to
    // (x,0,z).  (x,0,z) is normalised, so are the coordinates of the plane,
    // which simplifies the computation of the scalar product.
    float pitch = ( normal.getX()*X + normal.getZ()*Z );  // use ( x,0,z)
    float roll  = (-normal.getX()*Z + normal.getZ()*X );  // use (-z,0,x)

    // The actual angle computed above is between the normal and the (x,y,0)
    // line, so to compute the actual angles 90 degrees must be subtracted.
    setPitch(-acosf(pitch) + NINETY_DEGREE_RAD);
    setRoll (-acosf(roll)  + NINETY_DEGREE_RAD);
}   // setPitchRoll

