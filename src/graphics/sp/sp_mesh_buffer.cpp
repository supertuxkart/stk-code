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

#include "graphics/sp/sp_mesh_buffer.hpp"
#include "graphics/sp/sp_texture.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/graphics_restrictions.hpp"
#include "graphics/material.hpp"
#include "graphics/sp/sp_shader.hpp"
#include "graphics/sp/sp_shader_manager.hpp"
#include "graphics/sp/sp_texture_manager.hpp"
#include "race/race_manager.hpp"
#include "mini_glm.hpp"
#include "utils/string_utils.hpp"

#include <set>
#ifndef SERVER_ONLY
#include <ge_main.hpp>
#endif

namespace SP
{
// ----------------------------------------------------------------------------
SPMeshBuffer::~SPMeshBuffer()
{
#ifndef SERVER_ONLY
    for (unsigned i = 0; i < DCT_FOR_VAO; i++)
    {
        if (m_vao[i] != 0)
        {
            glDeleteVertexArrays(1, &m_vao[i]);
        }
        if (m_ins_array[i] != 0)
        {
            if (CVS->isARBBufferStorageUsable())
            {
                glBindBuffer(GL_ARRAY_BUFFER, m_ins_array[i]);
                glUnmapBuffer(GL_ARRAY_BUFFER);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }
            glDeleteBuffers(1, &m_ins_array[i]);
        }
    }
    if (m_ibo != 0)
    {
        glDeleteBuffers(1, &m_ibo);
    }
    if (m_vbo != 0)
    {
        glDeleteBuffers(1, &m_vbo);
    }
#endif
}   // ~SPMeshBuffer

// ----------------------------------------------------------------------------
void SPMeshBuffer::initDrawMaterial()
{
#ifndef SERVER_ONLY
    Material* m = std::get<2>(m_stk_material[0]);
    if (RaceManager::get()->getReverseTrack() && m->getMirrorAxisInReverse() != ' ')
    {
        for (unsigned i = 0; i < getVertexCount(); i++)
        {
            using namespace MiniGLM;
            if (m->getMirrorAxisInReverse() == 'V')
            {
                m_vertices[i].m_all_uvs[1] =
                    toFloat16(1.0f - toFloat32(m_vertices[i].m_all_uvs[1]));
            }
            else
            {
                m_vertices[i].m_all_uvs[0] =
                    toFloat16(1.0f - toFloat32(m_vertices[i].m_all_uvs[0]));
            }
        }
    }   // reverse track and texture needs mirroring
#endif
}   // initDrawMaterial

// ----------------------------------------------------------------------------
bool SPMeshBuffer::initTexture()
{
#ifndef SERVER_ONLY
    for (unsigned i = 0; i < m_stk_material.size(); i++)
    {
        for (unsigned j = 0; j < 6; j++)
        {
            if (!m_textures[i][j]->initialized())
            {
                return false;
            }
        }
    }
#endif
    return true;
}   // initTexture

// ----------------------------------------------------------------------------
void SPMeshBuffer::uploadGLMesh()
{
    if (m_uploaded_gl)
    {
        return;
    }
    m_uploaded_gl = true;
#ifndef SERVER_ONLY
    if (!m_shaders[0])
    {
        Log::warn("SPMeshBuffer", "%s shader is missing",
            std::get<2>(m_stk_material[0])->getShaderName().c_str());
        return;
    }

    m_textures.resize(m_stk_material.size());
    for (unsigned i = 0; i < m_stk_material.size(); i++)
    {
        for (unsigned j = 0; j < 6; j++)
        {
            m_textures[i][j] = SPTextureManager::get()->getTexture
                (m_shaders[0]->hasTextureLayer(j) ?
                std::get<2>(m_stk_material[i])->getSamplerPath(j) : "",
                j == 0 ? std::get<2>(m_stk_material[i]) : NULL,
                m_shaders[0]->isSrgbForTextureLayer(j),
                std::get<2>(m_stk_material[i])->getContainerId());
        }
        // Use the original spm uv texture 1 and 2 for compare in scene manager
        m_tex_cmp[std::get<2>(m_stk_material[i])->getSamplerPath(0) +
            std::get<2>(m_stk_material[i])->getSamplerPath(1)] = i;
    }

    bool use_2_uv = std::get<2>(m_stk_material[0])->use2UV();
    bool use_tangents = m_shaders[0]->useTangents();
    const unsigned pitch = 48 - (use_tangents ? 0 : 4) - (use_2_uv ? 0 : 4) -
        (m_skinned ? 0 : 16);
    m_pitch = pitch;

    if (m_vbo != 0)
    {
        glDeleteBuffers(1, &m_vbo);
    }
    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    unsigned v_size = (unsigned)m_vertices.size() * pitch;
    glBufferData(GL_ARRAY_BUFFER, v_size, NULL, GL_DYNAMIC_DRAW);
    size_t offset = 0;
    char* ptr = (char*)glMapBufferRange(GL_ARRAY_BUFFER, 0, v_size,
        GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT |
        GL_MAP_INVALIDATE_BUFFER_BIT);
    v_size = 0;
    for (unsigned i = 0 ; i < m_vertices.size(); i++)
    {
        offset = 0;
        memcpy(ptr + v_size + offset, &m_vertices[i].m_position.X, 12);
        offset += 12;

        memcpy(ptr + v_size + offset, &m_vertices[i].m_normal, 12);
        offset += 4;

        video::SColor vc = m_vertices[i].m_color;
        if (CVS->isDeferredEnabled())
        {
            vc.setRed(srgb255ToLinear(vc.getRed()));
            vc.setGreen(srgb255ToLinear(vc.getGreen()));
            vc.setBlue(srgb255ToLinear(vc.getBlue()));
        }
        memcpy(ptr + v_size + offset, &vc, 4);
        offset += 4;

        memcpy(ptr + v_size + offset, &m_vertices[i].m_all_uvs[0], 4);
        offset += 4;
        if (use_2_uv)
        {
            memcpy(ptr + v_size + offset, &m_vertices[i].m_all_uvs[2], 4);
            offset += 4;
        }
        if (use_tangents)
        {
            memcpy(ptr + v_size + offset, &m_vertices[i].m_tangent, 4);
            offset += 4;
        }
        if (m_skinned)
        {
            memcpy(ptr + v_size + offset, &m_vertices[i].m_joint_idx[0], 16);
        }
        v_size += pitch;
    }
    glUnmapBuffer(GL_ARRAY_BUFFER);

    SPTextureManager::get()->increaseGLCommandFunctionCount(1);
    SPTextureManager::get()->addGLCommandFunction
        (std::bind(&SPMeshBuffer::initTexture, this));

    if (m_ibo != 0)
    {
        glDeleteBuffers(1, &m_ibo);
    }
    glGenBuffers(1, &m_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * 2,
        m_indices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

#endif
}   // uploadGLMesh

// ----------------------------------------------------------------------------
void SPMeshBuffer::recreateVAO(unsigned i)
{
#ifndef SERVER_ONLY
    if (!m_shaders[0])
    {
        return;
    }
    bool use_2_uv = std::get<2>(m_stk_material[0])->use2UV();
    bool use_tangents = m_shaders[0]->useTangents();
    const unsigned pitch = m_pitch;

    size_t offset = 0;

    if (m_ins_array[i] == 0)
    {
        glGenBuffers(1, &m_ins_array[i]);
    }
    else
    {
        if (CVS->isARBBufferStorageUsable())
        {
            glBindBuffer(GL_ARRAY_BUFFER, m_ins_array[i]);
            glUnmapBuffer(GL_ARRAY_BUFFER);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        glDeleteBuffers(1, &m_ins_array[i]);
        glGenBuffers(1, &m_ins_array[i]);
    }
    glBindBuffer(GL_ARRAY_BUFFER, m_ins_array[i]);
#ifndef USE_GLES2
    if (CVS->isARBBufferStorageUsable())
    {
        glBufferStorage(GL_ARRAY_BUFFER, m_gl_instance_size[i] * 44, NULL,
            GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
        m_ins_dat_mapped_ptr[i] = glMapBufferRange(GL_ARRAY_BUFFER, 0,
            m_gl_instance_size[i] * 44,
            GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    }
    else
#endif
    {
        glBufferData(GL_ARRAY_BUFFER, m_gl_instance_size[i] * 44, NULL,
            GL_DYNAMIC_DRAW);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (m_vao[i] != 0)
    {
        glDeleteVertexArrays(1, &m_vao[i]);
    }
    glGenVertexArrays(1, &m_vao[i]);
    glBindVertexArray(m_vao[i]);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, pitch, (void*)offset);
    offset += 12;

    // Normal, if 10bit vector normalization is wrongly done by drivers, use
    // original value and normalize ourselves in shader
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_INT_2_10_10_10_REV,
        GraphicsRestrictions::isDisabled
        (GraphicsRestrictions::GR_CORRECT_10BIT_NORMALIZATION) ?
        GL_FALSE : GL_TRUE, pitch, (void*)offset);
    offset += 4;

    // Vertex color
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, pitch,
        (void*)offset);
    offset += 4;

    // 1st texture coordinates
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_HALF_FLOAT, GL_FALSE, pitch, (void*)offset);
    offset += 4;
    if (use_2_uv)
    {
        // 2nd texture coordinates
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 2, GL_HALF_FLOAT, GL_FALSE, pitch,
            (void*)offset);
        offset += 4;
    }
    else
    {
        glDisableVertexAttribArray(4);
    }

    if (use_tangents)
    {
        // Tangent and bi-tanget sign
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_INT_2_10_10_10_REV,
            GraphicsRestrictions::isDisabled
            (GraphicsRestrictions::GR_CORRECT_10BIT_NORMALIZATION) ?
            GL_FALSE : GL_TRUE, pitch, (void*)offset);
            offset += 4;
    }
    else
    {
        glDisableVertexAttribArray(5);
    }

    if (m_skinned)
    {
        // 4 Joint indices
        glEnableVertexAttribArray(6);
        glVertexAttribIPointer(6, 4, GL_SHORT, pitch, (void*)offset);
        offset += 8;
        // 4 Joint weights
        glEnableVertexAttribArray(7);
        glVertexAttribPointer(7, 4, GL_HALF_FLOAT, GL_FALSE, pitch,
            (void*)offset);
        offset += 8;
    }
    else
    {
        glDisableVertexAttribArray(6);
        glDisableVertexAttribArray(7);
    }
    glDisableVertexAttribArray(13);
    glDisableVertexAttribArray(14);
    glDisableVertexAttribArray(15);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBindBuffer(GL_ARRAY_BUFFER, m_ins_array[i]);
    // Origin
    glEnableVertexAttribArray(8);
    glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, 44, (void*)0);
    glVertexAttribDivisor(8, 1);
    // Rotation (quaternion in 4 32bit floats)
    glEnableVertexAttribArray(9);
    glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, 44, (void*)12);
    glVertexAttribDivisor(9, 1);
    // Scale (3 half floats and .w unused)
    glEnableVertexAttribArray(10);
    glVertexAttribPointer(10, 4, GL_HALF_FLOAT, GL_FALSE, 44, (void*)28);
    glVertexAttribDivisor(10, 1);
    // Texture translation
    glEnableVertexAttribArray(11);
    glVertexAttribPointer(11, 2, GL_SHORT, GL_TRUE, 44, (void*)36);
    glVertexAttribDivisor(11, 1);
    // Misc data (skinning offset and hue change)
    glEnableVertexAttribArray(12);
    glVertexAttribIPointer(12, 2, GL_SHORT, 44, (void*)40);
    glVertexAttribDivisor(12, 1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif
}   // uploadGLMesh

// ----------------------------------------------------------------------------
void SPMeshBuffer::uploadInstanceData()
{
#ifndef SERVER_ONLY
    for (unsigned i = 0; i < DCT_FOR_VAO; i++)
    {
        if (m_ins_dat[i].empty())
        {
            continue;
        }

        unsigned new_size =
            m_gl_instance_size[i] == 0 ? 1 : m_gl_instance_size[i];
        while (m_ins_dat[i].size() > new_size)
        {
            // Power of 2 allocation strategy, like std::vector in gcc
            new_size <<= 1;
        }
        if (new_size != m_gl_instance_size[i])
        {
            m_gl_instance_size[i] = new_size;
            recreateVAO(i);
        }
        if (CVS->isARBBufferStorageUsable())
        {
            memcpy(m_ins_dat_mapped_ptr[i], m_ins_dat[i].data(),
                m_ins_dat[i].size() * 44);
        }
        else
        {
            glBindBuffer(GL_ARRAY_BUFFER, m_ins_array[i]);
            void* ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0,
                m_ins_dat[i].size() * 44, GL_MAP_WRITE_BIT |
                GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
            memcpy(ptr, m_ins_dat[i].data(), m_ins_dat[i].size() * 44);
            glUnmapBuffer(GL_ARRAY_BUFFER);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
    }
#endif
    m_uploaded_instance = true;
}   // uploadInstanceData

// ----------------------------------------------------------------------------
void SPMeshBuffer::enableTextureMatrix(unsigned mat_id)
{
#ifndef SERVER_ONLY
    assert(mat_id < m_stk_material.size());
    // Make the 31 bit in normal to be 1
    uploadGLMesh();
    if (m_vbo == 0 || m_ibo == 0)
        return;
    auto& ret = m_stk_material[mat_id];
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    std::set<uint16_t> used_vertices;
    for (unsigned int j = 0; j < std::get<1>(ret); j += 3)
    {
        for (unsigned int k = 0; k < 3; k++)
        {
            const uint16_t vertex_id = m_indices[std::get<0>(ret) + j + k];
            auto ret = used_vertices.find(vertex_id);
            if (ret == used_vertices.end())
            {
                if ((m_vertices[vertex_id].m_normal & (1 << 30)) != 0)
                {
                    // Already enabled
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    return;
                }
                used_vertices.insert(vertex_id);
                m_vertices[vertex_id].m_normal |= 1 << 30;
                glBufferSubData(GL_ARRAY_BUFFER,
                    (vertex_id * m_pitch) + 12 /*3 position*/, 4,
                    &m_vertices[vertex_id].m_normal);
            }
        }
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif
}   // enableTextureMatrix

// ----------------------------------------------------------------------------
void SPMeshBuffer::reloadTextureCompare()
{
    assert(!m_textures.empty());
    m_tex_cmp.clear();
    for (unsigned i = 0; i < m_stk_material.size(); i++)
    {
        const std::string name =
            m_textures[i][0]->getPath() + m_textures[i][1]->getPath();
        m_tex_cmp[name] = i;
    }
}   // reloadTextureCompare

// ----------------------------------------------------------------------------
void SPMeshBuffer::setSTKMaterial(Material* m)
{
    m_stk_material[0] = std::make_tuple(0u, getIndexCount(), m);
#ifndef SERVER_ONLY
    // Used by b3d mesh loader, clean up later after SP is removed
    if (GE::getVKDriver() != NULL)
        return;
#endif
    const std::string shader_name =
        std::get<2>(m_stk_material[0])->getShaderName();
    const std::string skinned_shader_name =
        std::get<2>(m_stk_material[0])->getShaderName() + "_skinned";

    m_shaders[0] = SPShaderManager::get()->getSPShader(shader_name);
    if (!m_shaders[0])
    {
        Log::warn("SPMeshBuffer", "%s shader is missing, fallback to solid",
            shader_name.c_str());
        m_shaders[0] = SPShaderManager::get()->getSPShader("solid");
    }

    m_shaders[1] = SPShaderManager::get()->getSPShader(skinned_shader_name);
    if (!m_shaders[1])
        m_shaders[1] = SPShaderManager::get()->getSPShader("solid_skinned");
}   // setSTKMaterial

}
