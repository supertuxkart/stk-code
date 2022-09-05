//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2018 SuperTuxKart-Team
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

#include "graphics/sp/sp_mesh_node.hpp"
#include "graphics/sp/sp_base.hpp"
#include "graphics/sp/sp_mesh.hpp"
#include "graphics/sp/sp_mesh_buffer.hpp"
#include "graphics/sp/sp_shader.hpp"
#include "graphics/sp/sp_shader_manager.hpp"
#include "graphics/graphics_restrictions.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material.hpp"
#include <ge_render_info.hpp>

#include "../../../lib/irrlicht/source/Irrlicht/CBoneSceneNode.h"
#include <algorithm>
#include <ge_animation.hpp>

namespace SP
{
// ----------------------------------------------------------------------------
SPMeshNode::SPMeshNode(IAnimatedMesh* mesh, ISceneNode* parent,
                       ISceneManager* mgr, s32 id,
                       const std::string& debug_name,
                       const core::vector3df& position,
                       const core::vector3df& rotation,
                       const core::vector3df& scale,
                       std::shared_ptr<GE::GERenderInfo> render_info)
          : CAnimatedMeshSceneNode(mesh, parent, mgr, id, position, rotation,
                                   scale)
{
    m_glow_color = video::SColorf(0.0f, 0.0f, 0.0f);
    m_mesh = NULL;
    m_first_render_info = render_info;
    m_animated = false;
    m_skinning_offset = -32768;
    m_saved_transition_frame = -1.0f;
    m_is_in_shadowpass = true;
}   // SPMeshNode

// ----------------------------------------------------------------------------
SPMeshNode::~SPMeshNode()
{
    cleanJoints();
    cleanRenderInfo();
}   // ~SPMeshNode

// ----------------------------------------------------------------------------
void SPMeshNode::cleanRenderInfo()
{
    m_render_info.clear();
}   // cleanRenderInfo

// ----------------------------------------------------------------------------
void SPMeshNode::setAnimationState(bool val)
{
    if (!m_mesh)
    {
        return;
    }
#ifndef SERVER_ONLY
    m_animated = !m_mesh->isStatic() && val &&
        !GraphicsRestrictions::isDisabled
        (GraphicsRestrictions::GR_HARDWARE_SKINNING);
#endif
}   // setAnimationState

// ----------------------------------------------------------------------------
void SPMeshNode::setMesh(irr::scene::IAnimatedMesh* mesh)
{
    m_glow_color = video::SColorf(0.0f, 0.0f, 0.0f);
    m_skinning_offset = -32768;
    m_saved_transition_frame = -1.0f;
    m_animated = false;
    m_mesh = static_cast<SPMesh*>(mesh);
    CAnimatedMeshSceneNode::setMesh(mesh);
    cleanJoints();
    cleanRenderInfo();
    if (m_mesh)
    {
        m_texture_matrices.resize(m_mesh->getMeshBufferCount(),
            {{ 0.0f, 0.0f }});
        if (!m_mesh->isStatic())
        {
#ifndef SERVER_ONLY
            m_animated = !m_mesh->isStatic() &&
                !GraphicsRestrictions::isDisabled
                (GraphicsRestrictions::GR_HARDWARE_SKINNING);
#endif
            unsigned bone_idx = 0;
            m_skinning_matrices.resize(m_mesh->getJointCount());
            for (GE::Armature& arm : m_mesh->getArmatures())
            {
                for (const std::string& bone_name : arm.m_joint_names)
                {
                    m_joint_nodes[bone_name] = new CBoneSceneNode(this,
                        irr_driver->getSceneManager(), 0, bone_idx++,
                        bone_name.c_str());
                    m_joint_nodes.at(bone_name)->drop();
                    m_joint_nodes.at(bone_name)->setSkinningSpace(EBSS_GLOBAL);
                }
            }
        }
        if (m_first_render_info)
        {
            m_render_info.resize(m_mesh->getMeshBufferCount(),
                m_first_render_info);
            for (unsigned i = 0; i < m_mesh->getMeshBufferCount(); i++)
            {
                SP::SPMeshBuffer* spmb = m_mesh->getSPMeshBuffer(i);
                const std::vector<Material*>& m = spmb->getAllSTKMaterials();
                bool keep_render_info_for_mb = false;
                for (unsigned j = 0; j < m.size(); j++)
                {
                    Material* mat = m[j];
                    keep_render_info_for_mb =
                        keep_render_info_for_mb || mat->isColorizable();
                }
                if (!keep_render_info_for_mb)
                {
                    m_render_info[i].reset();
                }
            }
        }
    }
}   // setMesh

// ----------------------------------------------------------------------------
IBoneSceneNode* SPMeshNode::getJointNode(const c8* joint_name)
{
    auto ret = m_joint_nodes.find(joint_name);
    if (ret != m_joint_nodes.end())
    {
        return ret->second;
    }
    return NULL;
}   // getJointNode

// ----------------------------------------------------------------------------
void SPMeshNode::OnAnimate(u32 time_ms)
{
    m_skinning_offset = -32768;
    if (m_mesh->isStatic() || !m_animated)
    {
        IAnimatedMeshSceneNode::OnAnimate(time_ms);
        return;
    }
    CAnimatedMeshSceneNode::OnAnimate(time_ms);
}   // OnAnimate

// ----------------------------------------------------------------------------
IMesh* SPMeshNode::getMeshForCurrentFrame()
{
    if (m_mesh->isStatic() || !m_animated)
    {
        return m_mesh;
    }
    m_mesh->getSkinningMatrices(getFrameNr(), m_skinning_matrices,
        m_saved_transition_frame, TransitingBlend);
    recursiveUpdateAbsolutePosition();

    for (GE::Armature& arm : m_mesh->getArmatures())
    {
        for (unsigned i = 0; i < arm.m_joint_names.size(); i++)
        {
            m_joint_nodes.at(arm.m_joint_names[i])->setAbsoluteTransformation
                (AbsoluteTransformation * arm.m_world_matrices[i].first);
        }
    }
    return m_mesh;
}   // getMeshForCurrentFrame

// ----------------------------------------------------------------------------
int SPMeshNode::getTotalJoints() const
{
    return m_mesh ? m_mesh->getJointCount() : 0;
}   // getTotalJoints

// ----------------------------------------------------------------------------
SPShader* SPMeshNode::getShader(unsigned mesh_buffer_id) const
{
#ifndef SERVER_ONLY
    if (!m_mesh || mesh_buffer_id < m_mesh->getMeshBufferCount())
    {
        SPShader* shader =
            m_mesh->getSPMeshBuffer(mesh_buffer_id)->getShader(m_animated);
        if (shader && shader->isTransparent())
        {
            // Use real transparent shader first
            return shader;
        }
        if (m_first_render_info && m_first_render_info->isTransparent())
        {
            return SPShaderManager::get()->getSPShader
                (std::string("ghost") + (m_animated ? "_skinned" : "")).get();
        }
        return shader;
    }
#endif
    return NULL;
}   // getShader

// ----------------------------------------------------------------------------
void SPMeshNode::setTransitionTime(f32 Time)
{
    if (Time == 0.0f)
    {
        TransitingBlend = TransitionTime = Transiting = 0;
        m_saved_transition_frame = -1.0;
    }
    else
    {
        const u32 ttime = (u32)core::floor32(Time * 1000.0f);
        TransitionTime = ttime;
        Transiting = core::reciprocal((f32)TransitionTime);
        TransitingBlend = 0.0f;
        m_saved_transition_frame = getFrameNr();
    }
}   // setTransitionTime

}
