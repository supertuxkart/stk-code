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
#include "config/user_config.hpp"
#include "config/stk_config.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/graphics_restrictions.hpp"
#include "guiengine/message_queue.hpp"
#include "io/file_manager.hpp"
#include "utils/file_utils.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"

#include <fstream>
#include <sstream>

#include <IFileSystem.h>

// ----------------------------------------------------------------------------
/** Returns a string with the content of header.txt (which contains basic
 *  shader defines).
 */
const std::string& ShaderFilesManager::getHeader()
{
    // Stores the content of header.txt, to avoid reading this file repeatedly.
    static std::string shader_header;

    // Only read file first time
    if (shader_header.empty())
    {
        std::ifstream stream(FileUtils::getPortableReadingPath(
            file_manager->getShader("header.txt")), std::ios::in);
        if (stream.is_open())
        {
            std::string line = "";
            while (std::getline(stream, line))
                shader_header += "\n" + line;
            stream.close();
        }
    }   // if shader_header.empty()

    return shader_header;
}   // getHeader

// ----------------------------------------------------------------------------
void ShaderFilesManager::readFile(const std::string& file, 
                                  std::ostringstream& code, bool not_header)
{
    std::string path = FileUtils::getPortableReadingPath(
        ((file.find('/') != std::string::npos ||
        file.find('\\') != std::string::npos) && not_header) ?
        file : file_manager->getShader(file));

    std::ifstream stream(path, std::ios::in);

    if (!stream.is_open())
    {
        Log::error("ShaderFilesManager", "Can not open '%s'.", file.c_str());
        return;
    }

    const std::string stk_include = "#stk_include";
    std::string line;

    while (std::getline(stream, line))
    {
        const std::size_t pos = line.find(stk_include);

        // load the custom file pointed by the #stk_include directive
        // we only look for #stk_include in official shader directory
        if (pos != std::string::npos)
        {
            // find the start "
            std::size_t pos = line.find("\"");
            if (pos == std::string::npos)
            {
                Log::error("ShaderFilesManager", "Invalid #stk_include"
                    " line: '%s'.", line.c_str());
                continue;
            }

            std::string filename = line.substr(pos + 1);

            // find the end "
            pos = filename.find("\"");
            if (pos == std::string::npos)
            {
                Log::error("ShaderFilesManager", "Invalid #stk_include"
                    " line: '%s'.", line.c_str());
                continue;
            }

            filename = filename.substr(0, pos);
            
            // read the whole include file
            readFile(filename, code, false/*not_header*/);
        }
        else
        {
            code << "\n" << line;
        }
    }

    stream.close();
}

// ----------------------------------------------------------------------------
/** Loads a single shader. This is NOT cached, use addShaderFile for that.
 *  \param file Filename of the shader to load.
 *  \param type Type of the shader.
 */
ShaderFilesManager::SharedShader ShaderFilesManager::loadShader
    (const std::string& full_path, unsigned type)
{
    GLuint* ss_ptr = new GLuint;
    *ss_ptr = glCreateShader(type);
    SharedShader ss(ss_ptr, [](GLuint* ss)
    {
        glDeleteShader(*ss);
        delete ss;
    });

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

    if (CVS->isARBExplicitAttribLocationUsable())
    {
#if !defined(USE_GLES2)
        code << "#extension GL_ARB_explicit_attrib_location : enable\n";
#endif
        code << "#define Explicit_Attrib_Location_Usable\n";
    }

    if (GraphicsRestrictions::isDisabled
        (GraphicsRestrictions::GR_CORRECT_10BIT_NORMALIZATION))
    {
        code << "#define Converts_10bit_Vector\n";
    }

    code << "//" << full_path << "\n";
    if (!CVS->isARBUniformBufferObjectUsable())
        code << "#define UBO_DISABLED\n";
    if (!CVS->isARBTextureBufferObjectUsable())
        code << "#define TBO_DISABLED\n";
    if (CVS->needsVertexIdWorkaround())
        code << "#define Needs_Vertex_Id_Workaround\n";
    if (CVS->isDeferredEnabled())
        code << "#define Advanced_Lighting_Enabled\n";

#if !defined(USE_GLES2)
    // shader compilation fails with some drivers if there is no precision
    // qualifier
    if (type == GL_FRAGMENT_SHADER)
        code << "precision highp float;\n";
#else
    int range[2], precision;
    glGetShaderPrecisionFormat(GL_FRAGMENT_SHADER, GL_HIGH_FLOAT, range,
        &precision);

    if (precision > 0)
    {
        code << "precision highp float;\n";
        code << "precision highp sampler2DArrayShadow;\n";
        code << "precision highp sampler2DArray;\n";
    }
    else
    {
        code << "precision mediump float;\n";
        code << "precision mediump sampler2DArrayShadow;\n";
        code << "precision mediump sampler2DArray;\n";
    }
#endif
    code << "#define MAX_BONES " << stk_config->m_max_skinning_bones << "\n";

    code << getHeader();

    readFile(full_path, code);

    Log::info("ShaderFilesManager", "Compiling shader: %s",
        full_path.c_str());
    const std::string &source  = code.str();
    char const *source_pointer = source.c_str();
    int len                    = (int)source.size();
    glShaderSource(*ss, 1, &source_pointer, &len);
    glCompileShader(*ss);

    GLint result = GL_FALSE;
    glGetShaderiv(*ss, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        // failed to compile
        int info_length;
        Log::error("ShaderFilesManager", "Error in shader %s",
            full_path.c_str());
        glGetShaderiv(*ss, GL_INFO_LOG_LENGTH, &info_length);
        if (info_length < 0)
            info_length = 1024;
        char *error_message = new char[info_length];
        error_message[0] = 0;
        glGetShaderInfoLog(*ss, info_length, NULL, error_message);
        Log::error("ShaderFilesManager", error_message);
        delete[] error_message;
        if (UserConfigParams::m_artist_debug_mode)
        {
            core::stringw err = StringUtils::insertValues(L"Shader file %s"
                " failed to compile, check stdout.log or console for details",
                full_path.c_str());
            MessageQueue::add(MessageQueue::MT_ERROR, err);
        }
        return NULL;
    }
    glGetError();

    return ss;
} // loadShader

// ----------------------------------------------------------------------------
/** Loads a single shader file, and add it to the loaded (cached) list
 *  \param file Filename of the shader to load.
 *  \param type Type of the shader.
 */
ShaderFilesManager::SharedShader ShaderFilesManager::addShaderFile
    (const std::string& full_path, unsigned type)
{
#ifdef DEBUG
    // Make sure no duplicated shader is added somewhere else
    auto i = m_shader_files_loaded.find(full_path);
    assert(i == m_shader_files_loaded.end());
#endif

    SharedShader ss = loadShader(full_path, type);
    m_shader_files_loaded[full_path] = ss;
    return ss;
}   // addShaderFile

// ----------------------------------------------------------------------------
/** Get a shader file. If the shader is not already in the cache it will be
 *  loaded and cached.
 *  \param file Filename of the shader to load.
 *  \param type Type of the shader.
 */
ShaderFilesManager::SharedShader ShaderFilesManager::getShaderFile
    (const std::string &file, unsigned type)
{
    const std::string full_path = (file.find('/') != std::string::npos ||
        file.find('\\') != std::string::npos) ?
        file : std::string(file_manager->getFileSystem()->getAbsolutePath
        (file_manager->getShadersDir().c_str()).c_str()) + file;
    // found in cache
    auto it = m_shader_files_loaded.find(full_path);
    if (it != m_shader_files_loaded.end())
        return it->second;

    // add to the cache now
    return addShaderFile(full_path, type);
}   // getShaderFile

#endif   // !SERVER_ONLY
