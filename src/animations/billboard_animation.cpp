//  $Id: billboard_animation.cpp 1681 2008-04-09 13:52:48Z hikerstk $
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

#include "animations/billboard_animation.hpp"
#include "graphics/irr_driver.hpp"
#include "io/file_manager.hpp"

class XMLNode;

/** A 2d billboard animation. */
BillboardAnimation::BillboardAnimation(const XMLNode &xml_node)
                  : AnimationBase(xml_node)
{
    std::string texture_name;
    float       width, height;

    xml_node.get("texture", &texture_name);
    xml_node.get("width",   &width       );
    xml_node.get("height",  &height      );
    core::vector3df xyz;
    xml_node.getXYZ(&xyz);
    video::ITexture *texture = 
        irr_driver->getTexture(file_manager->getTextureFile(texture_name));
    m_node = irr_driver->addBillboard(core::dimension2df(width, height), 
                                                       texture);
    m_node-> setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL );

    m_node->setPosition(xyz);
}   // BillboardAnimation

// ----------------------------------------------------------------------------
/** Update the animation, called one per time step.
 *  \param dt Time since last call. */
void BillboardAnimation::update(float dt)
{
    core::vector3df xyz(0, 0, 0);
    core::vector3df hpr(0, 0, 0);
    core::vector3df scale(1,1,1);
    AnimationBase::update(dt, &xyz, &hpr, &scale);
    m_node->setPosition(xyz);
    m_node->setScale(xyz);
    // Setting rotation doesn't make sense
}   // update
