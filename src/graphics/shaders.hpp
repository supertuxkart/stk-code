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

#ifndef HEADER_SHADERS_HPP
#define HEADER_SHADERS_HPP

#include "config/user_config.hpp"
#include "graphics/shader.hpp"
#include "graphics/shared_gpu_objects.hpp"
#include "graphics/texture_shader.hpp"

#include <IMeshSceneNode.h>
#include <IShaderConstantSetCallBack.h>
#include <EMaterialTypes.h>


using namespace irr;
class ParticleSystemProxy;


namespace MeshShader
{
class ObjectPass1Shader : public TextureShader<ObjectPass1Shader, 1,
                                             core::matrix4, core::matrix4>
{
public:
    ObjectPass1Shader();
};



class ObjectPass2Shader : public TextureShader<ObjectPass2Shader, 5, core::matrix4, core::matrix4>
{
public:
    ObjectPass2Shader();
};




class TransparentShader : public TextureShader<TransparentShader, 1, core::matrix4, core::matrix4>
{
public:
    TransparentShader();
};

class TransparentFogShader : public TextureShader<TransparentFogShader, 1, core::matrix4, core::matrix4, float, float, 
                                           float, float, float, video::SColorf>
{
public:
    TransparentFogShader();
};

}



template<typename T, typename... Args>
static void DrawFullScreenEffect(Args...args)
{
    T::getInstance()->use();
    glBindVertexArray(SharedGPUObjects::getFullScreenQuadVAO());
    T::getInstance()->setUniforms(args...);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

namespace FullScreenShader
{


class SunLightShader : public TextureShader<SunLightShader, 2,  core::vector3df, video::SColorf>
{
public:
    SunLightShader();
};

class LightspaceBoundingBoxShader : public TextureShader<LightspaceBoundingBoxShader, 1,
                                                  core::matrix4, float, float,
                                                  float, float>
{
public:
    LightspaceBoundingBoxShader();
};

class ShadowMatrixesGenerationShader : public Shader <ShadowMatrixesGenerationShader, core::matrix4>
{
public:
    ShadowMatrixesGenerationShader();
};

class DepthHistogramShader : public TextureShader<DepthHistogramShader, 1>
{
public:
    DepthHistogramShader();
};

class GlowShader : public TextureShader<GlowShader, 1>
{
public:
    GLuint vao;

    GlowShader();
};

class SSAOShader : public TextureShader<SSAOShader, 1, float, float, float>
{
public:
    SSAOShader();
};

class FogShader : public TextureShader<FogShader, 1, float, core::vector3df>
{
public:
    FogShader();
};

class MotionBlurShader : public TextureShader<MotionBlurShader, 2, core::matrix4, core::vector2df, float, float>
{
public:
    MotionBlurShader();
};

class GodFadeShader : public TextureShader<GodFadeShader, 1, video::SColorf>
{
public:
    GodFadeShader();
};

class GodRayShader : public TextureShader<GodRayShader, 1, core::vector2df>
{
public:
    GodRayShader();
};

class MLAAColorEdgeDetectionSHader : public TextureShader<MLAAColorEdgeDetectionSHader, 1, core::vector2df>
{
public:
    MLAAColorEdgeDetectionSHader();
};

class MLAABlendWeightSHader : public TextureShader<MLAABlendWeightSHader, 2, core::vector2df>
{
public:
    MLAABlendWeightSHader();
};

class MLAAGatherSHader : public TextureShader<MLAAGatherSHader, 2, core::vector2df>
{
public:
    MLAAGatherSHader();
};

}

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
#define STR(a) #a,

enum ShaderType
{
    FOREACH_SHADER(ENUM)

    ES_COUNT
};

#ifdef SHADER_NAMES
static const char *shader_names[] = {
    FOREACH_SHADER(STR)
};
#endif

class Shaders
{
private:
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
    // ------------------------------------------------------------------------



    void killShaders();

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

};   // class Shaders

#undef ENUM
#undef STR
#undef FOREACH_SHADER

#endif
