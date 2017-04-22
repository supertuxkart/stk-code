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
//  You should have received a copy éof the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef HEADER_DRAW_POLICIES_HPP
#define HEADER_DRAW_POLICIES_HPP

#ifndef SERVER_ONLY

#include "graphics/abstract_renderer.hpp"

using namespace irr;
class DrawCalls;

/** Rendering methods in this class only require
 *  OpenGL3.2 functions */
class GL3DrawPolicy
{
public:
    void drawSolidFirstPass     (const DrawCalls& draw_calls             ) const;
    
    void drawSolidSecondPass    (const DrawCalls& draw_calls,
                                 const std::vector<uint64_t>& handles,
                                 const std::vector<GLuint>& prefilled_tex) const;
                             
    void drawNormals            (const DrawCalls& draw_calls             ) const {}
    
    void drawGlow               (const DrawCalls& draw_calls,
                                 const std::vector<GlowData>& glows      ) const;
                                     
    void drawShadows            (const DrawCalls& draw_calls,
                                unsigned cascade                         ) const;
    
    void drawReflectiveShadowMap(const DrawCalls& draw_calls,
                                 const core::matrix4 &rsm_matrix         ) const;
};

/** Faster than GL3GeometryPasses.
  * Require OpenGL 4.0 (or higher)
  * or GL_ARB_base_instance and GL_ARB_draw_indirect extensions)*/
class IndirectDrawPolicy
{
public:
    void drawSolidFirstPass     (const DrawCalls& draw_calls             ) const;
    
    void drawSolidSecondPass    (const DrawCalls& draw_calls,
                                 const std::vector<uint64_t>& handles,
                                 const std::vector<GLuint>& prefilled_tex) const;
                             
    void drawNormals            (const DrawCalls& draw_calls             ) const;
    
    void drawGlow               (const DrawCalls& draw_calls,
                                 const std::vector<GlowData>& glows      ) const;
    
    void drawShadows            (const DrawCalls& draw_calls,
                                unsigned cascade                         ) const;
    
    void drawReflectiveShadowMap(const DrawCalls& draw_calls,
                                 const core::matrix4 &rsm_matrix         ) const;
};

/** Faster than IndirectGeometryPasses.
  * Require OpenGL AZDO extensions */
class MultidrawPolicy
{
public:
    void drawSolidFirstPass     (const DrawCalls& draw_calls             ) const;
    
    void drawSolidSecondPass    (const DrawCalls& draw_calls,
                                 const std::vector<uint64_t>& handles,
                                 const std::vector<GLuint>& prefilled_tex) const;
                             
    void drawNormals            (const DrawCalls& draw_calls             ) const;
    
    void drawGlow               (const DrawCalls& draw_calls,
                                 const std::vector<GlowData>& glows      ) const;
                                     
    void drawShadows            (const DrawCalls& draw_calls,
                                unsigned cascade                         ) const;
    
    void drawReflectiveShadowMap(const DrawCalls& draw_calls,
                                 const core::matrix4 &rsm_matrix         ) const;  
};

#endif   // !SERVER_ONLY
#endif   // HEADER_DRAW_POLICIES_HPP
