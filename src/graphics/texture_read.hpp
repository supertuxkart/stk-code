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

#ifndef HEADER_TEXTURE_READ_HPP
#define HEADER_TEXTURE_READ_HPP

#include "shaders_util.hpp"

#include <functional>


enum SamplerTypeNew {
    ST_MIN,
    ST_NEAREST_FILTERED = ST_MIN,
    ST_TRILINEAR_ANISOTROPIC_FILTERED,
    ST_TRILINEAR_CUBEMAP,
    ST_MAX = ST_TRILINEAR_CUBEMAP,

    ST_SEMI_TRILINEAR,
    ST_BILINEAR_FILTERED,
    ST_BILINEAR_CLAMPED_FILTERED,
    ST_NEARED_CLAMPED_FILTERED,
    ST_SHADOW_SAMPLER,
    ST_VOLUME_LINEAR_FILTERED,
    ST_TRILINEAR_CLAMPED_ARRAY2D
};


class TextureReadBaseNew
{
public:
    typedef  std::function<void(unsigned, unsigned)> BindFunction;

protected:
    static void   bindTextureNearest(GLuint tex_unit, GLuint tex_id);
    static void   bindTextureBilinear(GLuint texture_unit, GLuint tex);
    static void   bindTextureBilinearClamped(GLuint tex_unit, GLuint tex);
    static void   bindTextureNearestClamped(GLuint tex_unit, GLuint tex_id);
    static void   bindTextureTrilinearAnisotropic(GLuint tex_unit, GLuint tex_id);
    static void   bindTextureSemiTrilinear(GLuint tex_unit, GLuint tex_id);
    static void   bindCubemapTrilinear(unsigned tex_unit, unsigned tex);

    GLuint        createSamplers(SamplerTypeNew sampler_type);
private:

    static GLuint createNearestSampler();
    static GLuint createTrilinearSampler();

protected:
    static BindFunction m_all_bind_functions[];
    std::vector<BindFunction> m_bind_functions;
    static GLuint m_all_texture_types[];

};   // TextureReadBaseNew

// ============================================================================

template<SamplerTypeNew...tp>
class TextureReadNew : public TextureReadBaseNew
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
    void assignTextureNamesImpl(GLuint)
    {
        static_assert(N == sizeof...(tp), "Wrong number of texture name");
    }   // assignTextureNamesImpl

    // ------------------------------------------------------------------------
    /** Recursive implementation, peeling the texture unit and name off the
     *  list of arguments.
     */
    template<unsigned N, typename...Args>
    void assignTextureNamesImpl(GLuint program, GLuint tex_unit,
                                const char *name, SamplerTypeNew sampler_type,
                                Args...args)
    {

        m_sampler_ids.push_back(createSamplers(sampler_type));

        assert(sampler_type >= ST_MIN && sampler_type <= ST_MAX);
        m_texture_type.push_back(m_all_texture_types[sampler_type]);

        GLuint location = glGetUniformLocation(program, name);
        m_texture_location.push_back(location);
        glUniform1i(location, tex_unit);
        m_texture_units.push_back(tex_unit);

        // Duplicated assert
        assert(sampler_type >= ST_MIN && sampler_type <= ST_MAX);
        m_bind_functions.push_back( m_all_bind_functions[sampler_type]);

        assignTextureNamesImpl<N + 1>(program, args...);
    }   // assignTextureNamesImpl

    // ------------------------------------------------------------------------
public:
    /** The protected interface for setting sampler names - it is only called
    *  from instances.
    */
    template<typename...Args>
    void assignSamplerNames(GLuint program, Args...args)
    {
        glUseProgram(program);
        assignTextureNamesImpl<0>(program, args...);
        glUseProgram(0);
    }   // AssignSamplerNames


    // Variadic template implementation of setTextureUnits
    // ===================================================
    /** End of recursion, just check if number of arguments is correct. */
    template<int N>
    void setTextureUnitsImpl()
    {
        static_assert(N == sizeof...(tp), "Not enough texture set");
    }   // setTextureUnitsImpl

    // ------------------------------------------------------------------------
    /** The recursive implementation.
     */
    template<int N, typename... TexIds>
    void setTextureUnitsImpl(GLuint texid, TexIds... args)
    {
        if (getGLSLVersion() >= 330)
        {
            setTextureSampler(m_texture_type[N], m_texture_units[N], texid,
                              m_sampler_ids[N]);
        }
        else
        {
            m_bind_functions[N](m_texture_units[N], texid);
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
        static_assert(N == sizeof...(tp), "Not enough handle set");
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
        if (handle)
            glUniformHandleui64ARB(m_texture_location[N], handle);
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
    ~TextureReadNew()
    {
        for (unsigned i = 0; i < m_sampler_ids.size(); i++)
            glDeleteSamplers(1, &m_sampler_ids[i]);
    }   // ~TextureReadNew


};   // class TextureReadNew

// ============================================================================
// ============================================================================
// ============================================================================
// ============================================================================
// ============================================================================
// ============================================================================
// ============================================================================
// ============================================================================
// ============================================================================









template<SamplerType...tp>
class TextureRead
{
protected:
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
    void assignTextureNamesImpl(GLuint)
    {
        static_assert(N == sizeof...(tp), "Wrong number of texture name");
    }   // assignTextureNamesImpl

    // ------------------------------------------------------------------------
    /** Recursive implementation, peeling the texture unit and name off the
     *  list of arguments.
     */
    template<unsigned N, typename...Args>
    void assignTextureNamesImpl(GLuint program, GLuint tex_unit, 
                                const char *name, Args...args)
    {
        GLuint location = glGetUniformLocation(program, name);
        m_texture_location.push_back(location);
        glUniform1i(location, tex_unit);
        m_texture_units.push_back(tex_unit);
        assignTextureNamesImpl<N + 1>(program, args...);
    }   // assignTextureNamesImpl

    // ------------------------------------------------------------------------
protected:
    /** The protected interface for setting sampler names - it is only called
    *  from instances.
    */
    template<typename...Args>
    void assignSamplerNames(GLuint program, Args...args)
    {
        CreateSamplers<tp...>::exec(m_sampler_ids, m_texture_type);

        glUseProgram(program);
        assignTextureNamesImpl<0>(program, args...);
        glUseProgram(0);
    }   // AssignSamplerNames


    // Variadic template implementation of setTextureUnits
    // ===================================================
    /** End of recursion, just check if number of arguments is correct. */
    template<int N>
    void setTextureUnitsImpl()
    {
        static_assert(N == sizeof...(tp), "Not enough texture set");
    }   // setTextureUnitsImpl

    // ------------------------------------------------------------------------
    /** The recursive implementation.
     */
    template<int N, typename... TexIds>
    void setTextureUnitsImpl(GLuint texid, TexIds... args)
    {
        setTextureSampler(m_texture_type[N], m_texture_units[N], texid, 
                          m_sampler_ids[N]);
        setTextureUnitsImpl<N + 1>(args...);
    }   // setTextureUnitsImpl

    // ------------------------------------------------------------------------
public:
    /** Public implementation of setTextureUnits.
     */
    template<typename... TexIds>
    void setTextureUnits(TexIds... args)
    {
        if (getGLSLVersion() >= 330)
            setTextureUnitsImpl<0>(args...);
        else
            BindTexture<tp...>::template exec<0>(m_texture_units, args...);
    }   // SetTextureUnits


    // ========================================================================
    // Variadic template implementation of setTextureHandles.

    /** End of recursion, just checks at compile time if number of arguments
     *  is correct.
     *  \param N number of arguments. */
    template<int N>
    void setTextureHandlesImpl()
    {
        static_assert(N == sizeof...(tp), "Not enough handle set");
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
        if (handle)
            glUniformHandleui64ARB(m_texture_location[N], handle);
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
    ~TextureRead()
    {
        for (unsigned i = 0; i < m_sampler_ids.size(); i++)
            glDeleteSamplers(1, &m_sampler_ids[i]);
    }   // ~TextureRead


};   // class TextureRead

#endif
