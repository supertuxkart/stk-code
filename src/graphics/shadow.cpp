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
#include "karts/abstract_kart.hpp"
#include "karts/skidding.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material.hpp"
#include "graphics/sp/sp_shader_manager.hpp"
#include "graphics/sp/sp_dynamic_draw_call.hpp"
#include "physics/btKart.hpp"
#include "mini_glm.hpp"
#include "utils/vec3.hpp"

#ifndef SERVER_ONLY

#include <array>
#include <ge_vulkan_dynamic_spm_buffer.hpp>
#include <IMeshSceneNode.h>
#include <IVideoDriver.h>
#include <SMesh.h>
#include <SMeshBuffer.h>

Shadow::Shadow(Material* shadow_mat, const AbstractKart& kart)
      : m_node(NULL), m_shadow_enabled(false), m_kart(kart)
{
    if (CVS->isGLSL())
    {
        m_dy_dc = std::make_shared<SP::SPDynamicDrawCall>
            (scene::EPT_TRIANGLE_STRIP,
            SP::SPShaderManager::get()->getSPShader("alphablend"), shadow_mat);

        m_dy_dc->getVerticesVector().resize(4);
        video::S3DVertexSkinnedMesh* v = m_dy_dc->getVerticesVector().data();
        v[0].m_all_uvs[0] = 0;
        v[0].m_all_uvs[1] = 0;
        v[1].m_all_uvs[0] = 15360;
        v[1].m_all_uvs[1] = 0;
        v[3].m_all_uvs[0] = 15360;
        v[3].m_all_uvs[1] = 15360;
        v[2].m_all_uvs[0] = 0;
        v[2].m_all_uvs[1] = 15360;

        m_dy_dc->setVisible(false);
        SP::addDynamicDrawCall(m_dy_dc);
    }
    else
    {
        std::array<uint16_t, 6> indices = {{ 0, 1, 2, 0, 2, 3 }};
        scene::IMeshBuffer* buffer = NULL;
        if (irr_driver->getVideoDriver()->getDriverType() == video::EDT_VULKAN)
        {
            buffer = new GE::GEVulkanDynamicSPMBuffer();
            video::S3DVertexSkinnedMesh v;
            v.m_color = (video::SColor)-1;
            std::array<video::S3DVertexSkinnedMesh, 4> vertices =
                {{ v, v, v, v }};
            buffer->append(vertices.data(), vertices.size(), indices.data(),
                indices.size());
        }
        else
        {
            buffer = new scene::SMeshBuffer();
            video::S3DVertex v;
            v.Color = (video::SColor)-1;
            std::array<video::S3DVertex, 4> vertices = {{ v, v, v, v }};
            buffer->append(vertices.data(), vertices.size(), indices.data(),
                indices.size());
        }
        buffer->setTCoords(0, core::vector2df(0.0f, 0.0f));
        buffer->setTCoords(1, core::vector2df(1.0f, 0.0f));
        buffer->setTCoords(2, core::vector2df(1.0f, 1.0f));
        buffer->setTCoords(3, core::vector2df(0.0f, 1.0f));
        shadow_mat->setMaterialProperties(&buffer->getMaterial(), buffer);
        buffer->getMaterial().setTexture(0, shadow_mat->getTexture());
        buffer->setHardwareMappingHint(scene::EHM_STREAM);
        scene::SMesh* mesh = new scene::SMesh();
        mesh->addMeshBuffer(buffer);
        mesh->setBoundingBox(buffer->getBoundingBox());
        buffer->drop();
        std::string debug_name = m_kart.getIdent() + " (shadow)";
        m_node = static_cast<scene::IMeshSceneNode*>(
            irr_driver->addMesh(mesh, debug_name));
        mesh->drop();
    }
}   // Shadow

// ----------------------------------------------------------------------------
Shadow::~Shadow()
{
    if (m_dy_dc)
        m_dy_dc->removeFromSP();
    else if (m_node)
        irr_driver->removeNode(m_node);
}   // ~Shadow

// ----------------------------------------------------------------------------
/** Updates the simulated shadow. It takes the 4 suspension lengths of vehicle
 *  from ground to position the shadow quad exactly on the ground.
 *  It also disables the shadow if requested (e.g. if the kart is in the air).
 *  \param enabled If the shadow should be shown or not.
 */
void Shadow::update(bool enabled)
{
    if (enabled != m_shadow_enabled)
    {
        m_shadow_enabled = enabled;
        if (m_shadow_enabled)
        {
            if (m_dy_dc)
                m_dy_dc->setVisible(true);
            else if (m_node)
                m_node->setVisible(true);
        }
        else
        {
            if (m_dy_dc)
                m_dy_dc->setVisible(false);
            else if (m_node)
                m_node->setVisible(false);
        }
    }
    if (m_shadow_enabled && m_dy_dc)
    {
        video::S3DVertexSkinnedMesh* v = m_dy_dc->getVerticesVector().data();
        v[0].m_position.X = -1; v[0].m_position.Z =  1; v[0].m_position.Y = 0;
        v[1].m_position.X =  1; v[1].m_position.Z =  1; v[1].m_position.Y = 0;
        v[2].m_position.X = -1; v[2].m_position.Z = -1; v[2].m_position.Y = 0;
        v[3].m_position.X =  1; v[3].m_position.Z = -1; v[3].m_position.Y = 0;
        btTransform kart_trans = m_kart.getSmoothedTrans();
        btTransform skidding_rotation;
        skidding_rotation.setOrigin(Vec3(0, 0, 0));
        skidding_rotation.setRotation
            (btQuaternion(m_kart.getSkidding()->getVisualSkidRotation(), 0, 0));
        kart_trans *= skidding_rotation;
        for (unsigned i = 0; i < 4; i++)
        {
            const btWheelInfo& wi = m_kart.getVehicle()->getWheelInfo(i);
            Vec3 up_vector = kart_trans.getBasis().getColumn(1);
            up_vector = up_vector * (wi.m_raycastInfo.m_suspensionLength - 0.02f);
            Vec3 pos = kart_trans(Vec3(v[i].m_position)) - up_vector;
            v[i].m_position = pos.toIrrVector();
            v[i].m_normal = MiniGLM::compressVector3
                (Vec3(wi.m_raycastInfo.m_contactNormalWS).toIrrVector());
        }
        m_dy_dc->recalculateBoundingBox();
        m_dy_dc->setUpdateOffset(0);
    }
    else if (m_shadow_enabled && m_node)
    {
        Vec3 position[4];
        position[0] = Vec3(-1.0f, 0.0f,  1.0f);
        position[1] = Vec3( 1.0f, 0.0f,  1.0f);
        position[2] = Vec3( 1.0f, 0.0f, -1.0f);
        position[3] = Vec3(-1.0f, 0.0f, -1.0f);
        btTransform kart_trans = m_kart.getSmoothedTrans();
        btTransform skidding_rotation;
        skidding_rotation.setOrigin(Vec3(0, 0, 0));
        skidding_rotation.setRotation
            (btQuaternion(m_kart.getSkidding()->getVisualSkidRotation(), 0, 0));
        kart_trans *= skidding_rotation;
        scene::IMesh* mesh = m_node->getMesh();
        scene::IMeshBuffer* buffer = mesh->getMeshBuffer(0);
        for (unsigned i = 0; i < 4; i++)
        {
            const btWheelInfo& wi = m_kart.getVehicle()->getWheelInfo(i);
            Vec3 up_vector = kart_trans.getBasis().getColumn(1);
            up_vector = up_vector * (wi.m_raycastInfo.m_suspensionLength - 0.02f);
            Vec3 pos = kart_trans(position[i]) - up_vector;
            buffer->getPosition(i) = pos.toIrrVector();
            buffer->setNormal(i, Vec3(wi.m_raycastInfo.m_contactNormalWS)
                .toIrrVector());
        }
        buffer->recalculateBoundingBox();
        buffer->setDirtyOffset(0, irr::scene::EBT_VERTEX);
        mesh->setBoundingBox(buffer->getBoundingBox());
    }
}   // update

#endif
