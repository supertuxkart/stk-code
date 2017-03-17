//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011-2015 Joerg Henrichs
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


#ifndef HEADER_VEC3_HPP
#define HEADER_VEC3_HPP

#include <vector3d.h>
#include <vector2d.h>
using namespace irr;

#include "LinearMath/btVector3.h"
#include "LinearMath/btMatrix3x3.h"

#include "utils/constants.hpp"

/** A wrapper around bullets btVector3 to include conventient conversion
 *  functions (e.g. between btVector3 and the graphics specific 3d vector). */
class Vec3 : public btVector3
{
private:
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
    inline Vec3(const core::vector3df &v)  : btVector3(v.X, v.Y, v.Z)    {}
    // ------------------------------------------------------------------------
    /** Initialises a vector from a btVector3 (or a Vec3). */
    inline Vec3(const btVector3& a)        : btVector3(a)                {}
    // ------------------------------------------------------------------------
    /** Empty constructor. */
    inline Vec3()                          : btVector3(0, 0, 0)          {}
    // ------------------------------------------------------------------------
    /** Creates a 3d vector from three scalars. */
    inline Vec3(float x, float y, float z) : btVector3(x,y,z)            {}
    // ------------------------------------------------------------------------
    /** Creates a 3d vector from three scalars. */
    inline Vec3(float x, float y, float z, float w) : btVector3(x,y,z)
                                                     { setW(w);           }
    // ------------------------------------------------------------------------
    /** Initialises a 3d vector from one scalar value, which is used to
     *  initialise all components. */
    inline Vec3(float x)                   : btVector3(x,x,x)            {}
    // ------------------------------------------------------------------------
    /** Sets the heading, and computes pitch and roll dependent
     *  on the normal it is displayed on.
     *  \param heading The heading to set.
     *  \param normal The normal from which pitch and roll should be
     *         computed. */
    inline Vec3(float heading, const Vec3& normal)
    {
        setHeading(heading);
        setPitchRoll(normal);
    }   // Vec3(heading, normal)

    // ------------------------------------------------------------------------
    /** Sets the heading, pitch, roll of this vector that is used to store a
     *  rotation from a quaternion. */
    void                   setHPR(const btQuaternion& q);
    // ------------------------------------------------------------------------
    /** Returns a reference to the n-th element (x=0, y=1, z=2, w=3). */
    inline const float&    operator[](int n) const  { return m_floats[n]; }
    // ------------------------------------------------------------------------
    /** Returns a reference to the n-th element (x=0, y=1, z=2, w=3). */
    inline float&          operator[](int n)        { return m_floats[n]; }
    // ------------------------------------------------------------------------
    /** Returns the heading of a vector that is used to store a rotation. */
    inline const float     getHeading() const       { return m_floats[1]; }
    // ------------------------------------------------------------------------
    /** Returns the pitch of a vector that is used to store a rotation. */
    inline const float     getPitch() const         { return m_floats[0]; }
    // ------------------------------------------------------------------------
    /** Returns the roll of a vector that is used to store a rotation. */
    inline const float     getRoll() const          { return m_floats[2]; }
    // ------------------------------------------------------------------------
    /** Returns the W component (bullet vectors contain 4 elements, the last
     *  element is usually unused). */
    inline const float     getW() const             { return m_floats[3]; }
    // ------------------------------------------------------------------------
    /** Sets the heading of a vector that is used to store a rotation. */
    inline const void      setHeading(float f)      { m_floats[1] = f;    }
    // ------------------------------------------------------------------------
    /** Sets the pitch of a vector that is used to store a rotation. */
    inline const void      setPitch(float f)        { m_floats[0] = f;    }
    // ------------------------------------------------------------------------
    /** Sets the roll of a vector that is used to store a rotation. */
    inline const void      setRoll(float f)         { m_floats[2] = f;    }
    // ------------------------------------------------------------------------
    /** Converts a vec3 into an irrlicht vector (which is a simple type
     *  cast). */
    const core::vector3df& toIrrVector() const
    {
            return (const core::vector3df&)*this;
    }   // toIrrVector
    // ------------------------------------------------------------------------
    /** Converts a bullet HPR value into an irrlicht HPR value. */
    const core::vector3df  toIrrHPR() const
    {
        return core::vector3df(RAD_TO_DEGREE*(getX()),     // pitch
                               RAD_TO_DEGREE*(getY()),     // heading
                               RAD_TO_DEGREE*(getZ()) );   // roll
    }   // toIrrHPR
    // ------------------------------------------------------------------------
    /** Returns the X and Z component as an irrlicht 2d vector. */
    const core::vector2df  toIrrVector2d() const
    {
        return core::vector2df(m_floats[0], m_floats[2]);
    }   // toIrrVector2d
    // ------------------------------------------------------------------------
    /** Converts degree values stored in this vec3 to radians. */
    void degreeToRad()
    {
        m_floats[0]*=DEGREE_TO_RAD;
        m_floats[1]*=DEGREE_TO_RAD;
        m_floats[2]*=DEGREE_TO_RAD;
    }   // degreeToRad
    // ------------------------------------------------------------------------
    /** Sets this = a. */
    Vec3& operator=(const btVector3& a)   {*(btVector3*)this=a; return *this;}
    // ------------------------------------------------------------------------
    /** Sets the rotation given by the quaternion as HPR vector. */
    Vec3& operator=(const btQuaternion& q) {setHPR(q); return *this;}

    // ------------------------------------------------------------------------
    /** Operator== of btQuadWord also compares m_floats[3], which is not
     *  useful (and wrong in certain circumstances). */
    bool operator==(const Vec3& other) const
    {
        return ((m_floats[2]==other.m_floats[2]) &&
                (m_floats[1]==other.m_floats[1]) &&
                (m_floats[0]==other.m_floats[0])   );
    }

    // ------------------------------------------------------------------------
    /** Operator!= of btQuadWord also compares m_floats[3], which is not
     *  useful (and wrong in certain circumstances). */
    bool operator!=(const Vec3& other) const
    {
        return ((m_floats[2]!=other.m_floats[2]) ||
                (m_floats[1]!=other.m_floats[1]) ||
                (m_floats[0]!=other.m_floats[0])   );
    }

    // ------------------------------------------------------------------------
    /** Computes this = this - v1. */
    Vec3  operator-(const Vec3& v1) const {return (Vec3)(*(btVector3*)this
                                                         -(btVector3)v1); }
    // ------------------------------------------------------------------------
    /** Computes this = this - v1. On VS this special version is needed,
     *  since otherwise Vec3-btVector3 is ont unique (could be cast to
     *  btVector3-btVector3, or convert btVector3 to Vec3()). */
    Vec3 operator-(const btVector3 v1) const
    {
        return *(btVector3*)this - v1;
    }
    // ------------------------------------------------------------------------
    /** Helper functions to treat this vec3 as a 2d vector. This returns the
     *  square of the length of the first 2 dimensions. */
    float length2_2d() const     { return m_floats[0]*m_floats[0]
                                        + m_floats[2]*m_floats[2]; }
    // ------------------------------------------------------------------------
    /** Returns the length of this vector in the plane, i.e. the vector is
     *  used as a 2d vector. */
    // ------------------------------------------------------------------------
    /** Returns the length of the vector using only the x/z coordinates. */
    float length_2d() const {return sqrtf(  m_floats[0]*m_floats[0]
                                          + m_floats[2]*m_floats[2]);}
    // ------------------------------------------------------------------------
    /** Sets this = max(this, a) componentwise.
     *  \param Vector to compare with. */
    void max(const Vec3& a) {if(a.getX()>m_floats[0]) m_floats[0]=a.getX();
                             if(a.getY()>m_floats[1]) m_floats[1]=a.getY();
                             if(a.getZ()>m_floats[2]) m_floats[2]=a.getZ();}
    // ------------------------------------------------------------------------
    /** Sets this = min(this, a) componentwise.
     *  \param a Vector to compare with. */
    void min(const Vec3& a) {if(a.getX()<m_floats[0]) m_floats[0]=a.getX();
                             if(a.getY()<m_floats[1]) m_floats[1]=a.getY();
                             if(a.getZ()<m_floats[2]) m_floats[2]=a.getZ();}
    // ------------------------------------------------------------------------
    /** Determines which side of a line this point is. This is using
     *  a 2d projection (into the X-Z plane).
     *  \param start The start point of the line.
     *  \param end   The end point of the line.
     *  \return >0 Point is to the left side; <0 if it's on the right.
     */
    float sideOfLine2D(const Vec3& start, const Vec3& end) const
    {
        return (end.getX()-start.getX())*(m_floats[2]-start.getZ()) -
               (end.getZ()-start.getZ())*(m_floats[0]-start.getX());

    }   // sideOfLine2D

    float sideofPlane(const Vec3& x1, const Vec3& x2, const Vec3& x3) const
    {
        return ((x2 - x1).cross(x3 - x1)).dot(*this - x1);
    } // sideOfPlane
};    // Vec3

#endif
