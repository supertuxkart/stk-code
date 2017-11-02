//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2015 SuperTuxKart-Team
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

#ifndef HEADER_DRAW_TOOLS_HPP
#define HEADER_DRAW_TOOLS_HPP

#include "graphics/shader.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/stk_mesh.hpp"

// ----------------------------------------------------------------------------
/** Variadic template to draw a mesh (using OpenGL 3.2 function)
 * using specified shader with per mesh custom uniforms.*/
template<int...list>
struct CustomUnrollArgs;

// ----------------------------------------------------------------------------
template<int n, int...list>
struct CustomUnrollArgs<n, list...>
{
    /** Draw a mesh using specified shader (require OpenGL 3.2)
        *  \tparam S The shader to use.
        *  \param t First tuple element is the mesh to draw, next elements are per mesh uniforms values
        *  \param args Shader other uniforms values
        */  
    template<typename S,
             typename ...TupleTypes,
             typename ...Args>
    static void drawMesh(const std::tuple<TupleTypes...> &t,
                         Args... args)
    {
        CustomUnrollArgs<list...>::template drawMesh<S>(t, std::get<n>(t), args...);
    }   // drawMesh
    
};   // CustomUnrollArgs

// ----------------------------------------------------------------------------
/** Partial specialisation of CustomUnrollArgs to end the recursion */
template<>
struct CustomUnrollArgs<>
{ 
    template<typename S,
             typename ...TupleTypes,
             typename ...Args>
    static void drawMesh(const std::tuple<TupleTypes...> &t,
                         Args... args)
    {
        irr_driver->increaseObjectCount(); //TODO: move somewhere else
        GLMesh *mesh = std::get<0>(t);
        if (!mesh->mb->getMaterial().BackfaceCulling)
            glDisable(GL_CULL_FACE);
        S::getInstance()->setUniforms(args...);
        glDrawElementsBaseVertex(mesh->PrimitiveType,
                                (int)mesh->IndexCount,
                                mesh->IndexType,
                                (GLvoid *)mesh->vaoOffset,
                                (int)mesh->vaoBaseVertex);
        if (!mesh->mb->getMaterial().BackfaceCulling)
            glEnable(GL_CULL_FACE);
    }   // drawMesh
};   // CustomUnrollArgs

// ----------------------------------------------------------------------------
/** Variadic template to implement TexExpander*/
template<typename T, int N>
struct TexExpanderImpl
{
    template<typename...TupleArgs,
             typename... Args>
    static void expandTex(const GLMesh &mesh,
                          const std::tuple<TupleArgs...> &tex_swizzle,
                          Args... args)
    {
        size_t idx = std::get<sizeof...(TupleArgs) - N>(tex_swizzle);
        TexExpanderImpl<T, N - 1>::template expandTex(mesh, tex_swizzle,
            args..., mesh.textures[idx]->getOpenGLTextureName());
    }   // ExpandTex
};   // TexExpanderImpl

// ----------------------------------------------------------------------------
/** Partial specialisation of TexExpanderImpl to end the recursion */
template<typename T>
struct TexExpanderImpl<T, 0>
{
    template<typename...TupleArgs, typename... Args>
    static void expandTex(const GLMesh &mesh,
                          const std::tuple<TupleArgs...> &tex_swizzle,
                          Args... args)
    {
        T::getInstance()->setTextureUnits(args...);
    }   // ExpandTex
};   // TexExpanderImpl

// ----------------------------------------------------------------------------
template<typename T>
struct TexExpander
{
    /** Bind textures.
        *  \param mesh The mesh which owns the textures
        *  \param tex_swizzle Indices of texture id in mesh texture array
        *  \param args Other textures ids (each of them will be bound)
        */  
    template<typename...TupleArgs,
             typename... Args>
    static void expandTex(const GLMesh &mesh,
                          const std::tuple<TupleArgs...> &tex_swizzle,
                          Args... args)
    {
        TexExpanderImpl<T, sizeof...(TupleArgs)>::expandTex(mesh,
                                                            tex_swizzle,
                                                            args...);
    }   // ExpandTex
};   // TexExpander


// ----------------------------------------------------------------------------
/** Variadic template to implement HandleExpander*/
template<typename T, int N>
struct HandleExpanderImpl
{
    template<typename...TupleArgs, typename... Args>
    static void expand(uint64_t *texture_handles, 
                       const std::tuple<TupleArgs...> &tex_swizzle,
                       Args... args)
    {
        size_t idx = std::get<sizeof...(TupleArgs)-N>(tex_swizzle);
        HandleExpanderImpl<T, N - 1>::template expand(texture_handles,
                                                       tex_swizzle,
                                                       args...,
                                                       texture_handles[idx]);
    }   // Expand
};   // HandleExpanderImpl

// ----------------------------------------------------------------------------
/** Partial specialisation of TexExpanderImpl to end the recursion */
template<typename T>
struct HandleExpanderImpl<T, 0>
{
    template<typename...TupleArgs, typename... Args>
    static void expand(uint64_t *texture_handles,
                       const std::tuple<TupleArgs...> &tex_swizzle,
                       Args... args)
    {
        T::getInstance()->setTextureHandles(args...);
    }   // Expand
};   // HandleExpanderImpl

// ----------------------------------------------------------------------------
template<typename T>
struct HandleExpander
{
    /** Give acces to textures in shaders without first binding them
     * (require GL_ARB_bindless_texture extension) in order to reduce
     * driver overhead.
        *  \param texture_handles Array of handles
        *  \param tex_swizzle Indices of handles in textures_handles array
        *  \param args Other textures handles
        * (each of them will be accessible in shader)
        */  
    template<typename...TupleArgs,
             typename... Args>
    static void expand(uint64_t *texture_handles,
                       const std::tuple<TupleArgs...> &tex_swizzle,
                       Args... args)
    {
        HandleExpanderImpl<T, sizeof...(TupleArgs)>::expand(texture_handles,
                                                            tex_swizzle,
                                                            args...);
    }   // Expand
};   // HandleExpander

// ----------------------------------------------------------------------------
template <typename Shader>
inline void handleSkinning(Shader* s)
{
    if (SkinnedMeshShader* sms = dynamic_cast<SkinnedMeshShader*>(s))
    {
        sms->bindSkinningTexture();
    }
}

#endif //HEADER_DRAW_TOOLS_HPP
