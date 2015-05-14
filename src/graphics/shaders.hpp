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
#include "graphics/texture_read.hpp"

#include <IMeshSceneNode.h>
#include <IShaderConstantSetCallBack.h>
#include <EMaterialTypes.h>


using namespace irr;
class ParticleSystemProxy;


namespace MeshShader
{
class ObjectPass1Shader : public Shader<ObjectPass1Shader, core::matrix4, core::matrix4>, 
                          public TextureReadNew<ST_TRILINEAR_ANISOTROPIC_FILTERED>
{
public:
    ObjectPass1Shader();
};



class ObjectPass2Shader : public Shader<ObjectPass2Shader, core::matrix4, core::matrix4>,
    public TextureReadNew<ST_NEAREST_FILTERED, ST_NEAREST_FILTERED, ST_BILINEAR_FILTERED,
                        ST_TRILINEAR_ANISOTROPIC_FILTERED, ST_TRILINEAR_ANISOTROPIC_FILTERED>
{
public:
    ObjectPass2Shader();
};




class TransparentShader : public Shader<TransparentShader, core::matrix4, core::matrix4>,
                          public TextureReadNew<ST_TRILINEAR_ANISOTROPIC_FILTERED>
{
public:
    TransparentShader();
};

class TransparentFogShader : public Shader<TransparentFogShader, core::matrix4, core::matrix4, float, float, 
                                           float, float, float, video::SColorf>,
                             public TextureReadNew<ST_TRILINEAR_ANISOTROPIC_FILTERED>
{
public:
    TransparentFogShader();
};


class ColorizeShader : public Shader<ColorizeShader, core::matrix4, video::SColorf>
{
public:
    ColorizeShader();
};

class InstancedColorizeShader : public Shader<InstancedColorizeShader>
{
public:
    InstancedColorizeShader();
};



class RefShadowShader : public Shader<RefShadowShader, int, core::matrix4>,
                        public TextureReadNew<ST_TRILINEAR_ANISOTROPIC_FILTERED>
{
public:
    RefShadowShader();
};

class InstancedRefShadowShader : public Shader<InstancedRefShadowShader, int>,
                                 public TextureReadNew<ST_TRILINEAR_ANISOTROPIC_FILTERED>
{
public:
    InstancedRefShadowShader();
};


class DisplaceMaskShader : public Shader<DisplaceMaskShader, core::matrix4>
{
public:
    DisplaceMaskShader();
};

class DisplaceShader : public Shader<DisplaceShader, core::matrix4, core::vector2df, core::vector2df>,
                       public TextureReadNew<ST_BILINEAR_FILTERED, ST_BILINEAR_FILTERED,
                                             ST_BILINEAR_FILTERED, ST_TRILINEAR_ANISOTROPIC_FILTERED>
{
public:
    DisplaceShader();
};

class SkyboxShader : public Shader<SkyboxShader>,
                     public TextureReadNew<ST_TRILINEAR_CUBEMAP>
{
public:
    SkyboxShader();
    GLuint vao;
};

class NormalVisualizer : public Shader<NormalVisualizer, video::SColor>
{
public:
    NormalVisualizer();
};

class ViewFrustrumShader : public Shader<ViewFrustrumShader, video::SColor, int>
{
public:
    GLuint frustrumvao;

    ViewFrustrumShader();
};

}

#define MAXLIGHT 32

namespace LightShader
{
    struct PointLightInfo
    {
        float posX;
        float posY;
        float posZ;
        float energy;
        float red;
        float green;
        float blue;
        float radius;
    };


    class PointLightShader : public Shader<PointLightShader>,
                             public TextureReadNew<ST_NEAREST_FILTERED, ST_NEAREST_FILTERED>
    {
    public:
        GLuint vbo;
        GLuint vao;
        PointLightShader();
    };

    class PointLightScatterShader : public Shader<PointLightScatterShader, float, core::vector3df>,
                                     public TextureReadNew<ST_NEAREST_FILTERED>
    {
    public:
        GLuint vbo;
        GLuint vao;
        PointLightScatterShader();
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


class SunLightShader : public Shader<SunLightShader, core::vector3df, video::SColorf>,
                       public TextureReadNew<ST_NEAREST_FILTERED, ST_NEAREST_FILTERED>
{
public:
    SunLightShader();
};

class ShadowedSunLightShaderPCF : public Shader<ShadowedSunLightShaderPCF, float, float, float, float, float>,
                                  public TextureReadNew<ST_NEAREST_FILTERED, ST_NEAREST_FILTERED, ST_SHADOW_SAMPLER>
{
public:
    ShadowedSunLightShaderPCF();
};

class ShadowedSunLightShaderESM : public Shader<ShadowedSunLightShaderESM, float, float, float, float>,
                                  public TextureReadNew<ST_NEAREST_FILTERED, ST_NEAREST_FILTERED,
                                                        ST_TRILINEAR_CLAMPED_ARRAY2D>
{
public:
    ShadowedSunLightShaderESM();
};

class RadianceHintsConstructionShader : public Shader<RadianceHintsConstructionShader, core::matrix4, 
                                                      core::matrix4, core::vector3df, video::SColorf>,
                                        public TextureReadNew<ST_BILINEAR_FILTERED, ST_BILINEAR_FILTERED, ST_BILINEAR_FILTERED>
{
public:
    RadianceHintsConstructionShader();
};

// Workaround for a bug found in kepler nvidia linux and fermi nvidia windows
class NVWorkaroundRadianceHintsConstructionShader : public Shader<NVWorkaroundRadianceHintsConstructionShader,
                                                                   core::matrix4, core::matrix4, core::vector3df,
                                                                   int, video::SColorf>,
                                                    public TextureReadNew<ST_BILINEAR_FILTERED,
                                                                          ST_BILINEAR_FILTERED,
                                                                          ST_BILINEAR_FILTERED>
{
public:
    NVWorkaroundRadianceHintsConstructionShader();
};

class RHDebug : public Shader<RHDebug, core::matrix4, core::vector3df>
{
public:
    GLuint TU_SHR, TU_SHG, TU_SHB;

    RHDebug();
};

class GlobalIlluminationReconstructionShader : public Shader<GlobalIlluminationReconstructionShader,
                                                             core::matrix4, core::matrix4, core::vector3df>,
                                               public TextureReadNew<ST_NEAREST_FILTERED, ST_NEAREST_FILTERED,
                                                                     ST_VOLUME_LINEAR_FILTERED,
                                                                     ST_VOLUME_LINEAR_FILTERED,
                                                                     ST_VOLUME_LINEAR_FILTERED>
{
public:
    GlobalIlluminationReconstructionShader();
};

class HorizontalBlurShader : public Shader<HorizontalBlurShader, core::vector2df>,
                             public TextureReadNew<ST_BILINEAR_CLAMPED_FILTERED>
{
public:
    HorizontalBlurShader();
};


class PassThroughShader : public Shader<PassThroughShader, int, int>,
                          public TextureReadNew<ST_BILINEAR_FILTERED>
{
public:
    PassThroughShader();
};

class LayerPassThroughShader : public Shader<LayerPassThroughShader, int>
{
public:
    GLuint TU_texture;
    GLuint vao;

    LayerPassThroughShader();
};

class LinearizeDepthShader : public Shader<LinearizeDepthShader, float, float>,
                             public TextureReadNew<ST_BILINEAR_FILTERED>
{
public:
    LinearizeDepthShader();
};

class LightspaceBoundingBoxShader : public Shader<LightspaceBoundingBoxShader, 
                                                  core::matrix4, float, float,
                                                  float, float>,
                                    public TextureReadNew<ST_NEAREST_FILTERED >
{
public:
    LightspaceBoundingBoxShader();
};

class ShadowMatrixesGenerationShader : public Shader <ShadowMatrixesGenerationShader, core::matrix4>
{
public:
    ShadowMatrixesGenerationShader();
};

class DepthHistogramShader : public Shader<DepthHistogramShader>, 
                             public TextureReadNew<ST_NEAREST_FILTERED>
{
public:
    DepthHistogramShader();
};

class GlowShader : public Shader<GlowShader>,
                   public TextureReadNew<ST_BILINEAR_FILTERED>
{
public:
    GLuint vao;

    GlowShader();
};

class SSAOShader : public Shader<SSAOShader, float, float, float>,
                   public TextureReadNew<ST_SEMI_TRILINEAR>
{
public:
    SSAOShader();
};

class FogShader : public Shader<FogShader, float, core::vector3df>,
                  public TextureReadNew<ST_NEAREST_FILTERED>
{
public:
    FogShader();
};

class MotionBlurShader : public Shader<MotionBlurShader, core::matrix4, core::vector2df, float, float>,
                         public TextureReadNew<ST_BILINEAR_CLAMPED_FILTERED, ST_NEAREST_FILTERED>
{
public:
    MotionBlurShader();
};

class GodFadeShader : public Shader<GodFadeShader, video::SColorf>,
                      public TextureReadNew<ST_BILINEAR_FILTERED>
{
public:
    GodFadeShader();
};

class GodRayShader : public Shader<GodRayShader, core::vector2df>,
                     public TextureReadNew<ST_BILINEAR_FILTERED>
{
public:
    GodRayShader();
};

class MLAAColorEdgeDetectionSHader : public Shader<MLAAColorEdgeDetectionSHader, core::vector2df>,
                                    public TextureReadNew<ST_NEAREST_FILTERED>
{
public:
    MLAAColorEdgeDetectionSHader();
};

class MLAABlendWeightSHader : public Shader<MLAABlendWeightSHader, core::vector2df>,
                              public TextureReadNew<ST_BILINEAR_FILTERED, ST_NEAREST_FILTERED>
{
public:
    MLAABlendWeightSHader();
};

class MLAAGatherSHader : public Shader<MLAAGatherSHader, core::vector2df>,
                         public TextureReadNew<ST_NEAREST_FILTERED, ST_NEAREST_FILTERED>
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
