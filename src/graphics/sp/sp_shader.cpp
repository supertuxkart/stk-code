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

#include "graphics/sp/sp_shader.hpp"
#include "graphics/shader_files_manager.hpp"
#include "graphics/sp/sp_base.hpp"
#include "graphics/sp/sp_uniform_assigner.hpp"
#include "utils/no_copy.hpp"
#include "utils/string_utils.hpp"

#include <ITexture.h>
#include <string>

namespace SP
{
std::unordered_map<std::string, std::pair<unsigned, SamplerType> >
    g_prefilled_names =
    {
#ifdef USE_GLES2
        { "skinning_tex", { 0, ST_NEAREST_CLAMPED } }
#else
        { "skinning_tex", { 0, ST_TEXTURE_BUFFER } }
#endif
    };

// ----------------------------------------------------------------------------
SPShader::~SPShader()
{
#ifndef SERVER_ONLY
    for (unsigned rp = RP_1ST; rp < RP_COUNT; rp++)
    {
        if (m_program[rp] != 0)
        {
            glDeleteProgram(m_program[rp]);
        }
        for (auto& p : m_uniforms[rp])
        {
            delete p.second;
        }
    }
#endif
}   // ~SPShader

// ----------------------------------------------------------------------------
void SPShader::addShaderFile(const std::string& name, GLint shader_type,
                             RenderPass rp)
{
#ifndef SERVER_ONLY
    auto shader_id = ShaderFilesManager::getInstance()
        ->getShaderFile(name, shader_type);
    if (shader_id)
    {
        m_shaders.insert(shader_id);
        glAttachShader(m_program[rp], *shader_id);
    }
#endif
}   // addShaderFile

// ----------------------------------------------------------------------------
void SPShader::linkShaderFiles(RenderPass rp)
{
#ifndef SERVER_ONLY
    glLinkProgram(m_program[rp]);
    GLint result = GL_FALSE;
    glGetProgramiv(m_program[rp], GL_LINK_STATUS, &result);
    if (result == GL_FALSE)
    {
        Log::error("SPShader", "Error when linking shader %s in pass %d",
            m_name.c_str(), (int)rp);
        int info_length;
        glGetProgramiv(m_program[rp], GL_INFO_LOG_LENGTH, &info_length);
        char *error_message = new char[info_length];
        glGetProgramInfoLog(m_program[rp], info_length, NULL, error_message);
        Log::error("SPShader", error_message);
        delete[] error_message;
    }
    // After linking all shaders can be detached
    GLuint shaders[10] = {};
    GLsizei count = 0;
    glGetAttachedShaders(m_program[rp], 10, &count, shaders);
    for (unsigned i = 0; i < count; i++)
    {
        glDetachShader(m_program[rp], shaders[i]);
    }
#endif
}   // linkShaderFiles

// ----------------------------------------------------------------------------
void SPShader::addAllTextures(RenderPass rp)
{
#ifndef SERVER_ONLY
    // Built-in prefilled shaders first
    for (auto &p : g_prefilled_names)
    {
        const char* s = p.first.c_str();
        GLuint loc = glGetUniformLocation(m_program[rp], s);
        if (loc == GL_INVALID_INDEX)
        {
            continue;
        }
        const unsigned i = (unsigned)m_prefilled_samplers[rp].size();
        glUniform1i(loc, i);
#ifdef USE_GLES2
        m_prefilled_samplers[rp].emplace_back(i, p.first, p.second.second,
            p.first == "tex_array" ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D);
#else
        m_prefilled_samplers[rp].emplace_back(i, p.first, p.second.second,
            p.second.second == ST_TEXTURE_BUFFER ?
            GL_TEXTURE_BUFFER :
            p.first == "tex_array" ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D);
#endif
    }

    // Add tex_layer_0-5 if exists in shader, sampler is always ST_TRILINEAR,
    // texture type is always GL_TEXTURE_2D
    for (unsigned i = 0; i < 6; i++)
    {
        std::string texture_name = "tex_layer_";
        texture_name += StringUtils::toString(i);
        GLuint loc = glGetUniformLocation(m_program[rp], texture_name.c_str());
        if (loc == GL_INVALID_INDEX)
        {
            continue;
        }
        const unsigned idx =
            unsigned(m_prefilled_samplers[rp].size() + m_samplers[rp].size());
        glUniform1i(loc, idx);
        m_samplers[rp].emplace_back(i, idx);
    }
#endif
}   // addPrefilledTextures

// ----------------------------------------------------------------------------
void SPShader::addCustomPrefilledTextures(SamplerType st, GLuint texture_type,
                                          const std::string& name,
                                          std::function<GLuint()> func,
                                          RenderPass rp)
{
#ifndef SERVER_ONLY
    assert(func != NULL);
    const char* s = name.c_str();
    GLuint loc = glGetUniformLocation(m_program[rp], s);
    if (loc == GL_INVALID_INDEX)
    {
        Log::warn("SPShader", "Missing custom prefilled texture %s in shader"
            " files.", s);
        return;
    }
    const unsigned i =
        unsigned(m_samplers[rp].size() + m_prefilled_samplers[rp].size());
    glUniform1i(loc, i);
    m_prefilled_samplers[rp].emplace_back(i, name, st, texture_type);
    m_custom_prefilled_getter[rp][name] = func;
#endif
}   // addCustomPrefilledTextures

// ----------------------------------------------------------------------------
void SPShader::bindPrefilledTextures(RenderPass rp)
{
#ifndef SERVER_ONLY
    for (auto& p : m_prefilled_samplers[rp])
    {
        glActiveTexture(GL_TEXTURE0 + std::get<0>(p));
        auto it = g_prefilled_names.find(std::get<1>(p));
        if (it != g_prefilled_names.end())
        {
            glBindTexture(std::get<3>(p), sp_prefilled_tex[it->second.first]);
            glBindSampler(std::get<0>(p), getSampler(std::get<2>(p)));
        }
        else
        {
            glBindTexture(std::get<3>(p),
                m_custom_prefilled_getter[rp].at(std::get<1>(p))());
            glBindSampler(std::get<0>(p), getSampler(std::get<2>(p)));
        }
    }
#endif
}   // bindPrefilledTextures

// ----------------------------------------------------------------------------
void SPShader::bindTextures(const std::array<GLuint, 6>& tex, RenderPass rp)
{
#ifndef SERVER_ONLY
    for (auto& p : m_samplers[rp])
    {
        glActiveTexture(GL_TEXTURE0 + p.second);
        glBindTexture(GL_TEXTURE_2D, tex[p.first]);
        glBindSampler(p.second, getSampler(ST_TRILINEAR));
    }
#endif
}   // bindTextures

// ----------------------------------------------------------------------------
void SPShader::addUniform(const std::string& name, const std::type_info& ti,
                          RenderPass rp)
{
#ifndef SERVER_ONLY
    const char* s = name.c_str();
    GLuint location = glGetUniformLocation(m_program[rp], s);
    if (location == GL_INVALID_INDEX)
    {
        Log::warn("SPShader", "Missing uniform %s in shader files.", s);
        return;
    }
    m_uniforms[rp][name] = new SPUniformAssigner(ti, location);
#endif
}   // addUniform

// ----------------------------------------------------------------------------
void SPShader::setUniformsPerObject(SPPerObjectUniform* sppou,
                                    std::vector<SPUniformAssigner*>* ua_used,
                                    RenderPass rp)
{
#ifndef SERVER_ONLY
    if (sppou->isEmpty())
    {
        return;
    }
    for (auto& p : m_uniforms[rp])
    {
        if (sppou->assignUniform(p.first, p.second))
        {
            ua_used->push_back(p.second);
        }
    }
#endif
}   // setUniformsPerObject

// ----------------------------------------------------------------------------
SPUniformAssigner* SPShader::getUniformAssigner(const std::string& name,
                                                RenderPass rp)
{
    auto ret = m_uniforms[rp].find(name);
    if (ret == m_uniforms[rp].end())
    {
        return NULL;
    }
    return ret->second;
}   // getUniformAssigner

}
