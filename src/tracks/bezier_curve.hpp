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

#ifndef HEADER_BEZIER_HPP
#define HEADER_BEZIER_HPP

#include <vector>

#include "utils/aligned_array.hpp"
#include "utils/vec3.hpp"

class XMLNode;

/**
  * A class to manage bezier curves and interpolation.
  * \ingroup tracks
  */
class BezierCurve
{
private:
    /** A data structure to store one bezier control point and
     *  the two handles. */
    struct BezierData
    {
        /** The control point. */
        Vec3 m_control_point;
        /** First handle, i.e. the one towards the previous point. */
        Vec3 m_handle1;
        /** Second handle, i.e. the one towards the next point. */
        Vec3 m_handle2;
    };   // BezierData

    /** Vector with all control points and handles. */
    AlignedArray<BezierData> m_all_data;
public:
    BezierCurve(const XMLNode &node);
    Vec3 getXYZ(float t) const;
    Vec3 getHPR(float t) const;

    /** Returns the number of points in this bezier curve. */
    unsigned int getNumPoints() const {return (unsigned int) m_all_data.size(); }
};   // BezierCurve
#endif
