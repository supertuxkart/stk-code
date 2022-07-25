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

#include "graphics/sp/sp_mesh.hpp"
#include "graphics/sp/sp_mesh_buffer.hpp"
#include "graphics/material.hpp"
#include "mini_glm.hpp"

#include <ge_animation.hpp>
#include <algorithm>

namespace SP
{
// ----------------------------------------------------------------------------
SPMesh::SPMesh()
{
    m_fps = 0.025f;
    m_bind_frame = 0;
    m_total_joints = 0;
    m_joint_using = 0;
    m_frame_count = 0;
}   // SPMesh

// ----------------------------------------------------------------------------
SPMesh::~SPMesh()
{
    for (unsigned i=0; i < m_buffer.size(); i++)
    {
        if (m_buffer[i])
        {
            m_buffer[i]->drop();
        }
    }
}   // ~SPMesh

// ----------------------------------------------------------------------------
IMeshBuffer* SPMesh::getMeshBuffer(u32 nr) const
{
    if (nr < m_buffer.size())
    {
        return m_buffer[nr];
    }
    return NULL;
}   // getMeshBuffer

// ----------------------------------------------------------------------------
SPMeshBuffer* SPMesh::getSPMeshBuffer(u32 nr) const
{
    if (nr < m_buffer.size())
    {
        return m_buffer[nr];
    }
    return NULL;
}   // getMeshBuffer

// ----------------------------------------------------------------------------
IMeshBuffer* SPMesh::getMeshBuffer(const video::SMaterial &material) const
{
    for (unsigned i = 0; i < m_buffer.size(); i++)
    {
        if (m_buffer[i]->getMaterial() == material)
        {
            return m_buffer[i];
        }
    }
    return NULL;
}   // getMeshBuffer

// ----------------------------------------------------------------------------
const c8* SPMesh::getJointName(u32 number) const
{
    if (number >= m_joint_using)
    {
        return "";
    }
    int i = 0;
    int j = number;
    while (j >= int(m_all_armatures[i].m_joint_used))
    {
        j -= int(m_all_armatures[i].m_joint_used);
        i++;
    }
    return m_all_armatures.at(i).m_joint_names[j].c_str();

}   // getJointName

// ----------------------------------------------------------------------------
s32 SPMesh::getJointIDWithArm(const c8* name, unsigned* arm_id) const
{
    for (unsigned i = 0; i < m_all_armatures.size(); i++)
    {
        const GE::Armature& arm = m_all_armatures[i];
        auto found = std::find(arm.m_joint_names.begin(),
            arm.m_joint_names.end(), name);
        if (found != arm.m_joint_names.end())
        {
            if (arm_id != NULL)
            {
                *arm_id = i;
            }
            return (int)(found - arm.m_joint_names.begin());
        }
    }
    return -1;
}   // getJointIDWithArm

// ----------------------------------------------------------------------------
void SPMesh::getSkinningMatrices(f32 frame, std::vector<core::matrix4>& dest,
                                 float frame_interpolating, float rate)
{
    unsigned accumulated_joints = 0;
    for (unsigned i = 0; i < m_all_armatures.size(); i++)
    {
        m_all_armatures[i].getPose(frame, &dest[accumulated_joints],
            frame_interpolating, rate);
        accumulated_joints += m_all_armatures[i].m_joint_used;
    }

}   // getSkinningMatrices

// ----------------------------------------------------------------------------
void SPMesh::updateBoundingBox()
{
    m_bounding_box.reset(0.0f, 0.0f, 0.0f);
    for (unsigned i = 0; i < m_buffer.size(); i++)
    {
        m_buffer[i]->recalculateBoundingBox();
        m_bounding_box.addInternalBox(m_buffer[i]->getBoundingBox());
    }
}   // updateBoundingBox

// ----------------------------------------------------------------------------
void SPMesh::finalize()
{
    updateBoundingBox();
    for (GE::Armature& arm : getArmatures())
    {
        arm.getInterpolatedMatrices((float)m_bind_frame);
        for (auto& p : arm.m_world_matrices)
        {
            p.second = false;
        }
        for (unsigned i = 0; i < arm.m_joint_names.size(); i++)
        {
            core::matrix4 m;
            arm.getWorldMatrix(arm.m_interpolated_matrices, i).getInverse(m);
            arm.m_joint_matrices[i] = m;
        }
    }
    m_bounding_box.reset(0.0f, 0.0f, 0.0f);
    // Sort with same shader name
    std::sort(m_buffer.begin(), m_buffer.end(),
        [](const SPMeshBuffer* a, const SPMeshBuffer* b)->bool
        {
            return a->getSTKMaterial()->getShaderName() <
                b->getSTKMaterial()->getShaderName();
        });

    for (unsigned i = 0; i < m_buffer.size(); i++)
    {
        m_buffer[i]->recalculateBoundingBox();
        m_bounding_box.addInternalBox(m_buffer[i]->getBoundingBox());
        m_buffer[i]->initDrawMaterial();
        if (!isStatic())
        {
            m_buffer[i]->enableSkinningData();
        }
    }

    auto itr = m_buffer.begin();
    while (itr != m_buffer.end())
    {
        auto itr_next = itr + 1;
        if (itr_next != m_buffer.end() &&
            (*itr)->getSTKMaterial()->getShaderName() ==
            (*itr_next)->getSTKMaterial()->getShaderName())
        {
            if ((*itr)->combineMeshBuffer(*itr_next))
            {
                (*itr)->recalculateBoundingBox();
                delete *itr_next;
                m_buffer.erase(itr_next);
                continue;
            }
        }
        itr++;
    }

    for (unsigned i = 0; i < m_buffer.size(); i++)
    {
        m_buffer[i]->shrinkToFit();
    }

}   // finalize

}
