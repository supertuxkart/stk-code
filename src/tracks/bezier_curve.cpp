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

#include "tracks/bezier_curve.hpp"

#include "io/xml_node.hpp"

BezierCurve::BezierCurve(const XMLNode &node)
{
    for(unsigned int i=0; i<node.getNumNodes(); i++)
    {
        const XMLNode *p = node.getNode(i);
        BezierData b;
        p->get("c",  &b.m_control_point);
        p->get("h1", &b.m_handle1);
        p->get("h2", &b.m_handle2);
        m_all_data.push_back(b);
    }   // for i<node.getNumNodes()
}   // BezierCurve

// ----------------------------------------------------------------------------
Vec3 BezierCurve::getXYZ(float t) const
{
    unsigned int i=int(t);   // FIXME: have to figure out which point we want here
    if(i>=(unsigned int)m_all_data.size()-1)
        return m_all_data[i].m_control_point;

    const BezierData &p0 = m_all_data[i];
    const BezierData &p1 = m_all_data[i+1];

    Vec3 c = 3*(p0.m_handle2-p0.m_control_point);
    Vec3 b = 3*(p1.m_handle1-p0.m_handle2)-c;
    Vec3 a = p1.m_control_point - p0.m_control_point - c - b;

    t = t-i;
    Vec3 r = a*t*t*t + b*t*t + c*t + p0.m_control_point;
    return r;
}   // getXYZ
// ----------------------------------------------------------------------------
Vec3 BezierCurve::getHPR(float t) const
{
    // FIXME: not yet implemented
    Vec3 hpr(0,0,0);
    return hpr;
}   // getHPR

