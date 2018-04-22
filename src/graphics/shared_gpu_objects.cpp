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

#ifndef SERVER_ONLY

#include "graphics/shared_gpu_objects.hpp"
#include "config/stk_config.hpp"
#include "graphics/central_settings.hpp"
#include "utils/log.hpp"

GLuint SharedGPUObjects::m_sky_tri_vbo;
GLuint SharedGPUObjects::m_frustrum_vbo;
GLuint SharedGPUObjects::m_frustrum_indices;
GLuint SharedGPUObjects::m_View_projection_matrices_ubo;
GLuint SharedGPUObjects::m_lighting_data_ubo;
GLuint SharedGPUObjects::m_full_screen_quad_vao;
GLuint SharedGPUObjects::m_ui_vao;
GLuint SharedGPUObjects::m_quad_buffer;
GLuint SharedGPUObjects::m_quad_vbo;
bool   SharedGPUObjects::m_has_been_initialised = false;

#include "matrix4.h"

/** Initialises m_full_screen_quad_vbo.
 */
void SharedGPUObjects::initQuadVBO()
{
    const float QUAD_VERTEX[] =
    {
        -1., -1., 0., 0.,   // UpperLeft
        -1.,  1., 0., 1.,   // LowerLeft
         1., -1., 1., 0.,   // UpperRight
         1.,  1., 1., 1.    // LowerRight
    }; 
    glGenBuffers(1, &m_quad_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(float), QUAD_VERTEX,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    const float TRI_VERTEX[] =
    {
        -1., -1.,
        -1.,  3.,
         3., -1.,
    };
    GLuint tri_vbo;
    glGenBuffers(1, &tri_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, tri_vbo);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), TRI_VERTEX,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, &m_full_screen_quad_vao);
    glBindVertexArray(m_full_screen_quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, tri_vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glBindVertexArray(0);
}   // initQuadVBO

// ----------------------------------------------------------------------------
void SharedGPUObjects::initQuadBuffer()
{
    const float QUAD_VERTEX[] =
    {
        -1., -1., -1.,  1.,   // UpperLeft
        -1.,  1., -1., -1.,   // LowerLeft
         1., -1.,  1.,  1.,   // UpperRight
         1.,  1.,  1., -1.    // LowerRight 
    };
    glGenBuffers(1, &m_quad_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_quad_buffer);
    glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(float), QUAD_VERTEX,
        GL_STATIC_DRAW);

    glGenVertexArrays(1, &m_ui_vao);
    glBindVertexArray(m_ui_vao);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, m_quad_buffer);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (GLvoid *)(2 * sizeof(float)));
    glBindVertexArray(0);
}   // initQuadBuffer

// ----------------------------------------------------------------------------
void SharedGPUObjects::initSkyTriVBO()
{
    const float TRI_VERTEX[] =
    {
        -1., -1., 1.,
        -1.,  3., 1.,
         3., -1., 1.,
    };

    glGenBuffers(1, &m_sky_tri_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_sky_tri_vbo);
    glBufferData(GL_ARRAY_BUFFER, 3 * 3 * sizeof(float), TRI_VERTEX,
                 GL_STATIC_DRAW);
}   // initSkyTriVBO

// ----------------------------------------------------------------------------
void SharedGPUObjects::initFrustrumVBO()
{
    glGenBuffers(1, &m_frustrum_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_frustrum_vbo);
    glBufferData(GL_ARRAY_BUFFER, 8 * 3 * sizeof(float), 0, GL_DYNAMIC_DRAW);

    int INDICES[24] = {
        0, 1, 1, 3, 3, 2, 2, 0,
        4, 5, 5, 7, 7, 6, 6, 4,
        0, 4, 1, 5, 2, 6, 3, 7,
    };

    glGenBuffers(1, &m_frustrum_indices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_frustrum_indices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 12 * 2 * sizeof(int), INDICES,
                 GL_STATIC_DRAW);
}   // initFrustrumVBO

// ----------------------------------------------------------------------------
void SharedGPUObjects::initShadowVPMUBO()
{
    assert(CVS->isARBUniformBufferObjectUsable());
    glGenBuffers(1, &m_View_projection_matrices_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, m_View_projection_matrices_ubo);
    glBufferData(GL_UNIFORM_BUFFER, (16 * 9 + 2) * sizeof(float), 0,
                 GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}   // initShadowVPMUBO

// ----------------------------------------------------------------------------
void SharedGPUObjects::initLightingDataUBO()
{
    assert(CVS->isARBUniformBufferObjectUsable());
    glGenBuffers(1, &m_lighting_data_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, m_lighting_data_ubo);
    glBufferData(GL_UNIFORM_BUFFER, 36 * sizeof(float), 0, GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}   // initLightingDataUBO

// ----------------------------------------------------------------------------
void SharedGPUObjects::init()
{
    if (m_has_been_initialised)
        return;
    initQuadVBO();
    initQuadBuffer();
    initSkyTriVBO();
    initFrustrumVBO();
    if (CVS->isARBUniformBufferObjectUsable())
    {
        initShadowVPMUBO();
        initLightingDataUBO();
    }

    m_has_been_initialised = true;
}   // SharedGPUObjects

// ----------------------------------------------------------------------------
/** A simple reset function. Atm it actually only resets the
 *  m_has_been_initialised flag (all opengl data gets reset anyway when this
 *  function is called).
 */
void SharedGPUObjects::reset()
{
    m_has_been_initialised = false;
}   // reset

#endif   // !SERVER_ONLY

