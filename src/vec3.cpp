//  $Id: vec3.cpp 1954 2008-05-20 10:01:26Z scifly $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008 Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#include "vec3.hpp"

void Vec3::setHPR(const btMatrix3x3& m)
{
    float f[4][4];
    m.getOpenGLSubMatrix((float*)f);

//     sgSetCoord(m_curr_pos, f);    
//void sgSetCoord ( sgCoord *dst, const sgMat4 src )

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

    SGfloat cp = cos(getY());

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
