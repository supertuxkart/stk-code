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

#ifndef HEADER_SHADER_HPP
#define HEADER_SHADER_HPP

#include "graphics/gl_headers.hpp"
#include "utils/singleton.hpp"

#include <matrix4.h>
#include <SColor.h>
#include <vector3d.h>

#include <string>
#include <vector>

class ShaderBase
{
private:
    // Static members
    /** Stores the context of header.txt, to avoid reading
    *  this file repeatedly. */
    static std::string m_shader_header;

    /** Maintains a list of all shaders. */
    static std::vector<ShaderBase *> m_all_shaders;

    enum AttributeType
    {
        OBJECT,
        PARTICLES_SIM,
        PARTICLES_RENDERING,
    };

protected:
    /** OpenGL's program id. */
    GLuint m_program;

private:
    // ------------------------------------------------------------------------
    // Ends vararg template
    template<typename ... Types>
    void loadAndAttachShader(GLint ProgramID)
    {
        return;
    }   // loadAndAttachShader
    // ------------------------------------------------------------------------
    template<typename ... Types>
    void loadAndAttachShader(GLint shader_type, const std::string &name,
                             Types ... args)
    {
        GLint shader_id = loadShader(name, shader_type);
        glAttachShader(m_program, shader_id);
        glDeleteShader(shader_id);
        loadAndAttachShader(shader_type, args...);
    }   // loadAndAttachShader

    const std::string& getHeader();

    GLuint loadShader(const std::string &file, unsigned type);
    void setAttribute(AttributeType type);

public:
        ShaderBase();
    int loadTFBProgram(const std::string &vertex_file_path,
                       const char **varyings,
                       unsigned varyingscount);

    // ------------------------------------------------------------------------
    /** Activates the shader calling glUseProgram. */
    void use() { glUseProgram(m_program); }
};   // ShaderBase

// ============================================================================
template<typename T, typename... Args>
class Shader : public ShaderBase, public Singleton<T>
{
private:
    std::vector<GLuint> m_uniforms;

    /** Finds the specified uniform block and assigns a binding point to it. */
    void bindPoint(const char *name, int index)
    {
        GLuint block_index = glGetUniformBlockIndex(m_program, name);
        if (block_index != GL_INVALID_INDEX)
            glUniformBlockBinding(m_program, block_index, index);
    }   // bindPoint
    // ------------------------------------------------------------------------
    /** End of recursive implementation of assignUniforms. */
    void assignUniformsImpl()
    {
        bindPoint("MatrixData",   0);
        bindPoint("LightingData", 1);
    }   // assignUniformsImpl

    // ------------------------------------------------------------------------
    /** Recursive implementation of assignniforms. It collects the unfirom
     *  locations in m_uniform, then recursing.
     *  \param name Name of the uniform.
     */
    template<typename... U>
    void assignUniformsImpl(const char* name, U... rest)
    {
        m_uniforms.push_back(glGetUniformLocation(m_program, name));
        assignUniformsImpl(rest...);
    }   // assignUniformsImpl

    // ------------------------------------------------------------------------
    /** End of recursion for setUniforms implementation.
     */
    template<unsigned N = 0>
    void setUniformsImpl() const
    {
    }   // setUniformImpl
    // ------------------------------------------------------------------------
    /** Implementation for setUniforms for a matrix uniform. */
    template<unsigned N = 0, typename... Args>
    void setUniformsImpl(const irr::core::matrix4 &mat, Args... arg) const
    {
        glUniformMatrix4fv(m_uniforms[N], 1, GL_FALSE, mat.pointer());
        setUniformsImpl<N + 1>(arg...);
    }   // setUniformImpl

    // ------------------------------------------------------------------------
    template<unsigned N = 0, typename... Args>
    void setUniformsImpl(const irr::video::SColorf &col, Args... arg) const
    {
        glUniform3f(m_uniforms[N], col.r, col.g, col.b);
        setUniformsImpl<N + 1>(arg...);
    }  // setUniformsImpl

    // ------------------------------------------------------------------------
    template<unsigned N = 0, typename... Args>
    void setUniformsImpl(const irr::video::SColor &col, Args... arg) const
    {
        glUniform4i(m_uniforms[N], col.getRed(), c ol.getGreen(),
                                   col.getBlue(), col.getAlpha());
        setUniformsImpl<N + 1>(arg...);
    }   // setUniformsImpl

    // ------------------------------------------------------------------------
    template<unsigned N = 0, typename... Args>
    void setUniformsImpl(const irr::core::vector3df &v, Args... arg) const
    {
        glUniform3f(m_uniforms[N], v.X, v.Y, v.Z);
        setUniformsImpl<N + 1>(arg...);
    }   // setUniformsImpl


    // ------------------------------------------------------------------------
    template<unsigned N = 0, typename... Args>
    void setUniformsImpl(const irr::core::vector2df &v, Args... arg) const
    {
        glUniform2f(m_uniforms[N], v.X, v.Y);
        setUniformsImpl<N + 1>(arg...);
    }   // setUniformsImpl

    // ------------------------------------------------------------------------
    template<unsigned N = 0, typename... Args>
    void setUniformsImpl(const irr::core::dimension2df &v, Args... arg) const
    {
        glUniform2f(m_uniforms[N], v.Width, v.Height);
        setUniformsImpl<N + 1>(arg...);
    }   // setUniformsImpl

    // ------------------------------------------------------------------------
    template<unsigned N = 0, typename... Args>
    void setUniformsImpl(float f, Args... arg) const
    {
        glUniform1f(m_uniforms[N], f);
        setUniformsImpl<N + 1>(arg...);
    }   // setUniformsImpl

    // ------------------------------------------------------------------------
    template<unsigned N = 0, typename... Args>
    void setUniformsImpl(int f, Args... arg) const
    {
        glUniform1i(m_uniforms[N], f);
        setUniformsImpl<N + 1>(arg...);
    }   // setUniformsImpl

    // ------------------------------------------------------------------------
    template<unsigned N = 0, typename... Args>
    void setUniformsImpl(const std::vector<float> &v, Args... arg) const
    {
        glUniform1fv(m_uniforms[N], (int)v.size(), v.data());
        setUniformsImpl<N + 1>(arg...);
    }   // setUniformsImpl

public:
    // ------------------------------------------------------------------------
    /** This variadic template collects all names of uniforms in
     *  a std::vector. */
    template<typename... U>
    void assignUniforms(U... rest)
    {
        static_assert(sizeof...(rest) == sizeof...(Args),
                      "Count of Uniform's name mismatch");
        assignUniformsImpl(rest...);
    }   // assignUniforms

    // ------------------------------------------------------------------------
    void setUniforms(const Args & ... args) const
    {
        if (needsUBO())
            bypassUBO(m_program);
        setUniformsImpl(args...);
    }


};   // Shader

// ============================================================================
#endif
