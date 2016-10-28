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
#include "utils/tuple.hpp"
#include <SColor.h>
#include <S3DVertex.h>

/**
\page geometry_passes Geometry Rendering Overview

\section adding_material Adding a solid material

You need to consider twice before adding a new material : in the worst case a material requires 8 shaders :
one for each solid pass, one for shadow pass, one for RSM pass, and you need to double that for instanced version.

You need to declare a new enum in MeshMaterial and to write the corresponding dispatch code in getMeshMaterialFromType
and to create a new List* structure for non instanced.

Then you need to write the code in draw_calls.cpp that will add any mesh with the new material to their corresponding
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

#if !defined(USE_GLES2)
// ----------------------------------------------------------------------------
void AbstractGeometryPasses::prepareShadowRendering(const FrameBuffer& shadow_framebuffer) const
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
}

// ----------------------------------------------------------------------------
void AbstractGeometryPasses::shadowPostProcessing(const ShadowMatrices& shadow_matrices,
                                                  const FrameBuffer& shadow_framebuffer,
                                                  const FrameBuffer& scalar_framebuffer,
                                                  const PostProcessing* post_processing) const
{
    ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_SHADOW_POSTPROCESS));

    if (CVS->isARBTextureViewUsable())
    {
        const std::pair<float, float>* shadow_scales 
            = shadow_matrices.getShadowScales();

        for (unsigned i = 0; i < 2; i++)
        {
            post_processing->renderGaussian6BlurLayer(
                shadow_framebuffer, scalar_framebuffer, i,
                2.f * shadow_scales[0].first / shadow_scales[i].first,
                2.f * shadow_scales[0].second / shadow_scales[i].second);
        }
    }
    glBindTexture(GL_TEXTURE_2D_ARRAY, shadow_framebuffer.getRTT()[0]);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
}
#endif // !defined(USE_GLES2)

AbstractGeometryPasses::AbstractGeometryPasses()
{
    m_displace_tex = irr_driver->getTexture(FileManager::TEXTURE, "displace.png");
}

// ----------------------------------------------------------------------------
void AbstractGeometryPasses::setFirstPassRenderTargets(const std::vector<GLuint>& prefilled_textures)
{    
    m_prefilled_textures = prefilled_textures;

#if !defined(USE_GLES2)
    if (CVS->isAZDOEnabled())
    {
        m_textures_handles.clear();
        for(size_t i=0;i<m_prefilled_textures.size();i++)
        {
            uint64_t handle = 0;
            handle = glGetTextureSamplerHandleARB(m_prefilled_textures[i],
                Shaders::ObjectPass2Shader::getInstance()->m_sampler_ids[i]);
            if (!glIsTextureHandleResidentARB(handle))
                glMakeTextureHandleResidentARB(handle);
            m_textures_handles.push_back(handle);
        }
    }
#endif // !defined(USE_GLES2)
}

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
void AbstractGeometryPasses::renderTransparent(const DrawCalls& draw_calls,
                                               const FrameBuffer& tmp_framebuffer,
                                               const FrameBuffer& displace_framebuffer,
                                               const FrameBuffer& colors_framebuffer,
                                               const PostProcessing* post_processing)
{
    irr_driver->setPhase(TRANSPARENT_PASS);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glEnable(GL_CULL_FACE);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    renderTransparenPass<Shaders::TransparentShader, video::EVT_STANDARD, 3, 2, 1>(
                         TexUnits(RenderGeometry::TexUnit(0, true)),
                                  ListGhostKart::getInstance());

    renderTransparenPass<Shaders::TransparentShader, video::EVT_TANGENTS, 3, 2, 1>(
                         TexUnits(RenderGeometry::TexUnit(0, true)),
                                  ListGhostKartTangents::getInstance());

    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
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
                             video::EVT_STANDARD, 3, 2, 1>(
                                  TexUnits(RenderGeometry::TexUnit(0, true)),
                                           ListBlendTransparent::getInstance());
        glBlendFunc(GL_ONE, GL_ONE);
        renderTransparenPass<Shaders::TransparentShader, video::EVT_STANDARD, 3, 2, 1>(
                             TexUnits(RenderGeometry::TexUnit(0, true)),
                                      ListAdditiveTransparent::getInstance());
    }

    draw_calls.renderBillboardList();

    if (!CVS->isDefferedEnabled())
        return;

    // Render displacement nodes
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
    if (ListDisplacement::getInstance()->size() > 0)
    {
        tmp_framebuffer.bind();
        glClear(GL_COLOR_BUFFER_BIT);
    }
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

    if (ListDisplacement::getInstance()->size() > 0)
    {
        displace_framebuffer.bind();
        glClear(GL_COLOR_BUFFER_BIT);
    }
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
            colors_framebuffer.getRTT()[0],
            tmp_framebuffer.getRTT()[0],
            getTextureGLuint(mesh.textures[0]));
        DisplaceShader::getInstance()->use();
        DisplaceShader::getInstance()->setUniforms(AbsoluteTransformation,
            core::vector2df(cb->getDirX(), cb->getDirY()),
            core::vector2df(cb->getDir2X(), cb->getDir2Y()));

        glDrawElementsBaseVertex(ptype, (int)count, itype, (GLvoid *)mesh.vaoOffset,
                                 (int)mesh.vaoBaseVertex);
    }

    colors_framebuffer.bind();
    glStencilFunc(GL_EQUAL, 1, 0xFF);
    post_processing->renderPassThrough(displace_framebuffer.getRTT()[0],
                                       colors_framebuffer.getWidth(),
                                       colors_framebuffer.getHeight());
    glDisable(GL_STENCIL_TEST);

}   // renderTransparent



