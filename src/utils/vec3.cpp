//  $Id: vec3.cpp 1954 2008-05-20 10:01:26Z scifly $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008 Joerg Henrichs
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

#include "utils/constants.hpp"

void Vec3::setHPR(const btMatrix3x3& m)
{
    float f[4][4];
    m.getOpenGLSubMatrix((float*)f);

    float s = m.getColumn(0).length();

    if ( s <= 0.00001 )
    {
        fprintf(stderr,"setHPR: bad matrix\n");
        setValue(0,0,0);
        return ;
    }
    s=1/s;

#define CLAMPTO1(x) x<-1 ? -1 : (x>1 ? 1 : x)

    setY(asin(CLAMPTO1(m.getRow(2).getY())));

    float cp = cos(getY());

    /* If pointing nearly vertically up - then heading is ill-defined */


    if ( cp > -0.00001 && cp < 0.00001 )
    {
        float cr = CLAMPTO1( m.getRow(1).getX()*s); 
        float sr = CLAMPTO1(-m.getRow(1).getZ()*s);

        setX(0.0f);
        setZ(atan2(sr, cr ));
    }
    else
    {
        cp = s / cp ; // includes the scaling factor
        float sr = CLAMPTO1( -m.getRow(2).getX() * cp );
        float cr = CLAMPTO1(  m.getRow(2).getZ() * cp );
        float sh = CLAMPTO1( -m.getRow(0).getY() * cp );
        float ch = CLAMPTO1(  m.getRow(1).getY() * cp );

        if ( (sh == 0.0f && ch == 0.0f) || (sr == 0.0f && cr == 0.0f) )
        {
            cr = CLAMPTO1( m.getRow(1).getX()*s);
            sr = CLAMPTO1(-m.getRow(1).getZ()*s) ;

            setX(0.0f);
        }
        else
            setX(atan2(sh, ch ));

        setZ(atan2(sr, cr ));
    }
}   // setHPR

// ----------------------------------------------------------------------------
void Vec3::degreeToRad()
{
    m_x=DEGREE_TO_RAD*m_x;
    m_y=DEGREE_TO_RAD*m_y;      
    m_z=DEGREE_TO_RAD*m_z;
}   // degreeToRad

// ----------------------------------------------------------------------------
/** Sets the pitch and the roll of this vector to follow the normal given. The
 *  heading is taken from this vector.
 *  \param normal The normal vector to which pitch and roll should be aligned.
 */
void Vec3::setPitchRoll(const Vec3 &normal)
{
    const float X =-sin(m_x);
    const float Y = cos(m_x);
    // Compute the angle between the normal of the plane and the line to
    // (x,y,0).  (x,y,0) is normalised, so are the coordinates of the plane,
    // simplifying the computation of the scalar product.
    float pitch = ( normal.getX()*X + normal.getY()*Y );  // use ( x,y,0)
    float roll  = (-normal.getX()*Y + normal.getY()*X );  // use (-y,x,0)

    // The actual angle computed above is between the normal and the (x,y,0)
    // line, so to compute the actual angles 90 degrees must be subtracted.
    m_y = acosf(pitch) - NINETY_DEGREE_RAD;
    m_z = acosf(roll)  - NINETY_DEGREE_RAD;
}   // setPitchRoll

// ----------------------------------------------------------------------------
/** Converts a bullet HPR value into an irrlicht HPR value.
 *  FIXME: this function should be inlined, but while debugging it's
 *  easier (compile-time wise) to modify it here :)
 */
const core::vector3df Vec3::toIrrHPR() const
{
    core::vector3df r(RAD_TO_DEGREE*(-getY()),     // pitch
                      RAD_TO_DEGREE*(-getX()),     // heading
                      RAD_TO_DEGREE*(-getZ()) );   // roll
    return r;

}  // toIrrHPR
// ----------------------------------------------------------------------------
const core::vector3df Vec3::toIrrVector() const
{
    core::vector3df v(m_x, m_z, m_y);
    return v;
}   // toIrrVector
