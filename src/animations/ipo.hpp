//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009  Joerg Henrichs
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

#ifndef HEADER_IPO_HPP
#define HEADER_IPO_HPP

#include <string>
#include <vector>

#include <vector2d.h>
#include <vector3d.h>
using namespace irr;

#include "utils/no_copy.hpp"
#include "utils/vec3.hpp"

class XMLNode;

/** \brief A class to manage a single blender IPO curve.
  * \ingroup animations
  */
class Ipo : public NoCopy
{
public:
    /** All supported ipo types. LOCXYZ is basically a curve, the
     *  IPO is actually a 3d curve, without a time axis, only the
     *  actual data points. */
    enum IpoChannelType {IPO_LOCX,   IPO_LOCY,   IPO_LOCZ,
                         IPO_LOCXYZ,
                         IPO_ROTX,   IPO_ROTY,   IPO_ROTZ,
                         IPO_SCALEX, IPO_SCALEY, IPO_SCALEZ,
                         IPO_MAX};
    static const std::string m_all_channel_names[IPO_MAX];
private:
    /** The type of this IPO. */
    IpoChannelType m_channel;

    /** The three interpolations defined by blender. */
    enum {IP_CONST, IP_LINEAR, IP_BEZIER}                   m_interpolation;
    /** The four extend types. */
    enum {ET_CONST, ET_EXTRAP, ET_CYCLIC_EXTRAP, ET_CYCLIC} m_extend;

    /** The actual control points. */
    std::vector<Vec3>  m_points;

    /** Only used for bezier curves: the two handles. */
    std::vector<Vec3>  m_handle1, m_handle2;

    /** Frames per second for this animation. */
    float m_fps;

    /** Time of the first control point. */
    float m_start_time;

    /** Time of the last control point. */
    float m_end_time;

    /** Which control points will be the next one (so m_next_n-1 and
     *  m_next_n are the control points to use now). This just reduces
     *  lookup time in get(t). To allow modifying this in get() const,
     *  it is declared mutable). */
    mutable unsigned int m_next_n;

    /** Stores the inital position of the object. */
    Vec3 m_initial_xyz;

    /** Stores the inital rotation of the object. */
    Vec3 m_initial_hpr;

    void extend(float x, unsigned int n);
    float getCubicBezier(float t, float p0, float p1, 
                         float p2, float p3) const;
public:
          Ipo(const XMLNode &curve, float fps);
    void  update(float time, Vec3 *xyz, Vec3 *hpr, Vec3 *scale);
    float get(float time) const;
    void  setInitialTransform(const Vec3 &xyz, const Vec3 &hpr);
    void  reset();

    void  extendStart(float x);
    void  extendEnd(float x);

    // ------------------------------------------------------------------------
    /** Returns the raw data points for this IPO. */
    const std::vector<Vec3>& getPoints() const { return m_points; }
    // ------------------------------------------------------------------------
    /** Returns the last specified time (i.e. not considering any extend 
     *  types). */
    float getEndTime() const { return m_end_time; }
};   // Ipo

#endif

