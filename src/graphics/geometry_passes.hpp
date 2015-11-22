//  SuperTuxéKart - a fun racing game with go-kart
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
//  You should have received a copy éof the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef HEADER_GEOMETRY_PASSES_HPP
#define HEADER_GEOMETRY_PASSES_HPP

#include "graphics/draw_calls.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/shadow_matrices.hpp"
#include "utils/profiler.hpp"
#include <ITexture.h>


/** Rendering methods in this class only require
 *  OpenGL3.2 functions */
class GL3DrawPolicy
{
public:
    
    void drawNormals(const DrawCalls& draw_calls) const {}
    
    void drawShadows(const DrawCalls& draw_calls, unsigned cascade) const;
    void drawReflectiveShadowMap(const DrawCalls& draw_calls,
                                 const core::matrix4 &rsm_matrix) const;
};

/** Faster than GL3GeometryPasses.
  * Require at least OpenGL 4.0
  * or GL_ARB_base_instance and GL_ARB_draw_indirect extensions)*/
class IndirectDrawPolicy
{
public:
    
    void drawNormals(const DrawCalls& draw_calls) const;
    
    void drawShadows(const DrawCalls& draw_calls, unsigned cascade) const;
    void drawReflectiveShadowMap(const DrawCalls& draw_calls,
                                 const core::matrix4 &rsm_matrix) const;    
};

/** Faster than IndirectGeometryPasses.
  * Require OpenGL AZDO extensions */
class MultidrawPolicy
{
public:

    void drawNormals(const DrawCalls& draw_calls) const;

    void drawShadows(const DrawCalls& draw_calls, unsigned cascade) const;
    void drawReflectiveShadowMap(const DrawCalls& draw_calls,
                                 const core::matrix4 &rsm_matrix) const;    
};



class AbstractGeometryPasses
{
protected:

    irr::core::vector3df m_wind_dir;
    irr::video::ITexture *m_displace_tex;

    void prepareShadowRendering(const FrameBuffer& shadow_framebuffer) const;
    void shadowPostProcessing(const ShadowMatrices& shadow_matrices,
                              const FrameBuffer& shadow_framebuffer) const;

public:
    AbstractGeometryPasses();
    virtual ~AbstractGeometryPasses(){}
    
    void renderSolidFirstPass(const DrawCalls& draw_calls);
    void renderSolidSecondPass( const DrawCalls& draw_calls,
                                unsigned render_target_diffuse,
                                unsigned render_target_specular,
                                unsigned render_target_half_red);
                                
    virtual void renderNormalsVisualisation(const DrawCalls& draw_calls) const = 0;
    
    void renderGlow(const DrawCalls& draw_calls, const std::vector<GlowData>& glows);
    
    void renderTransparent(const DrawCalls& draw_calls, 
                           unsigned render_target);
                           
    virtual void renderShadows(const DrawCalls& draw_calls,
                               const ShadowMatrices& shadow_matrices,
                               const FrameBuffer& shadow_framebuffer) const = 0;
                       
                       
    virtual void renderReflectiveShadowMap(const DrawCalls& draw_calls,
                                           const ShadowMatrices& shadow_matrices,
                                           const FrameBuffer& reflective_shadow_map_framebuffer) const = 0 ;


};

template<typename DrawPolicy>
class GeometryPasses: public AbstractGeometryPasses, public DrawPolicy
{
public:

    void renderNormalsVisualisation(const DrawCalls& draw_calls) const
    {
        DrawPolicy::drawNormals(draw_calls);
    }


    void renderShadows(const DrawCalls& draw_calls,
                                   const ShadowMatrices& shadow_matrices,
                                   const FrameBuffer& shadow_framebuffer) const
    {
        prepareShadowRendering(shadow_framebuffer);

        for (unsigned cascade = 0; cascade < 4; cascade++)
        {
            ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_SHADOWS_CASCADE0 + cascade));
            DrawPolicy::drawShadows(draw_calls, cascade);
        }

        glDisable(GL_POLYGON_OFFSET_FILL);

        if (CVS->isESMEnabled())
            shadowPostProcessing(shadow_matrices, shadow_framebuffer);
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
                                           
};


#endif //HEADER_GEOMETRY_PASSES_HPP
