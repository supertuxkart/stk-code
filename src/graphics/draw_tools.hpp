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
/** Draw a mesh using specified shader (require OpenGL 3.2)
    *  \param mesh The mesh to draw
    *  \param args The shader uniforms values
    */
template<typename S, typename...Uniforms>
void draw(const GLMesh *mesh, Uniforms... args)
{
    irr_driver->IncreaseObjectCount(); //TODO: move somewhere else
    S::getInstance()->setUniforms(args...);
    glDrawElementsBaseVertex(mesh->PrimitiveType,
                             (int)mesh->IndexCount,
                             mesh->IndexType,
                             (GLvoid *)mesh->vaoOffset,
                             (int)mesh->vaoBaseVertex);
}   // draw

// ----------------------------------------------------------------------------
/** Variadic template to draw a mesh (using OpenGL 3.2 function)
 * with per mesh custom uniforms.*/
template<int...list>
struct CustomUnrollArgs;

// ----------------------------------------------------------------------------
template<int n, int...list>
struct CustomUnrollArgs<n, list...>
{
    /** Draw a mesh using specified shader (require OpenGL 3.2)
        *  \param cascade The cascade shadow map index
        *  \param t First tuple element is the mesh to draw, next elements are uniforms values
        *  \param args The shader uniforms values
        */  
    template<typename S,
             typename ...TupleTypes,
             typename ...Args>
    static void drawMesh(const STK::Tuple<TupleTypes...> &t,
                         Args... args)
    {
        CustomUnrollArgs<list...>::template drawMesh<S>(t, STK::tuple_get<n>(t), args...);
    }   // drawMesh
    
    // ----------------------------------------------------------------------------
    /** Draw shadow mesh using specified shader (require OpenGL 3.2)
        *  \param t First tuple element is the mesh to draw, next elements are uniforms values
        *  \param args The shader uniforms values
        */  
    template<typename S,
             typename ...TupleTypes,
             typename ...Args>
    static void drawShadow(unsigned cascade,
                           const STK::Tuple<TupleTypes...> &t,
                           Args... args)
    {
        CustomUnrollArgs<list...>::template drawShadow<S>(cascade, t, STK::tuple_get<n>(t), args...);
    }   // drawShadow
    
    // ----------------------------------------------------------------------------
    /** Draw mesh reflective shadow map using specified shader (require OpenGL 3.2)
        *  \param rsm_matrix The reflective shadow map matrix
        *  \param t First tuple element is the mesh to draw, next elements are uniforms values
        *  \param args The shader uniforms values
        */  
    template<typename S,
             typename ...TupleTypes,
             typename ...Args>
    static void drawReflectiveShadowMap(const irr::core::matrix4 &rsm_matrix,
                                        const STK::Tuple<TupleTypes...> &t,
                                        Args... args)
    {
        CustomUnrollArgs<list...>::template drawReflectiveShadowMap<S>(rsm_matrix, t, STK::tuple_get<n>(t), args...);
    }   // drawReflectiveShadowMap
};   // CustomUnrollArgs

// ----------------------------------------------------------------------------
/** Partial specialisation of CustomUnrollArgs to end the recursion */
template<>
struct CustomUnrollArgs<>
{ 
    template<typename S,
             typename ...TupleTypes,
             typename ...Args>
    static void drawMesh(const STK::Tuple<TupleTypes...> &t,
                         Args... args)
    {
        draw<S>(STK::tuple_get<0>(t), args...);
    }   // drawMesh
    
    // ---------------------------------------------------------------------------- 
    template<typename S,
             typename ...TupleTypes,
             typename ...Args>
    static void drawShadow(unsigned cascade,
                           const STK::Tuple<TupleTypes...> &t,
                           Args... args)
    {
        draw<S>(STK::tuple_get<0>(t), cascade, args...);
    }   // drawShadow
    
    // ---------------------------------------------------------------------------- 
    template<typename S,
             typename ...TupleTypes,
             typename ...Args>
    static void drawReflectiveShadowMap(const irr::core::matrix4 &rsm_matrix,
                                        const STK::Tuple<TupleTypes...> &t,
                                        Args... args)
    {
        draw<S>(STK::tuple_get<0>(t), rsm_matrix, args...);
    }   // drawReflectiveShadowMap
};   // CustomUnrollArgs



// ----------------------------------------------------------------------------
/** Variadic template to apply textures parameters.*/
template<typename T, int N>
struct TexExpander_impl
{
    template<typename...TupleArgs,
             typename... Args>
    static void ExpandTex(const GLMesh &mesh,
                          const STK::Tuple<TupleArgs...> &TexSwizzle,
                          Args... args)
    {
        size_t idx = STK::tuple_get<sizeof...(TupleArgs) - N>(TexSwizzle);
        TexExpander_impl<T, N - 1>::template
            ExpandTex(mesh, TexSwizzle, 
                      args..., getTextureGLuint(mesh.textures[idx]));
    }   // ExpandTex
};   // TexExpander_impl

// ----------------------------------------------------------------------------
template<typename T>
struct TexExpander_impl<T, 0>
{
    template<typename...TupleArgs, typename... Args>
    static void ExpandTex(const GLMesh &mesh,
                          const STK::Tuple<TupleArgs...> &TexSwizzle,
                          Args... args)
    {
        T::getInstance()->setTextureUnits(args...);
    }   // ExpandTex
};   // TexExpander_impl

// ----------------------------------------------------------------------------
template<typename T>
struct TexExpander
{
    template<typename...TupleArgs, typename... Args>
    static void ExpandTex(const GLMesh &mesh,
                          const STK::Tuple<TupleArgs...> &TexSwizzle,
                          Args... args)
    {
        TexExpander_impl<T, sizeof...(TupleArgs)>::ExpandTex(mesh, TexSwizzle,
                                                             args...);
    }   // ExpandTex
};   // TexExpander





// ----------------------------------------------------------------------------
template<typename T, int N>
struct HandleExpander_impl
{
    template<typename...TupleArgs, typename... Args>
    static void Expand(uint64_t *TextureHandles, 
                       const STK::Tuple<TupleArgs...> &TexSwizzle, Args... args)
    {
        size_t idx = STK::tuple_get<sizeof...(TupleArgs)-N>(TexSwizzle);
        HandleExpander_impl<T, N - 1>::template 
            Expand(TextureHandles, TexSwizzle, args..., TextureHandles[idx]);
    }   // Expand
};   // HandleExpander_impl

// ----------------------------------------------------------------------------
template<typename T>
struct HandleExpander_impl<T, 0>
{
    template<typename...TupleArgs, typename... Args>
    static void Expand(uint64_t *TextureHandles,
                       const STK::Tuple<TupleArgs...> &TexSwizzle, Args... args)
    {
        T::getInstance()->setTextureHandles(args...);
    }   // Expand
};   // HandleExpander_impl

// ----------------------------------------------------------------------------
template<typename T>
struct HandleExpander
{
    template<typename...TupleArgs, typename... Args>
    static void Expand(uint64_t *TextureHandles, const STK::Tuple<TupleArgs...> &TexSwizzle, Args... args)
    {
        HandleExpander_impl<T, sizeof...(TupleArgs)>::Expand(TextureHandles, TexSwizzle, args...);
    }   // Expand
};   // HandleExpander





#endif //HEADER_DRAW_TOOLS_HPP