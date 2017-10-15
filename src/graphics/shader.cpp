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
    loadAndAttachShader(GL_FRAGMENT_SHADER, "tfb_dummy.frag");
#endif
    if (!CVS->isARBExplicitAttribLocationUsable())
        setAttribute(PARTICLES_SIM);

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
void ShaderBase::bypassUBO() const
{
    GLint VM = glGetUniformLocation(m_program, "ViewMatrix");
    glUniformMatrix4fv(VM, 1, GL_FALSE, irr_driver->getViewMatrix().pointer());

    GLint PM = glGetUniformLocation(m_program, "ProjectionMatrix");
    glUniformMatrix4fv(PM, 1, GL_FALSE, irr_driver->getProjMatrix().pointer());

    GLint PVM = glGetUniformLocation(m_program, "ProjectionViewMatrix");
    glUniformMatrix4fv(PVM, 1, GL_FALSE, irr_driver->getProjViewMatrix().pointer());

    GLint IVM = glGetUniformLocation(m_program, "InverseViewMatrix");
    glUniformMatrix4fv(IVM, 1, GL_FALSE, irr_driver->getInvViewMatrix().pointer());

    GLint IPM = glGetUniformLocation(m_program, "InverseProjectionMatrix");
    glUniformMatrix4fv(IPM, 1, GL_FALSE, irr_driver->getInvProjMatrix().pointer());

    GLint Screen = glGetUniformLocation(m_program, "screen");
    glUniform2f(Screen, irr_driver->getCurrentScreenSize().X,
                        irr_driver->getCurrentScreenSize().Y);

    const SHCoefficients* sh_coeff = irr_driver->getSHCoefficients();

    GLint bLmn = glGetUniformLocation(m_program, "blueLmn[0]");
    glUniform1fv(bLmn, 9, sh_coeff->blue_SH_coeff);

    GLint gLmn = glGetUniformLocation(m_program, "greenLmn[0]");
    glUniform1fv(gLmn, 9, sh_coeff->green_SH_coeff);

    GLint rLmn = glGetUniformLocation(m_program, "redLmn[0]");
    glUniform1fv(rLmn, 9, sh_coeff->red_SH_coeff);

    GLint sun_dir = glGetUniformLocation(m_program, "sun_direction");
    const core::vector3df &sd = irr_driver->getSunDirection();
    glUniform3f(sun_dir, sd.X, sd.Y, sd.Z);

    GLint sun_col = glGetUniformLocation(m_program, "sun_col");
    const video::SColorf& sc = irr_driver->getSunColor();
    glUniform3f(sun_col, sc.getRed(), sc.getGreen(), sc.getBlue());

    GLint sun_angle = glGetUniformLocation(m_program, "sun_angle");
    glUniform1f(sun_angle, 0.54f);
}   // bypassUBO

// ----------------------------------------------------------------------------
/** Constructor, which adds the shader to all instantiated shaders (for the
 *  reload-all-shaders debug option).
 */
ShaderBase::ShaderBase()
{
}   // ShaderBase

// ----------------------------------------------------------------------------
void ShaderBase::updateShaders()
{
    for (unsigned int i = 0; i < m_all_kill_functions.size(); i++)
    {
        m_all_kill_functions[i]();
    }
    m_all_kill_functions.clear();
}   // updateShaders

// ----------------------------------------------------------------------------
void ShaderBase::setAttribute(AttributeType type)
{
    switch (type)
    {
    case OBJECT:
        glBindAttribLocation(m_program, 0, "Position");
        glBindAttribLocation(m_program, 1, "Normal");
        glBindAttribLocation(m_program, 2, "Color");
        glBindAttribLocation(m_program, 3, "Texcoord");
        glBindAttribLocation(m_program, 4, "SecondTexcoord");
        glBindAttribLocation(m_program, 5, "Tangent");
        glBindAttribLocation(m_program, 6, "Bitangent");
        glBindAttribLocation(m_program, 7, "Origin");
        glBindAttribLocation(m_program, 8, "Orientation");
        glBindAttribLocation(m_program, 9, "Scale");
        glBindAttribLocation(m_program, 10, "misc_data");
        break;
    case PARTICLES_SIM:
        glBindAttribLocation(m_program, 0, "particle_position");
        glBindAttribLocation(m_program, 1, "lifetime");
        glBindAttribLocation(m_program, 2, "particle_velocity");
        glBindAttribLocation(m_program, 3, "size");
        glBindAttribLocation(m_program, 4, "particle_position_initial");
        glBindAttribLocation(m_program, 5, "lifetime_initial");
        glBindAttribLocation(m_program, 6, "particle_velocity_initial");
        glBindAttribLocation(m_program, 7, "size_initial");
        if (CVS->needsVertexIdWorkaround())
        {
            glBindAttribLocation(m_program, 8, "vertex_id");
        }
        break;
    case PARTICLES_RENDERING:
        glBindAttribLocation(m_program, 0, "Position");
        glBindAttribLocation(m_program, 1, "color_lifetime");
        glBindAttribLocation(m_program, 2, "size");
        glBindAttribLocation(m_program, 3, "Texcoord");
        glBindAttribLocation(m_program, 4, "quadcorner");
        glBindAttribLocation(m_program, 5, "rotationvec");
        glBindAttribLocation(m_program, 6, "anglespeed");
        break;
    case SKINNED_MESH:
        glBindAttribLocation(m_program, 0, "Position");
        glBindAttribLocation(m_program, 1, "Normal");
        glBindAttribLocation(m_program, 2, "Color");
        glBindAttribLocation(m_program, 3, "Data1");
        glBindAttribLocation(m_program, 4, "Data2");
        glBindAttribLocation(m_program, 5, "Joint");
        glBindAttribLocation(m_program, 6, "Weight");
        glBindAttribLocation(m_program, 7, "Origin");
        glBindAttribLocation(m_program, 8, "Orientation");
        glBindAttribLocation(m_program, 9, "Scale");
        glBindAttribLocation(m_program, 10, "misc_data");
        glBindAttribLocation(m_program, 15, "skinning_offset");
        break;
    }
}   // setAttribute

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

// ============================================================================

#endif   // !SERVER_ONLY
