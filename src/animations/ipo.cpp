//  $Id: ipo.cpp 1681 2008-04-09 13:52:48Z hikerstk $
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

//static std::string Ipo::m_all_names[2] = {std::string("a"), std::string("b")};


Ipo::Ipo(const XMLNode &curve, float fps)
{
	if(curve.getName()!="curve")
	{
		fprintf(stderr, "Expected 'curve' for animation, got '%s' --> Ignored.\n", 
			curve.getName().c_str());
		return;
	}
	std::string type;   curve.get("type",          &type  );
	if(type=="RotX") m_channel=IPO_ROTX;
	else if(type=="RotY") m_channel=IPO_ROTY;
	else if(type=="RotZ") m_channel=IPO_ROTZ;
	std::string interp; curve.get("interpolation", &interp);
	if     (interp=="const" ) m_interpolation = IP_CONST;
	else if(interp=="linear") m_interpolation = IP_LINEAR;
	else                      m_interpolation = IP_BEZIER;

	m_min_time = 999999;
	m_max_time = -1;
	for(unsigned int i=0; i<curve.getNumNodes(); i++)
	{
		const XMLNode *node = curve.getNode(i);
		Vec3 xy;  
		node->get2("c", &xy);
		// Convert blender's frame number (1 ...) into time (0 ...)
		float t = (xy.getX()-1)/fps;
		if(t<m_min_time) m_min_time = t;
		if(t>m_max_time) m_max_time = t;
		xy.setX(t);
		m_points.push_back(xy);
		if(m_interpolation==IP_BEZIER)
		{
			Vec3 handle; 
			node->get2("h1", &handle); 
			handle.setX((xy.getX()-1)/fps);
			m_handle1.push_back(handle);
			node->get2("h2", &handle); 
			handle.setX((xy.getX()-1)/fps);
			m_handle2.push_back(handle);
		}
	}   // for i<getNumNodes()
	
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
	m_time = 0;
}   // reset

// ----------------------------------------------------------------------------
/** Updates the time of this ipo and interpolates the new position and 
 *  rotation (taking the cycle length etc. into account).
 *  \param dt Time since last call.
 *  \param xyz The position that needs to be updated.
 *  \param hpr The rotation that needs to be updated.
 */
void Ipo::update(float dt, core::vector3df *xyz, core::vector3df *hpr)
{
	m_time += dt;
	if(m_time>m_max_time) m_time = 0;
		
	switch(m_channel)
	{
	case Ipo::IPO_LOCX : xyz->X = get(); break;
	case Ipo::IPO_LOCY : xyz->Y = get(); break;
	case Ipo::IPO_LOCZ : xyz->Z = get(); break;
	case Ipo::IPO_ROTX : hpr->Y = get()*10.0f; break;
	case Ipo::IPO_ROTY : hpr->Y = get()*10.0f; break;
	case Ipo::IPO_ROTZ : hpr->Z = get()*10.0f; break;
	}    // switch

}   // update

// ----------------------------------------------------------------------------
/** Returns the interpolated value at the current time (which this objects
 *  keeps track of).
 */
float Ipo::get() const
{
	if(m_time<m_min_time) 
	{
		// FIXME: should take extend into account!
		return 0;
	}
	unsigned int n=0;
	// Search for the first point in the (sorted) array which is greater or equal
	// to the current time. 
	// FIXME: we should store the last point to speed this up!
	while(n<m_points.size()-1 && m_time >=m_points[n].getX())
		n++;
	n--;
	switch(m_interpolation)
	{
	case IP_CONST  : return m_points[n].getY();
	case IP_LINEAR : {
						float t = m_time-m_points[n].getX();
						return m_points[n].getY() + t*(m_points[n+1].getY()-m_points[n].getY()) /
							                          (m_points[n+1].getX()-m_points[n].getX());
					 }
	case IP_BEZIER: {  // FIXME: for now only:
						return m_points[n].getY();
					}
	}   // switch
	// Keep the compiler happy:
	return 0;
}   // get
// ----------------------------------------------------------------------------
