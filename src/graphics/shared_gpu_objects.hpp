//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 SuperTuxKart-Team
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

#ifndef HEADER_SHARED_GPU_OBJECTS_HPP
#define HEADER_SHARED_GPU_OBJECTS_HPP

#include "graphics/gl_headers.hpp"

#include <assert.h>

class SharedGPUObjects
{
private:
    static bool m_has_been_initialised;
    static GLuint m_sky_tri_vbo;
    static GLuint m_frustrum_vbo;
    static GLuint m_frustrum_indices;
    static GLuint m_View_projection_matrices_ubo;
    static GLuint m_lighting_data_ubo;
    static GLuint m_full_screen_quad_vao;
    static GLuint m_ui_vao;
    static GLuint m_quad_buffer;
    static GLuint m_quad_vbo;
    static GLuint m_skinning_tex;
    static GLuint m_skinning_buf;

    static void initQuadVBO();
    static void initQuadBuffer();
    static void initSkyTriVBO();
    static void initFrustrumVBO();
    static void initShadowVPMUBO();
    static void initLightingDataUBO();
    static void initSkinning();

public:
    static void init();
    static void reset();
    // ------------------------------------------------------------------------
    static GLuint getSkyTriVBO() 
    {
        assert(m_has_been_initialised);
        return m_sky_tri_vbo;
    }   // getSkyTriVBO
    // ------------------------------------------------------------------------
    static GLuint getFrustrumVBO() 
    {
        assert(m_has_been_initialised);
        return m_frustrum_vbo;
    }   // getFrustrumVBO
    // ------------------------------------------------------------------------
    static GLuint getFrustrumIndices() 
    {
        assert(m_has_been_initialised);
        return m_frustrum_indices;
    }   // getFrustrumIndices
    // ------------------------------------------------------------------------
    static GLuint getViewProjectionMatricesUBO() 
    {
        assert(m_has_been_initialised);
        return m_View_projection_matrices_ubo;
    }   // getViewProjectionMatricesUBO
    // ------------------------------------------------------------------------
    static GLuint getLightingDataUBO() 
    {
        assert(m_has_been_initialised);
        return m_lighting_data_ubo;
    }   // getLightingDataUBO
    // -------------- ----------------------------------------------------------
    static GLuint getFullScreenQuadVAO() 
    {
        assert(m_has_been_initialised);
        return m_full_screen_quad_vao;
    }   // getFullScreenQuadVAO
    // ------------------------------------------------------------------------
    static GLuint getUI_VAO() 
    {
        assert(m_has_been_initialised);
        return m_ui_vao;
    }  // getUI_VAO
    // ------------------------------------------------------------------------
    static GLuint getQuadBuffer() 
    {
        assert(m_has_been_initialised);
        return m_quad_buffer;
    }   // getQuadBuffer
    // ------------------------------------------------------------------------
    static GLuint getQuadVBO()
    {
        assert(m_has_been_initialised);
        return m_quad_vbo;
    }   // getQuadVBO
    // ------------------------------------------------------------------------
    static GLuint getSkinningTexture()
    {
        assert(m_has_been_initialised);
        return m_skinning_tex;
    }   // getSkinningTexture
    // ------------------------------------------------------------------------
    static GLuint getSkinningBuffer()
    {
        assert(m_has_been_initialised);
        return m_skinning_buf;
    }   // getSkinningBuffer

};   // class SharedGPUObjects


#endif
