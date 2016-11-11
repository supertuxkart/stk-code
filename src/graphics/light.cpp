//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 Lauri Kasanen
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

#include "graphics/light.hpp"

#include <ISceneManager.h>

core::aabbox3df LightNode::box;

LightNode::LightNode(scene::ISceneManager* mgr, scene::ISceneNode* parent, float e, float d, float r, float g, float b):
                     ISceneNode(parent == NULL ? mgr->getRootSceneNode() : parent, mgr, -1)
{
    m_energy = e;
    m_radius = d;
    m_energy_multiplier = 1.0f;
    m_color[0] = r;
    m_color[1] = g;
    m_color[2] = b;

#ifdef __LIGHT_NODE_VISUALISATION__
    m_viz_added = false;
#endif
}

LightNode::~LightNode()
{
}

void LightNode::render()
{
    return;
}

void LightNode::OnRegisterSceneNode()
{ // This node is only drawn manually.

#ifdef __LIGHT_NODE_VISUALISATION__
    if (!m_viz_added)
    {
        scene::IMeshSceneNode* viz = irr_driver->addSphere(0.5f, video::SColor(255, m_color[0]*255, m_color[1]*255, m_color[2]*255));
        viz->setPosition(this->getAbsolutePosition());
        m_viz_added = true;
    }
#endif
}
