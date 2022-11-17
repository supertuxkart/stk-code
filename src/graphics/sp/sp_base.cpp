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

#ifndef SERVER_ONLY

#include "graphics/sp/sp_base.hpp"
#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/frame_buffer.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/shader_based_renderer.hpp"
#include "graphics/shared_gpu_objects.hpp"
#include "graphics/shader_based_renderer.hpp"
#include "graphics/post_processing.hpp"
#include <ge_render_info.hpp>
#include "graphics/rtts.hpp"
#include "graphics/shaders.hpp"
#include "graphics/sp/sp_dynamic_draw_call.hpp"
#include "graphics/sp/sp_instanced_data.hpp"
#include "graphics/sp/sp_per_object_uniform.hpp"
#include "graphics/sp/sp_mesh.hpp"
#include "graphics/sp/sp_mesh_buffer.hpp"
#include "graphics/sp/sp_mesh_node.hpp"
#include "graphics/sp/sp_shader.hpp"
#include "graphics/sp/sp_shader_manager.hpp"
#include "graphics/sp/sp_texture.hpp"
#include "graphics/sp/sp_texture_manager.hpp"
#include "graphics/sp/sp_uniform_assigner.hpp"
#include "guiengine/engine.hpp"
#include "tracks/track.hpp"
#include "utils/log.hpp"
#include "utils/helpers.hpp"
#include "utils/profiler.hpp"
#include "utils/string_utils.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <ge_main.hpp>

#include <IrrlichtDevice.h>

using namespace GE;

namespace SP
{

// ----------------------------------------------------------------------------
ShaderBasedRenderer* g_stk_sbr = NULL;
// ----------------------------------------------------------------------------
std::array<float, 16>* g_joint_ptr = NULL;
// ----------------------------------------------------------------------------
bool sp_culling = true;
// ----------------------------------------------------------------------------
bool sp_apitrace = false;
// ----------------------------------------------------------------------------
bool sp_debug_view = false;
// ----------------------------------------------------------------------------
bool g_handle_shadow = false;
// ----------------------------------------------------------------------------
SPShader* g_normal_visualizer = NULL;
// ----------------------------------------------------------------------------
SPShader* g_glow_shader = NULL;
// ----------------------------------------------------------------------------
// std::string is layer_1 and layer_2 texture name combined
typedef std::unordered_map<SPShader*, std::unordered_map<std::string,
    std::unordered_set<SPMeshBuffer*> > > DrawCall;

DrawCall g_draw_calls[DCT_FOR_VAO];
// ----------------------------------------------------------------------------
std::vector<std::pair<SPShader*, std::vector<std::pair<std::array<GLuint, 6>,
    std::vector<std::pair<SPMeshBuffer*, int/*material_id*/> > > > > >
    g_final_draw_calls[DCT_FOR_VAO];
// ----------------------------------------------------------------------------
std::unordered_map<unsigned, std::pair<core::vector3df,
    std::unordered_set<SPMeshBuffer*> > > g_glow_meshes;
// ----------------------------------------------------------------------------
std::unordered_set<SPMeshBuffer*> g_instances;
// ----------------------------------------------------------------------------
std::array<GLuint, ST_COUNT> g_samplers;
// ----------------------------------------------------------------------------
// Check sp_shader.cpp for the name
std::array<GLuint, 1> sp_prefilled_tex;
// ----------------------------------------------------------------------------
std::atomic<uint32_t> sp_max_texture_size(2048);
// ----------------------------------------------------------------------------
std::vector<float> g_bounding_boxes;
// ----------------------------------------------------------------------------
std::vector<std::shared_ptr<SPDynamicDrawCall> > g_dy_dc;
// ----------------------------------------------------------------------------
float g_frustums[5][24] = { { } };
// ----------------------------------------------------------------------------
unsigned sp_solid_poly_count = 0;
// ----------------------------------------------------------------------------
unsigned sp_shadow_poly_count = 0;
// ----------------------------------------------------------------------------
unsigned sp_cur_player = 0;
// ----------------------------------------------------------------------------
unsigned sp_cur_buf_id[MAX_PLAYER_COUNT] = {};
// ----------------------------------------------------------------------------
unsigned g_skinning_offset = 0;
// ----------------------------------------------------------------------------
std::vector<SPMeshNode*> g_skinning_mesh;
// ----------------------------------------------------------------------------
int sp_cur_shadow_cascade = 0;
// ----------------------------------------------------------------------------
void initSTKRenderer(ShaderBasedRenderer* sbr)
{
    g_stk_sbr = sbr;
}   // initSTKRenderer
// ----------------------------------------------------------------------------
GLuint sp_mat_ubo[MAX_PLAYER_COUNT][3] = {};
// ----------------------------------------------------------------------------
GLuint sp_fog_ubo = 0;
// ----------------------------------------------------------------------------
core::vector3df sp_wind_dir;
// ----------------------------------------------------------------------------
GLuint g_skinning_tex;
// ----------------------------------------------------------------------------
GLuint g_skinning_buf;
// ----------------------------------------------------------------------------
unsigned g_skinning_size;
// ----------------------------------------------------------------------------
ShaderBasedRenderer* getRenderer()
{
    return g_stk_sbr;
}   // getRenderer

// ----------------------------------------------------------------------------
void displaceUniformAssigner(SP::SPUniformAssigner* ua)
{
    static std::array<float, 4> g_direction = {{ 0, 0, 0, 0 }};
    if (!Track::getCurrentTrack())
    {
        ua->setValue(g_direction);
        return;
    }
    const float time = irr_driver->getDevice()->getTimer()->getTime() /
        1000.0f;
    const float speed = Track::getCurrentTrack()->getDisplacementSpeed();

    float strength = time;
    strength = fabsf(noise2d(strength / 10.0f)) * 0.006f + 0.002f;

    core::vector3df wind = irr_driver->getWind() * strength * speed;
    g_direction[0] += wind.X;
    g_direction[1] += wind.Z;

    strength = time * 0.56f + sinf(time);
    strength = fabsf(noise2d(0.0, strength / 6.0f)) * 0.0095f + 0.0025f;

    wind = irr_driver->getWind() * strength * speed;
    wind.rotateXZBy(cosf(time));
    g_direction[2] += wind.X;
    g_direction[3] += wind.Z;
    ua->setValue(g_direction);
}   // displaceUniformAssigner

// ----------------------------------------------------------------------------
void displaceShaderInit(SPShader* shader)
{
    shader->addShaderFile("sp_pass.vert", GL_VERTEX_SHADER, RP_1ST);
    shader->addShaderFile("white.frag", GL_FRAGMENT_SHADER, RP_1ST);
    shader->linkShaderFiles(RP_1ST);
    shader->use(RP_1ST);
    shader->addBasicUniforms(RP_1ST);
    shader->setUseFunction([]()->void
        {
            assert(g_stk_sbr->getRTTs() != NULL);
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);
            glDisable(GL_CULL_FACE);
            glDisable(GL_BLEND);
            glClear(GL_STENCIL_BUFFER_BIT);
            glEnable(GL_STENCIL_TEST);
            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            g_stk_sbr->getRTTs()->getFBO(FBO_RGBA_1).bind(),
            glClear(GL_COLOR_BUFFER_BIT);
        }, RP_1ST);
    shader->addShaderFile("sp_pass.vert", GL_VERTEX_SHADER, RP_RESERVED);
    shader->addShaderFile("sp_displace.frag", GL_FRAGMENT_SHADER, RP_RESERVED);
    shader->linkShaderFiles(RP_RESERVED);
    shader->use(RP_RESERVED);
    shader->addBasicUniforms(RP_RESERVED);
    shader->addAllUniforms(RP_RESERVED);
    shader->setUseFunction([]()->void
        {
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);
            glDisable(GL_CULL_FACE);
            glDisable(GL_BLEND);
            glEnable(GL_STENCIL_TEST);
            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            g_stk_sbr->getRTTs()->getFBO(FBO_TMP1_WITH_DS).bind(),
            glClear(GL_COLOR_BUFFER_BIT);
        }, RP_RESERVED);
    SPShaderManager::addPrefilledTexturesToShader(shader,
        {std::make_tuple("displacement_tex", "displace.png", false/*srgb*/,
        ST_BILINEAR)}, RP_RESERVED);
    shader->addCustomPrefilledTextures(ST_BILINEAR,
        GL_TEXTURE_2D, "mask_tex", []()->GLuint
        {
            return g_stk_sbr->getRTTs()->getFBO(FBO_RGBA_1).getRTT()[0];
        }, RP_RESERVED);
    shader->addCustomPrefilledTextures(ST_BILINEAR,
        GL_TEXTURE_2D, "color_tex", []()->GLuint
        {
            return g_stk_sbr->getRTTs()->getFBO(FBO_COLORS).getRTT()[0];
        }, RP_RESERVED);
    shader->addAllTextures(RP_RESERVED);
    shader->setUnuseFunction([]()->void
        {
            g_stk_sbr->getRTTs()->getFBO(FBO_COLORS).bind();
            glStencilFunc(GL_EQUAL, 1, 0xFF);
            g_stk_sbr->getPostProcessing()->renderPassThrough
                (g_stk_sbr->getRTTs()->getFBO(FBO_TMP1_WITH_DS).getRTT()[0],
                g_stk_sbr->getRTTs()->getFBO(FBO_COLORS).getWidth(),
                g_stk_sbr->getRTTs()->getFBO(FBO_COLORS).getHeight());
            glDisable(GL_STENCIL_TEST);
        }, RP_RESERVED);
    static_cast<SPPerObjectUniform*>(shader)
        ->addAssignerFunction("direction", displaceUniformAssigner);
}   // displaceShaderInit

// ----------------------------------------------------------------------------
void resizeSkinning(unsigned number)
{
    const irr::core::matrix4 m;
    g_skinning_size = number;



    if (!CVS->isARBTextureBufferObjectUsable())
    {
        glBindTexture(GL_TEXTURE_2D, g_skinning_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, number, 0, GL_RGBA,
            GL_FLOAT, NULL);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4, 1, GL_RGBA, GL_FLOAT,
            m.pointer());
        glBindTexture(GL_TEXTURE_2D, 0);
        static std::vector<std::array<float, 16> >
            tmp_buf(stk_config->m_max_skinning_bones);
        g_joint_ptr = tmp_buf.data();
    }
    else
    {
#ifndef USE_GLES2
        glBindBuffer(GL_TEXTURE_BUFFER, g_skinning_buf);
        if (CVS->isARBBufferStorageUsable())
        {
            glBufferStorage(GL_TEXTURE_BUFFER, number << 6, NULL,
                GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
            g_joint_ptr = (std::array<float, 16>*)glMapBufferRange(
                GL_TEXTURE_BUFFER, 0, 64,
                GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
            memcpy(g_joint_ptr, m.pointer(), 64);
            glUnmapBuffer(GL_TEXTURE_BUFFER);
            g_joint_ptr = (std::array<float, 16>*)glMapBufferRange(
                GL_TEXTURE_BUFFER, 64, (number - 1) << 6,
                GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
        }
        else
        {
            glBufferData(GL_TEXTURE_BUFFER, number << 6, NULL, GL_DYNAMIC_DRAW);
            glBufferSubData(GL_TEXTURE_BUFFER, 0, 64, m.pointer());
        }
        glBindTexture(GL_TEXTURE_BUFFER, g_skinning_tex);
        glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, g_skinning_buf);
        glBindTexture(GL_TEXTURE_BUFFER, 0);
#endif
    }

}   // resizeSkinning

// ----------------------------------------------------------------------------
void initSkinning()
{
    static_assert(sizeof(std::array<float, 16>) == 64, "No padding");

    int max_size = 0;

    if (!CVS->isARBTextureBufferObjectUsable())
    {
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_size);
    
        if (stk_config->m_max_skinning_bones > (unsigned)max_size)
        {
            Log::warn("SharedGPUObjects", "Too many bones for skinning, max: %d",
                      max_size);
            stk_config->m_max_skinning_bones = max_size;
        }
        Log::info("SharedGPUObjects", "Hardware Skinning enabled, method: %u"
                  " (max bones) * 16 RGBA float texture",
                  stk_config->m_max_skinning_bones);
    }
    else
    {
#ifndef USE_GLES2
        glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &max_size);
        if (stk_config->m_max_skinning_bones << 6 > (unsigned)max_size)
        {
            Log::warn("SharedGPUObjects", "Too many bones for skinning, max: %d",
                      max_size >> 6);
            stk_config->m_max_skinning_bones = max_size >> 6;
        }
        Log::info("SharedGPUObjects", "Hardware Skinning enabled, method: TBO, "
                  "max bones: %u", stk_config->m_max_skinning_bones);
#endif
    }


    // Reserve 1 identity matrix for non-weighted vertices
    // All buffer / skinning texture start with 2 bones for power of 2 increase
    const irr::core::matrix4 m;
    glGenTextures(1, &g_skinning_tex);
#ifndef USE_GLES2
    if (CVS->isARBTextureBufferObjectUsable())
    {
        glGenBuffers(1, &g_skinning_buf);
    }
#endif
    resizeSkinning(stk_config->m_max_skinning_bones);

    sp_prefilled_tex[0] = g_skinning_tex;
}   // initSkinning

// ----------------------------------------------------------------------------
void loadShaders()
{
    SPShaderManager::get()->loadSPShaders(file_manager->getShadersDir());

    // Displace shader is not specifiable in XML due to complex callback
    std::shared_ptr<SPShader> sps;
    if (CVS->isDeferredEnabled())
    {
        // This displace shader will be drawn the last in transparent pass
        sps = std::make_shared<SPShader>("displace", displaceShaderInit,
            true/*transparent_shader*/, 999/*drawing_priority*/,
            true/*use_alpha_channel*/);
        SPShaderManager::get()->addSPShader(sps->getName(), sps);
    }
    else
    {
        // Fallback shader
        SPShaderManager::get()->addSPShader("displace",
            SPShaderManager::get()->getSPShader("alphablend"));
    }

    // ========================================================================
    // Glow shader
    // ========================================================================
    if (CVS->isDeferredEnabled())
    {
        sps = std::make_shared<SPShader>
            ("sp_glow_shader", [](SPShader* shader)
            {
                shader->addShaderFile("sp_pass.vert", GL_VERTEX_SHADER,
                    RP_1ST);
                shader->addShaderFile("colorize.frag", GL_FRAGMENT_SHADER,
                    RP_1ST);
                shader->linkShaderFiles(RP_1ST);
                shader->use(RP_1ST);
                shader->addBasicUniforms(RP_1ST);
                shader->addAllUniforms(RP_1ST);
            });
        SPShaderManager::get()->addSPShader(sps->getName(), sps);
        g_glow_shader = sps.get();

        // ====================================================================
        // Normal visualizer
        // ====================================================================
#ifndef USE_GLES2
        if (CVS->isARBGeometryShadersUsable())
        {
            sps = std::make_shared<SPShader>
                ("sp_normal_visualizer", [](SPShader* shader)
                {
                    shader->addShaderFile("sp_normal_visualizer.vert",
                        GL_VERTEX_SHADER, RP_1ST);
                    shader->addShaderFile("sp_normal_visualizer.geom",
                        GL_GEOMETRY_SHADER, RP_1ST);
                    shader->addShaderFile("sp_normal_visualizer.frag",
                        GL_FRAGMENT_SHADER, RP_1ST);
                    shader->linkShaderFiles(RP_1ST);
                    shader->use(RP_1ST);
                    shader->addBasicUniforms(RP_1ST);
                    shader->addAllUniforms(RP_1ST);
                    shader->addAllTextures(RP_1ST);
                });
            SPShaderManager::get()->addSPShader(sps->getName(), sps);
            g_normal_visualizer = sps.get();
        }
#endif
    }

    SPShaderManager::get()->setOfficialShaders();

}   // loadShaders

// ----------------------------------------------------------------------------
void resetEmptyFogColor()
{
    if (GUIEngine::isNoGraphics())
    {
        return;
    }
    glBindBuffer(GL_UNIFORM_BUFFER, sp_fog_ubo);
    std::vector<float> fog_empty;
    fog_empty.resize(8, 0.0f);
    glBufferData(GL_UNIFORM_BUFFER, 8 * sizeof(float), fog_empty.data(),
        GL_DYNAMIC_DRAW);
}   // resetEmptyFogColor

// ----------------------------------------------------------------------------
void init()
{
    if (GUIEngine::isNoGraphics())
    {
        return;
    }

    initSkinning();
    for (unsigned i = 0; i < MAX_PLAYER_COUNT; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            glGenBuffers(1, &sp_mat_ubo[i][j]);
            glBindBuffer(GL_UNIFORM_BUFFER, sp_mat_ubo[i][j]);
            glBufferData(GL_UNIFORM_BUFFER, (16 * 9 + 2) * sizeof(float), NULL,
                GL_DYNAMIC_DRAW);
        }
    }

    glGenBuffers(1, &sp_fog_ubo);
    resetEmptyFogColor();
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, sp_fog_ubo);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    for (unsigned st = ST_NEAREST; st < ST_COUNT; st++)
    {
        if ((SamplerType)st == ST_TEXTURE_BUFFER)
        {
            g_samplers[ST_TEXTURE_BUFFER] = 0;
            continue;
        }
        switch ((SamplerType)st)
        {
            case ST_NEAREST:
            {
                unsigned id;
                glGenSamplers(1, &id);
                glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glSamplerParameteri(id, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glSamplerParameteri(id, GL_TEXTURE_WRAP_T, GL_REPEAT);
                if (CVS->isEXTTextureFilterAnisotropicUsable())
                    glSamplerParameterf(id, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.);
                g_samplers[ST_NEAREST] = id;
                break;
            }
            case ST_NEAREST_CLAMPED:
            {
                unsigned id;
                glGenSamplers(1, &id);
                glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glSamplerParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glSamplerParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                if (CVS->isEXTTextureFilterAnisotropicUsable())
                    glSamplerParameterf(id, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.);
                g_samplers[ST_NEAREST_CLAMPED] = id;
                break;
            }
            case ST_TRILINEAR:
            {
                unsigned id;
                glGenSamplers(1, &id);
                glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glSamplerParameteri(id,
                    GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glSamplerParameteri(id, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glSamplerParameteri(id, GL_TEXTURE_WRAP_T, GL_REPEAT);
                if (CVS->isEXTTextureFilterAnisotropicUsable())
                {
                    int aniso = UserConfigParams::m_anisotropic;
                    if (aniso == 0) aniso = 1;
                    glSamplerParameterf(id, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                        (float)aniso);
                }
                g_samplers[ST_TRILINEAR] = id;
                break;
            }
            case ST_TRILINEAR_CLAMPED:
            {
                unsigned id;
                glGenSamplers(1, &id);
                glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glSamplerParameteri(id,
                    GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glSamplerParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glSamplerParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                if (CVS->isEXTTextureFilterAnisotropicUsable())
                {
                    int aniso = UserConfigParams::m_anisotropic;
                    if (aniso == 0) aniso = 1;
                    glSamplerParameterf(id, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                        (float)aniso);
                }
                g_samplers[ST_TRILINEAR_CLAMPED] = id;
                break;
            }
            case ST_BILINEAR:
            {
                unsigned id;
                glGenSamplers(1, &id);
                glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glSamplerParameteri(id, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glSamplerParameteri(id, GL_TEXTURE_WRAP_T, GL_REPEAT);
                if (CVS->isEXTTextureFilterAnisotropicUsable())
                    glSamplerParameterf(id, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.);
                g_samplers[ST_BILINEAR] = id;
                break;
            }
            case ST_BILINEAR_CLAMPED:
            {
                unsigned id;
                glGenSamplers(1, &id);
                glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glSamplerParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glSamplerParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                if (CVS->isEXTTextureFilterAnisotropicUsable())
                    glSamplerParameterf(id, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.);
                g_samplers[ST_BILINEAR_CLAMPED] = id;
                break;
            }
            case ST_SEMI_TRILINEAR:
            {
                unsigned id;
                glGenSamplers(1, &id);
                glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_NEAREST);
                glSamplerParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glSamplerParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                if (CVS->isEXTTextureFilterAnisotropicUsable())
                    glSamplerParameterf(id, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.);
                g_samplers[ST_SEMI_TRILINEAR] = id;
                break;
            }
            case ST_SHADOW:
            {
                unsigned id;
                glGenSamplers(1, &id);
                glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
                glSamplerParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glSamplerParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glSamplerParameterf(id, GL_TEXTURE_COMPARE_MODE,
                    GL_COMPARE_REF_TO_TEXTURE);
                glSamplerParameterf(id, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
                g_samplers[ST_SHADOW] = id;
                break;
            }
            default:
                break;
        }
    }
}   // init

// ----------------------------------------------------------------------------
void destroy()
{
    g_dy_dc.clear();
    SPTextureManager::get()->stopThreads();
    SPShaderManager::destroy();
    g_glow_shader = NULL;
    g_normal_visualizer = NULL;
    SPTextureManager::destroy();

#ifndef USE_GLES2
    if (CVS->isARBTextureBufferObjectUsable() && 
        CVS->isARBBufferStorageUsable())
    {
        glBindBuffer(GL_TEXTURE_BUFFER, g_skinning_buf);
        glUnmapBuffer(GL_TEXTURE_BUFFER);
        glBindBuffer(GL_TEXTURE_BUFFER, 0);
    }
    glDeleteBuffers(1, &g_skinning_buf);
#endif
    glDeleteTextures(1, &g_skinning_tex);

    for (unsigned i = 0; i < MAX_PLAYER_COUNT; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            glDeleteBuffers(1, &sp_mat_ubo[i][j]);
        }
    }
    glDeleteBuffers(1, &sp_fog_ubo);
    glDeleteSamplers((unsigned)g_samplers.size() - 1, g_samplers.data());

}   // destroy

// ----------------------------------------------------------------------------
GLuint getSampler(SamplerType st)
{
    assert(st < ST_COUNT);
    return g_samplers[st];
}   // getSampler

// ----------------------------------------------------------------------------
SPShader* getGlowShader()
{
    return g_glow_shader;
}   // getGlowShader

// ----------------------------------------------------------------------------
SPShader* getNormalVisualizer()
{
    return g_normal_visualizer;
}   // getNormalVisualizer


// ----------------------------------------------------------------------------
inline core::vector3df getCorner(const core::aabbox3df& bbox, unsigned n)
{
    switch (n)
    {
    case 0:
        return irr::core::vector3df(bbox.MinEdge.X, bbox.MinEdge.Y,
        bbox.MinEdge.Z);
    case 1:
        return irr::core::vector3df(bbox.MaxEdge.X, bbox.MinEdge.Y,
        bbox.MinEdge.Z);
    case 2:
        return irr::core::vector3df(bbox.MinEdge.X, bbox.MaxEdge.Y,
        bbox.MinEdge.Z);
    case 3:
        return irr::core::vector3df(bbox.MaxEdge.X, bbox.MaxEdge.Y,
        bbox.MinEdge.Z);
    case 4:
        return irr::core::vector3df(bbox.MinEdge.X, bbox.MinEdge.Y,
        bbox.MaxEdge.Z);
    case 5:
        return irr::core::vector3df(bbox.MaxEdge.X, bbox.MinEdge.Y,
        bbox.MaxEdge.Z);
    case 6:
        return irr::core::vector3df(bbox.MinEdge.X, bbox.MaxEdge.Y,
        bbox.MaxEdge.Z);
    case 7:
        return irr::core::vector3df(bbox.MaxEdge.X, bbox.MaxEdge.Y,
        bbox.MaxEdge.Z);
    default:
        assert(false);
        return irr::core::vector3df(0);
    }
}   // getCorner

// ----------------------------------------------------------------------------
void addEdgeForViz(const core::vector3df& p0, const core::vector3df& p1)
{
    g_bounding_boxes.push_back(p0.X);
    g_bounding_boxes.push_back(p0.Y);
    g_bounding_boxes.push_back(p0.Z);
    g_bounding_boxes.push_back(p1.X);
    g_bounding_boxes.push_back(p1.Y);
    g_bounding_boxes.push_back(p1.Z);
}   // addEdgeForViz

// ----------------------------------------------------------------------------
void prepareDrawCalls()
{
    if (!sp_culling)
    {
        return;
    }
    g_bounding_boxes.clear();
    sp_wind_dir = core::vector3df(1.0f, 0.0f, 0.0f) *
        (irr_driver->getDevice()->getTimer()->getTime() / 1000.0f) * 1.5f;
    sp_solid_poly_count = sp_shadow_poly_count = 0;
    // 1st one is identity
    g_skinning_offset = 1;
    g_skinning_mesh.clear();
    mathPlaneFrustumf(g_frustums[0], irr_driver->getProjViewMatrix());
    g_handle_shadow = Track::getCurrentTrack() &&
        Track::getCurrentTrack()->hasShadows() && CVS->isDeferredEnabled() &&
        CVS->isShadowEnabled();

    if (g_handle_shadow)
    {
        mathPlaneFrustumf(g_frustums[1],
            g_stk_sbr->getShadowMatrices()->getSunOrthoMatrices()[0]);
        mathPlaneFrustumf(g_frustums[2],
            g_stk_sbr->getShadowMatrices()->getSunOrthoMatrices()[1]);
        mathPlaneFrustumf(g_frustums[3],
            g_stk_sbr->getShadowMatrices()->getSunOrthoMatrices()[2]);
        mathPlaneFrustumf(g_frustums[4],
            g_stk_sbr->getShadowMatrices()->getSunOrthoMatrices()[3]);
    }

    for (auto& p : g_draw_calls)
    {
        p.clear();
    }
    for (auto& p : g_final_draw_calls)
    {
        p.clear();
    }
    g_glow_meshes.clear();
    g_instances.clear();
}

// ----------------------------------------------------------------------------
void addObject(SPMeshNode* node)
{
    if (!sp_culling)
    {
        return;
    }

    if (node->getSPM() == NULL)
    {
        return;
    }

    const core::matrix4& model_matrix = node->getAbsoluteTransformation();
    bool added_for_skinning = false;
    for (unsigned m = 0; m < node->getSPM()->getMeshBufferCount(); m++)
    {
        SPMeshBuffer* mb = node->getSPM()->getSPMeshBuffer(m);
        SPShader* shader = node->getShader(m);
        if (shader == NULL)
        {
            continue;
        }
        core::aabbox3df bb = mb->getBoundingBox();
        model_matrix.transformBoxEx(bb);
        std::vector<bool> discard;
        const bool handle_shadow = node->isInShadowPass() &&
            g_handle_shadow && shader->hasShader(RP_SHADOW);
        discard.resize((handle_shadow ? 5 : 1), false);

        for (int dc_type = 0; dc_type < (handle_shadow ? 5 : 1); dc_type++)
        {
            for (int i = 0; i < 24; i += 4)
            {
                bool outside = true;
                for (int j = 0; j < 8; j++)
                {
                    const float dist =
                        getCorner(bb, j).X * g_frustums[dc_type][i] +
                        getCorner(bb, j).Y * g_frustums[dc_type][i + 1] +
                        getCorner(bb, j).Z * g_frustums[dc_type][i + 2] +
                        g_frustums[dc_type][i + 3];
                    outside = outside && dist < 0.0f;
                    if (!outside)
                    {
                        break;
                    }
                }
                if (outside)
                {
                    discard[dc_type] = true;
                    break;
                }
            }
        }
        if (handle_shadow ?
            (discard[0] && discard[1] && discard[2] && discard[3] &&
            discard[4]) : discard[0])
        {
            continue;
        }

        if (irr_driver->getBoundingBoxesViz())
        {
            addEdgeForViz(getCorner(bb, 0), getCorner(bb, 1));
            addEdgeForViz(getCorner(bb, 1), getCorner(bb, 5));
            addEdgeForViz(getCorner(bb, 5), getCorner(bb, 4));
            addEdgeForViz(getCorner(bb, 4), getCorner(bb, 0));
            addEdgeForViz(getCorner(bb, 2), getCorner(bb, 3));
            addEdgeForViz(getCorner(bb, 3), getCorner(bb, 7));
            addEdgeForViz(getCorner(bb, 7), getCorner(bb, 6));
            addEdgeForViz(getCorner(bb, 6), getCorner(bb, 2));
            addEdgeForViz(getCorner(bb, 0), getCorner(bb, 2));
            addEdgeForViz(getCorner(bb, 1), getCorner(bb, 3));
            addEdgeForViz(getCorner(bb, 5), getCorner(bb, 7));
            addEdgeForViz(getCorner(bb, 4), getCorner(bb, 6));
        }

        mb->uploadGLMesh();
        // For first frame only need the vbo to be initialized
        if (!added_for_skinning && node->getAnimationState())
        {
            added_for_skinning = true;
            int skinning_offset = g_skinning_offset + node->getTotalJoints();
            if (skinning_offset > int(stk_config->m_max_skinning_bones))
            {
                Log::error("SPBase", "No enough space to render skinned"
                    " mesh %s! Max joints can hold: %d",
                    node->getName(), stk_config->m_max_skinning_bones);
                return;
            }
            node->setSkinningOffset(g_skinning_offset);
            g_skinning_mesh.push_back(node);
            g_skinning_offset = skinning_offset;
        }

        float hue = node->getRenderInfo(m) ?
            node->getRenderInfo(m)->getHue() : 0.0f;
        SPInstancedData id = SPInstancedData
            (node->getAbsoluteTransformation(), node->getTextureMatrix(m)[0],
            node->getTextureMatrix(m)[1], hue,
            (short)node->getSkinningOffset());

        for (int dc_type = 0; dc_type < (handle_shadow ? 5 : 1); dc_type++)
        {
            if (discard[dc_type])
            {
                continue;
            }
            if (dc_type == 0)
            {
                sp_solid_poly_count += mb->getIndexCount() / 3;
            }
            else
            {
                sp_shadow_poly_count += mb->getIndexCount() / 3;
            }
            if (shader->isTransparent())
            {
                // Transparent shader should always uses mesh samplers
                // All transparent draw calls go DCT_TRANSPARENT
                if (dc_type == 0)
                {
                    auto& ret = g_draw_calls[DCT_TRANSPARENT][shader];
                    for (auto& p : mb->getTextureCompare())
                    {
                        ret[p.first].insert(mb);
                    }
                    mb->addInstanceData(id, DCT_TRANSPARENT);
                }
                else
                {
                    continue;
                }
            }
            else
            {
                // Check if shader for render pass uses mesh samplers
                const RenderPass check_pass =
                    dc_type == DCT_NORMAL ? RP_1ST : RP_SHADOW;
                const bool sampler_less = shader->samplerLess(check_pass);
                auto& ret = g_draw_calls[dc_type][shader];
                if (sampler_less)
                {
                    ret[""].insert(mb);
                }
                else
                {
                    for (auto& p : mb->getTextureCompare())
                    {
                        ret[p.first].insert(mb);
                    }
                }
                mb->addInstanceData(id, (DrawCallType)dc_type);
                if (UserConfigParams::m_glow && node->hasGlowColor() &&
                    CVS->isDeferredEnabled() && dc_type == DCT_NORMAL)
                {
                    video::SColorf gc = node->getGlowColor();
                    unsigned key = gc.toSColor().color;
                    auto ret = g_glow_meshes.find(key);
                    if (ret == g_glow_meshes.end())
                    {
                        g_glow_meshes[key] = std::make_pair(
                            core::vector3df(gc.r, gc.g, gc.b),
                            std::unordered_set<SPMeshBuffer*>());
                    }
                    g_glow_meshes.at(key).second.insert(mb);
                }
            }
            g_instances.insert(mb);
        }
    }
}

// ----------------------------------------------------------------------------
void handleDynamicDrawCall()
{
    for (unsigned dc_num = 0; dc_num < g_dy_dc.size(); dc_num++)
    {
        SPDynamicDrawCall* dydc = g_dy_dc[dc_num].get();
        if (!dydc->isRemoving())
        {
            // They need to be updated independent of culling result
            // otherwise some data will be missed if offset update is used
            g_instances.insert(dydc);
        }
        if (!dydc->isVisible() || dydc->notReadyFromDrawing() ||
            dydc->isRemoving() || !sp_culling)
        {
            continue;
        }

        SPShader* shader = dydc->getShader();
        core::aabbox3df bb = dydc->getBoundingBox();
        dydc->getAbsoluteTransformation().transformBoxEx(bb);
        std::vector<bool> discard;
        const bool handle_shadow =
            g_handle_shadow && shader->hasShader(RP_SHADOW);
        discard.resize((handle_shadow ? 5 : 1), false);
        for (int dc_type = 0; dc_type < (handle_shadow ? 5 : 1); dc_type++)
        {
            for (int i = 0; i < 24; i += 4)
            {
                bool outside = true;
                for (int j = 0; j < 8; j++)
                {
                    const float dist =
                        getCorner(bb, j).X * g_frustums[dc_type][i] +
                        getCorner(bb, j).Y * g_frustums[dc_type][i + 1] +
                        getCorner(bb, j).Z * g_frustums[dc_type][i + 2] +
                        g_frustums[dc_type][i + 3];
                    outside = outside && dist < 0.0f;
                    if (!outside)
                    {
                        break;
                    }
                }
                if (outside)
                {
                    discard[dc_type] = true;
                    break;
                }
            }
        }
        if (handle_shadow ?
            (discard[0] && discard[1] && discard[2] && discard[3] &&
            discard[4]) : discard[0])
        {
            continue;
        }

        if (irr_driver->getBoundingBoxesViz())
        {
            addEdgeForViz(getCorner(bb, 0), getCorner(bb, 1));
            addEdgeForViz(getCorner(bb, 1), getCorner(bb, 5));
            addEdgeForViz(getCorner(bb, 5), getCorner(bb, 4));
            addEdgeForViz(getCorner(bb, 4), getCorner(bb, 0));
            addEdgeForViz(getCorner(bb, 2), getCorner(bb, 3));
            addEdgeForViz(getCorner(bb, 3), getCorner(bb, 7));
            addEdgeForViz(getCorner(bb, 7), getCorner(bb, 6));
            addEdgeForViz(getCorner(bb, 6), getCorner(bb, 2));
            addEdgeForViz(getCorner(bb, 0), getCorner(bb, 2));
            addEdgeForViz(getCorner(bb, 1), getCorner(bb, 3));
            addEdgeForViz(getCorner(bb, 5), getCorner(bb, 7));
            addEdgeForViz(getCorner(bb, 4), getCorner(bb, 6));
        }

        for (int dc_type = 0; dc_type < (handle_shadow ? 5 : 1); dc_type++)
        {
            if (discard[dc_type])
            {
                continue;
            }
            if (dc_type == 0)
            {
                sp_solid_poly_count += dydc->getVertexCount();
            }
            else
            {
                sp_shadow_poly_count += dydc->getVertexCount();
            }
            if (shader->isTransparent())
            {
                // Transparent shader should always uses mesh samplers
                // All transparent draw calls go DCT_TRANSPARENT
                if (dc_type == 0)
                {
                    auto& ret = g_draw_calls[DCT_TRANSPARENT][shader];
                    for (auto& p : dydc->getTextureCompare())
                    {
                        ret[p.first].insert(dydc);
                    }
                }
                else
                {
                    continue;
                }
            }
            else
            {
                // Check if shader for render pass uses mesh samplers
                const RenderPass check_pass =
                    dc_type == DCT_NORMAL ? RP_1ST : RP_SHADOW;
                const bool sampler_less = shader->samplerLess(check_pass);
                auto& ret = g_draw_calls[dc_type][shader];
                if (sampler_less)
                {
                    ret[""].insert(dydc);
                }
                else
                {
                    for (auto& p : dydc->getTextureCompare())
                    {
                        ret[p.first].insert(dydc);
                    }
                }
            }
        }
    }
}

// ----------------------------------------------------------------------------
void updateModelMatrix()
{
    // Make sure all textures (with handles) are loaded
    SPTextureManager::get()->checkForGLCommand(true/*before_scene*/);
    if (!sp_culling)
    {
        return;
    }
    irr_driver->setSkinningJoint(g_skinning_offset - 1);

    for (unsigned i = 0; i < DCT_FOR_VAO; i++)
    {
        DrawCall* dc = &g_draw_calls[(DrawCallType)i];
        // Sort dc based on the drawing priority of shaders
        // The larger the drawing priority int, the last it will be drawn
        using DrawCallPair = std::pair<SPShader*,
            std::unordered_map<std::string,
            std::unordered_set<SPMeshBuffer*> > >;
        std::vector<DrawCallPair> sorted_dc;
        for (auto& p : *dc)
        {
            sorted_dc.push_back(p);
        }
        std::sort(sorted_dc.begin(), sorted_dc.end(),
            [](const DrawCallPair& a, const DrawCallPair& b)->bool
            {
                return a.first->getDrawingPriority() <
                    b.first->getDrawingPriority();
            });
        for (unsigned dc = 0; dc < sorted_dc.size(); dc++)
        {
            auto& p = sorted_dc[dc];
            g_final_draw_calls[i].emplace_back(p.first,
            std::vector<std::pair<std::array<GLuint, 6>,
                std::vector<std::pair<SPMeshBuffer*, int> > > >());

            unsigned texture = 0;
            for (auto& q : p.second)
            {
                if (q.second.empty())
                {
                    continue;
                }
                std::array<GLuint, 6> texture_names =
                    {{ 0, 0, 0, 0, 0, 0 }};
                int material_id =
                    (*(q.second.begin()))->getMaterialID(q.first);

                if (material_id != -1)
                {
                    const std::array<std::shared_ptr<SPTexture>, 6>& textures =
                        (*(q.second.begin()))->getSPTexturesByMaterialID
                        (material_id);
                    texture_names =
                        {{
                            textures[0]->getTextureHandler(),
                            textures[1]->getTextureHandler(),
                            textures[2]->getTextureHandler(),
                            textures[3]->getTextureHandler(),
                            textures[4]->getTextureHandler(),
                            textures[5]->getTextureHandler()
                        }};
                }
                g_final_draw_calls[i][dc].second.emplace_back
                    (texture_names,
                    std::vector<std::pair<SPMeshBuffer*, int> >());
                for (SPMeshBuffer* spmb : q.second)
                {
                    g_final_draw_calls[i][dc].second[texture].second.push_back
                        (std::make_pair(spmb, material_id == -1 ?
                        -1 : spmb->getMaterialID(q.first)));
                }
                texture++;
            }
        }
    }
}

// ----------------------------------------------------------------------------
void uploadSkinningMatrices()
{
    if (g_skinning_mesh.empty())
    {
        return;
    }

    unsigned buffer_offset = 0;
#ifndef USE_GLES2
    if (CVS->isARBTextureBufferObjectUsable() && 
        !CVS->isARBBufferStorageUsable())
    {
        glBindBuffer(GL_TEXTURE_BUFFER, g_skinning_buf);
        g_joint_ptr = (std::array<float, 16>*)
            glMapBufferRange(GL_TEXTURE_BUFFER, 64, (g_skinning_offset - 1) * 64,
            GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT |
            GL_MAP_INVALIDATE_RANGE_BIT);
    }
#endif

    for (unsigned i = 0; i < g_skinning_mesh.size(); i++)
    {
        memcpy(g_joint_ptr + buffer_offset,
            g_skinning_mesh[i]->getSkinningMatrices(),
            g_skinning_mesh[i]->getTotalJoints() * 64);
        buffer_offset += g_skinning_mesh[i]->getTotalJoints();
    }

    if (!CVS->isARBTextureBufferObjectUsable())
    {
        glBindTexture(GL_TEXTURE_2D, g_skinning_tex);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 1, 4, buffer_offset, GL_RGBA,
            GL_FLOAT, g_joint_ptr);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
#ifndef USE_GLES2
    if (CVS->isARBTextureBufferObjectUsable() && 
        !CVS->isARBBufferStorageUsable())
    {
        glUnmapBuffer(GL_TEXTURE_BUFFER);
        glBindBuffer(GL_TEXTURE_BUFFER, 0);
    }
#endif
}

// ----------------------------------------------------------------------------
void uploadAll()
{
    uploadSkinningMatrices();
    glBindBuffer(GL_UNIFORM_BUFFER,
        sp_mat_ubo[sp_cur_player][sp_cur_buf_id[sp_cur_player]]);
    /*void* ptr = glMapBufferRange(GL_UNIFORM_BUFFER, 0,
        (16 * 9 + 2) * sizeof(float), GL_MAP_WRITE_BIT |
        GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    memcpy(ptr, g_stk_sbr->getShadowMatrices()->getMatricesData(),
        (16 * 9 + 2) * sizeof(float));
    glUnmapBuffer(GL_UNIFORM_BUFFER);*/
    glBufferSubData(GL_UNIFORM_BUFFER, 0, (16 * 9 + 2) * sizeof(float),
        g_stk_sbr->getShadowMatrices()->getMatricesData());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    for (SPMeshBuffer* spmb : g_instances)
    {
        spmb->uploadInstanceData();
    }

    g_dy_dc.erase(std::remove_if(g_dy_dc.begin(), g_dy_dc.end(),
        [] (std::shared_ptr<SPDynamicDrawCall> dc)
        {
            return dc->isRemoving();
        }), g_dy_dc.end());
}

// ----------------------------------------------------------------------------
void drawSPDebugView()
{
    if (g_normal_visualizer == NULL)
    {
        return;
    }
    g_normal_visualizer->use();
    g_normal_visualizer->bindPrefilledTextures();
    for (unsigned i = 0; i < g_final_draw_calls[0].size(); i++)
    {
        auto& p = g_final_draw_calls[0][i];
        for (unsigned j = 0; j < p.second.size(); j++)
        {
            for (unsigned k = 0; k < p.second[j].second.size(); k++)
            {
                // Make sure tangents and joints are not drawn undefined
                glVertexAttrib4f(5, 0.0f, 0.0f, 0.0f, 0.0f);
                glVertexAttribI4i(6, 0, 0, 0, 0);
                glVertexAttrib4f(7, 0.0f, 0.0f, 0.0f, 0.0f);
                p.second[j].second[k].first->draw(DCT_NORMAL,
                    -1/*material_id*/);
            }
        }
    }
    for (unsigned i = 0; i < g_final_draw_calls[5].size(); i++)
    {
        auto& p = g_final_draw_calls[5][i];
        for (unsigned j = 0; j < p.second.size(); j++)
        {
            for (unsigned k = 0; k < p.second[j].second.size(); k++)
            {
                // Make sure tangents and joints are not drawn undefined
                glVertexAttrib4f(5, 0.0f, 0.0f, 0.0f, 0.0f);
                glVertexAttribI4i(6, 0, 0, 0, 0);
                glVertexAttrib4f(7, 0.0f, 0.0f, 0.0f, 0.0f);
                p.second[j].second[k].first->draw(DCT_TRANSPARENT,
                    -1/*material_id*/);
            }
        }
    }
    g_normal_visualizer->unuse();
}

// ----------------------------------------------------------------------------
void drawGlow()
{
    if (g_glow_meshes.empty())
    {
        return;
    }
    g_glow_shader->use();
    SPUniformAssigner* glow_color_assigner =
        g_glow_shader->getUniformAssigner("col");
    assert(glow_color_assigner != NULL);
    for (auto& p : g_glow_meshes)
    {
        glow_color_assigner->setValue(p.second.first);
        for (SPMeshBuffer* spmb : p.second.second)
        {
            spmb->draw(DCT_NORMAL, -1/*material_id*/);
        }
    }
    g_glow_shader->unuse();
}

// ----------------------------------------------------------------------------
void draw(RenderPass rp, DrawCallType dct)
{
    std::stringstream profiler_name;
    profiler_name << "SP::Draw " << dct << " with " << rp;
    PROFILER_PUSH_CPU_MARKER(profiler_name.str().c_str(),
        (uint8_t)(float(dct + rp + 2) / float(DCT_FOR_VAO + RP_COUNT) * 255.0f),
        (uint8_t)(float(dct + 1) / (float)DCT_FOR_VAO * 255.0f) ,
        (uint8_t)(float(rp + 1) / (float)RP_COUNT * 255.0f));

    assert(dct < DCT_FOR_VAO);
    for (unsigned i = 0; i < g_final_draw_calls[dct].size(); i++)
    {
        auto& p = g_final_draw_calls[dct][i];
        if (!p.first->hasShader(rp))
        {
            continue;
        }
        p.first->use(rp);
        static std::vector<SPUniformAssigner*> shader_uniforms;
        p.first->setUniformsPerObject(static_cast<SPPerObjectUniform*>
            (p.first), &shader_uniforms, rp);
        p.first->bindPrefilledTextures(rp);
        for (unsigned j = 0; j < p.second.size(); j++)
        {
            p.first->bindTextures(p.second[j].first, rp);
            for (unsigned k = 0; k < p.second[j].second.size(); k++)
            {
                static std::vector<SPUniformAssigner*> draw_call_uniforms;
                p.first->setUniformsPerObject(static_cast<SPPerObjectUniform*>
                    (p.second[j].second[k].first), &draw_call_uniforms, rp);
                p.second[j].second[k].first->draw(dct,
                    p.second[j].second[k].second/*material_id*/);
                for (SPUniformAssigner* ua : draw_call_uniforms)
                {
                    ua->reset();
                }
                draw_call_uniforms.clear();
            }
        }
        for (SPUniformAssigner* ua : shader_uniforms)
        {
            ua->reset();
        }
        shader_uniforms.clear();
        p.first->unuse(rp);
    }
    PROFILER_POP_CPU_MARKER();
}   // draw

// ----------------------------------------------------------------------------
void drawBoundingBoxes()
{
    Shaders::ColoredLine *line = Shaders::ColoredLine::getInstance();
    line->use();
    line->bindVertexArray();
    line->bindBuffer();
    line->setUniforms(irr::video::SColor(255, 255, 0, 0));
    const float *tmp = g_bounding_boxes.data();
    for (unsigned int i = 0; i < g_bounding_boxes.size(); i += 1024 * 6)
    {
        unsigned count = std::min((unsigned)g_bounding_boxes.size() - i,
            (unsigned)1024 * 6);
        glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(float), &tmp[i]);
        glDrawArrays(GL_LINES, 0, count / 3);
    }
}   // drawBoundingBoxes

// ----------------------------------------------------------------------------
void addDynamicDrawCall(std::shared_ptr<SPDynamicDrawCall> dy_dc)
{
    g_dy_dc.push_back(dy_dc);
}   // addDynamicDrawCall

// ----------------------------------------------------------------------------
SPMesh* convertEVTStandard(irr::scene::IMesh* mesh,
                           const irr::video::SColor* color)
{
    SPMesh* spm = new SPMesh();
    Material* material = material_manager->getDefaultSPMaterial("solid");
    for (unsigned i = 0; i < mesh->getMeshBufferCount(); i++)
    {
        std::vector<video::S3DVertexSkinnedMesh> vertices;
        scene::IMeshBuffer* mb = mesh->getMeshBuffer(i);
        if (!mb)
        {
            continue;
        }
        assert(mb->getVertexType() == video::EVT_STANDARD);
        video::S3DVertex* v_ptr = (video::S3DVertex*)mb->getVertices();
        for (unsigned j = 0; j < mb->getVertexCount(); j++)
        {
            video::S3DVertexSkinnedMesh sp;
            sp.m_position = v_ptr[j].Pos;
            sp.m_normal = MiniGLM::compressVector3(v_ptr[j].Normal);
            sp.m_color = color ? *color : v_ptr[j].Color;
            sp.m_all_uvs[0] = MiniGLM::toFloat16(v_ptr[j].TCoords.X);
            sp.m_all_uvs[1] = MiniGLM::toFloat16(v_ptr[j].TCoords.Y);
            vertices.push_back(sp);
        }
        uint16_t* idx_ptr = mb->getIndices();
        std::vector<uint16_t> indices(idx_ptr, idx_ptr + mb->getIndexCount());
        SPMeshBuffer* buffer = new SPMeshBuffer();
        buffer->setSPMVertices(vertices);
        buffer->setIndices(indices);
        buffer->setSTKMaterial(material);
        spm->addSPMeshBuffer(buffer);
    }
    mesh->drop();
    spm->updateBoundingBox();
    return spm;
}   // convertEVTStandard

// ----------------------------------------------------------------------------
void uploadSPM(irr::scene::IMesh* mesh)
{
    if (!CVS->isGLSL())
    {
        return;
    }
    SP::SPMesh* spm = dynamic_cast<SP::SPMesh*>(mesh);
    if (spm)
    {
        for (u32 i = 0; i < spm->getMeshBufferCount(); i++)
        {
            SP::SPMeshBuffer* mb = spm->getSPMeshBuffer(i);
            mb->uploadGLMesh();
        }
    }
}   // uploadSPM

// ----------------------------------------------------------------------------
void setMaxTextureSize()
{
    const unsigned max =
        (UserConfigParams::m_high_definition_textures & 0x01) == 0 ?
        UserConfigParams::m_max_texture_size : 2048;
    sp_max_texture_size.store(max);
}   // setMaxTextureSize

}

#endif
