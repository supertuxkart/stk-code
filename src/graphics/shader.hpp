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

#ifndef HEADER_SHADER_HPP
#define HEADER_SHADER_HPP

#include "graphics/central_settings.hpp"
#include "graphics/gl_headers.hpp"
#include "graphics/shader_files_manager.hpp"
#include "graphics/shared_gpu_objects.hpp"
#include "utils/singleton.hpp"

#include <matrix4.h>
#include <SColor.h>
#include <vector3d.h>

#include <string>
#include <vector>

/** A simple non-templated base class. It is used to store some enums used in
 *  templates, the actual header for a shader, and a statis list of all kill
 *  functions (which delete all singletons, and therefore forces a reload of all
 *  shaders).
 *  It has some conventient templated functions to load a set of shaders.
 */
class ShaderBase
{
protected:
    /** Maintains a list of all shaders. */
    static std::vector<void (*)()> m_all_kill_functions;

    enum AttributeType
    {
        OBJECT,
        PARTICLES_SIM,
        PARTICLES_RENDERING,
        SKINNED_MESH,
    };   // AttributeType

    /** OpenGL's program id. */
    GLuint m_program;

    void bypassUBO() const;

    // ========================================================================
    /** Ends recursion. */
    template<typename ... Types>
    void loadAndAttachShader()
    {
        return;
    }   // loadAndAttachShader
    // ------------------------------------------------------------------------
    template<typename ... Types>
    void loadAndAttachShader(GLint shader_type, const std::string &name,
                             Types ... args)
    {
        GLint shader_id = ShaderFilesManager::getInstance()
            ->getShaderFile(name, shader_type);
        glAttachShader(m_program, shader_id);
        GLint is_deleted = GL_TRUE;
        glGetShaderiv(shader_id, GL_DELETE_STATUS, &is_deleted);
        if (is_deleted == GL_FALSE)
            glDeleteShader(shader_id);
        loadAndAttachShader(args...);
    }   // loadAndAttachShader
    // ------------------------------------------------------------------------
    /** Convenience interface using const char. */
    template<typename ... Types>
    void loadAndAttachShader(GLint shader_type, const char *name,
                             Types ... args)
    {
        loadAndAttachShader(shader_type, std::string(name), args...);
    }   // loadAndAttachShader
    // ------------------------------------------------------------------------
    void setAttribute(AttributeType type);

public:
        ShaderBase();
    int loadTFBProgram(const std::string &vertex_file_path,
                       const char **varyings,
                       unsigned varyingscount);
    static void updateShaders();
    GLuint createVAO();
    // ------------------------------------------------------------------------
    /** Activates the shader calling glUseProgram. */
    void use() { glUseProgram(m_program); }
    // ------------------------------------------------------------------------
    GLuint getUniformLocation(const char *name)
    {
        return glGetUniformLocation(m_program, name);
    }   // getUniformLocation
};   // ShaderBase

// ============================================================================
/** The main templated base class for all shaders that do not use textures.
 *  The template arguments are the types of the shader's uniforms. This allows
 *  compile time checks for the number of arguments when setting the name of
 *  the shader arguments.
 */
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


    // ========================================================================
    // assignUniforms: Variadic Template
protected:
    /** This variadic template collects all names of uniforms in
     *  a std::vector. It used assignUnfiromsImpl for the actual recursive
     *  implementation. */
    template<typename... U>
    void assignUniforms(U... rest)
    {
        static_assert(sizeof...(rest) == sizeof...(Args),
                      "Count of Uniform's name mismatch");
        assignUniformsImpl(rest...);
    }   // assignUniforms
private:
    // ------------------------------------------------------------------------
    /** End of recursive implementation of assignUniforms. */
    void assignUniformsImpl()
    {
        bindPoint("MatrixesData", 0);
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


    // ==============================================
    // setUniforms: Variadic template implementation.

public:
    /** Sets the uniforms for this shader. */
    void setUniforms(const Args & ... args) const
    {
        if (!CVS->isARBUniformBufferObjectUsable())
            bypassUBO();
        setUniformsImpl(args...);
    }   // setUniforms
    // ------------------------------------------------------------------------
private:
    /** Implementation for setUniforms for a vector<float> uniform. */
    template<unsigned N = 0, typename... Args1>
    void setUniformsImpl(const std::vector<float> &v, Args1... arg) const
    {
        glUniform1fv(m_uniforms[N], (int)v.size(), v.data());
        setUniformsImpl<N + 1>(arg...);
    }   // setUniformsImpl

    // ------------------------------------------------------------------------
    /** End of recursion for setUniforms implementation.
     */
    template<unsigned N = 0>
    void setUniformsImpl() const
    {
    }   // setUniformImpl

    // ------------------------------------------------------------------------
    /** Implementation for setUniforms for a matrix uniform. */
    template<unsigned N = 0, typename... Args1>
    void setUniformsImpl(const irr::core::matrix4 &mat, Args1... arg) const
    {
        glUniformMatrix4fv(m_uniforms[N], 1, GL_FALSE, mat.pointer());
        setUniformsImpl<N + 1>(arg...);
    }   // setUniformImpl

    // ------------------------------------------------------------------------
    /** Implementation for setUniforms for a matrix SColorF values. */
    template<unsigned N = 0, typename... Args1>
    void setUniformsImpl(const irr::video::SColorf &col, Args1... arg) const
    {
        glUniform3f(m_uniforms[N], col.r, col.g, col.b);
        setUniformsImpl<N + 1>(arg...);
    }  // setUniformsImpl

    // ------------------------------------------------------------------------
    /** Implementation for setUniforms for a SColor uniform. */
    template<unsigned N = 0, typename... Args1>
    void setUniformsImpl(const irr::video::SColor &col, Args1... arg) const
    {
        glUniform4i(m_uniforms[N], col.getRed(), col.getGreen(),
                                   col.getBlue(), col.getAlpha());
        setUniformsImpl<N + 1>(arg...);
    }   // setUniformsImpl

    // ------------------------------------------------------------------------
    /** Implementation for setUniforms for a vector3df uniform. */
    template<unsigned N = 0, typename... Args1>
    void setUniformsImpl(const irr::core::vector3df &v, Args1... arg) const
    {
        glUniform3f(m_uniforms[N], v.X, v.Y, v.Z);
        setUniformsImpl<N + 1>(arg...);
    }   // setUniformsImpl

    // ------------------------------------------------------------------------
    /** Implementation for setUniforms for a vector2df uniform. */
    template<unsigned N = 0, typename... Args1>
    void setUniformsImpl(const irr::core::vector2df &v, Args1... arg) const
    {
        glUniform2f(m_uniforms[N], v.X, v.Y);
        setUniformsImpl<N + 1>(arg...);
    }   // setUniformsImpl

    // ------------------------------------------------------------------------
    /** Implementation for setUniforms for a dimension2df uniform. */
    template<unsigned N = 0, typename... Args1>
    void setUniformsImpl(const irr::core::dimension2df &v, Args1... arg) const
    {
        glUniform2f(m_uniforms[N], v.Width, v.Height);
        setUniformsImpl<N + 1>(arg...);
    }   // setUniformsImpl

    // ------------------------------------------------------------------------
    /** Implementation for setUniforms for a float uniform. */
    template<unsigned N = 0, typename... Args1>
    void setUniformsImpl(float f, Args1... arg) const
    {
        glUniform1f(m_uniforms[N], f);
        setUniformsImpl<N + 1>(arg...);
    }   // setUniformsImpl

    // ------------------------------------------------------------------------
    /** Implementation for setUniforms for an int uniform. */
    template<unsigned N = 0, typename... Args1>
    void setUniformsImpl(int f, Args1... arg) const
    {
        glUniform1i(m_uniforms[N], f);
        setUniformsImpl<N + 1>(arg...);
    }   // setUniformsImpl


    // printFileList: Variadic template for printing a list of shader filenames
    // ========================================================================
    /** Variadic template to print a list of file names.
    *  \param shader_type Ignored (used since the variadic calling function
    *         has this parameter).
    *  \param filepath Name of the file to print.
    */
protected:
    template<typename ...Types>
    void printFileList(GLint shader_type, const char *filepath, Types ... args)
    {
        Log::error("shader", filepath);
        printFileList(args...);
    }   // printFileList

    // ------------------------------------------------------------------------
    /** End recursion for variadic template. */
private:
    template<typename ...Types>
    void printFileList()
    {
        return;
    }   // printFileList


    // Variadic template implementation of assignTextureUnit
    // ========================================================================
public:
    /** Variadic top level/public interface. It does the calls to glUseProgram
     *  and uses assignTextureUnitNoUse() in recursion to avoid unnecessary
     *  calls to glUseProgram.
     *  \param index Index of the texture.
     *  \param uniform Uniform name.
     */
    template<typename... T1>
    void assignTextureUnit(GLuint index, const char* uniform, T1... rest)
    {
        glUseProgram(m_program);
        GLuint uniform_loc = glGetUniformLocation(m_program, uniform);
        glUniform1i(uniform_loc, index);
        // Avoid doing any additional glUseProgram for the remaining calls
        assignTextureUnitNoUse(rest...);
        glUseProgram(0);
    }   // assignTextureUnit

private:
    // ------------------------------------------------------------------------
    /** End of recursion. */
    void assignTextureUnitNoUse() {}

    // ------------------------------------------------------------------------
    /** Recursive implementation of assignTextureUnit, but without the call
     *  to gluseProgram (which is done by the public interface). */
    template<typename... T1>
    void assignTextureUnitNoUse(GLuint index, const char* uniform, T1... rest)
    {
        GLuint uniform_loc = glGetUniformLocation(m_program, uniform);
        glUniform1i(uniform_loc, index);
        assignTextureUnitNoUse(rest...);
    }   // assignTextureUnitNoUse

    // ========================================================================

public:

    /** Constructor. Adds the static kill function of this shader to the list
     *  of all kill function, which is used for the debugging feature of
     *  reloading all shaders.
     */
    Shader()
    {
        m_all_kill_functions.push_back(this->kill);
    }   // Shader

    // ------------------------------------------------------------------------
    /** Load a list of shaders and links them all together.
     */
    template<typename ... Types>
    void loadProgram(AttributeType type, Types ... args)
    {
        m_program = glCreateProgram();
        loadAndAttachShader(args...);
        if (!CVS->isARBExplicitAttribLocationUsable())
            setAttribute(type);
        glLinkProgram(m_program);

        GLint Result = GL_FALSE;
        glGetProgramiv(m_program, GL_LINK_STATUS, &Result);
        if (Result == GL_FALSE) {
            int info_length;
            Log::error("Shader", "Error when linking these shaders :");
            printFileList(args...);
            glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &info_length);
            char *error_message = new char[info_length];
            glGetProgramInfoLog(m_program, info_length, NULL, error_message);
            Log::error("Shader", error_message);
            delete[] error_message;
        }
    }   // loadProgram

    // ------------------------------------------------------------------------
    virtual void bindCustomTextures() {}
    // ------------------------------------------------------------------------
    void drawFullScreenEffect(Args...args)
    {
        use();
        glBindVertexArray(SharedGPUObjects::getFullScreenQuadVAO());
        setUniforms(args...);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }   // drawFullScreenEffect

};   // Shader

// ============================================================================
class SkinnedMeshShader
{
private:
    GLuint m_skinning_tex_location;
public:
    SkinnedMeshShader() : m_skinning_tex_location(0) {}
    // ------------------------------------------------------------------------
    template <typename Shader>
    void init(Shader* s)
    {
        s->use();
        m_skinning_tex_location = s->getUniformLocation("skinning_tex");
        glUniform1i(m_skinning_tex_location, 15);
    }
    // ------------------------------------------------------------------------
    void bindSkinningTexture()
    {
        glActiveTexture(GL_TEXTURE0 + 15);
#ifdef USE_GLES2
        glBindTexture(GL_TEXTURE_2D, SharedGPUObjects::getSkinningTexture());
#else
        glBindTexture(GL_TEXTURE_BUFFER,
            SharedGPUObjects::getSkinningTexture());
#endif
        glBindSampler(15, 0);
    }
};


#endif

#endif   // !SERVER_ONLY

