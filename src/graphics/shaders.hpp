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

#ifndef HEADER_SHADERS_HPP
#define HEADER_SHADERS_HPP

#include "graphics/shader.hpp"
#include "graphics/shared_gpu_objects.hpp"
#include "graphics/texture_shader.hpp"

#include <IMeshSceneNode.h>
#include <IShaderConstantSetCallBack.h>
#include <EMaterialTypes.h>


using namespace irr;
class ParticleSystemProxy;

#define FOREACH_SHADER(ACT) \
    ACT(ES_NORMAL_MAP) \
    ACT(ES_NORMAL_MAP_LIGHTMAP) \
    ACT(ES_SKYBOX) \
    ACT(ES_SPLATTING) \
    ACT(ES_WATER) \
    ACT(ES_WATER_SURFACE) \
    ACT(ES_SPHERE_MAP) \
    ACT(ES_GRASS) \
    ACT(ES_GRASS_REF) \
    ACT(ES_MOTIONBLUR) \
    ACT(ES_GAUSSIAN3H) \
    ACT(ES_GAUSSIAN3V) \
    ACT(ES_MIPVIZ) \
    ACT(ES_OBJECT_UNLIT) \
    ACT(ES_OBJECTPASS) \
    ACT(ES_OBJECTPASS_REF) \
    ACT(ES_OBJECTPASS_RIMLIT) \
    ACT(ES_DISPLACE) \

#define ENUM(a) a,

enum ShaderType
{
    FOREACH_SHADER(ENUM)
    ES_COUNT
};
#undef ENUM


class Shaders
{
private:
    static const char *shader_names[];
    static bool m_has_been_initialised;

    static int m_shaders[ES_COUNT];

    static video::IShaderConstantSetCallBack *m_callbacks[ES_COUNT];

    static void check(const int num);
    static void loadShaders();
public:
    static void init();
    static void destroy();
    // ------------------------------------------------------------------------
    /** Returns the material type of a shader. 
     *  \param num The shader type.
     */
    static video::E_MATERIAL_TYPE getShader(const ShaderType num)
    {
        assert(m_has_been_initialised);
        assert(num < ES_COUNT);
        return (video::E_MATERIAL_TYPE)m_shaders[num];
    }   // getShader

    // ------------------------------------------------------------------------
    /** Returns the callback for the specified shader type.
     */
    static video::IShaderConstantSetCallBack* getCallback(const ShaderType num)
    {
        return m_has_been_initialised ? m_callbacks[num] : NULL;
    }   // getCallback

    // ========================================================================
    /** Shader to draw a colored line.
     */
    class ColoredLine : public Shader<ColoredLine, video::SColor>
    {
    private:
        GLuint m_vao, m_vbo;
    public:
        ColoredLine();

        // --------------------------------------------------------------------
        /** Bind the vertex array of this shader. */
        void bindVertexArray()
        {
            glBindVertexArray(m_vao);
        }   // bindVertexArray
        // --------------------------------------------------------------------
        /** Binds the vbo of this shader. */
        void bindBuffer()
        {
            glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        }   // bindBuffer
    };   // class ColoredLine

    // ========================================================================
    class TransparentShader : public TextureShader<TransparentShader, 1,
                                                   core::matrix4, core::vector2df,
                                                   float >
    {
    public:
        TransparentShader();
    };   // TransparentShader

    // ========================================================================
    class TransparentFogShader : public TextureShader<TransparentFogShader, 1,
                                     core::matrix4, core::vector2df, float, float,
                                     float, float, float, video::SColorf >
    {
    public:
        TransparentFogShader();
    };   // TransparentFogShader
    // ========================================================================
    class SkinnedTransparentShader : public TextureShader<SkinnedTransparentShader, 1,
                                                 core::matrix4, core::vector2df,
                                                 int, float >,
                                     public SkinnedMeshShader
    {
    public:
        SkinnedTransparentShader();
    };   // SkinnedTransparentShader

    // ========================================================================


};   // class Shaders

#endif

#endif   // SHADER_ONLY
