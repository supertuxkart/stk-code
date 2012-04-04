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

#include "animations/ipo.hpp"

#include "io/xml_node.hpp"

const std::string Ipo::m_all_channel_names[IPO_MAX] =
                {"LocX", "LocY", "LocZ", 
                 "RotX", "RotY", "RotZ",
                 "ScaleX", "ScaleY", "ScaleZ" };

Ipo::Ipo(const XMLNode &curve, float fps)
{
    if(curve.getName()!="curve")
    {
        fprintf(stderr, "Expected 'curve' for animation, got '%s' --> Ignored.\n", 
            curve.getName().c_str());
        return;
    }
    std::string channel;
    curve.get("channel", &channel);
    m_channel=IPO_MAX;
    for(unsigned int i=IPO_LOCX; i<IPO_MAX; i++)
    {
        if(m_all_channel_names[i]==channel) m_channel=(IpoChannelType)i;
    }
    if(m_channel==IPO_MAX)
    {
        fprintf(stderr, "Unknown animation channel: '%s' - aborting.\n", channel.c_str());
        exit(-1);
    }

    std::string interp; 
    curve.get("interpolation", &interp);
    if     (interp=="const" ) m_interpolation = IP_CONST;
    else if(interp=="linear") m_interpolation = IP_LINEAR;
    else                      m_interpolation = IP_BEZIER;

    for(unsigned int i=0; i<curve.getNumNodes(); i++)
    {
        const XMLNode *node = curve.getNode(i);
        core::vector2df xy;  
        node->get("c", &xy);
        // Convert blender's frame number (1 ...) into time (0 ...)
        float t = (xy.X-1)/fps;
                
        xy.X = t;
        m_points.push_back(xy);
        if(m_interpolation==IP_BEZIER)
        {
            core::vector2df handle; 
            node->get("h1", &handle); 
            handle.X = (xy.X-1)/fps;
            m_handle1.push_back(handle);
            node->get("h2", &handle); 
            handle.X = (xy.X-1)/fps;
            m_handle2.push_back(handle);
        }
    }   // for i<getNumNodes()

    // ATM no other extends are supported, so hardcode the only one
    // that works!
    m_extend = ET_CYCLIC;

    reset();
}   // Ipo

// ----------------------------------------------------------------------------
/** Stores the initial transform. This is necessary for relative IPOs.
 *  \param xyz Position of the object.
 *  \param hpr Rotation of the object.
 */
void  Ipo::setInitialTransform(const core::vector3df &xyz, 
                                const core::vector3df &hpr)
{
    m_initial_xyz = xyz;
    m_initial_hpr = hpr;
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
 *  rotation (taking the cycle length etc. into account).
 *  \param time Current time for which to determine the interpolation.
 *  \param xyz The position that needs to be updated.
 *  \param hpr The rotation that needs to be updated.
 */
void Ipo::update(float time, core::vector3df *xyz, core::vector3df *hpr,
                 core::vector3df *scale)
{        
    switch(m_channel)
    {
    case Ipo::IPO_LOCX   : xyz->X   =  get(time); break;
    case Ipo::IPO_LOCY   : xyz->Y   =  get(time); break;
    case Ipo::IPO_LOCZ   : xyz->Z   =  get(time); break;
    case Ipo::IPO_ROTX   : hpr->X   =  get(time); break;
    case Ipo::IPO_ROTY   : hpr->Y   =  get(time); break;
    case Ipo::IPO_ROTZ   : hpr->Z   =  get(time); break;
    case Ipo::IPO_SCALEX : scale->X =  get(time); break;
    case Ipo::IPO_SCALEY : scale->Y =  get(time); break;
    case Ipo::IPO_SCALEZ : scale->Z =  get(time); break;
    default: assert(false); // shut up compiler warning
    }    // switch

}   // update

// ----------------------------------------------------------------------------
/** Returns the interpolated value at the current time (which this objects
 *  keeps track of).
 *  \param time The time for which the interpolated value should be computed.
 */
float Ipo::get(float time) const
{
    // Avoid crash in case that only one point is given for this IPO.
    if(m_next_n==0) 
        return m_points[0].Y;

    // Time was reset since the last cached value for n,
    // reset n to start from the beginning again.
    if(time < m_points[m_next_n-1].X)
        m_next_n = 1;
    // Search for the first point in the (sorted) array which is greater or equal
    // to the current time. 
    while(m_next_n<m_points.size()-1 && time >=m_points[m_next_n].X)
        m_next_n++;
    int n = m_next_n - 1;
    switch(m_interpolation)
    {
    case IP_CONST  : return m_points[n].Y;
    case IP_LINEAR : {
                        float t = time-m_points[n].X;
                        return m_points[n].Y + t*(m_points[n+1].Y-m_points[n].Y) /
                                                 (m_points[n+1].X-m_points[n].X);
                     }
    case IP_BEZIER: {  
                        if(n==m_points.size()-1)
                        {
                            // FIXME: only const implemented atm.
                            return m_points[n].Y;
                        }
                        core::vector2df c = 3.0f*(m_handle2[n]-m_points[n]);
                        core::vector2df b = 3.0f*(m_handle1[n+1]-m_handle2[n])-c;
                        core::vector2df a = m_points[n+1] - m_points[n] - c - b;
                        float t = (time-m_points[n].X)/(m_points[n+1].X-m_points[n].X);
                        core::vector2df r = ((a*t+b)*t+c)*t+m_points[n];
                        return r.Y;
                    }
    }   // switch
    // Keep the compiler happy:
    return 0;
}   // get

// ----------------------------------------------------------------------------
/** Inserts a new start point at the beginning of the IPO to make sure that
 *  this IPO starts with X.
 *  \param x The minimum value for which this IPO should be defined.
 */
void Ipo::extendStart(float x)
{
    assert(m_points[0].X > x);
    extend(x, 0);
}   // extendStart
// ----------------------------------------------------------------------------
/** Inserts an additional point at the end of the IPO to make sure that this
 *  IPO ends with X.
 *  \param x The maximum value for which this IPO should be defined.
 */
void Ipo::extendEnd(float x)
{
    assert(m_points[m_points.size()-1].X < x);
    extend(x, m_points.size()-1);
}   // extendEnd

// ----------------------------------------------------------------------------
/** Extends the IPO either at the beginning (n=0) or at the end (n=size()-1). 
 *  This is used by AnimationBase to make sure all IPOs of one curve have the 
 *  same cycle.
 *  \param x The X value to which the IPO must be extended.
 *  \param n The index at (before/after) which to extend.
 */
void Ipo::extend(float x, unsigned int n)
{
    switch (m_interpolation)
    {
        case IP_CONST:
        {
            core::vector2df new_point(x,  m_points[n].Y);
            if(n==0)
                m_points.insert(m_points.begin(), new_point);
            else
                m_points.push_back( new_point);
            break;
        }
        case IP_LINEAR:
        {
            core::vector2df new_point(x, m_points[n].Y);
            if(n=0)
                m_points.insert(m_points.begin(), new_point);
            else
                m_points.push_back(new_point);
            break;
        }
        case IP_BEZIER:
        {
            // FIXME: I'm somewhat dubious this is the correct way to extend handles
            core::vector2df new_h1 = m_handle1[n] + 
                                     core::vector2df(x - m_points[n].X ,0);
            core::vector2df new_h2 = m_handle2[n] + 
                                     core::vector2df(x - m_points[n].X ,0);
            core::vector2df new_p(x, m_points[n].Y);
            if(n==0)
            {
                m_handle1.insert(m_handle1.begin(), new_h1);
                m_handle2.insert(m_handle2.begin(), new_h2);
                m_points.insert(m_points.begin(), new_p);
            }
            else
            {
                m_handle1.push_back(new_h1);
                m_handle2.push_back(new_h2);
                m_points.push_back(new_p);
            }
            break;
        }            
    }
}   // extend

