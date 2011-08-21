//  $Id: ipo.hpp 1681 2008-04-09 13:52:48Z hikerstk $
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
    /** All supported ipo types. */
    enum IpoChannelType {IPO_LOCX,   IPO_LOCY,   IPO_LOCZ, 
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
    std::vector<core::vector2df>  m_points;
    /** Only used for bezier curves: the two handles. */
    std::vector<core::vector2df>  m_handle1, m_handle2;

    /** Current time in cycle. */
    float m_time;

    /** Minium time when this animations starts, usually 0. */
    float m_min_time;

    /** Time this animation finishes (or cycles). */
    float m_max_time;

    /** Frames per second for this animation. */
    float m_fps;

    /** Stores the inital position of the object. */
    core::vector3df m_initial_xyz;
    /** Stores the inital rotation of the object. */
    core::vector3df m_initial_hpr;
public:
            Ipo(const XMLNode &curve, float fps);
      void  update(float dt, core::vector3df *xyz, core::vector3df *hpr,
                   core::vector3df *scale);
      float get() const;
      void  setInitialTransform(const core::vector3df &xyz, 
                                const core::vector3df &hpr);
      void  reset();
    
      void  extendTo(float x);
    
    const std::vector<core::vector2df>& getPoints() const { return m_points; }

    
};   // Ipo

#endif

