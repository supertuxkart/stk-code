//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2016 SuperTuxKart-Team
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

#include "graphics/shader_files_manager.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/shared_gpu_objects.hpp"
#include "io/file_manager.hpp"
#include "utils/log.hpp"

#include <fstream>
#include <sstream>

std::string ShaderFilesManager::m_shader_header = "";

// ----------------------------------------------------------------------------
/** Returns a string with the content of header.txt (which contains basic
 *  shader defines).
 */
const std::string& ShaderFilesManager::getHeader()
{
    // Only read file first time
    if (m_shader_header.empty())
    {
        std::ifstream stream(file_manager->getShader("header.txt"),
            std::ios::in);
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
/** Loads a single shader file, and add it to the loaded list
 *  \param file Filename of the shader to load.
 *  \param type Type of the shader.
 */
GLuint ShaderFilesManager::addShaderFile(const std::string &file,
                                         unsigned type)
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
    // shader compilation fails with some drivers if there is no precision
    // qualifier
    if (type == GL_FRAGMENT_SHADER)
        code << "precision mediump float;\n";
#else
    int range[2], precision;
    glGetShaderPrecisionFormat(GL_FRAGMENT_SHADER, GL_HIGH_FLOAT, range,
        &precision);

    if (precision > 0)
        code << "precision highp float;\n";
    else
        code << "precision mediump float;\n";
#endif
    code << "#define MAX_BONES " << SharedGPUObjects::getMaxMat4Size() << "\n";

    code << getHeader();

    std::ifstream stream(file_manager->getShader(file), std::ios::in);
    if (stream.is_open())
    {
        std::string line = "";
        while (getline(stream, line))
        {
            const std::string stk_include = "#stk_include";
            std::size_t pos = line.find(stk_include);
            if (pos != std::string::npos)
            {
                std::size_t pos = line.find("\"");
                if (pos == std::string::npos)
                {
                    Log::error("ShaderFilesManager", "Invalid #stk_include"
                        " line: '%s'.", line.c_str());
                    continue;
                }

                std::string filename = line.substr(pos + 1);

                pos = filename.find("\"");
                if (pos == std::string::npos)
                {
                    Log::error("ShaderFilesManager", "Invalid #stk_include"
                        " line: '%s'.", line.c_str());
                    continue;
                }

                filename = filename.substr(0, pos);

                std::ifstream include_stream(file_manager->getShader(filename),
                    std::ios::in);
                if (!include_stream.is_open())
                {
                    Log::error("ShaderFilesManager", "Couldn't open included"
                        " shader: '%s'.", filename.c_str());
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
                code << "\n" << line;
            }
        }

        stream.close();
    }
    else
    {
        Log::error("ShaderFilesManager", "Can not open '%s'.", file.c_str());
    }

    Log::info("ShaderFilesManager", "Compiling shader : %s", file.c_str());
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
        Log::error("ShaderFilesManager", "Error in shader %s", file.c_str());
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &info_length);
        if (info_length < 0)
            info_length = 1024;
        char *error_message = new char[info_length];
        error_message[0] = 0;
        glGetShaderInfoLog(id, info_length, NULL, error_message);
        Log::error("ShaderFilesManager", error_message);
        delete[] error_message;
    }

    glGetError();
    m_shader_files_loaded[file] = id;
    return id;

}   // addShaderFile

// ----------------------------------------------------------------------------
GLuint ShaderFilesManager::getShaderFile(const std::string &file,
                                       unsigned type)
{
    std::unordered_map<std::string, GLuint>::const_iterator i =
        m_shader_files_loaded.find(file);
    if (i != m_shader_files_loaded.end())
        return i->second;
    else
        return addShaderFile(file, type);

}   // getShaderFile

#endif   // !SERVER_ONLY
