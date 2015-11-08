 //  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2015 SuperTuxKart-Team
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

#include "graphics/geometry_passes.hpp"
#include "graphics/callbacks.hpp"
#include "graphics/draw_tools.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/materials.hpp"
#include "graphics/post_processing.hpp"
#include "graphics/rtts.hpp"
#include "graphics/shaders.hpp"
#include "graphics/stk_scene_manager.hpp"
#include "modes/world.hpp"
#include "utils/profiler.hpp"
#include "utils/tuple.hpp"
#include <S3DVertex.h>




/**
\page geometry_passes Geometry Rendering Overview

\section adding_material Adding a solid material

You need to consider twice before adding a new material : in the worst case a material requires 8 shaders :
one for each solid pass, one for shadow pass, one for RSM pass, and you need to double that for instanced version.

You need to declare a new enum in MeshMaterial and to write the corresponding dispatch code in getMeshMaterialFromType
and to create 2 new List* structures (one for standard and one for instanced version).

Then you need to write the code in shader_based_renderer.cpp that will add any mesh with the new material to their corresponding
lists : in handleSTKCommon for the standard version and in the body of prepareDrawCalls for instanced version.

\section vertex_layout Available Vertex Layout

There are 3 different layout that comes from Irrlicht loading routines :
EVT_STANDARD, EVT_2TCOORDS, EVT_TANGENT.

Below are the attributes for each vertex layout and their predefined location.

\subsection EVT_STANDARD
layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec4 Color;
layout(location = 3) in vec2 Texcoord;

\subsection EVT_2TCOORDS
layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec4 Color;
layout(location = 3) in vec2 Texcoord;
layout(location = 4) in vec2 SecondTexcoord;

\subsection EVT_TANGENT
layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec4 Color;
layout(location = 3) in vec2 Texcoord;
layout(location = 5) in vec3 Tangent;
layout(location = 6) in vec3 Bitangent;

*/




// ============================================================================
namespace RenderGeometry
{
    struct TexUnit
    {
        GLuint m_id;
        bool m_premul_alpha;

        TexUnit(GLuint id, bool premul_alpha)
        {
            m_id = id;
            m_premul_alpha = premul_alpha;
        }
    };   // struct TexUnit

    // ------------------------------------------------------------------------
    template <typename T>
    std::vector<TexUnit> TexUnits(T curr) // required on older clang versions
    {
        std::vector<TexUnit> v;
        v.push_back(curr);
        return v;
    }   // TexUnits

    // ------------------------------------------------------------------------
    // required on older clang versions
    template <typename T, typename... R>
    std::vector<TexUnit> TexUnits(T curr, R... rest)
    {
        std::vector<TexUnit> v;
        v.push_back(curr);
        VTexUnits(v, rest...);
        return v;
    }   // TexUnits

    // ------------------------------------------------------------------------
    // required on older clang versions
    template <typename T, typename... R>
    void VTexUnits(std::vector<TexUnit>& v, T curr, R... rest)
    {
        v.push_back(curr);
        VTexUnits(v, rest...);
    }   // VTexUnits
    // ------------------------------------------------------------------------
    template <typename T>
    void VTexUnits(std::vector<TexUnit>& v, T curr)
    {
        v.push_back(curr);
    }   // VTexUnits
}   // namespace RenderGeometry

using namespace RenderGeometry;




// ----------------------------------------------------------------------------
template<typename T, int ...List>
void renderMeshes1stPass(const DrawCalls& draw_calls)
{
    auto &meshes = T::List::getInstance()->SolidPass;
    T::FirstPassShader::getInstance()->use();
    if (CVS->isARBBaseInstanceUsable())
        glBindVertexArray(VAOManager::getInstance()->getVAO(T::VertexType));
    for (unsigned i = 0; i < meshes.size(); i++)
    {
        GLMesh &mesh = *(STK::tuple_get<0>(meshes.at(i)));
        if (!CVS->isARBBaseInstanceUsable())
            glBindVertexArray(mesh.vao);
        if (mesh.VAOType != T::VertexType)
        {
#ifdef DEBUG
            Log::error("Materials", "Wrong vertex Type associed to pass 1 (hint texture : %s)", mesh.textures[0]->getName().getPath().c_str());
#endif
            continue;
        }

        if (CVS->isAZDOEnabled())
            HandleExpander<typename T::FirstPassShader>::template expand(mesh.TextureHandles, T::FirstPassTextures);
        else
            TexExpander<typename T::FirstPassShader>::template expandTex(mesh, T::FirstPassTextures);
        CustomUnrollArgs<List...>::template drawMesh<typename T::FirstPassShader>(meshes.at(i));
    }
}   // renderMeshes1stPass

// ----------------------------------------------------------------------------
template<typename T, typename...Args>
void renderInstancedMeshes1stPass(const DrawCalls& draw_calls, Args...args)
{
    std::vector<GLMesh *> &meshes = T::InstancedList::getInstance()->SolidPass;
    T::InstancedFirstPassShader::getInstance()->use();
    T::InstancedFirstPassShader::getInstance()->setUniforms(args...);

    
    glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(T::VertexType, T::Instance));
    for (unsigned i = 0; i < meshes.size(); i++)
    {
        GLMesh *mesh = meshes[i];
#ifdef DEBUG
        if (mesh->VAOType != T::VertexType)
        {
            Log::error("RenderGeometry", "Wrong instanced vertex format (hint : %s)", 
                mesh->textures[0]->getName().getPath().c_str());
            continue;
        }
#endif
        TexExpander<typename T::InstancedFirstPassShader>::template expandTex(*mesh, T::FirstPassTextures);

        draw_calls.drawIndirectSolidCmd(T::MaterialType, i);
    }
}   // renderInstancedMeshes1stPass

// ----------------------------------------------------------------------------
template<typename T, typename...Args>
void multidraw1stPass(const DrawCalls& draw_calls, Args...args)
{
    T::InstancedFirstPassShader::getInstance()->use();
    glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(T::VertexType, T::Instance));
    if (!draw_calls.isSolidCmdEmpty(T::MaterialType))
    {
        T::InstancedFirstPassShader::getInstance()->setUniforms(args...);
        draw_calls.multidrawIndirectSolidCmd(T::MaterialType);
    }
}   // multidraw1stPass


GeometryPasses::GeometryPasses()
{
    m_displace_tex = irr_driver->getTexture(FileManager::TEXTURE, "displace.png");    
}

// ----------------------------------------------------------------------------
void GeometryPasses::renderSolidFirstPass(const DrawCalls& draw_calls)
{
    m_wind_dir = getWindDir(); //TODO: why this function instead of Wind::getWind()?

    if (CVS->supportsIndirectInstancingRendering())
        draw_calls.bindSolidCmd();

    {
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_SOLID_PASS1));
        irr_driver->setPhase(SOLID_NORMAL_AND_DEPTH_PASS);

        draw_calls.renderImmediateDrawList();

        //TODO: is it useful if AZDO enabled?
        renderMeshes1stPass<DefaultMaterial, 2, 1>(draw_calls);
        renderMeshes1stPass<SplattingMat, 2, 1>(draw_calls);
        renderMeshes1stPass<UnlitMat, 3, 2, 1>(draw_calls);
        renderMeshes1stPass<AlphaRef, 3, 2, 1>(draw_calls);
        renderMeshes1stPass<GrassMat, 3, 2, 1>(draw_calls);
        renderMeshes1stPass<NormalMat, 2, 1>(draw_calls);
        renderMeshes1stPass<SphereMap, 2, 1>(draw_calls);
        renderMeshes1stPass<DetailMat, 2, 1>(draw_calls);

        if (CVS->isAZDOEnabled())
        {
            multidraw1stPass<DefaultMaterial>(draw_calls);
            multidraw1stPass<AlphaRef>(draw_calls);
            multidraw1stPass<SphereMap>(draw_calls);
            multidraw1stPass<UnlitMat>(draw_calls);
            multidraw1stPass<GrassMat>(draw_calls, m_wind_dir);

            multidraw1stPass<NormalMat>(draw_calls);
            multidraw1stPass<DetailMat>(draw_calls);
        }
        else if (CVS->supportsIndirectInstancingRendering())
        {
            draw_calls.drawIndirectSolidFirstPass();
        }
    }
}   // renderSolidFirstPass


// ----------------------------------------------------------------------------
template<typename T, int...List>
void renderMeshes2ndPass( const std::vector<uint64_t> &Prefilled_Handle,
    const std::vector<GLuint> &Prefilled_Tex)
{
    auto &meshes = T::List::getInstance()->SolidPass;
    T::SecondPassShader::getInstance()->use();
    if (CVS->isARBBaseInstanceUsable())
        glBindVertexArray(VAOManager::getInstance()->getVAO(T::VertexType));
    for (unsigned i = 0; i < meshes.size(); i++)
    {
        GLMesh &mesh = *(STK::tuple_get<0>(meshes.at(i)));
        if (!CVS->isARBBaseInstanceUsable())
            glBindVertexArray(mesh.vao);

        if (mesh.VAOType != T::VertexType)
        {
#ifdef DEBUG
            Log::error("Materials", "Wrong vertex Type associed to pass 2 "
                                    "(hint texture : %s)", 
                       mesh.textures[0]->getName().getPath().c_str());
#endif
            continue;
        }

        if (CVS->isAZDOEnabled())
            HandleExpander<typename T::SecondPassShader>::template 
                expand(mesh.TextureHandles, T::SecondPassTextures, 
                       Prefilled_Handle[0], Prefilled_Handle[1],
                       Prefilled_Handle[2]);
        else
            TexExpander<typename T::SecondPassShader>::template 
                expandTex(mesh, T::SecondPassTextures, Prefilled_Tex[0], 
                          Prefilled_Tex[1], Prefilled_Tex[2]);
        CustomUnrollArgs<List...>::template drawMesh<typename T::SecondPassShader>(meshes.at(i));
    }
}   // renderMeshes2ndPass

// ----------------------------------------------------------------------------
template<typename T, typename...Args>
void renderInstancedMeshes2ndPass(const DrawCalls& draw_calls, const std::vector<GLuint> &Prefilled_tex, Args...args)
{
    std::vector<GLMesh *> &meshes = T::InstancedList::getInstance()->SolidPass;
    T::InstancedSecondPassShader::getInstance()->use();
    glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(T::VertexType,
                                                                T::Instance));
    for (unsigned i = 0; i < meshes.size(); i++)
    {
        GLMesh *mesh = meshes[i];
        TexExpander<typename T::InstancedSecondPassShader>::template
            expandTex(*mesh, T::SecondPassTextures, Prefilled_tex[0],
                      Prefilled_tex[1], Prefilled_tex[2]);
        T::InstancedSecondPassShader::getInstance()->setUniforms(args...);
        draw_calls.drawIndirectSolidCmd(T::MaterialType, i);
    }
}   // renderInstancedMeshes2ndPass

// ----------------------------------------------------------------------------
template<typename T, typename...Args>
void multidraw2ndPass(const DrawCalls& draw_calls, const std::vector<uint64_t> &Handles, Args... args)
{
    T::InstancedSecondPassShader::getInstance()->use();
    glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(T::VertexType,
                                                                T::Instance));
    uint64_t nulltex[10] = {};
    if (!draw_calls.isSolidCmdEmpty(T::MaterialType))
    {
        HandleExpander<typename T::InstancedSecondPassShader>::template
            expand(nulltex, T::SecondPassTextures, Handles[0], Handles[1],
                   Handles[2]);
        T::InstancedSecondPassShader::getInstance()->setUniforms(args...);
        draw_calls.multidrawIndirectSolidCmd(T::MaterialType);
    }
}   // multidraw2ndPass


// ----------------------------------------------------------------------------
void GeometryPasses::renderSolidSecondPass( const DrawCalls& draw_calls,
                                            unsigned render_target_diffuse,
                                            unsigned render_target_specular,
                                            unsigned render_target_half_red)
            {
    irr_driver->setPhase(SOLID_LIT_PASS);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    uint64_t DiffuseHandle = 0, SpecularHandle = 0, SSAOHandle = 0, DepthHandle = 0;

    if (CVS->isAZDOEnabled())
    {
        DiffuseHandle = glGetTextureSamplerHandleARB(render_target_diffuse,
                          Shaders::ObjectPass2Shader::getInstance()->m_sampler_ids[0]);
        if (!glIsTextureHandleResidentARB(DiffuseHandle))
            glMakeTextureHandleResidentARB(DiffuseHandle);

        SpecularHandle = glGetTextureSamplerHandleARB(render_target_specular,
                           Shaders::ObjectPass2Shader::getInstance()->m_sampler_ids[1]);
        if (!glIsTextureHandleResidentARB(SpecularHandle))
            glMakeTextureHandleResidentARB(SpecularHandle);

        SSAOHandle = glGetTextureSamplerHandleARB(render_target_half_red,
                        Shaders::ObjectPass2Shader::getInstance()->m_sampler_ids[2]);
        if (!glIsTextureHandleResidentARB(SSAOHandle))
            glMakeTextureHandleResidentARB(SSAOHandle);

        DepthHandle = glGetTextureSamplerHandleARB(irr_driver->getDepthStencilTexture(),
                     Shaders::ObjectPass2Shader::getInstance()->m_sampler_ids[3]);
        if (!glIsTextureHandleResidentARB(DepthHandle))
            glMakeTextureHandleResidentARB(DepthHandle);
    }

    if (CVS->supportsIndirectInstancingRendering())
        draw_calls.bindSolidCmd();
        
    {
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_SOLID_PASS2));

        irr_driver->setPhase(SOLID_LIT_PASS);

        draw_calls.renderImmediateDrawList();

        std::vector<GLuint> DiffSpecSSAOTex = 
            createVector<GLuint>(render_target_diffuse, 
                                 render_target_specular,
                                 render_target_half_red);

        //TODO: is it useful when AZDO enabled?
        renderMeshes2ndPass<DefaultMaterial, 3, 1>(createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle), DiffSpecSSAOTex);
        renderMeshes2ndPass<AlphaRef, 3, 1 >(createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle), DiffSpecSSAOTex);
        renderMeshes2ndPass<UnlitMat, 3, 1>(createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle), DiffSpecSSAOTex);
        renderMeshes2ndPass<SplattingMat, 1>(createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle), DiffSpecSSAOTex);
        renderMeshes2ndPass<SphereMap, 2, 1>(createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle), DiffSpecSSAOTex);
        renderMeshes2ndPass<DetailMat, 1>(createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle), DiffSpecSSAOTex);
        renderMeshes2ndPass<GrassMat, 3, 1>(createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle), DiffSpecSSAOTex);
        renderMeshes2ndPass<NormalMat, 3, 1>(createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle), DiffSpecSSAOTex);

        if (CVS->isAZDOEnabled())
        {
            multidraw2ndPass<DefaultMaterial>(draw_calls, createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle, 0, 0));
            multidraw2ndPass<AlphaRef>(draw_calls, createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle, 0, 0));
            multidraw2ndPass<SphereMap>(draw_calls, createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle, 0));
            multidraw2ndPass<UnlitMat>(draw_calls, createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle, 0));
            multidraw2ndPass<NormalMat>(draw_calls, createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle, 0, 0));
            multidraw2ndPass<DetailMat>(draw_calls, createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle, 0, 0, 0));

            // template does not work with template due to extra depth texture
            {
                GrassMat::InstancedSecondPassShader::getInstance()->use();
                glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(GrassMat::VertexType,
                                                                           GrassMat::Instance));
                uint64_t nulltex[10] = {};
                if (!draw_calls.isSolidCmdEmpty(GrassMat::MaterialType))
                {
                    HandleExpander<GrassMat::InstancedSecondPassShader>
                         ::expand(nulltex, GrassMat::SecondPassTextures, DiffuseHandle,
                                  SpecularHandle, SSAOHandle, DepthHandle);
                    GrassMat::InstancedSecondPassShader::getInstance()->setUniforms(m_wind_dir,
                                                             irr_driver->getSunDirection());
                    draw_calls.multidrawIndirectSolidCmd(GrassMat::MaterialType);
                }
            }
        }
        else if (CVS->supportsIndirectInstancingRendering())
        {
            std::vector<GLuint> prefilled_tex =
                createVector<GLuint>(render_target_diffuse, 
                                     render_target_specular,
                                     render_target_half_red,
                                     irr_driver->getDepthStencilTexture());
            draw_calls.drawIndirectSolidSecondPass(prefilled_tex);
        }
    }
}   // renderSolidSecondPass

// ----------------------------------------------------------------------------
template<typename T>
static void renderInstancedMeshNormals(const DrawCalls& draw_calls)
{
    std::vector<GLMesh *> &meshes = T::InstancedList::getInstance()->SolidPass;
    NormalVisualizer::getInstance()->use();
    glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(T::VertexType, T::Instance));
    for (unsigned i = 0; i < meshes.size(); i++)
    {
        NormalVisualizer::getInstance()->setUniforms(video::SColor(255, 0, 255, 0));
        draw_calls.drawIndirectSolidCmd(T::MaterialType, i);
    }
}   // renderInstancedMeshNormals

// ----------------------------------------------------------------------------
template<typename T>
static void renderMultiMeshNormals(const DrawCalls& draw_calls)
{
    NormalVisualizer::getInstance()->use();
    glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(T::VertexType, T::Instance));
    if (!draw_calls.isSolidCmdEmpty(T::MaterialType))
    {
        NormalVisualizer::getInstance()->setUniforms(video::SColor(255, 0, 255, 0));
        draw_calls.multidrawIndirectSolidCmd(T::MaterialType);
    }
}   // renderMultiMeshNormals

// ----------------------------------------------------------------------------
void GeometryPasses::renderNormalsVisualisation(const DrawCalls& draw_calls)
{
    if (CVS->isAZDOEnabled()) {
        renderMultiMeshNormals<DefaultMaterial>(draw_calls);
        renderMultiMeshNormals<AlphaRef>(draw_calls);
        renderMultiMeshNormals<UnlitMat>(draw_calls);
        renderMultiMeshNormals<SphereMap>(draw_calls);
        renderMultiMeshNormals<DetailMat>(draw_calls);
        renderMultiMeshNormals<NormalMat>(draw_calls);
    }
    else if (CVS->supportsIndirectInstancingRendering())
    {
        renderInstancedMeshNormals<DefaultMaterial>(draw_calls);
        renderInstancedMeshNormals<AlphaRef>(draw_calls);
        renderInstancedMeshNormals<UnlitMat>(draw_calls);
        renderInstancedMeshNormals<SphereMap>(draw_calls);
        renderInstancedMeshNormals<DetailMat>(draw_calls);
        renderInstancedMeshNormals<NormalMat>(draw_calls);
    }
}   // renderNormalsVisualisation

// ----------------------------------------------------------------------------
template<typename Shader, enum video::E_VERTEX_TYPE VertexType, int...List, 
         typename... TupleType>
void renderTransparenPass(const std::vector<RenderGeometry::TexUnit> &TexUnits, 
                          std::vector<STK::Tuple<TupleType...> > *meshes)
{
    Shader::getInstance()->use();
    if (CVS->isARBBaseInstanceUsable())
        glBindVertexArray(VAOManager::getInstance()->getVAO(VertexType));
    for (unsigned i = 0; i < meshes->size(); i++)
    {
        GLMesh &mesh = *(STK::tuple_get<0>(meshes->at(i)));
        if (!CVS->isARBBaseInstanceUsable())
            glBindVertexArray(mesh.vao);
        if (mesh.VAOType != VertexType)
        {
#ifdef DEBUG
            Log::error("Materials", "Wrong vertex Type associed to pass 2 "
                                    "(hint texture : %s)",
                       mesh.textures[0]->getName().getPath().c_str());
#endif
            continue;
        }

        if (CVS->isAZDOEnabled())
            Shader::getInstance()->setTextureHandles(mesh.TextureHandles[0]);
        else
            Shader::getInstance()->setTextureUnits(getTextureGLuint(mesh.textures[0]));
        CustomUnrollArgs<List...>::template drawMesh<Shader>(meshes->at(i));
    }
}   // renderTransparenPass

// ----------------------------------------------------------------------------
void GeometryPasses::renderTransparent(const DrawCalls& draw_calls, unsigned render_target)
{

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glDisable(GL_CULL_FACE);

    irr_driver->setPhase(TRANSPARENT_PASS);

    draw_calls.renderImmediateDrawList();

    if (CVS->isARBBaseInstanceUsable())
        glBindVertexArray(VAOManager::getInstance()->getVAO(video::EVT_STANDARD));

    if (World::getWorld() && World::getWorld()->isFogEnabled())
    {
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        renderTransparenPass<Shaders::TransparentFogShader, video::EVT_STANDARD,
                             8, 7, 6, 5, 4, 3, 2, 1>(
                             TexUnits(RenderGeometry::TexUnit(0, true)),
                              ListBlendTransparentFog::getInstance());
        glBlendFunc(GL_ONE, GL_ONE);
        renderTransparenPass<Shaders::TransparentFogShader, 
                             video::EVT_STANDARD, 8, 7, 6, 5, 4, 3, 2, 1>(
                             TexUnits(RenderGeometry::TexUnit(0, true)), 
                                       ListAdditiveTransparentFog::getInstance());
    }
    else
    {
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        renderTransparenPass<Shaders::TransparentShader,
                             video::EVT_STANDARD, 2, 1>(
                                  TexUnits(RenderGeometry::TexUnit(0, true)),
                                           ListBlendTransparent::getInstance());
        glBlendFunc(GL_ONE, GL_ONE);
        renderTransparenPass<Shaders::TransparentShader, video::EVT_STANDARD, 2, 1>(
                             TexUnits(RenderGeometry::TexUnit(0, true)),
                                      ListAdditiveTransparent::getInstance());
    }

    draw_calls.renderBillboardList();

    if (!CVS->isDefferedEnabled())
        return;

    // Render displacement nodes
    irr_driver->getFBO(FBO_TMP1_WITH_DS).bind();
    glClear(GL_COLOR_BUFFER_BIT);
    irr_driver->getFBO(FBO_DISPLACE).bind();
    glClear(GL_COLOR_BUFFER_BIT);

    DisplaceProvider * const cb =
        (DisplaceProvider *)Shaders::getCallback(ES_DISPLACE);
    cb->update();

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_BLEND);
    glClear(GL_STENCIL_BUFFER_BIT);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    if (CVS->isARBBaseInstanceUsable())
        glBindVertexArray(VAOManager::getInstance()->getVAO(video::EVT_2TCOORDS));
    // Generate displace mask
    // Use RTT_TMP4 as displace mask
    irr_driver->getFBO(FBO_TMP1_WITH_DS).bind();
    for (unsigned i = 0; i < ListDisplacement::getInstance()->size(); i++)
    {
        const GLMesh &mesh =
            *(STK::tuple_get<0>(ListDisplacement::getInstance()->at(i)));
        if (!CVS->isARBBaseInstanceUsable())
            glBindVertexArray(mesh.vao);
        const core::matrix4 &AbsoluteTransformation
            = STK::tuple_get<1>(ListDisplacement::getInstance()->at(i));
        if (mesh.VAOType != video::EVT_2TCOORDS)
        {
#ifdef DEBUG
            Log::error("Materials", "Displacement has wrong vertex type");
#endif
            continue;
        }

        GLenum ptype = mesh.PrimitiveType;
        GLenum itype = mesh.IndexType;
        size_t count = mesh.IndexCount;

        DisplaceMaskShader::getInstance()->use();
        DisplaceMaskShader::getInstance()->setUniforms(AbsoluteTransformation);
        glDrawElementsBaseVertex(ptype, (int)count, itype,
                                 (GLvoid *)mesh.vaoOffset, (int)mesh.vaoBaseVertex);
    }

    irr_driver->getFBO(FBO_DISPLACE).bind();
    for (unsigned i = 0; i < ListDisplacement::getInstance()->size(); i++)
    {
        const GLMesh &mesh = 
            *(STK::tuple_get<0>(ListDisplacement::getInstance()->at(i)));
        if (!CVS->isARBBaseInstanceUsable())
            glBindVertexArray(mesh.vao);
        const core::matrix4 &AbsoluteTransformation =
            STK::tuple_get<1>(ListDisplacement::getInstance()->at(i));
        if (mesh.VAOType != video::EVT_2TCOORDS)
            continue;

        GLenum ptype = mesh.PrimitiveType;
        GLenum itype = mesh.IndexType;
        size_t count = mesh.IndexCount;
        // Render the effect
        DisplaceShader::getInstance()->setTextureUnits(
            getTextureGLuint(m_displace_tex),
            irr_driver->getRenderTargetTexture(RTT_COLOR),
            irr_driver->getRenderTargetTexture(RTT_TMP1),
            getTextureGLuint(mesh.textures[0]));
        DisplaceShader::getInstance()->use();
        DisplaceShader::getInstance()->setUniforms(AbsoluteTransformation,
            core::vector2df(cb->getDirX(), cb->getDirY()),
            core::vector2df(cb->getDir2X(), cb->getDir2Y()));

        glDrawElementsBaseVertex(ptype, (int)count, itype, (GLvoid *)mesh.vaoOffset,
                                 (int)mesh.vaoBaseVertex);
    }

    irr_driver->getFBO(FBO_COLORS).bind();
    glStencilFunc(GL_EQUAL, 1, 0xFF);
    irr_driver->getPostProcessing()->renderPassThrough(render_target,
                                                       irr_driver->getFBO(FBO_COLORS).getWidth(), 
                                                       irr_driver->getFBO(FBO_COLORS).getHeight());
    glDisable(GL_STENCIL_TEST);

}   // renderTransparent

// ----------------------------------------------------------------------------
template<typename T, int...List>
void renderShadow(unsigned cascade)
{
    auto &t = T::List::getInstance()->Shadows[cascade];
    T::ShadowPassShader::getInstance()->use();
    if (CVS->isARBBaseInstanceUsable())
        glBindVertexArray(VAOManager::getInstance()->getVAO(T::VertexType));
    for (unsigned i = 0; i < t.size(); i++)
    {
        GLMesh *mesh = STK::tuple_get<0>(t.at(i));
        if (!CVS->isARBBaseInstanceUsable())
            glBindVertexArray(mesh->vao);
        if (CVS->isAZDOEnabled())
            HandleExpander<typename T::ShadowPassShader>::template expand(mesh->TextureHandles, T::ShadowTextures);
        else
            TexExpander<typename T::ShadowPassShader>::template expandTex(*mesh, T::ShadowTextures);
        CustomUnrollArgs<List...>::template drawMesh<typename T::ShadowPassShader>(t.at(i), cascade);
    }   // for i
}   // renderShadow

// ----------------------------------------------------------------------------
template<typename T, typename...Args>
void renderInstancedShadow(unsigned cascade, Args ...args)
{
    T::InstancedShadowPassShader::getInstance()->use();
    glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(T::VertexType,
                                                          InstanceTypeShadow));
    std::vector<GLMesh *> &t = T::InstancedList::getInstance()->Shadows[cascade];
    for (unsigned i = 0; i < t.size(); i++)
    {
        GLMesh *mesh = t[i];

        TexExpander<typename T::InstancedShadowPassShader>::template 
                                       expandTex(*mesh, T::ShadowTextures);
        T::InstancedShadowPassShader::getInstance()->setUniforms(args..., cascade);
        size_t tmp = ShadowPassCmd::getInstance()->Offset[cascade][T::MaterialType] + i;
        glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT,
                               (const void*)((tmp) 
                               * sizeof(DrawElementsIndirectCommand)));
    }   // for i

}   // renderInstancedShadow

// ----------------------------------------------------------------------------
template<typename T, typename...Args>
static void multidrawShadow(unsigned i, Args ...args)
{
    T::InstancedShadowPassShader::getInstance()->use();
    glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(T::VertexType,
                                                           InstanceTypeShadow));
    if (ShadowPassCmd::getInstance()->Size[i][T::MaterialType])
    {
        T::InstancedShadowPassShader::getInstance()->setUniforms(args..., i);
        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, 
            (const void*)(ShadowPassCmd::getInstance()->Offset[i][T::MaterialType] 
            * sizeof(DrawElementsIndirectCommand)),
            (int)ShadowPassCmd::getInstance()->Size[i][T::MaterialType], 
            sizeof(DrawElementsIndirectCommand));
    }
}   // multidrawShadow


// ----------------------------------------------------------------------------
void GeometryPasses::renderShadows(const ShadowMatrices& shadow_matrices,
                                   const FrameBuffer& shadow_framebuffer)
{
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    
    shadow_framebuffer.bind();
    if (!CVS->isESMEnabled())
    {
        glDrawBuffer(GL_NONE);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.5, 50.);
    }
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    glClearColor(1., 1., 1., 1.);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glClearColor(0., 0., 0., 0.);

    for (unsigned cascade = 0; cascade < 4; cascade++)
    {
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_SHADOWS_CASCADE0 + cascade));

        renderShadow<DefaultMaterial, 1>(cascade);
        renderShadow<SphereMap, 1>(cascade);
        renderShadow<DetailMat, 1>(cascade);
        renderShadow<SplattingMat, 1>(cascade);
        renderShadow<NormalMat, 1>(cascade);
        renderShadow<AlphaRef, 1>(cascade);
        renderShadow<UnlitMat, 1>(cascade);
        renderShadow<GrassMat, 3, 1>(cascade);

        if (CVS->supportsIndirectInstancingRendering())
            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, ShadowPassCmd::getInstance()->drawindirectcmd);

        if (CVS->isAZDOEnabled())
        {
            multidrawShadow<DefaultMaterial>(cascade);
            multidrawShadow<DetailMat>(cascade);
            multidrawShadow<NormalMat>(cascade);
            multidrawShadow<AlphaRef>(cascade);
            multidrawShadow<UnlitMat>(cascade);
            multidrawShadow<GrassMat>(cascade, m_wind_dir);
        }
        else if (CVS->supportsIndirectInstancingRendering())
        {
            renderInstancedShadow<DefaultMaterial>(cascade);
            renderInstancedShadow<DetailMat>(cascade);
            renderInstancedShadow<AlphaRef>(cascade);
            renderInstancedShadow<UnlitMat>(cascade);
            renderInstancedShadow<GrassMat>(cascade, m_wind_dir);
            renderInstancedShadow<NormalMat>(cascade);
        }
    }

    glDisable(GL_POLYGON_OFFSET_FILL);

    if (CVS->isESMEnabled())
    {
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_SHADOW_POSTPROCESS));

        if (CVS->isARBTextureViewUsable())
        {
            const std::pair<float, float>* shadow_scales 
                = shadow_matrices.getShadowScales();

            for (unsigned i = 0; i < 2; i++)
            {
                irr_driver->getPostProcessing()->renderGaussian6BlurLayer(
                    shadow_framebuffer, i,
                    2.f * shadow_scales[0].first / shadow_scales[i].first,
                    2.f * shadow_scales[0].second / shadow_scales[i].second);
            }
        }
        glBindTexture(GL_TEXTURE_2D_ARRAY, shadow_framebuffer.getRTT()[0]);
        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    }
}   // renderShadows


// ----------------------------------------------------------------------------
template<typename T, int... Selector>
void drawRSM(const core::matrix4 & rsm_matrix)
{
    T::RSMShader::getInstance()->use();
    if (CVS->isARBBaseInstanceUsable())
        glBindVertexArray(VAOManager::getInstance()->getVAO(T::VertexType));
    auto t = T::List::getInstance()->RSM;
    for (unsigned i = 0; i < t.size(); i++)
    {
        std::vector<GLuint> Textures;
        GLMesh *mesh = STK::tuple_get<0>(t.at(i));
        if (!CVS->isARBBaseInstanceUsable())
            glBindVertexArray(mesh->vao);
        if (CVS->isAZDOEnabled())
            HandleExpander<typename T::RSMShader>::template expand(mesh->TextureHandles, T::RSMTextures);
        else
            TexExpander<typename T::RSMShader>::template expandTex(*mesh, T::RSMTextures);
        CustomUnrollArgs<Selector...>::template drawMesh<typename T::RSMShader>(t.at(i), rsm_matrix);
    }
}   // drawRSM

// ----------------------------------------------------------------------------
template<typename T, typename...Args>
void renderRSMShadow(Args ...args)
{
    T::InstancedRSMShader::getInstance()->use();
    glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(T::VertexType, InstanceTypeRSM));
    auto t = T::InstancedList::getInstance()->RSM;
    for (unsigned i = 0; i < t.size(); i++)
    {
        std::vector<GLuint> Textures;
        GLMesh *mesh = t[i];

        TexExpander<typename T::InstancedRSMShader>::template expandTex(*mesh, T::RSMTextures);
        T::InstancedRSMShader::getInstance()->setUniforms(args...);
        glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT,
           (const void*)((RSMPassCmd::getInstance()->Offset[T::MaterialType] + i)
           * sizeof(DrawElementsIndirectCommand)));
    }
}   // renderRSMShadow

// ----------------------------------------------------------------------------
template<typename T, typename... Args>
void multidrawRSM(Args...args)
{
    T::InstancedRSMShader::getInstance()->use();
    glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(T::VertexType,
                                                               InstanceTypeRSM));
    if (RSMPassCmd::getInstance()->Size[T::MaterialType])
    {
        T::InstancedRSMShader::getInstance()->setUniforms(args...);
        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT,
            (const void*)(RSMPassCmd::getInstance()->Offset[T::MaterialType] 
               * sizeof(DrawElementsIndirectCommand)),
            (int)RSMPassCmd::getInstance()->Size[T::MaterialType],
            sizeof(DrawElementsIndirectCommand));
    }
}   // multidrawRSM

// ----------------------------------------------------------------------------
void GeometryPasses::renderReflectiveShadowMap(const ShadowMatrices& shadow_matrices,
                                               const FrameBuffer& reflective_shadow_map_framebuffer)
{
    ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_RSM));
    reflective_shadow_map_framebuffer.bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const core::matrix4 &rsm_matrix = shadow_matrices.getRSMMatrix();
    drawRSM<DefaultMaterial, 3, 1>(rsm_matrix);
    drawRSM<AlphaRef, 3, 1>(rsm_matrix);
    drawRSM<NormalMat, 3, 1>(rsm_matrix);
    drawRSM<UnlitMat, 3, 1>(rsm_matrix);
    drawRSM<DetailMat, 3, 1>(rsm_matrix);
    drawRSM<SplattingMat, 1>(rsm_matrix);

    if (CVS->supportsIndirectInstancingRendering())
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER,
                     RSMPassCmd::getInstance()->drawindirectcmd);

    if (CVS->isAZDOEnabled())
    {
        multidrawRSM<DefaultMaterial>(rsm_matrix);
        multidrawRSM<NormalMat>(rsm_matrix);
        multidrawRSM<AlphaRef>(rsm_matrix);
        multidrawRSM<UnlitMat>(rsm_matrix);
        multidrawRSM<DetailMat>(rsm_matrix);
    }
    else if (CVS->supportsIndirectInstancingRendering())
    {
        renderRSMShadow<DefaultMaterial>(rsm_matrix);
        renderRSMShadow<AlphaRef>(rsm_matrix);
        renderRSMShadow<UnlitMat>(rsm_matrix);
        renderRSMShadow<NormalMat>(rsm_matrix);
        renderRSMShadow<DetailMat>(rsm_matrix);
    }
}   // renderRSM
