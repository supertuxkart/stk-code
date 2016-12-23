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

#ifndef SERVER_ONLY

#ifndef HEADER_GEOMETRY_PASSES_HPP
#define HEADER_GEOMETRY_PASSES_HPP

#include "graphics/central_settings.hpp"
#include "graphics/draw_calls.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/shadow_matrices.hpp"
#include "utils/profiler.hpp"
#include <ITexture.h>

class AbstractGeometryPasses
{
protected:

    irr::video::ITexture *m_displace_tex;

    std::vector<GLuint>   m_prefilled_textures;    
    std::vector<uint64_t> m_textures_handles;
    
#if !defined(USE_GLES2)
    void prepareShadowRendering(const FrameBuffer& shadow_framebuffer) const;
    void shadowPostProcessing(const ShadowMatrices& shadow_matrices,
                              const FrameBuffer& shadow_framebuffer,
                              const FrameBuffer& scalar_framebuffer,
                              const PostProcessing* post_processing) const;
#endif // !defined(USE_GLES2)
   
public:
    AbstractGeometryPasses();
    virtual ~AbstractGeometryPasses(){}
    
    void setFirstPassRenderTargets(const std::vector<GLuint>& prefilled_textures,
                                   const std::vector<uint64_t>& prefilled_handles);
    
    virtual void renderSolidFirstPass(const DrawCalls& draw_calls) const = 0;
    
    virtual void renderSolidSecondPass( const DrawCalls& draw_calls) const = 0;
                                
    virtual void renderNormalsVisualisation(const DrawCalls& draw_calls) const = 0;
    
    virtual void renderGlowingObjects(const DrawCalls& draw_calls,
                                      const std::vector<GlowData>& glows,
                                      const FrameBuffer& glow_framebuffer) const = 0;
    
    void renderTransparent(const DrawCalls& draw_calls,
                           const FrameBuffer& tmp_framebuffer,
                           const FrameBuffer& displace_framebuffer,
                           const FrameBuffer& colors_framebuffer,
                           const PostProcessing* post_processing);

#if !defined(USE_GLES2)                           
    virtual void renderShadows (const DrawCalls& draw_calls,
                                const ShadowMatrices& shadow_matrices,
                                const FrameBuffer& shadow_framebuffer,
                                const FrameBuffer& scalar_framebuffer,
                                const PostProcessing* post_processing  ) const = 0;
                       
                       
    virtual void renderReflectiveShadowMap(const DrawCalls& draw_calls,
                                           const ShadowMatrices& shadow_matrices,
                                           const FrameBuffer& reflective_shadow_map_framebuffer) const = 0 ;
#endif // !defined(USE_GLES2)
};

template<typename DrawPolicy>
class GeometryPasses: public AbstractGeometryPasses, public DrawPolicy
{
public:
    // ----------------------------------------------------------------------------
    /** Render the solid first pass (depth and normals)*/ 
    void renderSolidFirstPass(const DrawCalls& draw_calls) const
    {
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_SOLID_PASS1));
        irr_driver->setPhase(SOLID_NORMAL_AND_DEPTH_PASS);
        draw_calls.renderImmediateDrawList();
        DrawPolicy::drawSolidFirstPass(draw_calls);
    }   // renderSolidFirstPass

    // ----------------------------------------------------------------------------
    /** Render the solid second pass (apply lighting on materials) */
    void renderSolidSecondPass( const DrawCalls& draw_calls) const
    {
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_SOLID_PASS2));
        irr_driver->setPhase(SOLID_LIT_PASS);
        draw_calls.renderImmediateDrawList();
        DrawPolicy::drawSolidSecondPass(draw_calls,
                                        m_textures_handles,
                                        m_prefilled_textures);
    }


    void renderNormalsVisualisation(const DrawCalls& draw_calls) const
    {
        DrawPolicy::drawNormals(draw_calls);
    }

    void renderGlowingObjects(const DrawCalls& draw_calls,
                              const std::vector<GlowData>& glows,
                              const FrameBuffer& glow_framebuffer) const
    {
        irr_driver->getSceneManager()->setCurrentRendertime(scene::ESNRP_SOLID);
        glow_framebuffer.bind();
        glClearStencil(0);
        glClearColor(0, 0, 0, 0);
        glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilFunc(GL_ALWAYS, 1, ~0);
        glEnable(GL_STENCIL_TEST);

        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glDisable(GL_BLEND);
        
        DrawPolicy::drawGlow(draw_calls, glows);
        
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glDisable(GL_STENCIL_TEST);
    }

#if !defined(USE_GLES2)
    void renderShadows(const DrawCalls& draw_calls,
                       const ShadowMatrices& shadow_matrices,
                       const FrameBuffer& shadow_framebuffer,
                       const FrameBuffer& scalar_framebuffer,
                       const PostProcessing* post_processing) const
    {
        prepareShadowRendering(shadow_framebuffer);

        for (unsigned cascade = 0; cascade < 4; cascade++)
        {
            ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_SHADOWS_CASCADE0 + cascade));
            DrawPolicy::drawShadows(draw_calls, cascade);
        }

        glDisable(GL_POLYGON_OFFSET_FILL);

        if (CVS->isESMEnabled())
            shadowPostProcessing(shadow_matrices, shadow_framebuffer,
                                 scalar_framebuffer, post_processing);
    }


    void renderReflectiveShadowMap(const DrawCalls& draw_calls,
                                   const ShadowMatrices& shadow_matrices,
                                   const FrameBuffer& reflective_shadow_map_framebuffer) const
    {
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_RSM));
        reflective_shadow_map_framebuffer.bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        DrawPolicy::drawReflectiveShadowMap(draw_calls, shadow_matrices.getRSMMatrix());
    }
#endif // !defined(USE_GLES2)

};


#endif   // !SERVER_ONLY
#endif   // HEADER_GEOMETRY_PASSES_HPP
