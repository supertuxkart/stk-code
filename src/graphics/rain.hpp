//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011  Joerg Henrichs, Marianne Gagnon
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

#ifndef HEADER_RAIN_HPP
#define HEADER_RAIN_HPP

class PerCameraNode;

#include <irrlicht.h>
const int RAIN_RING_COUNT = 5;

class Rain
{
    PerCameraNode* m_node[RAIN_RING_COUNT];

    std::vector<irr::video::SMaterial*> m_materials;
    
    float m_y;
    
public:
    Rain(irr::scene::ICameraSceneNode* camera, irr::scene::ISceneNode* parent);
    ~Rain();
    
    void update(float dt);
    void setPosition(const irr::core::vector3df& position);
    
    void setCamera(scene::ICameraSceneNode* camera);
};

#endif
