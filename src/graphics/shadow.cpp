//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2013  Joerg Henrichs
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

#include "graphics/shadow.hpp"
#include "graphics/irr_driver.hpp"
#include "karts/kart_properties.hpp"

#include <IMesh.h>
#include <IMeshSceneNode.h>
#include <ISceneNode.h>

Shadow::Shadow(const KartProperties *kart_properties,
               scene::ISceneNode *node,
               float y_offset = 0.0)
{
    m_shadow_enabled = false;
    video::SMaterial m;
    m.setTexture(0, kart_properties->getShadowTexture());
    m.BackfaceCulling = false;
    m.MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
    m.setFlag(video::EMF_ZWRITE_ENABLE , false);
    m_mesh   = irr_driver->createQuadMesh(&m, /*create_one_quad*/true);
    scene::IMeshBuffer *buffer = m_mesh->getMeshBuffer(0);
    irr::video::S3DVertex* v=(video::S3DVertex*)buffer->getVertices();
    float scale = kart_properties->getShadowScale();
    float x_offset = kart_properties->getShadowXOffset();
    float z_offset = kart_properties->getShadowXOffset();
    v[0].Pos.X = -scale+x_offset; v[0].Pos.Z =  scale+z_offset; v[0].Pos.Y = 0.01f-y_offset;
    v[1].Pos.X =  scale+x_offset; v[1].Pos.Z =  scale+z_offset; v[1].Pos.Y = 0.01f-y_offset;
    v[2].Pos.X =  scale+x_offset; v[2].Pos.Z = -scale+z_offset; v[2].Pos.Y = 0.01f-y_offset;
    v[3].Pos.X = -scale+x_offset; v[3].Pos.Z = -scale+z_offset; v[3].Pos.Y = 0.01f-y_offset;
    v[0].TCoords = core::vector2df(0,0);
    v[1].TCoords = core::vector2df(1,0);
    v[2].TCoords = core::vector2df(1,1);
    v[3].TCoords = core::vector2df(0,1);
    core::vector3df normal(0, 0, 1.0f);
    v[0].Normal  = normal;
    v[1].Normal  = normal;
    v[2].Normal  = normal;
    v[3].Normal  = normal;
    buffer->recalculateBoundingBox();

    m_node   = irr_driver->addMesh(m_mesh, "shadow");
#ifdef DEBUG
    m_node->setName("shadow");
#endif

    m_mesh->drop();   // the node grabs the mesh, so we can drop this reference
    m_node->setAutomaticCulling(scene::EAC_OFF);
    m_parent_kart_node = node;
    m_parent_kart_node->addChild(m_node);
}   // Shadow

// ----------------------------------------------------------------------------
Shadow::~Shadow()
{
    // Note: the mesh was not loaded from disk, so it is not cached,
    // and does not need to be removed. It's clean up when removing the node
    m_parent_kart_node->removeChild(m_node);
}   // ~Shadow

// ----------------------------------------------------------------------------
/** Updates the simulated shadow. It takes the offset (distance from visual
 *  chassis to ground) to position the shadow quad exactly on the ground.
 *  It also disables the shadow if requested (e.g. if the kart is in the air).
 *  \param enabled If the shadow should be shown or not.
 *  \param offset Distance from visual chassis to ground = position of the
 *         shadow to make sure it is exactly on the ground.
 */
void Shadow::update(bool enabled, float offset)
{
    m_node->setVisible(enabled);
    core::vector3df pos = m_node->getPosition();
    pos.Y = offset;
    m_node->setPosition(pos);
}   // update