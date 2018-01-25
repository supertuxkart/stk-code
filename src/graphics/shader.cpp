//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 SuperTuxKart-Team
//            (C) 2015      Joerg Henrichs
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

#include "graphics/shader.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/spherical_harmonics.hpp"
#include "utils/log.hpp"

#include <fstream>
#include <sstream>
#include <stdio.h>

std::vector<void(*)()>  ShaderBase::m_all_kill_functions;

// ----------------------------------------------------------------------------
/** Loads a transform feedback buffer shader with a given number of varying
 *  parameters.
*/
int ShaderBase::loadTFBProgram(const std::string &shader_name,
                               const char **varyings,
                               unsigned varying_count)
{
    m_program = glCreateProgram();
    loadAndAttachShader(GL_VERTEX_SHADER, shader_name);
#ifdef USE_GLES2
    loadAndAttachShader(GL_FRAGMENT_SHADER, "white.frag");
#endif

    glTransformFeedbackVaryings(m_program, varying_count, varyings,
                               GL_INTERLEAVED_ATTRIBS);
    glLinkProgram(m_program);

    GLint result = GL_FALSE;
    glGetProgramiv(m_program, GL_LINK_STATUS, &result);
    if (result == GL_FALSE)
    {
        int info_log_length;
        glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &info_log_length);
        char *error_message = new char[info_log_length];
        glGetProgramInfoLog(m_program, info_log_length, NULL, error_message);
        Log::error("ShaderBase", error_message);
        delete[] error_message;
    }

    glGetError();

    return m_program;
}   // loadTFBProgram

// ----------------------------------------------------------------------------
/** Constructor, which adds the shader to all instantiated shaders (for the
 *  reload-all-shaders debug option).
 */
ShaderBase::ShaderBase()
{
}   // ShaderBase

// ----------------------------------------------------------------------------
void ShaderBase::killShaders()
{
    for (unsigned int i = 0; i < m_all_kill_functions.size(); i++)
    {
        m_all_kill_functions[i]();
    }
    m_all_kill_functions.clear();
}   // killShaders

// ----------------------------------------------------------------------------
GLuint ShaderBase::createVAO()
{
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    GLuint attrib_position = glGetAttribLocation(m_program, "Position");
    GLuint attrib_texcoord = glGetAttribLocation(m_program, "Texcoord");
    glBindBuffer(GL_ARRAY_BUFFER, SharedGPUObjects::getQuadVBO());
    glEnableVertexAttribArray(attrib_position);
    glEnableVertexAttribArray(attrib_texcoord);
    glVertexAttribPointer(attrib_position, 2, GL_FLOAT, GL_FALSE,
                          4 * sizeof(float), 0);
    glVertexAttribPointer(attrib_texcoord, 2, GL_FLOAT, GL_FALSE,
                          4 * sizeof(float), (GLvoid*)(2 * sizeof(float)));
    glBindVertexArray(0);
    return vao;
}   // createVAO

#endif   // !SERVER_ONLY
