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

#include "graphics/sp/sp_shader_manager.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/sp/sp_base.hpp"
#include "graphics/sp/sp_shader.hpp"
#include "graphics/sp/sp_texture.hpp"
#include "graphics/sp/sp_texture_manager.hpp"
#include "graphics/sp/sp_uniform_assigner.hpp"
#include "tracks/track.hpp"
#include "utils/string_utils.hpp"
#include "utils/log.hpp"

#include <algorithm>
#include <IFileSystem.h>

namespace SP
{
SPShaderManager* SPShaderManager::m_spsm = NULL;
// ----------------------------------------------------------------------------
SPShaderManager::SPShaderManager()
{
#ifndef SERVER_ONLY
    m_official_sampler_types =
    {
        { "nearest", ST_NEAREST },
        { "nearest_clamped", ST_NEAREST_CLAMPED },
        { "bilinear", ST_BILINEAR },
        { "bilinear_clamped", ST_BILINEAR_CLAMPED },
        { "trilinear", ST_TRILINEAR },
        { "trilinear_clamped", ST_TRILINEAR_CLAMPED },
        { "semi_trilinear", ST_SEMI_TRILINEAR }
    };

    m_official_uniform_assigner_functions =
    {
        { "shadowCascadeUniformAssigner", [](SPUniformAssigner* ua)
            {
                ua->setValue(sp_cur_shadow_cascade);
            }
        },
        { "windDirectionUniformAssigner", [](SPUniformAssigner* ua)
            {
                ua->setValue(sp_wind_dir);
            }
        },
        { "isDuringDayUniformAssigner", [](SPUniformAssigner* ua)
            {
                int is_during_day = Track::getCurrentTrack() ?
                Track::getCurrentTrack()->getIsDuringDay() ? 1 : 0 : 0;
                ua->setValue(is_during_day);
            }
        },
        { "zeroAlphaUniformAssigner", [](SPUniformAssigner* ua)
            {
                ua->setValue(0.0f);
            }
        },
        { "fogUniformAssigner", [](SPUniformAssigner* ua)
            {
                int fog_enable = Track::getCurrentTrack() ?
                    Track::getCurrentTrack()->isFogEnabled() ? 1 : 0 : 0;
                ua->setValue(fog_enable);
            }
        },
        { "ghostAlphaUniformAssigner", [](SPUniformAssigner* ua)
            {
                float alpha = 1.0f;
                if (Track::getCurrentTrack())
                {
                    const video::SColor& c = Track::getCurrentTrack()
                        ->getSunColor();
                    float y = 0.2126f * c.getRed() + 0.7152f * c.getGreen() +
                        0.0722f * c.getBlue();
                    alpha = y > 128.0f ? 0.5f : 0.35f;
                }
                ua->setValue(alpha);
           }
       }
    };

    m_official_use_functions =
    {
        { "alphaBlendUse", []()
            {
                glEnable(GL_DEPTH_TEST);
                glDepthMask(GL_FALSE);
                glDisable(GL_CULL_FACE);
                glEnable(GL_BLEND);
                glBlendEquation(GL_FUNC_ADD);
                glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            }
        },
        { "additiveUse", []()
            {
                glEnable(GL_DEPTH_TEST);
                glDepthMask(GL_FALSE);
                glDisable(GL_CULL_FACE);
                glEnable(GL_BLEND);
                glBlendEquation(GL_FUNC_ADD);
                glBlendFunc(GL_ONE, GL_ONE);
            }
        },
        { "ghostUse", []()
            {
                glEnable(GL_DEPTH_TEST);
                glDepthMask(GL_TRUE);
                glEnable(GL_CULL_FACE);
                glEnable(GL_BLEND);
                glBlendEquation(GL_FUNC_ADD);
                glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            }
        }
    };
#endif
}   // SPShaderManager

// ----------------------------------------------------------------------------
SPShaderManager::~SPShaderManager()
{
    m_official_shaders.clear();
    m_shaders.clear();
}   // ~SPShaderManager

// ----------------------------------------------------------------------------
void SPShaderManager::loadEachShader(const std::string& file_name)
{
#ifndef SERVER_ONLY
    std::unique_ptr<XMLNode> xml(file_manager->createXMLTree(file_name));
    if (!xml || xml->getName() != "spshader")
    {
        Log::error("SPShaderManager", "Invalid SPShader file %s",
            file_name.c_str());
        return;
    }

    ShaderInfo si;

    const XMLNode* shader_info = xml->getNode("shader-info");
    if (!shader_info)
    {
        Log::error("SPShaderManager", "Missing shader-info header in file %s",
            file_name.c_str());
        return;
    }

    shader_info->get("name", &si.m_shader_name);
    if (si.m_shader_name.empty())
    {
        Log::error("SPShaderManager", "Empty shader name in file %s",
            file_name.c_str());
        return;
    }
    else if (si.m_shader_name.find("_skinned") != std::string::npos)
    {
        Log::error("SPShaderManager", "_skinned name is reserved for auto"
            " skinned mesh shader adding");
        return;
    }
    else if (getSPShader(si.m_shader_name))
    {
        Log::error("SPShaderManager", "%s shader already exists",
            si.m_shader_name.c_str());
        return;
    }

    shader_info->get("fallback-shader", &si.m_fallback_name);
    shader_info->get("transparent", &si.m_transparent_shader);
    shader_info->get("drawing-priority", &si.m_drawing_priority);
    shader_info->get("use-alpha-channel", &si.m_use_alpha_channel);
    shader_info->get("use-tangents", &si.m_use_tangents);
    std::string srgb_prop;
    shader_info->get("srgb", &srgb_prop);
    std::vector<std::string> srgb_props = StringUtils::split(srgb_prop, ' ');
    if (srgb_props.size() == 6)
    {
        for (unsigned i = 0; i < 6; i++)
        {
            si.m_srgb[i] = srgb_props[i] == "Y";
        }
    }
    else if (!srgb_prop.empty())
    {
        Log::error("SPShaderManager", "Invalid srgb properties in shader");
    }

    std::array<PassInfo, 2> pi;
    loadPassInfo(xml->getNode("first-pass"), pi[0]);
    if (!si.m_transparent_shader && CVS->isDeferredEnabled())
    {
        loadPassInfo(xml->getNode("shadow-pass"), pi[1]);
    }
    if (pi[0].m_vertex_shader.empty())
    {
        Log::error("SPShaderManager", "Missing first pass vertex shader in"
            " file %s", file_name.c_str());
        return;
    }
    if (!si.m_fallback_name.empty() && !CVS->isDeferredEnabled())
    {
        std::shared_ptr<SPShader> fallback_shader =
            getSPShader(si.m_fallback_name);
        if (!fallback_shader)
        {
            Log::error("SPShaderManager", "%s fallback shader missing",
                si.m_fallback_name.c_str());
        }
        else
        {
            addSPShader(si.m_shader_name, fallback_shader);
            if (!pi[0].m_skinned_mesh_shader.empty())
            {
                std::shared_ptr<SPShader> fallback_skinned_shader =
                    getSPShader(si.m_fallback_name + "_skinned");
                if (!fallback_skinned_shader)
                {
                    Log::error("SPShaderManager", "%s fallback skinned mesh"
                        " shader missing", si.m_fallback_name.c_str());
                }
                addSPShader(si.m_shader_name + "_skinned",
                    fallback_skinned_shader);
            }
        }
        return;
    }

    UniformAssigners ua;
    const XMLNode* uniform_assigners = xml->getNode("uniform-assigners");
    if (uniform_assigners)
    {
        for (unsigned i = 0; i < uniform_assigners->getNumNodes(); i++)
        {
            const XMLNode* uniform_assigner = uniform_assigners->getNode(i);
            if (uniform_assigner->getName() == "uniform-assigner")
            {
                std::string name, function;
                uniform_assigner->get("name", &name);
                uniform_assigner->get("function", &function);
                if (!name.empty() && !function.empty() &&
                    m_official_uniform_assigner_functions.find(function) !=
                    m_official_uniform_assigner_functions.end())
                {
                    ua.emplace_back(name, m_official_uniform_assigner_functions
                        .at(function));
                }
                else
                {
                    Log::error("SPShaderManager", "Invalid uniform assigner"
                        " %s", function.c_str());
                }
            }
        }
    }

    addSPShader(si.m_shader_name, buildSPShader(si, pi, ua, false/*skinned*/));
    if (!pi[0].m_skinned_mesh_shader.empty())
    {
        addSPShader(si.m_shader_name + "_skinned", buildSPShader(si, pi, ua,
            true/*skinned*/));
    }
#endif
}   // loadEachShader

// ----------------------------------------------------------------------------
void SPShaderManager::loadPassInfo(const XMLNode* pass, PassInfo& pi)
{
    if (!pass)
    {
        return;
    }

    std::string use_function, unuse_function;
    pass->get("use-function", &use_function);
    if (!use_function.empty() && m_official_use_functions.find(use_function) !=
        m_official_use_functions.end())
    {
        pi.m_use_function = m_official_use_functions.at(use_function);
    }

    pass->get("unuse-function", &unuse_function);
    if (!unuse_function.empty() &&
        m_official_unuse_functions.find(unuse_function) !=
        m_official_unuse_functions.end())
    {
        pi.m_unuse_function = m_official_unuse_functions.at(unuse_function);
    }

    pass->get("vertex-shader", &pi.m_vertex_shader);
    pi.m_vertex_shader = getShaderFullPath(pi.m_vertex_shader);

    pass->get("fragment-shader", &pi.m_fragment_shader);
    pi.m_fragment_shader = getShaderFullPath(pi.m_fragment_shader);

    pass->get("skinned-mesh-shader", &pi.m_skinned_mesh_shader);
    pi.m_skinned_mesh_shader = getShaderFullPath(pi.m_skinned_mesh_shader);

    const XMLNode* prefilled_textures = pass->getNode("prefilled-textures");
    if (prefilled_textures)
    {
        for (unsigned i = 0; i < prefilled_textures->getNumNodes(); i++)
        {
            const XMLNode* prefilled_texture = prefilled_textures->getNode(i);
            if (prefilled_texture->getName() == "prefilled-texture")
            {
                bool srgb = false;
                SamplerType st = ST_TRILINEAR;
                std::string name, file, srgb_props, sampler_props;
                prefilled_texture->get("name", &name);
                prefilled_texture->get("file", &file);
                prefilled_texture->get("srgb", &srgb_props);
#ifndef SERVER_ONLY
                if (!srgb_props.empty())
                {
                    srgb = srgb_props == "Y" && CVS->isDeferredEnabled();
                }
#endif
                prefilled_texture->get("sampler", &sampler_props);
                if (!sampler_props.empty() &&
                    m_official_sampler_types.find(sampler_props) !=
                    m_official_sampler_types.end())
                {
                    st = m_official_sampler_types.at(sampler_props);
                }
                if (!name.empty() && !file.empty())
                {
                    pi.m_prefilled_textures.emplace_back(name, file, srgb, st);
                }
                else
                {
                    Log::error("SPShaderManager", "Invalid prefilled texture");
                }
            }
        }
    }
}   // loadPassInfo

// ----------------------------------------------------------------------------
std::string SPShaderManager::getShaderFullPath(const std::string& name)
{
    if (name.empty())
    {
        return "";
    }
    std::string cur_location = m_shader_directory + name;
    if (file_manager->fileExists(cur_location))
    {
        return cur_location;
    }
    cur_location = file_manager->getAssetChecked(FileManager::SHADER, name);
    if (cur_location.empty())
    {
        return "";
    }
    return file_manager->getFileSystem()->getAbsolutePath(cur_location.c_str())
        .c_str();
}   // getShaderFullPath

// ----------------------------------------------------------------------------
std::shared_ptr<SPShader> SPShaderManager::buildSPShader(const ShaderInfo& si,
    const std::array<PassInfo, 2>& pi, const UniformAssigners& ua,
    bool skinned)
{
    std::shared_ptr<SPShader> sps;
#ifndef SERVER_ONLY
    sps = std::make_shared<SPShader>(si.m_shader_name,
        [this, pi, ua, skinned](SPShader* shader)
        {
            // First pass
            assert(!pi[0].m_vertex_shader.empty() ||
                (skinned && !pi[0].m_skinned_mesh_shader.empty()));

            SPPerObjectUniform* pou = static_cast<SPPerObjectUniform*>(shader);
            for (auto& p : ua)
            {
                pou->addAssignerFunction(p.first, p.second);
            }

            shader->addShaderFile(skinned ?
                pi[0].m_skinned_mesh_shader : pi[0].m_vertex_shader,
                GL_VERTEX_SHADER, RP_1ST);
            if (!pi[0].m_fragment_shader.empty())
            {
                shader->addShaderFile(pi[0].m_fragment_shader,
                    GL_FRAGMENT_SHADER, RP_1ST);
            }
            shader->linkShaderFiles(RP_1ST);
            shader->use(RP_1ST);
            shader->addBasicUniforms(RP_1ST);
            shader->addAllUniforms(RP_1ST);
            if (pi[0].m_use_function)
            {
                shader->setUseFunction(pi[0].m_use_function, RP_1ST);
            }
            if (pi[0].m_unuse_function)
            {
                shader->setUnuseFunction(pi[0].m_unuse_function, RP_1ST);
            }
            addPrefilledTexturesToShader(shader, pi[0].m_prefilled_textures,
                RP_1ST);
            shader->addAllTextures(RP_1ST);

            if (pi[1].m_vertex_shader.empty())
            {
                return;
            }
            // Shadow pass
            if (skinned && pi[1].m_skinned_mesh_shader.empty())
            {
                Log::warn("SPShader", "Missing skinned mesh vertex shader in"
                    " shadow pass");
                return;
            }
            shader->addShaderFile(skinned ?
                pi[1].m_skinned_mesh_shader : pi[1].m_vertex_shader,
                GL_VERTEX_SHADER, RP_SHADOW);
            if (!pi[1].m_fragment_shader.empty())
            {
                shader->addShaderFile(pi[1].m_fragment_shader,
                    GL_FRAGMENT_SHADER, RP_SHADOW);
            }
            shader->linkShaderFiles(RP_SHADOW);
            shader->use(RP_SHADOW);
            shader->addBasicUniforms(RP_SHADOW);
            shader->addAllUniforms(RP_SHADOW);
            if (pi[1].m_use_function)
            {
                shader->setUseFunction(pi[1].m_use_function, RP_SHADOW);
            }
            if (pi[1].m_unuse_function)
            {
                shader->setUnuseFunction(pi[1].m_unuse_function, RP_SHADOW);
            }
            addPrefilledTexturesToShader(shader, pi[1].m_prefilled_textures,
                RP_SHADOW);
            shader->addAllTextures(RP_SHADOW);
        }, si.m_transparent_shader, si.m_drawing_priority,
        si.m_use_alpha_channel, si.m_use_tangents, si.m_srgb);
#endif
    return sps;
}   // buildSPShader

// ----------------------------------------------------------------------------
void SPShaderManager::addPrefilledTexturesToShader(SPShader* s,
    const std::vector<std::tuple<std::string, std::string, bool, SamplerType> >
    &t, RenderPass rp)
{
#ifndef SERVER_ONLY
    for (auto& p : t)
    {
        std::string full_path;
        const std::string& relative_path =
            file_manager->searchTexture(std::get<1>(p)/*filename*/);
        if (relative_path.empty())
        {
            Log::warn("SPShader", "Cannot determine prefilled texture full"
                " path: %s", std::get<1>(p).c_str());
        }
        else
        {
            full_path = file_manager->getFileSystem()->getAbsolutePath
                (relative_path.c_str()).c_str();
        }
        if (!full_path.empty())
        {
            std::string cid;
            if (!file_manager->searchTextureContainerId(cid, std::get<1>(p)))
            {
                Log::warn("SPShader", "Missing container id for %s, no texture"
                    " compression for it will be done.",
                    std::get<1>(p).c_str());
            }
            std::shared_ptr<SPTexture> pt = SPTextureManager::get()
                ->getTexture(full_path, NULL/*material*/, std::get<2>(p), cid);
            s->addCustomPrefilledTextures(std::get<3>(p)/*sampler_type*/,
                GL_TEXTURE_2D, std::get<0>(p)/*name_in_shader*/, [pt]()->GLuint
                {
                    return pt->getTextureHandler();
                }, rp);
        }
    }
#endif
}   // addPrefilledTexturesToShader

// ----------------------------------------------------------------------------
void SPShaderManager::loadSPShaders(const std::string& directory_name)
{
    std::set<std::string> shaders;
    file_manager->listFiles(shaders, directory_name);
    for (auto it = shaders.begin(); it != shaders.end();)
    {
        if ((*it).find("sps") == std::string::npos ||
            (*it).find(".xml") == std::string::npos)
        {
            it = shaders.erase(it);
        }
        else
        {
            it++;
        }
    }
    if (shaders.empty())
    {
        return;
    }

    m_shader_directory = file_manager->getFileSystem()->getAbsolutePath
        (directory_name.c_str()).c_str();
    for (const std::string& file_name : shaders)
    {
        loadEachShader(m_shader_directory + file_name);
    }
    m_shader_directory = "";
}   // loadSPShaders

// ----------------------------------------------------------------------------
void SPShaderManager::unloadAll()
{
    for (auto& p : m_shaders)
    {
        p.second->unload();
    }
}   // unloadAll

// ----------------------------------------------------------------------------
void SPShaderManager::initAll()
{
    for (auto& p : m_shaders)
    {
        p.second->init();
    }
}   // initAll

// ----------------------------------------------------------------------------
void SPShaderManager::removeUnusedShaders()
{
    for (auto it = m_shaders.begin(); it != m_shaders.end();)
    {
        if (it->second.use_count() == 1)
        {
            it = m_shaders.erase(it);
        }
        else
        {
            it++;
        }
    }
}   // removeUnusedShaders

}
