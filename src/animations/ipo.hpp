//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015  Joerg Henrichs
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
    /** This object stores the read-only data of an IPO. It might be
     *  shared among several instances of an Ipo (e.g. in a cannon
     *  animation). */
    class IpoData
    {
    public:
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

        /** Time of the first control point. */
        float m_start_time;

        /** Time of the last control point. */
        float m_end_time;

        /** Stores the inital position of the object. */
        Vec3 m_initial_xyz;

        /** Stores the inital rotation of the object. */
        Vec3 m_initial_hpr;
    private:
        float  getCubicBezier(float t, float p0, float p1,
                              float p2, float p3) const;
        float  getCubicBezierDerivative(float t, float p0, float p1,
                                        float p2, float p3) const;
        void approximateBezier(float t0, float t1,
                               const Vec3 &p0, const Vec3 &p1,
                               const Vec3 &h0, const Vec3 &h2,
                               unsigned int rec_level = 0);
    public:
               IpoData(const XMLNode &curve, float fps, bool reverse);
        void   readCurve(const XMLNode &node, bool reverse);
        void   readIPO(const XMLNode &node, float fps, bool reverse);
        float  approximateLength(float t0, float t1,
                                 const Vec3 &p0, const Vec3 &p1,
                                 const Vec3 &h1, const Vec3 &h2);
        float  adjustTime(float time);
        float  get(float time, unsigned int index, unsigned int n);
        float  getDerivative(float time, unsigned int index, unsigned int n);

    };   // IpoData
    // ------------------------------------------------------------------------
    /** The actual data of the IPO. This can be shared between Ipo (e.g. each
     *  cannon animation will use the same IpoData block, but its own instance
     *  of Ipo, since data like m_next_n should not be shared). */
    IpoData *m_ipo_data;

    /** True if m_ipo_data is 'owned' by this object and therefore needs to be
     *  freed. If an Ipo is cloned, it will share a reference to m_ipo_data,
     *  and must therefore not free it. */
    bool m_own_ipo_data;

    /** Which control points will be the next one (so m_next_n-1 and
    *  m_next_n are the control points to use now). This just reduces
    *  lookup time in get(t). To allow modifying this in get() const,
    *  it is declared mutable). */
    mutable unsigned int m_next_n;

    void updateNextN(float *time) const;

    Ipo(const Ipo *ipo);
public:
             Ipo(const XMLNode &curve, float fps=25, bool reverse=false);
    virtual ~Ipo();
    Ipo     *clone();
    void     update(float time, Vec3 *xyz=NULL, Vec3 *hpr=NULL,
                                Vec3 *scale=NULL);
    void     getDerivative(float time, Vec3 *xyz);
    float    get(float time, unsigned int index) const;
    void     setInitialTransform(const Vec3 &xyz, const Vec3 &hpr);
    void     reset();
    // ------------------------------------------------------------------------
    /** Returns the raw data points for this IPO. */
    const std::vector<Vec3>& getPoints() const { return m_ipo_data->m_points; }
    // ------------------------------------------------------------------------
    /** Returns the last specified time (i.e. not considering any extend
     *  types). */
    float getEndTime() const { return m_ipo_data->m_end_time; }
};   // Ipo

#endif

