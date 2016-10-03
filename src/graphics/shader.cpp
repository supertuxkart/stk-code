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
#include "graphics/shared_gpu_objects.hpp"
#include "io/file_manager.hpp"
#include "utils/log.hpp"

#include <fstream>
#include <sstream>
#include <stdio.h>

std::string             ShaderBase::m_shader_header = "";
std::vector<void(*)()>  ShaderBase::m_all_kill_functions;

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
#if !defined(USE_GLES2)
    code << "#version " << CVS->getGLSLVersion()<<"\n";
#else
    if (CVS->isGLSL())
        code << "#version 300 es\n";
#endif

#if !defined(USE_GLES2)
    // Some drivers report that the compute shaders extension is available,
    // but they report only OpenGL 3.x version, and thus these extensions
    // must be enabled manually. Otherwise the shaders compilation will fail
    // because STK tries to use extensions which are available, but disabled
    // by default.
    if (type == GL_COMPUTE_SHADER)
    {
        if (CVS->isARBComputeShaderUsable())
            code << "#extension GL_ARB_compute_shader : enable\n";
        if (CVS->isARBImageLoadStoreUsable())
            code << "#extension GL_ARB_shader_image_load_store : enable\n";
        if (CVS->isARBArraysOfArraysUsable())
            code << "#extension GL_ARB_arrays_of_arrays : enable\n";
    }
#endif

    if (CVS->isAMDVertexShaderLayerUsable())
        code << "#extension GL_AMD_vertex_shader_layer : enable\n";

    if (CVS->isARBExplicitAttribLocationUsable())
        code << "#extension GL_ARB_explicit_attrib_location : enable\n";

    if (CVS->isAZDOEnabled())
    {
        code << "#extension GL_ARB_bindless_texture : enable\n";
        code << "#define Use_Bindless_Texture\n";
    }
    code << "//" << file << "\n";
    if (!CVS->isARBUniformBufferObjectUsable())
        code << "#define UBO_DISABLED\n";
    if (CVS->isAMDVertexShaderLayerUsable())
        code << "#define VSLayer\n";
    if (CVS->needsRGBBindlessWorkaround())
        code << "#define SRGBBindlessFix\n";

#if !defined(USE_GLES2)
    //shader compilation fails with some drivers if there is no precision qualifier
    if (type == GL_FRAGMENT_SHADER)
        code << "precision mediump float;\n";
#else
    int range[2], precision;
    glGetShaderPrecisionFormat(GL_FRAGMENT_SHADER, GL_HIGH_FLOAT, range, &precision);
    
    if (precision > 0)
        code << "precision highp float;\n";
    else
        code << "precision mediump float;\n";
#endif

    code << getHeader();

    std::ifstream stream(file_manager->getShader(file), std::ios::in);
    if (stream.is_open())
    {
        std::string Line = "";
        while (getline(stream, Line))
        {
            const std::string stk_include = "#stk_include";
            std::size_t pos = Line.find(stk_include);
            if (pos != std::string::npos)
            {
                std::size_t pos = Line.find("\"");
                if (pos == std::string::npos)
                {
                    Log::error("shader", "Invalid #stk_include line: '%s'.", Line.c_str());
                    continue;
                }

                std::string filename = Line.substr(pos+1);

                pos = filename.find("\"");
                if (pos == std::string::npos)
                {
                    Log::error("shader", "Invalid #stk_include line: '%s'.", Line.c_str());
                    continue;
                }

                filename = filename.substr(0, pos);

                std::ifstream include_stream(file_manager->getShader(filename), std::ios::in);
                if (!include_stream.is_open())
                {
                    Log::error("shader", "Couldn't open included shader: '%s'.", filename.c_str());
                    continue;
                }

                std::string include_line = "";
                while (getline(include_stream, include_line))
                {
                    code << "\n" << include_line;
                }

                include_stream.close();
            }
            else
            {
                code << "\n" << Line;
            }
        }

        stream.close();
    }
    else
    {
        Log::error("shader", "Can not open '%s'.", file.c_str());
    }

    Log::info("shader", "Compiling shader : %s", file.c_str());
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
        Log::error("GLWrap", "Error in shader %s", file.c_str());
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
    m_program = glCreateProgram();
    loadAndAttachShader(GL_VERTEX_SHADER, shader_name);
#ifdef USE_GLES2
    loadAndAttachShader(GL_FRAGMENT_SHADER, "tfb_dummy.frag");
#endif
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
    const float*  blue_SH_coeff = irr_driver->getSphericalHarmonics()->getBlueSHCoeff();
    glUniform1fv(bLmn, 9, blue_SH_coeff);

    GLint gLmn = glGetUniformLocation(m_program, "greenLmn[0]");
    const float*  green_SH_coeff = irr_driver->getSphericalHarmonics()->getGreenSHCoeff();
    glUniform1fv(gLmn, 9, green_SH_coeff);

    GLint rLmn = glGetUniformLocation(m_program, "redLmn[0]");
    const float*  red_SH_coeff = irr_driver->getSphericalHarmonics()->getRedSHCoeff();
    glUniform1fv(rLmn, 9, red_SH_coeff);

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
