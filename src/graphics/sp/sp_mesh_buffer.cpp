//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2017 SuperTuxKart-Team
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
#include "graphics/sp/sp_texture_manager.hpp"
#include "race/race_manager.hpp"
#include "utils/mini_glm.hpp"
#include "utils/string_utils.hpp"

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
    if (race_manager->getReverseTrack() && m->getMirrorAxisInReverse() != ' ')
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
    if (!(CVS->useArrayTextures() || CVS->isARBBindlessTextureUsable()))
    {
        return true;
    }
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    std::set<uint16_t> used_vertices;
    for (unsigned int j = 0; j < getIndexCount(); j += 3)
    {
        for (unsigned int k = 0; k < 3; k++)
        {
            const uint16_t vertex_id = m_indices[j + k];
            auto ret = used_vertices.find(vertex_id);
            if (ret == used_vertices.end())
            {
                used_vertices.insert(vertex_id);
                std::array<std::shared_ptr<SPTexture>, 6> textures =
                    getSPTextures(j);
                if (CVS->useArrayTextures())
                {
                    for (unsigned i = 0; i < 6; i++)
                    {
                        uint16_t array_index = (uint16_t)
                            textures[i]->getTextureArrayIndex();
                        glBufferSubData(GL_ARRAY_BUFFER,
                            (vertex_id * m_pitch) + (m_pitch - 12) + (i * 2),
                            2, &array_index);
                    }
                }
                else
                {
                    for (unsigned i = 0; i < 6; i++)
                    {
                        glBufferSubData(GL_ARRAY_BUFFER,
                            (vertex_id * m_pitch) + (m_pitch - 48) + (i * 8),
                            8, textures[i]->getTextureHandlePointer());
                    }
                }
            }
        }
    }
    assert(used_vertices.size() == m_vertices.size());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
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
    m_textures.resize(m_stk_material.size());
    for (unsigned i = 0; i < m_stk_material.size(); i++)
    {
        for (unsigned j = 0; j < 6; j++)
        {
            // Undo the effect of srgb on 0 and 1 channel of textures
            // which is the uv textures from .spm when advanced lighting
            m_textures[i][j] = SPTextureManager::get()->getTexture
                (std::get<2>(m_stk_material[i])->getSamplerPath(j),
                j == 0 ? std::get<2>(m_stk_material[i]) : NULL,
                j < 2 && CVS->isDefferedEnabled());
        }
        // Use .spm uv texture 1 and 2 for compare in scene manager
        m_tex_cmp[m_textures[i][0]->getPath() + m_textures[i][1]->getPath()] =
            i;
    }

    bool use_2_uv = std::get<2>(m_stk_material[0])->use2UV();
    bool use_tangents =
        std::get<2>(m_stk_material[0])->getShaderName() == "normalmap" &&
        CVS->isDefferedEnabled();
    const unsigned pitch = 48 - (use_tangents ? 0 : 4) - (use_2_uv ? 0 : 4) -
        (m_skinned ? 0 : 16) + (CVS->useArrayTextures() ? 12 :
        CVS->isARBBindlessTextureUsable() ? 48 : 0);
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
        if (CVS->isDefferedEnabled())
        {
            video::SColorf tmp(vc);
            vc.setRed(srgbToLinear(tmp.r));
            vc.setGreen(srgbToLinear(tmp.g));
            vc.setBlue(srgbToLinear(tmp.b));
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

    for (unsigned i = 0; i < DCT_FOR_VAO; i++)
    {
        recreateVAO(i);
    }
#endif
}   // uploadGLMesh

// ----------------------------------------------------------------------------
void SPMeshBuffer::recreateVAO(unsigned i)
{
#ifndef SERVER_ONLY
    bool use_2_uv = std::get<2>(m_stk_material[0])->use2UV();
    bool use_tangents =
        std::get<2>(m_stk_material[0])->getShaderName() == "normalmap" &&
        CVS->isDefferedEnabled();
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
        glBufferStorage(GL_ARRAY_BUFFER, m_gl_instance_size[i] * 32, NULL,
            GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
        m_ins_dat_mapped_ptr[i] = glMapBufferRange(GL_ARRAY_BUFFER, 0,
            m_gl_instance_size[i] * 32,
            GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    }
    else
#endif
    {
        glBufferData(GL_ARRAY_BUFFER, m_gl_instance_size[i] * 32, NULL,
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
        (GraphicsRestrictions::GR_10BIT_VECTOR) ? GL_FALSE : GL_TRUE, pitch,
        (void*)offset);
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
            (GraphicsRestrictions::GR_10BIT_VECTOR) ? GL_FALSE : GL_TRUE,
            pitch, (void*)offset);
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

    if (CVS->useArrayTextures())
    {
        // uvec4 + uvec2 for 6 texture array indices
        glEnableVertexAttribArray(13);
        glVertexAttribIPointer(13, 4, GL_UNSIGNED_SHORT, pitch, (void*)offset);
        offset += 8;
        glEnableVertexAttribArray(14);
        glVertexAttribIPointer(14, 2, GL_UNSIGNED_SHORT, pitch, (void*)offset);
        offset += 4;
        glDisableVertexAttribArray(15);
    }
    else if (CVS->isARBBindlessTextureUsable())
    {
        // 3 * 2 uvec2 for bindless samplers
        glEnableVertexAttribArray(13);
        glVertexAttribIPointer(13, 4, GL_UNSIGNED_INT, pitch, (void*)offset);
        offset += 16;
        glEnableVertexAttribArray(14);
        glVertexAttribIPointer(14, 4, GL_UNSIGNED_INT, pitch, (void*)offset);
        offset += 16;
        glEnableVertexAttribArray(15);
        glVertexAttribIPointer(15, 4, GL_UNSIGNED_INT, pitch, (void*)offset);
    }
    else
    {
        glDisableVertexAttribArray(13);
        glDisableVertexAttribArray(14);
        glDisableVertexAttribArray(15);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBindBuffer(GL_ARRAY_BUFFER, m_ins_array[i]);
    // Origin
    glEnableVertexAttribArray(8);
    glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, 32, (void*)0);
    glVertexAttribDivisorARB(8, 1);
    // Rotation (quaternion .xyz)
    glEnableVertexAttribArray(9);
    glVertexAttribPointer(9, 4, GL_INT_2_10_10_10_REV,
        GraphicsRestrictions::isDisabled
        (GraphicsRestrictions::GR_10BIT_VECTOR) ? GL_FALSE : GL_TRUE, 32,
        (void*)12);
    glVertexAttribDivisorARB(9, 1);
    // Scale (3 half floats and .w for quaternion .w)
    glEnableVertexAttribArray(10);
    glVertexAttribPointer(10, 4, GL_HALF_FLOAT, GL_FALSE, 32, (void*)16);
    glVertexAttribDivisorARB(10, 1);
    // Texture translation
    glEnableVertexAttribArray(11);
    glVertexAttribPointer(11, 2, GL_HALF_FLOAT, GL_FALSE, 32, (void*)24);
    glVertexAttribDivisorARB(11, 1);
    // Misc data (skinning offset and hue change)
    glEnableVertexAttribArray(12);
    glVertexAttribIPointer(12, 2, GL_SHORT, 32, (void*)28);
    glVertexAttribDivisorARB(12, 1);

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
        unsigned new_size = m_gl_instance_size[i];
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
                m_ins_dat[i].size() * 32);
        }
        else
        {
            glBindBuffer(GL_ARRAY_BUFFER, m_ins_array[i]);
            void* ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0,
                m_ins_dat[i].size() * 32, GL_MAP_WRITE_BIT |
                GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
            memcpy(ptr, m_ins_dat[i].data(), m_ins_dat[i].size() * 32);
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
    assert(mat_id < m_stk_material.size());
    // Make the 31 bit in normal to be 1
    uploadGLMesh();
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
}   // enableTextureMatrix

}
