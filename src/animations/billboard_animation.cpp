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
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "io/file_manager.hpp"

#include <ISceneManager.h>
#include <ICameraSceneNode.h>
#include <SColor.h>
#include <IBillboardSceneNode.h>

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
    
    m_fade_out_when_close = false;
    xml_node.get("fadeout", &m_fade_out_when_close);
    
    if (m_fade_out_when_close)
    {
        xml_node.get("start",  &m_fade_out_start);
        xml_node.get("end",    &m_fade_out_end  );
    }
    
    video::ITexture *texture = 
        irr_driver->getTexture(file_manager->getTextureFile(texture_name));
    m_node = irr_driver->addBillboard(core::dimension2df(width, height), 
                                                       texture);
    Material *stk_material = material_manager->getMaterial(texture_name);
    stk_material->setMaterialProperties(&(m_node->getMaterial(0)), NULL);

    m_node->setPosition(m_init_xyz);
}   // BillboardAnimation

// ----------------------------------------------------------------------------
/** Update the animation, called one per time step.
 *  \param dt Time since last call. */
void BillboardAnimation::update(float dt)
{
    if ( UserConfigParams::m_graphical_effects )
    {
        Vec3 xyz(m_node->getPosition());
        // Rotation doesn't make too much sense for a billboard, 
        // so just set it to 0
        Vec3 hpr(0, 0, 0);
        Vec3 scale = m_node->getScale();
        AnimationBase::update(dt, &xyz, &hpr, &scale);
        m_node->setPosition(xyz.toIrrVector());
        m_node->setScale(scale.toIrrVector());
        // Setting rotation doesn't make sense
    }
    
    if (m_fade_out_when_close)
    {
        scene::ICameraSceneNode* curr_cam = irr_driver->getSceneManager()->getActiveCamera();
        const float dist =  m_node->getPosition().getDistanceFrom( curr_cam->getPosition() );

        scene::IBillboardSceneNode* node = (scene::IBillboardSceneNode*)m_node;
        
        if (dist < m_fade_out_start)
        {
            node->setColor(video::SColor(0, 255, 255, 255));
        }
        else if (dist > m_fade_out_end)
        {
            node->setColor(video::SColor(255, 255, 255, 255));
        }
        else
        {
            int a = (int)(255*(dist - m_fade_out_start) / (m_fade_out_end - m_fade_out_start));
            node->setColor(video::SColor(a, 255, 255, 255));
        }
    }
}   // update
