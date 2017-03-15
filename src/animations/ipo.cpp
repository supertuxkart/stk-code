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

#include "animations/ipo.hpp"

#include "io/xml_node.hpp"
#include "utils/vs.hpp"
#include "utils/log.hpp"

#include <string.h>
#include <algorithm>
#include <cmath>

const std::string Ipo::m_all_channel_names[IPO_MAX] =
                {"LocX", "LocY", "LocZ", "LocXYZ",
                 "RotX", "RotY", "RotZ",
                 "ScaleX", "ScaleY", "ScaleZ" };

// ----------------------------------------------------------------------------
/** Initialise the Ipo from the specifications in the XML file.
 *  \param curve The XML node with the IPO data.
 *  \param fps Frames per second value, necessary to convert frame values
 *             into time.
 *  \param reverse If this is set to true, the ipo data will be reverse. This
 *             is used by the cannon if the track is driven in reverse.
 */
Ipo::IpoData::IpoData(const XMLNode &curve, float fps, bool reverse)
{
    if(curve.getName()!="curve")
    {
        Log::warn("Animations", "Expected 'curve' for animation, got '%s' --> Ignored.",
                  curve.getName().c_str());
        return;
    }
    std::string channel;
    curve.get("channel", &channel);
    m_channel=IPO_MAX;
    for(unsigned int i=IPO_LOCX; i<IPO_MAX; i++)
    {
        if(m_all_channel_names[i]==channel)
        {
            m_channel=(IpoChannelType)i;
            break;
        }
    }
    if(m_channel==IPO_MAX)
    {
        Log::error("Animation", "Unknown animation channel: '%s' --> Ignored",
                channel.c_str());
        return;
    }

    std::string interp;
    curve.get("interpolation", &interp);
    if     (interp=="const" ) m_interpolation = IP_CONST;
    else if(interp=="linear") m_interpolation = IP_LINEAR;
    else                      m_interpolation = IP_BEZIER;

    std::string extend;
    curve.get("extend", &extend);
    if      (extend=="cyclic") m_extend = ET_CYCLIC;
    else if (extend=="const" ) m_extend = ET_CONST;
    else
    {
        // For now extrap and cyclic_extrap do not work
        Log::warn("Animation", "Unsupported extend '%s' - defaulting to CONST.",
                  extend.c_str());
        m_extend = ET_CONST;
    }

    if(m_channel==IPO_LOCXYZ)
        readCurve(curve, reverse);
    else
        readIPO(curve, fps, reverse);

}   // IpoData

// ----------------------------------------------------------------------------
/** Reads a blender IPO curve, which constists of a frame number and a control
 *  point. This only handles a single axis.
 *  \param node The root node with all curve data points.
 *  \param fps Frames per second value, necessary to convert the frame based
 *             data from blender into times.
 *  \param reverse If this is set, the data are read in reverse. This is used
 *                 for a cannon in reverse mode.
 */
void Ipo::IpoData::readIPO(const XMLNode &curve, float fps, bool reverse)
{
    m_start_time =  999999.9f;
    m_end_time   = -999999.9f;
    for(unsigned int i=0; i<curve.getNumNodes(); i++)
    {
        int node_index = reverse ? curve.getNumNodes()-i-1 : i;
        const XMLNode *node = curve.getNode(node_index);
        core::vector2df xy;
        node->get("c", &xy);
        // Convert blender's frame number (1 ...) into time (0 ...)
        float t = (xy.X-1)/fps;
        Vec3 point(xy.Y, 0, 0, t);
        m_points.push_back(point);
        m_start_time = std::min(m_start_time, t);
        m_end_time   = std::max(m_end_time,   t);
        if(m_interpolation==IP_BEZIER)
        {
            Vec3 handle1, handle2;
            core::vector2df handle;
            node->get(reverse ? "h2" : "h1", &handle);
            handle1.setW((xy.X-1)/fps);
            handle1.setX(handle.Y);
            node->get(reverse ? "h1" : "h2", &handle);
            handle2.setW((xy.X-1)/fps);
            handle2.setX(handle.Y);
            m_handle1.push_back(handle1);
            m_handle2.push_back(handle2);
        }
    }   // for i<getNumNodes()
}   // IpoData::readIPO

// ----------------------------------------------------------------------------
/** Reads in 3 dimensional curve data - i.e. the xml file contains xyz, but no
 *  time. If the curve is using bezier interpolation, the curve is
 *  approximated by piecewise linear functions. Reason is that bezier curves
 *  can not (easily) be used for smooth (i.e. constant speed) driving:
 *  A linear time variation in [0, 1] will result in non-linear distances
 *  for the bezier function, which is a 3rd degree polynomial (--> the speed
 *  which is the deviation of this function is a 2nd degree polynomial, and
 *  therefore not constant!
 *  \param node The root node with all curve data points.
 *  \param reverse If this is set, the data are read in reverse. This is used
 *                 for a cannon in reverse mode.
 */
void Ipo::IpoData::readCurve(const XMLNode &curve, bool reverse)
{
    m_start_time =  0;
    m_end_time   = -999999.9f;
    float speed  = 30.0f;
    curve.get("speed", &speed);

    for(unsigned int i=0; i<curve.getNumNodes(); i++)
    {
        int node_index = reverse ? curve.getNumNodes()-i-1 : i;
        const XMLNode *node = curve.getNode(node_index);
        Vec3 point;
        node->get("c", &point);

        if(m_interpolation==IP_BEZIER)
        {
            Vec3 handle;
            node->get(reverse ? "h2" : "h1", &handle);
            m_handle1.push_back(handle);
            node->get(reverse ? "h1" : "h2", &handle);
            m_handle2.push_back(handle);
            if(i>0)
            {
                // We have to take a copy of the end point, since otherwise
                // it can happen that as more points are added to m_points
                // in the approximateBezier function, the data gets
                // reallocated and then the reference to the original point
                // is not correct anymore.
                Vec3 end_point = m_points[m_points.size()-1];
                approximateBezier(0.0f, 1.0f, end_point, point,
                                              m_handle2[i-1], m_handle1[i]);
            }
        }
        m_points.push_back(point);
    }   // for i<getNumNodes()

    // The handles of a bezier curve are not needed anymore and can be
    // removed now (since the bezier funciton has been replaced with a
    // piecewise linear
    if(m_interpolation==IP_BEZIER)
    {
        m_handle1.clear();
        m_handle2.clear();
        m_interpolation = IP_LINEAR;
    }

    if(m_points.size()==0) return;

    // Compute the time for each segment based on the speed and
    // store it in the W component.
    m_points[0].setW(0);
    for(unsigned int i=1; i<m_points.size(); i++)
    {
        m_points[i].setW( (m_points[i]-m_points[i-1]).length()/speed
                         + m_points[i-1].getW()                   );
    }
    m_end_time = m_points.back().getW();
}   // IpoData::readCurve

// ----------------------------------------------------------------------------
/** This function approximates a bezier curve by piecewise linear functions.
 *  It uses quite primitive approximations: if the estimated distance of
 *  the bezier curve at between t=t0 and t=t1 is greater than 2, it
 *  inserts one point at (t0+t1)/2, and recursively splits the two intervals
 *  further. End condition is either a maximum recursion depth of 6 or
 *  an estimated curve length of less than 2. It does not add any points
 *  at t=t0 or t=t1, only between this interval.
 *  \param t0, t1 The interval which is approximated.
 *  \param p0, p1, h0, h1: The bezier parameters.
 *  \param rec_level The recursion level to avoid creating too many points.
 */
void Ipo::IpoData::approximateBezier(float t0, float t1,
                               const Vec3 &p0, const Vec3 &p1,
                               const Vec3 &h0, const Vec3 &h1,
                               unsigned int rec_level)
{
    // Limit the granularity by limiting the recursion depth
    if(rec_level>6)
        return;

    float distance = approximateLength(t0, t1, p0, p1, h0, h1);
    // A more sophisticated estimation might be useful (e.g. taking the
    // difference between a linear approximation and the actual bezier
    // curve into accound.
    if(distance<=0.2f)
        return;

    // Insert one point at (t0+t1)/2. First split the left part of
    // the interval by a recursive call, then insert the point at
    // (t0+t1)/2, then approximate the right part of the interval.
    approximateBezier(t0, (t0+t1)*0.5f, p0, p1, h0, h1, rec_level + 1);
    Vec3 middle;
    for(unsigned int j=0; j<3; j++)
        middle[j] = getCubicBezier((t0+t1)*0.5f, p0[j], h0[j], h1[j], p1[j]);
    m_points.push_back(middle);
    approximateBezier((t0+t1)*0.5f, t1, p0, p1, h0, h1, rec_level + 1);

}   // approximateBezier

// ----------------------------------------------------------------------------
/** Approximates the length of a bezier curve using a simple Euler
 *  approximation by dividing the interval [t0, t1] into 10 pieces. Good enough
 *  for our needs in STK.
 *  \param t0, t1 Approximate for t in [t0, t1].
 *  \param p0, p1 The start and end point of the curve.
 *  \param h0, h1 The control points for the corresponding points.
 */
float Ipo::IpoData::approximateLength(float t0, float t1,
                                      const Vec3 &p0, const Vec3 &p1,
                                      const Vec3 &h0, const Vec3 &h1)
{
    assert(m_interpolation == IP_BEZIER);

    float distance=0;
    const unsigned int NUM_STEPS=10;
    float delta = (t1-t0)/NUM_STEPS;
    Vec3 prev_point;
    for(unsigned int j=0; j<3; j++)
        prev_point[j] = getCubicBezier(t0, p0[j], h0[j], h1[j], p1[j]);
    for(unsigned int i=1; i<=NUM_STEPS; i++)
    {
        float t = t0 + i * delta;
        Vec3 next_point;
        // Interpolate all three axis
        for(unsigned j=0; j<3; j++)
        {
            next_point[j] = getCubicBezier(t, p0[j], h0[j], h1[j], p1[j]);
        }
        distance  += (next_point - prev_point).length();
        prev_point = next_point;
    }   // for i< NUM_STEPS

    return distance;
}   // IpoData::approximateLength

// ----------------------------------------------------------------------------
/** Adjusts the time so that it is between start and end of this Ipo. This
 *  takes the extend type into account, e.g. cyclic animations will just
 *  use a modulo operation, while constant extends will return start or
 *  end time directly.
 *  \param time The time to adjust.
 */
float Ipo::IpoData::adjustTime(float time)
{
    if(time<m_start_time)
    {
        switch(m_extend)
        {
        case IpoData::ET_CYCLIC:
            time = m_start_time + fmodf(time, m_end_time-m_start_time); break;
        case ET_CONST:
            time = m_start_time; break;
        default:
            // FIXME: ET_CYCLIC_EXTRAP and ET_EXTRAP missing
            assert(false);
        }   // switch m_extend
    }   // if time < m_start_time

    else if(time > m_end_time)
    {
        switch(m_extend)
        {
        case ET_CYCLIC:
            time = m_start_time + fmodf(time, m_end_time-m_start_time); break;
        case ET_CONST:
            time = m_end_time; break;
        default:
            // FIXME: ET_CYCLIC_EXTRAP and ET_EXTRAP missing
            assert(false);
        }   // switch m_extend
    }   // if time > m_end_time
    return time;
}   // adjustTime

// ----------------------------------------------------------------------------
float Ipo::IpoData::get(float time, unsigned int index, unsigned int n)
{
    switch(m_interpolation)
    {
    case IP_CONST  : return m_points[n][index];
    case IP_LINEAR : {
                        float t = time-m_points[n].getW();
                        return m_points[n][index]
                             + t*(m_points[n+1][index]-m_points[n][index]) /
                                 (m_points[n+1].getW()-m_points[n].getW());
                     }
    case IP_BEZIER:  {  if(n==m_points.size()-1)
                        {
                            // FIXME: only const implemented atm.
                            return m_points[n][index];
                        }
                        float t = (time-m_points[n].getW())
                                / (m_points[n+1].getW()-m_points[n].getW());
                        return getCubicBezier(t,
                                              m_points [n  ][index],
                                              m_handle2[n  ][index],
                                              m_handle1[n+1][index],
                                              m_points [n+1][index]);
                    }
    }   // switch
    // Keep the compiler happy:
    return 0;
}   // IpoData::get

// ----------------------------------------------------------------------------
/** Computes a cubic bezier curve for a given t in [0,1] and four control
 *  points. The curve will go through p0 (t=0), p3 (t=1).
 *  \param t The parameter for the bezier curve, must be in [0,1].
 *  \param p0, p1, p2, p3 The four control points.
 */
float Ipo::IpoData::getCubicBezier(float t, float p0, float p1,
                                   float p2, float p3) const
{
    float c = 3.0f*(p1-p0);
    float b = 3.0f*(p2-p1)-c;
    float a = p3 - p0 - c - b;
    return ((a*t+b)*t+c)*t+p0;
}   // getCubicBezier

// ----------------------------------------------------------------------------
/** Determines the derivative of a IPO at a given point.
 *  \param time At what time value the derivative is to be computed.
 *  \param index IpoData is based on 3d data. The index specified which
 *         value to use (0=x, 1=y, 2=z).
 *  \param n Curve segment to be used for the computation. It must be correct
 *         for the specified time value.
 */
float Ipo::IpoData::getDerivative(float time, unsigned int index,
                                  unsigned int n)
{
    switch (m_interpolation)
    {
    case IP_CONST: return 0;   // Const --> Derivative is 0
    case IP_LINEAR: {
        return (m_points[n + 1][index] - m_points[n][index]) /
               (m_points[n + 1].getW() - m_points[n].getW());
    }
    case IP_BEZIER: {
        if (n == m_points.size() - 1)
        {
            // Only const, so derivative is 0
            return 0;
        }
        float t = (time - m_points[n].getW())
                / (m_points[n + 1].getW() - m_points[n].getW());
        return getCubicBezierDerivative(t,
                                        m_points [n    ][index],
                                        m_handle2[n    ][index],
                                        m_handle1[n + 1][index],
                                        m_points [n + 1][index] );
    }   // case IPBEZIER
    default:
        Log::warn("Ipo::IpoData", "Incorrect interpolation %d",
                  m_interpolation);
    }   // switch
    return 0;
}   // IpoData::getDerivative


// ----------------------------------------------------------------------------
/** Returns the derivative of a cubic bezier curve for a given t in [0,1] and
 *  four control points. The curve will go through p0 (t=0).
 *  \param t The parameter for the bezier curve, must be in [0,1].
 *  \param p0, p1, p2, p3 The four control points.
 */
float Ipo::IpoData::getCubicBezierDerivative(float t, float p0, float p1,
                                             float p2, float p3) const
{
    float c = 3.0f*(p1 - p0);
    float b = 3.0f*(p2 - p1) - c;
    float a = p3 - p0 - c - b;
    // f(t)      = ((a*t + b)*t + c)*t + p0;
    //           = a*t^3 +b*t^2 + c*t + p0
    // --> f'(t) = 3*a*t^2 + 2*b*t + c
    return (3*a * t + 2*b) * t + c;
}   // bezier

// ============================================================================
/** The Ipo constructor. Ipos can share the actual data to interpolate, which
 *  is stored in a separate IpoData object, see Ipo(const Ipo *ipo)
 *  constructor. This is used for cannons: the actual check line stores the
 *  'master' Ipo, and each actual IPO that animate a kart just use a copy
 *  of this read-only data.
 *  \param curve The XML data for this curve.
 *  \param fps Frames per second, used to convert all frame based value
 *         in the xml file into seconds.
 *  \param reverse If this is set to true, the ipo data will be reverse. This
 *             is used by the cannon if the track is driven in reverse.
 */
Ipo::Ipo(const XMLNode &curve, float fps, bool reverse)
{
    m_ipo_data     = new IpoData(curve, fps, reverse);
    m_own_ipo_data = true;
    reset();
}   // Ipo

// ----------------------------------------------------------------------------
/** A copy constructor. It shares the read-only data with the source Ipo
 *  \param ipo The ipo to copy from.
 */
Ipo::Ipo(const Ipo *ipo)
{
    // Share the read-only data
    m_ipo_data     = ipo->m_ipo_data;
    m_own_ipo_data = false;
    reset();
}   // Ipo(Ipo*)

// ----------------------------------------------------------------------------
/** Creates a copy of this object (the copy constructor is disabled in order
 *  to avoid implicit copies happening).
 */
Ipo  *Ipo::clone()
{
    return new Ipo(this);
}   // clone

// ----------------------------------------------------------------------------
/** The destructor only frees IpoData if it was created by this instance (and
 *  not if this instance was copied, therefore sharing the IpoData).
 */
Ipo::~Ipo()
{
    if(m_own_ipo_data)
        delete m_ipo_data;
}   // ~Ipo

// ----------------------------------------------------------------------------
/** Stores the initial transform. This is necessary for relative IPOs.
 *  \param xyz Position of the object.
 *  \param hpr Rotation of the object.
 */
void  Ipo::setInitialTransform(const Vec3 &xyz,
                               const Vec3 &hpr)
{
    m_ipo_data->m_initial_xyz = xyz;
    m_ipo_data->m_initial_hpr = hpr;
}   // setInitialTransform

// ----------------------------------------------------------------------------
/** Resets the IPO for (re)starting an animation.
 */
void Ipo::reset()
{
    m_next_n = 1;
}   // reset

// ----------------------------------------------------------------------------
/** Updates the time of this ipo and interpolates the new position and
 *  rotation (taking the cycle length etc. into account). If a NULL is
 *  given, the value is not updated.
 *  \param time Current time for which to determine the interpolation.
 *  \param xyz The position that needs to be updated (can be NULL).
 *  \param hpr The rotation that needs to be updated (can be NULL).
 *  \param scale The scale that needs to be updated (can be NULL)
 */
void Ipo::update(float time, Vec3 *xyz, Vec3 *hpr,Vec3 *scale)
{
    assert(!std::isnan(time));
    switch(m_ipo_data->m_channel)
    {
    case Ipo::IPO_LOCX   : if(xyz)   xyz  ->setX(get(time, 0)); break;
    case Ipo::IPO_LOCY   : if(xyz)   xyz  ->setY(get(time, 0)); break;
    case Ipo::IPO_LOCZ   : if(xyz)   xyz  ->setZ(get(time, 0)); break;
    case Ipo::IPO_ROTX   : if(hpr)   hpr  ->setX(get(time, 0)); break;
    case Ipo::IPO_ROTY   : if(hpr)   hpr  ->setY(get(time, 0)); break;
    case Ipo::IPO_ROTZ   : if(hpr)   hpr  ->setZ(get(time, 0)); break;
    case Ipo::IPO_SCALEX : if(scale) scale->setX(get(time, 0)); break;
    case Ipo::IPO_SCALEY : if(scale) scale->setY(get(time, 0)); break;
    case Ipo::IPO_SCALEZ : if(scale) scale->setZ(get(time, 0)); break;
    case Ipo::IPO_LOCXYZ :
        {
            if(xyz)
            {
                for(unsigned int j=0; j<3; j++)
                    (*xyz)[j] = get(time, j);
            }
            break;
        }

    default: assert(false); // shut up compiler warning
    }    // switch

}   // update

// ----------------------------------------------------------------------------
/** Updates the value of m_next_n to point to the right ipo segment based on
 *  the time.
 *  \param t Time for which m_next_n needs to be updated.
 */
void Ipo::updateNextN(float *time) const
{
    *time = m_ipo_data->adjustTime(*time);

    // Time was reset since the last cached value for n,
    // reset n to start from the beginning again.
    if (*time < m_ipo_data->m_points[m_next_n - 1].getW())
        m_next_n = 1;
    // Search for the first point in the (sorted) array which is greater or equal
    // to the current time.
    while (m_next_n < m_ipo_data->m_points.size() - 1 &&
        *time >= m_ipo_data->m_points[m_next_n].getW())
    {
        m_next_n++;
    }   // while
}   // updateNextN

// ----------------------------------------------------------------------------
/** Returns the interpolated value at the current time (which this objects
 *  keeps track of).
 *  \param time The time for which the interpolated value should be computed.
 */
float Ipo::get(float time, unsigned int index) const
{
    assert(!std::isnan(time));

    // Avoid crash in case that only one point is given for this IPO.
    if(m_next_n==0)
        return m_ipo_data->m_points[0][index];

    updateNextN(&time);

    float rval = m_ipo_data->get(time, index, m_next_n-1);
    assert(!std::isnan(rval));
    return rval;
}   // get

// ----------------------------------------------------------------------------
/** Returns the derivative for any location based curves.
 *  \param time Time for which the derivative is being computed.
 *  \param xyz Pointer where the results should be stored.
 */
void Ipo::getDerivative(float time, Vec3 *xyz)
{
    // Avoid crash in case that only one point is given for this IPO.
    if (m_next_n == 0)
    {
        // Derivative has no real meaning in case of a single point.
        // So just return a dummy value.
        xyz->setValue(1, 0, 0);
        return;
    }

    updateNextN(&time);
    switch (m_ipo_data->m_channel)
    {
    case Ipo::IPO_LOCX: xyz->setX(m_ipo_data->getDerivative(time, m_next_n, 0)); break;
    case Ipo::IPO_LOCY: xyz->setY(m_ipo_data->getDerivative(time, m_next_n, 0)); break;
    case Ipo::IPO_LOCZ: xyz->setZ(m_ipo_data->getDerivative(time, m_next_n, 0)); break;
    case Ipo::IPO_LOCXYZ:
    {
        if (xyz)
        {
            for (unsigned int j = 0; j < 3; j++)
                (*xyz)[j] = m_ipo_data->getDerivative(time, j, m_next_n-1);
        }
        break;
    }
    default: Log::warn("IPO", "Unexpected channel %d for derivate.",
                       m_ipo_data->m_channel                         );
        xyz->setValue(1, 0, 0);
        break;
    }   // switch


}   // getDerivative

