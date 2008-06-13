//  $Id: vec3.hpp 1954 2008-05-20 10:01:26Z scifly $
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


#ifndef HEADER_VEC3_H
#define HEADER_VEC3_H

#include <plib/sg.h>

#include "LinearMath/btVector3.h"
#include "LinearMath/btMatrix3x3.h"

class Vec3 : public btVector3
{
private:
    inline float clampToUnity(float f) {return f<-1?f:(f>1?1:f);}
public:
                   inline Vec3(sgVec3 a) : btVector3(a[0], a[1], a[2]) {}
                   inline Vec3(const btVector3& a) : btVector3(a)      {}
                   inline Vec3()                   : btVector3()       {}
                   inline Vec3(float x, float y, float z) 
                                                   : btVector3(x,y,z)  {}

    void                  setHPR(const btMatrix3x3& m);
    inline const float    operator[](int n) const         {return *(&m_x+n); }
    inline const float    getHeading() const       {return m_x; }
    inline const float    getPitch() const         {return m_y; }
    inline const float    getRoll() const          {return m_z; }
    inline const void     setHeading(float f)      {m_x = f;}
    inline const void     setPitch(float f)        {m_y = f;}
    inline const void     setRoll(float f)         {m_z = f;}
    float*                toFloat() const          {return (float*)this;     }
    void                  degreeToRad();
    Vec3&          operator=(const btVector3& a)   {*(btVector3*)this=a; return *this;}
    Vec3&          operator=(const btMatrix3x3& m) {setHPR(m);           return *this;}
};    // Vec3
#endif
