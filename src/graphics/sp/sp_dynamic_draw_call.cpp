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

#include "graphics/sp/sp_dynamic_draw_call.hpp"
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
SPDynamicDrawCall::SPDynamicDrawCall(scene::E_PRIMITIVE_TYPE pt,
                                     SPShader* shader, Material* m)
                 : SPMeshBuffer()
{
#ifndef SERVER_ONLY
    m_primitive_type = pt;
    m_shader = shader;
    m_stk_material[0] = std::make_tuple(0u, 0u, m);
    m_textures.resize(m_stk_material.size());
    for (unsigned j = 0; j < 6; j++)
    {
        m_textures[0][j] = SPTextureManager::get()->getTexture
            (std::get<2>(m_stk_material[0])->getSamplerPath(j),
            j == 0 ? std::get<2>(m_stk_material[0]) : NULL,
            j < 2 && CVS->isDefferedEnabled());
    }
    m_tex_cmp[m_textures[0][0]->getPath() + m_textures[0][1]->getPath()] = 0;
    m_pitch = 48;

    // Rerserve 4 vertices, and use m_ibo buffer for instance array
    glGenBuffers(1, &m_vbo);
    m_vertices.reserve(4);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, 4 * 48, NULL, GL_DYNAMIC_DRAW);
    glGenBuffers(1, &m_ibo);
    glBindBuffer(GL_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ARRAY_BUFFER, 32 + 48/*Max textures handles*/, NULL,
        GL_DYNAMIC_DRAW);
    SPInstancedData id = SPInstancedData(m_trans, 0.0f, 0.0f, 0.0f, 0);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 32, &id);
    SPTextureManager::get()->increaseGLCommandFunctionCount(1);
    SPTextureManager::get()->addGLCommandFunction
        (std::bind(&SPDynamicDrawCall::initTextureDyDc, this));
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, &m_vao[0]);
    glBindVertexArray(m_vao[0]);

    size_t offset = 0;
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 48, (void*)offset);
    offset += 12;

    // Normal, if 10bit vector normalization is wrongly done by drivers, use
    // original value and normalize ourselves in shader
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_INT_2_10_10_10_REV,
        GraphicsRestrictions::isDisabled
        (GraphicsRestrictions::GR_10BIT_VECTOR) ? GL_FALSE : GL_TRUE, 48,
        (void*)offset);
    offset += 4;

    // Vertex color
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, 48,
        (void*)offset);
    offset += 4;

    // 1st texture coordinates
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_HALF_FLOAT, GL_FALSE, 48, (void*)offset);
    offset += 4;
    // 2nd texture coordinates
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 2, GL_HALF_FLOAT, GL_FALSE, 48,
        (void*)offset);
    offset += 4;

    // Tangent and bi-tanget sign
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_INT_2_10_10_10_REV,
        GraphicsRestrictions::isDisabled
        (GraphicsRestrictions::GR_10BIT_VECTOR) ? GL_FALSE : GL_TRUE,
        48, (void*)offset);
        offset += 4;

    // 4 Joint indices
    glEnableVertexAttribArray(6);
    glVertexAttribIPointer(6, 4, GL_SHORT, 48, (void*)offset);
    offset += 8;
    // 4 Joint weights
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 4, GL_HALF_FLOAT, GL_FALSE, 48,
        (void*)offset);
    offset += 8;

    glBindBuffer(GL_ARRAY_BUFFER, m_ibo);
    // Origin
    glEnableVertexAttribArray(8);
    glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, 32 + 48, (void*)0);
    glVertexAttribDivisorARB(8, 1);
    // Rotation (quaternion .xyz)
    glEnableVertexAttribArray(9);
    glVertexAttribPointer(9, 4, GL_INT_2_10_10_10_REV,
        GraphicsRestrictions::isDisabled
        (GraphicsRestrictions::GR_10BIT_VECTOR) ? GL_FALSE : GL_TRUE, 32 + 48,
        (void*)12);
    glVertexAttribDivisorARB(9, 1);
    // Scale (3 half floats and .w for quaternion .w)
    glEnableVertexAttribArray(10);
    glVertexAttribPointer(10, 4, GL_HALF_FLOAT, GL_FALSE, 32 + 48, (void*)16);
    glVertexAttribDivisorARB(10, 1);
    // Texture translation
    glEnableVertexAttribArray(11);
    glVertexAttribPointer(11, 2, GL_HALF_FLOAT, GL_FALSE, 32 + 48, (void*)24);
    glVertexAttribDivisorARB(11, 1);
    // Misc data (skinning offset and hue change)
    glEnableVertexAttribArray(12);
    glVertexAttribIPointer(12, 2, GL_SHORT, 32 + 48, (void*)28);
    glVertexAttribDivisorARB(12, 1);

    if (CVS->useArrayTextures())
    {
        // uvec4 + uvec2 for 6 texture array indices
        glEnableVertexAttribArray(13);
        glVertexAttribIPointer(13, 4, GL_UNSIGNED_SHORT, 32 + 48, (void*)32);
        glVertexAttribDivisorARB(13, 1);
        glEnableVertexAttribArray(14);
        glVertexAttribIPointer(14, 2, GL_UNSIGNED_SHORT, 32 + 48, (void*)40);
        glVertexAttribDivisorARB(14, 1);
        glDisableVertexAttribArray(15);
    }
    else if (CVS->isARBBindlessTextureUsable())
    {
        // 3 * 2 uvec2 for bindless samplers
        glEnableVertexAttribArray(13);
        glVertexAttribIPointer(13, 4, GL_UNSIGNED_INT, 32 + 48, (void*)32);
        glVertexAttribDivisorARB(13, 1);
        glEnableVertexAttribArray(14);
        glVertexAttribIPointer(14, 4, GL_UNSIGNED_INT, 32 + 48, (void*)48);
        glVertexAttribDivisorARB(14, 1);
        glEnableVertexAttribArray(15);
        glVertexAttribIPointer(15, 4, GL_UNSIGNED_INT, 32 + 48, (void*)64);
        glVertexAttribDivisorARB(15, 1);
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif
}   // SPDynamicDrawCall

// ----------------------------------------------------------------------------
bool SPDynamicDrawCall::initTextureDyDc()
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
    glBindBuffer(GL_ARRAY_BUFFER, m_ibo);
    if (CVS->useArrayTextures())
    {
        for (unsigned i = 0; i < 6; i++)
        {
            uint16_t array_index = (uint16_t)
                m_textures[0][i]->getTextureArrayIndex();
            glBufferSubData(GL_ARRAY_BUFFER, 32 + (i * 2), 2, &array_index);
        }
    }
    else
    {
        for (unsigned i = 0; i < 6; i++)
        {
            glBufferSubData(GL_ARRAY_BUFFER, 32 + (i * 8), 8,
                m_textures[0][i]->getTextureHandlePointer());
        }
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif
    return true;
}   // initTextureDyDc

}