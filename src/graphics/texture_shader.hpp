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

#ifndef SHADER_ONLY

#ifndef HEADER_TEXTURE_SHADER_HPP
#define HEADER_TEXTURE_SHADER_HPP

#include "graphics/central_settings.hpp"
#include "graphics/gl_headers.hpp"
#include "graphics/shader.hpp"
#include "utils/cpp2011.hpp"

#include <assert.h>
#include <functional>
#include <vector>


enum SamplerTypeNew
{
    ST_MIN,
    ST_NEAREST_FILTERED = ST_MIN,
    ST_TRILINEAR_ANISOTROPIC_FILTERED,
    ST_TRILINEAR_CUBEMAP,
    ST_BILINEAR_FILTERED,
    ST_SHADOW_SAMPLER,
    ST_TRILINEAR_CLAMPED_ARRAY2D,
    ST_VOLUME_LINEAR_FILTERED,
    ST_NEARED_CLAMPED_FILTERED,
    ST_BILINEAR_CLAMPED_FILTERED,
    ST_SEMI_TRILINEAR,
#ifdef USE_GLES2
    ST_MAX = ST_SEMI_TRILINEAR
#else
    ST_TEXTURE_BUFFER,
    ST_MAX = ST_TEXTURE_BUFFER
#endif
};   // SamplerTypeNew

// ============================================================================
/** A simple non-templated base class for a shader that uses textures. A non
 *  templated base class is necessary to easily handle static objects (like
 *  list of all bind functions to call) - with templates each instance is a
 *  different class (with different static values).
 */
class TextureShaderBase
{
public:
    typedef  std::function<void(GLuint, GLuint)> BindFunction;

protected:
    static void   bindTextureNearest(GLuint tex_unit, GLuint tex_id);
    static void   bindTextureBilinear(GLuint texture_unit, GLuint tex_id);
    static void   bindTextureBilinearClamped(GLuint tex_unit, GLuint tex_id);
    static void   bindTextureNearestClamped(GLuint tex_unit, GLuint tex_id);
    static void   bindTextureTrilinearAnisotropic(GLuint tex_unit, GLuint tex_id);
    static void   bindTextureSemiTrilinear(GLuint tex_unit, GLuint tex_id);
    static void   bindCubemapTrilinear(GLuint tex_unit, GLuint tex_id);
    static void   bindTextureShadow(GLuint tex_unit, GLuint tex_id);
    static void   bindTrilinearClampedArrayTexture(GLuint tex_unit, GLuint tex_id);
    static void   bindTextureVolume(GLuint tex_unit, GLuint tex_id);
    static void   bindTextureBuffer(GLuint tex_unit, GLuint tex_id);

    GLuint        createSamplers(SamplerTypeNew sampler_type);
private:

    static GLuint createNearestSampler();
    static GLuint createTrilinearSampler();
    static GLuint createBilinearSampler();
    static GLuint createShadowSampler();
    static GLuint createTrilinearClampedArray();
    static GLuint createBilinearClampedSampler();
    static GLuint createSemiTrilinearSampler();
protected:
    static BindFunction m_all_bind_functions[];
    std::vector<BindFunction> m_bind_functions;
    static GLuint m_all_texture_types[];
};   // TextureshaderBase

// ========================================================================
/** Class C needs to be the newly declared shaders class (necessary for
 *  the instance template). NUM_TEXTURES is the number of texture units
 *  used in this shader. It is used to test at compile time that the
 *  right number of arguments are supplied to the variadic functions.
 */
template<class C, int NUM_TEXTURES, typename...tp>
class TextureShader : public TextureShaderBase
                    , public Shader<C, tp...>
{

private:

    std::vector<GLuint> m_texture_units;
    std::vector<GLenum> m_texture_type;
    std::vector<GLenum> m_texture_location;

public:
    std::vector<GLuint> m_sampler_ids;

    // A variadic template to assign texture names
    // ===========================================
private:
    /** End of recursive variadic template AssigTextureNames. It just
     *  checks if the number of arguments is correct.*/
    template<unsigned N, typename...Args>
    void assignTextureNamesImpl()
    {
        static_assert(N == NUM_TEXTURES, "Wrong number of texture names");
    }   // assignTextureNamesImpl

    // ------------------------------------------------------------------------
    /** Recursive implementation, peeling the texture unit and name off the
     *  list of arguments.
     */
    template<unsigned N, typename...Args>
    void assignTextureNamesImpl(GLuint tex_unit, const char *name,
                                SamplerTypeNew sampler_type, Args...args)
    {

        m_sampler_ids.push_back(createSamplers(sampler_type));

        assert(sampler_type >= ST_MIN && sampler_type <= ST_MAX);
        m_texture_type.push_back(m_all_texture_types[sampler_type]);

        GLuint location = this->getUniformLocation(name);
        m_texture_location.push_back(location);
        glUniform1i(location, tex_unit);
        m_texture_units.push_back(tex_unit);

        // Duplicated assert
        assert(sampler_type >= ST_MIN && sampler_type <= ST_MAX);
        m_bind_functions.push_back( m_all_bind_functions[sampler_type]);

        assignTextureNamesImpl<N + 1>(args...);
    }   // assignTextureNamesImpl

    // ------------------------------------------------------------------------
public:
    /** The protected interface for setting sampler names - it is only called
    *  from instances.
    */
    template<typename...Args>
    void assignSamplerNames(Args...args)
    {
        this->use();
        assignTextureNamesImpl<0>(args...);
        glUseProgram(0);
    }   // AssignSamplerNames


    // Variadic template implementation of setTextureUnits
    // ===================================================
    /** End of recursion, just check if number of arguments is correct. */
    template<int N>
    void setTextureUnitsImpl()
    {
        static_assert(N == NUM_TEXTURES, "Not enough texture set");
    }   // setTextureUnitsImpl

    // ------------------------------------------------------------------------
    /** The recursive implementation.
     */
    template<int N, typename... TexIds>
    void setTextureUnitsImpl(GLuint tex_id, TexIds... args)
    {
        if (CVS->isARBSamplerObjectsUsable())
        {
            glActiveTexture(GL_TEXTURE0 + m_texture_units[N]);
            glBindTexture(m_texture_type[N], tex_id);
            glBindSampler(m_texture_units[N], m_sampler_ids[N]);
        }
        else
        {
            m_bind_functions[N](m_texture_units[N], tex_id);
        }
        setTextureUnitsImpl<N + 1>(args...);
    }   // setTextureUnitsImpl

    // ------------------------------------------------------------------------
public:
    /** Public implementation of setTextureUnits.
     */
    template<typename... TexIds>
    void setTextureUnits(TexIds... args)
    {
        setTextureUnitsImpl<0>(args...);
    }   // setTextureUnits


    // ========================================================================
    // Variadic template implementation of setTextureHandles.

    /** End of recursion, just checks at compile time if number of arguments
     *  is correct.
     *  \param N number of arguments. */
    template<int N>
    void setTextureHandlesImpl()
    {
        static_assert(N == NUM_TEXTURES, "Not enough handles set");
    }   // setTextureHandlesImpl

    // ------------------------------------------------------------------------
    /** Recursive implementation of setTextureHandles.
     * \param N The number of the current argument in the recursion (or the
     *          recursion depth).
     *  \param handle The texture handle to set.
     */
    template<int N, typename... HandlesId>
    void setTextureHandlesImpl(uint64_t handle, HandlesId... args)
    {
#if !defined(USE_GLES2)
        if (handle)
            glUniformHandleui64ARB(m_texture_location[N], handle);
#endif
        setTextureHandlesImpl<N + 1>(args...);
    }   // setTextureHandlesImpl

    // ------------------------------------------------------------------------
public:
    /** The protected interface.
     *  \param ids The ids of all texture handls.
     */
    template<typename... HandlesId>
    void setTextureHandles(HandlesId... ids)
    {
        setTextureHandlesImpl<0>(ids...);
    }   // SetTextureHandles

public:
    // ------------------------------------------------------------------------
    /** Destructor which frees al lsampler ids.
     */
    ~TextureShader()
    {
        for (unsigned i = 0; i < m_sampler_ids.size(); i++)
            glDeleteSamplers(1, &m_sampler_ids[i]);
    }   // ~TextureShader

};   // class TextureShader

#endif

#endif   // SHADER_ONLY
