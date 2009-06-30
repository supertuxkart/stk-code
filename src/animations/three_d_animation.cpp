//  $Id: three_d_animation.cpp 1681 2008-04-09 13:52:48Z hikerstk $
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

#include "animations/three_d_animation.hpp"

#include <stdio.h>

#include "animations/ipo.hpp"
#include "graphics/irr_driver.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "tracks/bezier_curve.hpp"

ThreeDAnimation::ThreeDAnimation(const std::string &track_name, 
								 const XMLNode &node, float fps) 
               : AnimationBase(node, fps)
{
	std::string model_name;
	node.get("obj", &model_name);

	std::string full_path = file_manager->getTrackFile(model_name, track_name);
	m_mesh = irr_driver->getAnimatedMesh(full_path);
    if(!m_mesh)
    {
        fprintf(stderr, "Warning: animated model '%s' not found, aborting.\n",
                node.getName().c_str(), model_name.c_str());
        exit(-1);
    }
	m_animated_node = irr_driver->addAnimatedMesh(m_mesh);
	setInitialTransform(m_animated_node->getPosition(), m_animated_node->getRotation());
}   // ThreeDAnimation

// ----------------------------------------------------------------------------
/** Updates position and rotation of this model. Called once per time step.
 *  \param dt Time since last call.
 */
void ThreeDAnimation::update(float dt)
{
	core::vector3df xyz = m_animated_node->getPosition();
	core::vector3df hpr = m_animated_node->getRotation();
	AnimationBase::update(dt, &xyz, &hpr);     //updates all IPOs
	m_animated_node->setPosition(xyz);
	m_animated_node->setRotation(hpr);
}   // update