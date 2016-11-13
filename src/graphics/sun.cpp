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

#include "graphics/sun.hpp"

#include "graphics/irr_driver.hpp"

SunNode::SunNode(scene::ISceneManager* mgr, scene::ISceneNode* parent,
                 float r, float g, float b)
       : LightNode(mgr, parent, 0., 0., r, g, b)
{
    m_color[0] = r;
    m_color[1] = g;
    m_color[2] = b;
}   // SunNode

// ----------------------------------------------------------------------------
SunNode::~SunNode()
{
}   // ~SunNode

// ----------------------------------------------------------------------------
void SunNode::render()
{
    irr_driver->setSunColor(video::SColorf(m_color[0], m_color[1], m_color[2]));
    irr_driver->setSunDirection(getPosition());

}   // render
