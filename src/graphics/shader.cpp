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

#include "graphics/shader.hpp"

#include "graphics/central_settings.hpp"
#include "graphics/gl_headers.hpp"
#include "graphics/irr_driver.hpp"
#include "io/file_manager.hpp"
#include "utils/log.hpp"

#include <fstream>
#include <sstream>
#include <stdio.h>

std::string                ShaderBase::m_shader_header = "";
std::vector<ShaderBase *>  ShaderBase::m_all_shaders;

// ----------------------------------------------------------------------------
/** Returns a string with the content of header.txt (which contains basic
 *  shader defines).
 */
const std::string& ShaderBase::getHeader()
{
    // Only read file first time
    if (m_shader_header.empty())
    {
        std::ifstream stream(file_manager->getShader("header.txt"), std::ios::in);
        if (stream.is_open())
        {
            std::string line = "";
            while (getline(stream, line))
                m_shader_header += "\n" + line;
            stream.close();
        }
    }   // if m_shader_header.empty()

    return m_shader_header;
}   // getHeader

// ----------------------------------------------------------------------------
/** Loads a single shader.
 *  \param file Filename of the shader to load.
 *  \param type Type of the shader.
 */
GLuint ShaderBase::loadShader(const std::string &file, unsigned type)
{
    GLuint id = glCreateShader(type);
    
    std::ostringstream code;
    code << "#version " << CVS->getGLSLVersion()<<"\n";
    if (CVS->isAMDVertexShaderLayerUsable())
        code << "#extension GL_AMD_vertex_shader_layer : enable\n";
    if (CVS->isAZDOEnabled())
    {
        code << "#extension GL_ARB_bindless_texture : enable\n";
        code << "#define Use_Bindless_Texture\n";
    }
    code << "//" + std::string(file) + "\n";
    if (!CVS->isARBUniformBufferObjectUsable())
        code << "#define UBO_DISABLED\n";
    if (CVS->isAMDVertexShaderLayerUsable())
        code << "#define VSLayer\n";
    if (CVS->needsRGBBindlessWorkaround())
        code << "#define SRGBBindlessFix\n";
    code << getHeader();

    std::ifstream stream(file, std::ios::in);
    if (stream.is_open())
    {
        std::string Line = "";
        while (getline(stream, Line))
            code << "\n" + Line;
        stream.close();
    }
    else
        Log::error("shader", "Can not open '%s'.", file.c_str());

    Log::info("shader", "Compiling shader : %s", file);
    const std::string &source  = code.str();
    char const *source_pointer = source.c_str();
    int len                    = source.size();
    glShaderSource(id, 1, &source_pointer, &len);
    glCompileShader(id);

    GLint result = GL_FALSE;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int info_length;
        Log::error("GLWrap", "Error in shader %s", file);
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &info_length);
        if (info_length<0)
            info_length = 1024;
        char *error_message = new char[info_length];
        error_message[0] = 0;
        glGetShaderInfoLog(id, info_length, NULL, error_message);
        Log::error("GLWrap", error_message);
        delete[] error_message;
    }

    glGetError();

    return id;
}   // loadShader

// ----------------------------------------------------------------------------
/** Loads a transform feedback buffer shader with a given number of varying
 *  parameters.
*/
int ShaderBase::loadTFBProgram(const std::string &shader_name, 
                               const char **varyings,
                               unsigned varying_count)
{
    std::string full_path = file_manager->getShader(shader_name);
    m_program = glCreateProgram();
    loadAndAttachShader(GL_VERTEX_SHADER, full_path);
    if (CVS->getGLSLVersion() < 330)
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
        Log::error("GLWrap", error_message);
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

    GLint IVM = glGetUniformLocation(m_program, "InverseViewMatrix");
    glUniformMatrix4fv(IVM, 1, GL_FALSE, irr_driver->getInvViewMatrix().pointer());

    GLint IPM = glGetUniformLocation(m_program, "InverseProjectionMatrix");
    glUniformMatrix4fv(IPM, 1, GL_FALSE, irr_driver->getInvProjMatrix().pointer());

    GLint Screen = glGetUniformLocation(m_program, "screen");
    glUniform2f(Screen, irr_driver->getCurrentScreenSize().X, 
                        irr_driver->getCurrentScreenSize().Y);

    GLint bLmn = glGetUniformLocation(m_program, "blueLmn[0]");
    glUniform1fv(bLmn, 9, irr_driver->blueSHCoeff);

    GLint gLmn = glGetUniformLocation(m_program, "greenLmn[0]");
    glUniform1fv(gLmn, 9, irr_driver->greenSHCoeff);

    GLint rLmn = glGetUniformLocation(m_program, "redLmn[0]");
    glUniform1fv(rLmn, 9, irr_driver->redSHCoeff);

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
    m_all_shaders.push_back(this);
}   // ShaderBase

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
        break;
    case PARTICLES_RENDERING:
        glBindAttribLocation(m_program, 1, "lifetime");
        glBindAttribLocation(m_program, 2, "size");
        glBindAttribLocation(m_program, 4, "quadcorner");
        glBindAttribLocation(m_program, 5, "rotationvec");
        glBindAttribLocation(m_program, 6, "anglespeed");
        break;
    }
}   // setAttribute

// ============================================================================
