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
#include "config/user_config.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/shader_files_manager.hpp"
#include "graphics/sp/sp_base.hpp"
#include "graphics/sp/sp_uniform_assigner.hpp"
#include "guiengine/message_queue.hpp"
#include "utils/string_utils.hpp"

#include <map>

namespace SP
{
std::map<std::string, std::pair<unsigned, SamplerType> > 
                                                    SPShader::m_prefilled_names;
bool SPShader::m_sp_shader_debug = false;

SPShader::SPShader(const std::string& name,
                   const std::function<void(SPShader*)>& init_func,
                   bool transparent_shader, int drawing_priority,
                   bool use_alpha_channel, bool use_tangents,
                   const std::array<bool, 6>& srgb)
                 : m_name(name), m_init_function(init_func),
                   m_drawing_priority(drawing_priority),
                   m_transparent_shader(transparent_shader),
                   m_use_alpha_channel(use_alpha_channel),
                   m_use_tangents(use_tangents), m_srgb(srgb)
{
#ifndef SERVER_ONLY
    if (CVS->isARBTextureBufferObjectUsable())
    {
#ifndef USE_GLES2
        m_prefilled_names["skinning_tex"] = std::make_pair<unsigned, 
                                            SamplerType>(0, ST_TEXTURE_BUFFER);
#endif
    }
    else
    {
        m_prefilled_names["skinning_tex"] = std::make_pair<unsigned, 
                                            SamplerType>(0, ST_NEAREST_CLAMPED);        
    }
#endif
    
    memset(m_program, 0, 12);
    m_init_function(this);
}
// ----------------------------------------------------------------------------
void SPShader::addShaderFile(const std::string& name, GLint shader_type,
                             RenderPass rp)
{
#ifndef SERVER_ONLY
    if (m_program[rp] == 0)
    {
        m_program[rp] = glCreateProgram();
    }
    auto shader_file = ShaderFilesManager::getInstance()
        ->getShaderFile(name, shader_type);
    if (shader_file)
    {
        m_shader_files.push_back(shader_file);
        glAttachShader(m_program[rp], *shader_file);
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
    for (unsigned i = 0; i < (unsigned)count; i++)
    {
        glDetachShader(m_program[rp], shaders[i]);
    }
    if (result == GL_FALSE)
    {
        if (UserConfigParams::m_artist_debug_mode)
        {
            core::stringw err = StringUtils::insertValues(L"Shader %s failed"
                " to link, check stdout.log or console for details",
                m_name.c_str());
            MessageQueue::add(MessageQueue::MT_ERROR, err);
        }
        glDeleteProgram(m_program[rp]);
        m_program[rp] = 0;
    }
#endif
}   // linkShaderFiles

// ----------------------------------------------------------------------------
void SPShader::addAllTextures(RenderPass rp)
{
#ifndef SERVER_ONLY
    // Built-in prefilled shaders first
    for (auto &p : m_prefilled_names)
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
        m_samplers[rp][i] = idx;
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
void SPShader::bindPrefilledTextures(RenderPass rp) const
{
#ifndef SERVER_ONLY
    for (auto& p : m_prefilled_samplers[rp])
    {
        glActiveTexture(GL_TEXTURE0 + std::get<0>(p));
        auto it = m_prefilled_names.find(std::get<1>(p));
        if (it != m_prefilled_names.end())
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
void SPShader::bindTextures(const std::array<GLuint, 6>& tex,
                            RenderPass rp) const
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
void SPShader::addAllUniforms(RenderPass rp)
{
#ifndef SERVER_ONLY
    GLint total_uniforms = 0;
    glGetProgramiv(m_program[rp], GL_ACTIVE_UNIFORMS, &total_uniforms);
    static const std::map<GLenum, std::type_index> supported_types =
        {
            { GL_INT, std::type_index(typeid(int)) },
            { GL_FLOAT, std::type_index(typeid(float)) },
            { GL_FLOAT_MAT4, std::type_index(typeid(irr::core::matrix4)) },
            { GL_FLOAT_VEC4, std::type_index(typeid(std::array<float, 4>)) },
            { GL_FLOAT_VEC3, std::type_index(typeid(irr::core::vector3df)) },
            { GL_FLOAT_VEC2, std::type_index(typeid(irr::core::vector2df)) }
        };

    for (int i = 0; i < total_uniforms; i++)
    {
        GLint size;
        GLenum type;
        char name[100] = {};
        glGetActiveUniform(m_program[rp], i, 99, NULL, &size, &type, name);
        if (size != 1)
        {
            if (m_sp_shader_debug)
            {
                Log::debug("SPShader", "Array of uniforms is not supported in"
                    " shader %s for %s.", m_name.c_str(), name);
            }
            continue;
        }
        auto ret = supported_types.find(type);
        if (ret == supported_types.end())
        {
            if (m_sp_shader_debug)
                Log::debug("SPShader", "%d type not supported", (unsigned)type);
            continue;
        }
        GLuint location = glGetUniformLocation(m_program[rp], name);
        if (location == GL_INVALID_INDEX)
        {
            if (m_sp_shader_debug)
                Log::debug("SPShader", "%s uniform not found", name);
            continue;
        }
        m_uniforms[rp][name] = new SPUniformAssigner(ret->second, location);
    }
#endif
}   // addAllUniforms

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
                                                RenderPass rp) const
{
    auto ret = m_uniforms[rp].find(name);
    if (ret == m_uniforms[rp].end())
    {
        return NULL;
    }
    return ret->second;
}   // getUniformAssigner

// ----------------------------------------------------------------------------
void SPShader::unload()
{
#ifndef SERVER_ONLY
    for (unsigned rp = RP_1ST; rp < RP_COUNT; rp++)
    {
        if (m_program[rp] != 0)
        {
            glDeleteProgram(m_program[rp]);
            m_program[rp] = 0;
        }
        for (auto& p : m_uniforms[rp])
        {
            delete p.second;
        }
        m_uniforms[rp].clear();
        m_custom_prefilled_getter[rp].clear();
        m_prefilled_samplers[rp].clear();
        m_samplers[rp].clear();
        m_use_function[rp] = nullptr;
        m_unuse_function[rp] = nullptr;
    }
    m_shader_files.clear();
#endif
}   // unload

// ----------------------------------------------------------------------------
bool SPShader::isSrgbForTextureLayer(unsigned layer) const
{
#ifndef SERVER_ONLY
    if (!CVS->isDeferredEnabled())
    {
        return false;
    }
#endif
    assert(layer < 6);
    return m_srgb[layer];
}   // isSrgbForTextureLayer

}
