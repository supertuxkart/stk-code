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

#ifndef HEADER_SHARED_SHADERS_HPP
#define HEADER_SHARED_SHADERS_HPP

#include "graphics/shared_shader_manager.hpp"
#include "utils/cpp2011.hpp"

#define GET_SS(SS) SharedShaderManager::getInstance()->getSharedShader<SS>()

// ============================================================================
// Common shaders:
class SharedObject : public SharedShader
{
public:
    // ------------------------------------------------------------------------
    virtual const char* getName() OVERRIDE       { return "object_pass.vert"; }
    // ------------------------------------------------------------------------
    virtual unsigned getShaderType()  OVERRIDE     { return GL_VERTEX_SHADER; }
};   // SharedObject

// ============================================================================
class SharedPass1 : public SharedShader
{
public:
    // ------------------------------------------------------------------------
    virtual const char* getName() OVERRIDE      { return "object_pass1.frag"; }
    // ------------------------------------------------------------------------
    virtual unsigned getShaderType()  OVERRIDE   { return GL_FRAGMENT_SHADER; }
};   // SharedPass1

// ============================================================================
class SharedSkinning : public SharedShader
{
public:
    // ------------------------------------------------------------------------
    virtual const char* getName() OVERRIDE          { return "skinning.vert"; }
    // ------------------------------------------------------------------------
    virtual unsigned getShaderType() OVERRIDE      { return GL_VERTEX_SHADER; }
};   // SharedSkinning

// ============================================================================
class SharedTransparent : public SharedShader
{
public:
    // ------------------------------------------------------------------------
    virtual const char* getName() OVERRIDE       { return "transparent.frag"; }
    // ------------------------------------------------------------------------
    virtual unsigned getShaderType() OVERRIDE    { return GL_FRAGMENT_SHADER; }
};   // SharedTransparent

// ============================================================================
class SharedTexturedQuad : public SharedShader
{
public:
    // ------------------------------------------------------------------------
    virtual const char* getName() OVERRIDE      { return "texturedquad.vert"; }
    // ------------------------------------------------------------------------
    virtual unsigned getShaderType() OVERRIDE      { return GL_VERTEX_SHADER; }
};   // SharedTexturedQuad

// ============================================================================
class SharedColoredQuad : public SharedShader
{
public:
    // ------------------------------------------------------------------------
    virtual const char* getName() OVERRIDE       { return "coloredquad.frag"; }
    // ------------------------------------------------------------------------
    virtual unsigned getShaderType() OVERRIDE    { return GL_FRAGMENT_SHADER; }
};   // SharedColoredQuad

// ============================================================================
class SharedSlicedScreenQuad : public SharedShader
{
public:
    // ------------------------------------------------------------------------
    virtual const char* getName() OVERRIDE  { return "slicedscreenquad.vert"; }
    // ------------------------------------------------------------------------
    virtual unsigned getShaderType() OVERRIDE      { return GL_VERTEX_SHADER; }
};   // SharedSlicedScreenQuad

// ============================================================================
class SharedScreenQuad : public SharedShader
{
public:
    // ------------------------------------------------------------------------
    virtual const char* getName() OVERRIDE        { return "screenquad.vert"; }
    // ------------------------------------------------------------------------
    virtual unsigned getShaderType() OVERRIDE      { return GL_VERTEX_SHADER; }
};   // SharedScreenQuad

// ============================================================================
// Instanced shaders:
class SharedInstanced : public SharedShader
{
public:
    // ------------------------------------------------------------------------
    virtual const char* getName() OVERRIDE
                                       { return "instanced_object_pass.vert"; }
    // ------------------------------------------------------------------------
    virtual unsigned getShaderType() OVERRIDE      { return GL_VERTEX_SHADER; }
};   // SharedInstanced

// ============================================================================
class SharedInstancedSkinning : public SharedShader
{
public:
    // ------------------------------------------------------------------------
    virtual const char* getName() OVERRIDE
                                          { return "instanced_skinning.vert"; }
    // ------------------------------------------------------------------------
    virtual unsigned getShaderType() OVERRIDE      { return GL_VERTEX_SHADER; }
};   // SharedInstancedSkinning

// ============================================================================
class SharedInstancedPass1 : public SharedShader
{
public:
    // ------------------------------------------------------------------------
    virtual const char* getName() OVERRIDE
                                      { return "instanced_object_pass1.frag"; }
    // ------------------------------------------------------------------------
    virtual unsigned getShaderType() OVERRIDE    { return GL_FRAGMENT_SHADER; }
};   // SharedInstancedPass1

// ============================================================================
class SharedInstancedPass2 : public SharedShader
{
public:
    // ------------------------------------------------------------------------
    virtual const char* getName() OVERRIDE
                                      { return "instanced_object_pass2.frag"; }
    // ------------------------------------------------------------------------
    virtual unsigned getShaderType() OVERRIDE    { return GL_FRAGMENT_SHADER; }
};   // SharedInstancedPass2

// ============================================================================
class SharedInstancedRefPass1 : public SharedShader
{
public:
    // ------------------------------------------------------------------------
    virtual const char* getName() OVERRIDE
                                   { return "instanced_objectref_pass1.frag"; }
    // ------------------------------------------------------------------------
    virtual unsigned getShaderType() OVERRIDE    { return GL_FRAGMENT_SHADER; }
};   // SharedInstancedRefPass1

// ============================================================================
class SharedInstancedRefPass2 : public SharedShader
{
public:
    // ------------------------------------------------------------------------
    virtual const char* getName() OVERRIDE
                                   { return "instanced_objectref_pass2.frag"; }
    // ------------------------------------------------------------------------
    virtual unsigned getShaderType() OVERRIDE    { return GL_FRAGMENT_SHADER; }
};   // SharedInstancedRefPass2

// ============================================================================
class SharedInstancedUnlit : public SharedShader
{
public:
    // ------------------------------------------------------------------------
    virtual const char* getName() OVERRIDE
                                      { return "instanced_object_unlit.frag"; }
    // ------------------------------------------------------------------------
    virtual unsigned getShaderType() OVERRIDE    { return GL_FRAGMENT_SHADER; }
};   // SharedInstancedUnlit

// ============================================================================
class SharedInstancedNormal : public SharedShader
{
public:
    // ------------------------------------------------------------------------
    virtual const char* getName() OVERRIDE
                                         { return "instanced_normalmap.frag"; }
    // ------------------------------------------------------------------------
    virtual unsigned getShaderType() OVERRIDE    { return GL_FRAGMENT_SHADER; }
};   // SharedInstancedNormal

// ============================================================================
class SharedInstancedGrass : public SharedShader
{
public:
    // ------------------------------------------------------------------------
    virtual const char* getName() OVERRIDE
                                             { return "instanced_grass.vert"; }
    // ------------------------------------------------------------------------
    virtual unsigned getShaderType() OVERRIDE      { return GL_VERTEX_SHADER; }
};   // SharedInstancedGrass

// ============================================================================
class SharedInstancedGrassPass2 : public SharedShader
{
public:
    // ------------------------------------------------------------------------
    virtual const char* getName() OVERRIDE
                                       { return "instanced_grass_pass2.frag"; }
    // ------------------------------------------------------------------------
    virtual unsigned getShaderType() OVERRIDE    { return GL_FRAGMENT_SHADER; }
};   // SharedInstancedGrassPass2

// ============================================================================
class SharedInstancedShadow : public SharedShader
{
public:
    // ------------------------------------------------------------------------
    virtual const char* getName() OVERRIDE  { return "instanciedshadow.vert"; }
    // ------------------------------------------------------------------------
    virtual unsigned getShaderType() OVERRIDE      { return GL_VERTEX_SHADER; }
};   // SharedInstancedShadow

// ============================================================================
class SharedInstancedSkinningShadow : public SharedShader
{
public:
    // ------------------------------------------------------------------------
    virtual const char* getName() OVERRIDE
                                   { return "instanced_skinning_shadow.vert"; }
    // ------------------------------------------------------------------------
    virtual unsigned getShaderType() OVERRIDE      { return GL_VERTEX_SHADER; }
};   // SharedInstancedSkinningShadow

#endif

#endif   // !SERVER_ONLY

