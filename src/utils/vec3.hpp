//  $Id$
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


#ifndef HEADER_VEC3_HPP
#define HEADER_VEC3_HPP

#include "irrlicht.h"
using namespace irr;

#include "LinearMath/btVector3.h"
#include "LinearMath/btMatrix3x3.h"

/** A wrapper around bullets btVector3 to include conventient conversion
 *  functions (e.g. between btVector3 and the graphics specific 3d vector). */
class Vec3 : public btVector3
{
private:
    inline float clampToUnity(float f) {return f<-1?f:(f>1?1:f);}
    void         setPitchRoll(const Vec3 &normal);

public:
    /** Convert an irrlicht vector3df into the internal (bullet) format.
     *  Irrlicht's and STK's axis are different (STK: Z up, irrlicht: Y up).
     *  We might want to change this as well, makes it easier to work with 
     *  bullet and irrlicht together, without having to swap indices (bullet
     *  can handle any axis ordering). Note that toIrrVector swaps the 
     *  axis as well (so a vector3df can be stored in and restored from
     *  a vec3).
     */
    inline Vec3(const core::vector3df &v)  :  btVector3(v.X, v.Y, v.Z)   {}
    inline Vec3(const btVector3& a)        : btVector3(a)                {}
    inline Vec3()                          : btVector3()                 {}
    inline Vec3(float x, float y, float z) : btVector3(x,y,z)            {}
    inline Vec3(float x)                   : btVector3(x,x,x)            {}
    /** Sets the heading, and computes pitch and roll dependent
     *  on the normal it is displayed on.
     *  \param heading The heading to set.
     *  \param normal The normal from which pitch and roll should be computed. */
    inline Vec3(float heading, const Vec3& normal)
        {setHeading(heading);
         setPitchRoll(normal);}

    void                   setHPR(const btQuaternion& q);
    inline const float     operator[](int n) const  { return *(&m_x+n); }
    inline const float     getHeading() const       { return m_y;       }
    inline const float     getPitch() const         { return m_x;       }
    inline const float     getRoll() const          { return m_z;       }
    inline const void      setHeading(float f)      { m_y = f;          }
    inline const void      setPitch(float f)        { m_x = f;          }
    inline const void      setRoll(float f)         { m_z = f;          }
    /** Converts a Vec3 to an irrlicht 3d floating point vector. */
    const core::vector3df& toIrrVector()   const;
    const core::vector3df  toIrrHPR()      const;
    const core::vector2df  toIrrVector2d() const;
    void                   degreeToRad();
    Vec3&          operator=(const btVector3& a)   {*(btVector3*)this=a; return *this;}
    Vec3&          operator=(const btQuaternion& q){setHPR(q);           return *this;}
    Vec3           operator-(const Vec3& v1) const {return (Vec3)(*(btVector3*)this-(btVector3)v1);}
    /** Helper functions to treat this vec3 as a 2d vector. This returns the
     *  square of the length of the first 2 dimensions. */
    float          length2_2d() const              {return m_x*m_x + m_z*m_z;}
    /** Returns the length of this vector in the plane, i.e. the vector is 
     *  used as a 2d vector. */
    float          length_2d()  const              {return sqrt(m_x*m_x + m_z*m_z);}
    /** Sets this = max(this, a) componentwise.
     *  \param Vector to compare with. */
    void           max(const Vec3& a)              {if(a.getX()>m_x) m_x=a.getX();
                                                    if(a.getY()>m_y) m_y=a.getY();
                                                    if(a.getZ()>m_z) m_z=a.getZ();}
    /** Sets this = min(this, a) componentwise.
     *  \param a Vector to compare with. */
    void           min(const Vec3& a)              {if(a.getX()<m_x) m_x=a.getX();
                                                    if(a.getY()<m_y) m_y=a.getY();
                                                    if(a.getZ()<m_z) m_z=a.getZ();}
};    // Vec3


#endif
