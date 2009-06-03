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


#ifndef HEADER_COORD_HPP
#define HEADER_COORD_HPP

#include "LinearMath/btTransform.h"

#include "constants.hpp"
#include "utils/vec3.hpp"

/** A class that stores a translation and rotation. It is used to convert
 *  between bullet data structures and the data structure for the graphics.
 */
class Coord
{
private:
    /** Translation. */
    Vec3       m_xyz;
    /** Rotation as Eulerian HPR value. */
    Vec3       m_hpr;

    //sgCoord    m_coord;

    /** Sets the sgCoord data structures (and converts radians to degrees). */
    /*
    void setSgCoord()
    {
        sgSetCoord(&m_coord, m_xyz.toFloat(), m_hpr.toFloat());
        // Convert hpr in radians to degrees, which sg needs.
        for(int i=0; i<3; i++)
        {
            m_coord.hpr[i] = RAD_TO_DEGREE(m_coord.hpr[i]);
        }
    }*/

public:
    /** Constructor.
     *  \param xyz Translation.
     *  \param hpr Rotation.
     */
    Coord(const Vec3& xyz, const Vec3& hpr)
    {
        m_xyz = xyz;
        m_hpr = hpr;
        //setSgCoord();
    }   // Coord
    // ------------------------------------------------------------------------
    /** Constructor based on a bullet transformation (which is a translation
     *  and rotation as well).
     *  \param t The bullet transform.
     */
    Coord(const btTransform& t)
    {
        m_xyz = t.getOrigin();
        m_hpr.setHPR(t.getBasis());
        //setSgCoord();
    }   // Coord
    // ------------------------------------------------------------------------
    /** Default constructor. Sets xyz and hpr to 0. */
    Coord()
    {
        m_xyz = Vec3(0.0f);
        m_hpr = Vec3(0.0f);
    }
    // ------------------------------------------------------------------------
    /** Returns the translation. */
    const Vec3&    getXYZ()     const     { return m_xyz;              }
    /** Returns heading, pitch, rolll. */
    const Vec3&    getHPR()     const     { return m_hpr;              }
    /** Returns X. */
    float          getX()       const     { return m_xyz.getX();       }
    /** Returns Y. */
    float          getY()       const     { return m_xyz.getY();       }
    /** Returns Z. */
    float          getZ()       const     { return m_xyz.getZ();       }
    /** Returns the heading. */
    float          getHeading() const     { return m_hpr.getHeading(); }

    /** Sets hpr. \param a Heading, pitch and roll. */
    void           setHPR(const Vec3& a)  { m_hpr = a; /*setSgCoord();*/   }
    /** Sets xyz. \param a Coordinates. */
    void           setXYZ(const Vec3& a)  { m_xyz = a; /*setSgCoord();*/   }
    /** Sets X. \param x New X value. */
    void           setX(float x)          { m_xyz.setX(x);             }
    /** Sets Y. \param y New Y value. */
    void           setY(float y)          { m_xyz.setY(y);             }
    /** Sets Z. \param z New Z value. */
    void           setZ(float z)          { m_xyz.setZ(z);             }
};   // Coord

#endif
