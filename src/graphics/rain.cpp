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

#include "audio/sfx_base.hpp"
#include "audio/sfx_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/material.hpp"
#include "graphics/per_camera_node.hpp"
#include "graphics/rain.hpp"
#include "modes/world.hpp"
#include "states_screens/race_gui.hpp"
#include "utils/constants.hpp"
#include "utils/random_generator.hpp"

#include <SMeshBuffer.h>
#include <SMesh.h>

const float RAIN_RADIUS[RAIN_RING_COUNT] = { 1.0f, 3.0f, 6.0f, 12.0f, 24.0f };
const float RAIN_Y_TO = 25.0f;
const float RAIN_Y_FROM = -10.0f;
const float RAIN_DY = 2.5f;
const float RAIN_DX = 0.0f;

const float TEXTURE_X_TILES[RAIN_RING_COUNT] = { 2.0f, 2.5f, 3.5f, 5.0f, 8.0f };
const float TEXTURE_Y_TILES[RAIN_RING_COUNT] = { 8.0f, 7.0f, 6.0f, 4.0f, 4.0f };


Rain::Rain(irr::scene::ICameraSceneNode* camera, irr::scene::ISceneNode* parent, bool lightning)
{
    m_lightning = lightning;
    
    if (lightning) m_thunder_sound = sfx_manager->createSoundSource("thunder");
    
    Material* m = material_manager->getMaterial("rain.png");
    assert(m != NULL);
    
    RandomGenerator g;
    m_next_lightning = (float)g.get(35);
    
    for (int r=0; r<RAIN_RING_COUNT; r++)
    {
        m_x[r] = r/(float)RAIN_RING_COUNT;
        m_y[r] = r/(float)RAIN_RING_COUNT;
        
        scene::SMeshBuffer *buffer = new scene::SMeshBuffer();
        
        buffer->Material.setTexture(0, m->getTexture());
        m->setMaterialProperties(&buffer->Material, NULL);
        buffer->Material.ZWriteEnable = false;
        buffer->Material.BackfaceCulling = false;
        
        m_materials.push_back(&buffer->Material);
        
        video::S3DVertex v;
        v.Color.set(255,255,255,255);
        
        // create a cylinder mesh
        const int VERTICES = 17;
        
        for (int vid=0; vid<VERTICES*2; vid+=2)
        {
            const float ratio = float(vid) / float(VERTICES-1);
            const float angle = ratio * 2.0f * M_PI;
            
            v.Pos.X = cos(angle)*RAIN_RADIUS[r];
            v.Pos.Y = RAIN_Y_TO;
            v.Pos.Z = sin(angle)*RAIN_RADIUS[r];
            
            // offset the X coord in texturing so you don't see textures from
            // the different rings lining up
            v.TCoords.X = ratio*TEXTURE_X_TILES[r] + r/3.0f;
            v.TCoords.Y = TEXTURE_Y_TILES[r];
            buffer->Vertices.push_back(v);
            
            v.Pos.Y =  RAIN_Y_FROM;
            
            v.TCoords.Y = 0.0f;
            buffer->Vertices.push_back(v);
            
            if (vid > 0)
            {
                buffer->Indices.push_back(vid-2);
                buffer->Indices.push_back(vid-1);
                buffer->Indices.push_back(vid);
                buffer->Indices.push_back(vid-1);
                buffer->Indices.push_back(vid);
                buffer->Indices.push_back(vid+1);
            }
        }

        scene::SMesh* mesh = new scene::SMesh();
        mesh->addMeshBuffer(buffer);
        mesh->recalculateBoundingBox();
        
        m_node[r] = irr_driver->addPerCameraMesh(mesh, camera, parent);
        mesh->drop();
        
        buffer->drop();
    }
}   // Rain

// ----------------------------------------------------------------------------

Rain::~Rain()
{
    for (int r=0; r<RAIN_RING_COUNT; r++)
    {
        m_node[r]->remove();
    }
    
    if (m_lightning && m_thunder_sound != NULL) sfx_manager->deleteSFX(m_thunder_sound);
}

// ----------------------------------------------------------------------------

void Rain::update(float dt)
{
    //const int count = m_materials.size();
    for (int m=0; m<RAIN_RING_COUNT; m++)
    {
        m_x[m] = m_x[m] + dt*RAIN_DX;
        m_y[m] = m_y[m] + dt*RAIN_DY;
        if (m_x[m] > 1.0f) m_x[m] = fmod(m_x[m], 1.0f);
        if (m_y[m] > 1.0f) m_y[m] = fmod(m_y[m], 1.0f);
        
        core::matrix4& matrix = m_node[m]->getChild()->getMaterial(0).getTextureMatrix(0);

        matrix.setTextureTranslate(m_x[m], m_y[m]);
    }
    
    if (m_lightning)
    {
        m_next_lightning -= dt;
        
        if (m_next_lightning < 0.0f)
        {
            RaceGUIBase* gui_base = World::getWorld()->getRaceGUI();
            if (gui_base != NULL)
            {
                gui_base->doLightning();
                if (m_thunder_sound) m_thunder_sound->play();
            }
            
            RandomGenerator g;
            m_next_lightning = 35 + (float)g.get(35);
        }
    }
    
}   // update

// ----------------------------------------------------------------------------

void Rain::setPosition(const core::vector3df& position)
{
    for (int m=0; m<RAIN_RING_COUNT; m++)
    {
        m_node[m]->getChild()->setPosition(position);
    }
}   // setPosition

// ----------------------------------------------------------------------------

void Rain::setCamera(scene::ICameraSceneNode* camera)
{
    for (int n=0; n<RAIN_RING_COUNT; n++) m_node[n]->setCamera(camera);
}
