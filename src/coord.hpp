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


#ifndef HEADER_COORD_H
#define HEADER_COORD_H
#include <plib/sg.h>

#include "vec3.hpp"
#include "constants.hpp"
#include "LinearMath/btTransform.h"

class Coord
{
private:
    Vec3       m_xyz;
    Vec3       m_hpr;
    sgCoord    m_coord;

    void setSgCoord()
    {
        sgSetCoord(&m_coord, m_xyz.toFloat(), m_hpr.toFloat());
        // Convert hpr in radians to degrees, which sg needs.
        for(int i=0; i<3; i++)
        {
            m_coord.hpr[i] = RAD_TO_DEGREE(m_coord.hpr[i]);
        }
    }

public:
    Coord(const Vec3& xyz, const Vec3& hpr)
    {
        m_xyz = xyz;
        m_hpr = hpr;
        setSgCoord();
    }   // Coord
    // ------------------------------------------------------------------------
    Coord(const btTransform& t)
    {
        m_xyz = t.getOrigin();
        m_hpr.setHPR(t.getBasis());
        setSgCoord();
    }   // Coord
    // ------------------------------------------------------------------------
    const sgCoord& toSgCoord() const     { return m_coord;          }
    const Vec3&    getXYZ()    const     { return m_xyz;            }
    const Vec3&    getHPR()    const     { return m_hpr;            }
    void           setHPR(const Vec3& a) { m_hpr = a; setSgCoord(); }
    void           setXYZ(const Vec3& a) { m_xyz = a; setSgCoord(); }
};   // Coord

#endif
