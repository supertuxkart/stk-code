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
#include "graphics/shaders_util.hpp"
#include "graphics/texture_read.hpp"

#include <IMeshSceneNode.h>
#include <IShaderConstantSetCallBack.h>

using namespace irr;
class ParticleSystemProxy;

class SharedObject
{
public:
    static GLuint billboardvbo;
    static GLuint skytrivbo, frustrumvbo, frustrumindexes, ParticleQuadVBO;
    static GLuint ViewProjectionMatrixesUBO, LightingDataUBO;
    static GLuint FullScreenQuadVAO;
    static GLuint UIVAO;
};

namespace UtilShader
{
class ColoredLine : public Shader<ColoredLine, video::SColor>
{
public:
    GLuint vao, vbo;

    ColoredLine();
};

class SpecularIBLGenerator : public Shader<SpecularIBLGenerator, core::matrix4, float >,
                             public TextureRead<Trilinear_cubemap>
{
public:
    GLuint TU_Samples;
    SpecularIBLGenerator();
};
}


namespace MeshShader
{
class ObjectPass1Shader : public Shader<ObjectPass1Shader, core::matrix4, core::matrix4>, 
                          public TextureReadNew<ST_TRILINEAR_ANISOTROPIC_FILTERED>
{
public:
    ObjectPass1Shader();
};

class ObjectRefPass1Shader : public Shader<ObjectRefPass1Shader, core::matrix4, core::matrix4, core::matrix4>,
                             public TextureRead<Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    ObjectRefPass1Shader();
};

class GrassPass1Shader : public Shader<GrassPass1Shader, core::matrix4, core::matrix4, core::vector3df>, public TextureRead<Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    GrassPass1Shader();
};

class NormalMapShader : public Shader<NormalMapShader, core::matrix4, core::matrix4>, public TextureRead<Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    NormalMapShader();
};

class InstancedObjectPass1Shader : public Shader<InstancedObjectPass1Shader>, public TextureRead<Trilinear_Anisotropic_Filtered>
{
public:
    InstancedObjectPass1Shader();
};

class InstancedObjectRefPass1Shader : public Shader<InstancedObjectRefPass1Shader>, public TextureRead<Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    InstancedObjectRefPass1Shader();
};

class InstancedGrassPass1Shader : public Shader<InstancedGrassPass1Shader, core::vector3df>, public TextureRead<Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    InstancedGrassPass1Shader();
};

class InstancedNormalMapShader : public Shader<InstancedNormalMapShader>, public TextureRead<Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    InstancedNormalMapShader();
};

class ObjectPass2Shader : public Shader<ObjectPass2Shader, core::matrix4, core::matrix4>, public TextureRead<Nearest_Filtered, Nearest_Filtered, Bilinear_Filtered, Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    ObjectPass2Shader();
};

class InstancedObjectPass2Shader : public Shader<InstancedObjectPass2Shader>, public TextureRead<Nearest_Filtered, Nearest_Filtered, Bilinear_Filtered, Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    InstancedObjectPass2Shader();
};

class InstancedObjectRefPass2Shader : public Shader<InstancedObjectRefPass2Shader>, public TextureRead<Nearest_Filtered, Nearest_Filtered, Bilinear_Filtered, Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    InstancedObjectRefPass2Shader();
};

class DetailledObjectPass2Shader : public Shader<DetailledObjectPass2Shader, core::matrix4>, public TextureRead<Nearest_Filtered, Nearest_Filtered, Bilinear_Filtered, Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    DetailledObjectPass2Shader();
};

class InstancedDetailledObjectPass2Shader : public Shader<InstancedDetailledObjectPass2Shader>, public TextureRead<Nearest_Filtered, Nearest_Filtered, Bilinear_Filtered, Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    InstancedDetailledObjectPass2Shader();
};

class ObjectUnlitShader : public Shader<ObjectUnlitShader, core::matrix4, core::matrix4>, public TextureRead<Nearest_Filtered, Nearest_Filtered, Bilinear_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    ObjectUnlitShader();
};

class InstancedObjectUnlitShader : public Shader<InstancedObjectUnlitShader>, public TextureRead<Nearest_Filtered, Nearest_Filtered, Bilinear_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    InstancedObjectUnlitShader();
};

class ObjectRefPass2Shader : public Shader<ObjectRefPass2Shader, core::matrix4, core::matrix4>, public TextureRead<Nearest_Filtered, Nearest_Filtered, Bilinear_Filtered, Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    ObjectRefPass2Shader();
};

class GrassPass2Shader : public Shader<GrassPass2Shader, core::matrix4, core::vector3df>, public TextureRead<Nearest_Filtered, Nearest_Filtered, Bilinear_Filtered, Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    GrassPass2Shader();
};

class InstancedGrassPass2Shader : public Shader<InstancedGrassPass2Shader, core::vector3df, core::vector3df>, public TextureRead<Nearest_Filtered, Nearest_Filtered, Bilinear_Filtered, Nearest_Filtered, Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    InstancedGrassPass2Shader();
};

class SphereMapShader : public Shader<SphereMapShader, core::matrix4, core::matrix4>, public TextureRead<Nearest_Filtered, Nearest_Filtered, Bilinear_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    SphereMapShader();
};

class InstancedSphereMapShader : public Shader<InstancedSphereMapShader>, public TextureRead<Nearest_Filtered, Nearest_Filtered, Bilinear_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    InstancedSphereMapShader();
};

class SplattingShader : public Shader<SplattingShader, core::matrix4>, public TextureRead<Nearest_Filtered, Nearest_Filtered, Bilinear_Filtered, Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    SplattingShader();
};

class TransparentShader : public Shader<TransparentShader, core::matrix4, core::matrix4>, public TextureRead<Trilinear_Anisotropic_Filtered>
{
public:
    TransparentShader();
};

class TransparentFogShader : public Shader<TransparentFogShader, core::matrix4, core::matrix4, float, float, float, float, float, video::SColorf>, public TextureRead<Trilinear_Anisotropic_Filtered>
{
public:
    TransparentFogShader();
};

class BillboardShader : public Shader<BillboardShader, core::matrix4, core::matrix4, core::vector3df, core::dimension2df>, public TextureRead<Trilinear_Anisotropic_Filtered>
{
public:
    BillboardShader();
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

class ShadowShader : public Shader<ShadowShader, int, core::matrix4>, public TextureRead<>
{
public:
    ShadowShader();
};

class RSMShader : public Shader<RSMShader, core::matrix4, core::matrix4, core::matrix4>, public TextureRead<Trilinear_Anisotropic_Filtered>
{
public:
    RSMShader();
};

class InstancedRSMShader : public Shader<InstancedRSMShader, core::matrix4>, public TextureRead<Trilinear_Anisotropic_Filtered>
{
public:
    InstancedRSMShader();
};

class SplattingRSMShader : public Shader<SplattingRSMShader, core::matrix4, core::matrix4>,
    public TextureRead<Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    SplattingRSMShader();
};

class InstancedShadowShader : public Shader<InstancedShadowShader, int>, public TextureRead<>
{
public:
    InstancedShadowShader();
};

class RefShadowShader : public Shader<RefShadowShader, int, core::matrix4>, public TextureRead<Trilinear_Anisotropic_Filtered>
{
public:
    RefShadowShader();
};

class InstancedRefShadowShader : public Shader<InstancedRefShadowShader, int>, public TextureRead<Trilinear_Anisotropic_Filtered>
{
public:
    InstancedRefShadowShader();
};

class GrassShadowShader : public Shader<GrassShadowShader, int, core::matrix4, core::vector3df>, public TextureRead<Trilinear_Anisotropic_Filtered>
{
public:
    GrassShadowShader();
};

class InstancedGrassShadowShader : public Shader<InstancedGrassShadowShader, int, core::vector3df>, public TextureRead<Trilinear_Anisotropic_Filtered>
{
public:
    InstancedGrassShadowShader();
};

class DisplaceMaskShader : public Shader<DisplaceMaskShader, core::matrix4>
{
public:
    DisplaceMaskShader();
};

class DisplaceShader : public Shader<DisplaceShader, core::matrix4, core::vector2df, core::vector2df>, public TextureRead<Bilinear_Filtered, Bilinear_Filtered, Bilinear_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    DisplaceShader();
};

class SkyboxShader : public Shader<SkyboxShader>, public TextureRead<Trilinear_cubemap>
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


    class PointLightShader : public Shader<PointLightShader>, public TextureRead<Nearest_Filtered, Nearest_Filtered>
    {
    public:
        GLuint vbo;
        GLuint vao;
        PointLightShader();
    };

    class PointLightScatterShader : public Shader<PointLightScatterShader, float, core::vector3df>, public TextureRead<Nearest_Filtered>
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
    glBindVertexArray(SharedObject::FullScreenQuadVAO);
    T::getInstance()->setUniforms(args...);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

namespace FullScreenShader
{

class BloomShader : public Shader<BloomShader>, public TextureRead<Nearest_Filtered>
{
public:
    BloomShader();
};

class BloomBlendShader : public Shader<BloomBlendShader>, public TextureRead<Bilinear_Filtered, Bilinear_Filtered, Bilinear_Filtered>
{
public:
    BloomBlendShader();
};

class LensBlendShader : public Shader<LensBlendShader>, public TextureRead<Bilinear_Filtered, Bilinear_Filtered, Bilinear_Filtered>
{
public:
    LensBlendShader();
};


class ToneMapShader : public Shader<ToneMapShader, float>, public TextureRead<Nearest_Filtered>
{
public:

    ToneMapShader();
};

class DepthOfFieldShader : public Shader<DepthOfFieldShader>, public TextureRead<Bilinear_Filtered, Nearest_Filtered>
{
public:
    DepthOfFieldShader();
};

class SunLightShader : public Shader<SunLightShader, core::vector3df, video::SColorf>, public TextureRead<Nearest_Filtered, Nearest_Filtered>
{
public:
    SunLightShader();
};

class IBLShader : public Shader<IBLShader>, public TextureRead<Nearest_Filtered, Nearest_Filtered, Trilinear_cubemap>
{
public:
    IBLShader();
};

class DegradedIBLShader : public Shader<DegradedIBLShader>, public TextureRead<Nearest_Filtered>
{
public:
    DegradedIBLShader();
};

class ShadowedSunLightShaderPCF : public Shader<ShadowedSunLightShaderPCF, float, float, float, float, float>, public TextureRead<Nearest_Filtered, Nearest_Filtered, Shadow_Sampler>
{
public:
    ShadowedSunLightShaderPCF();
};

class ShadowedSunLightShaderESM : public Shader<ShadowedSunLightShaderESM, float, float, float, float>, public TextureRead<Nearest_Filtered, Nearest_Filtered, Trilinear_Clamped_Array2D>
{
public:
    ShadowedSunLightShaderESM();
};

class RadianceHintsConstructionShader : public Shader<RadianceHintsConstructionShader, core::matrix4, core::matrix4, core::vector3df, video::SColorf>, public TextureRead<Bilinear_Filtered, Bilinear_Filtered, Bilinear_Filtered>
{
public:
    RadianceHintsConstructionShader();
};

// Workaround for a bug found in kepler nvidia linux and fermi nvidia windows
class NVWorkaroundRadianceHintsConstructionShader : public Shader<NVWorkaroundRadianceHintsConstructionShader, core::matrix4, core::matrix4, core::vector3df, int, video::SColorf>, public TextureRead<Bilinear_Filtered, Bilinear_Filtered, Bilinear_Filtered>
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

class GlobalIlluminationReconstructionShader : public Shader<GlobalIlluminationReconstructionShader, core::matrix4, core::matrix4, core::vector3df>,
    public TextureRead<Nearest_Filtered, Nearest_Filtered, Volume_Linear_Filtered, Volume_Linear_Filtered, Volume_Linear_Filtered>
{
public:
    GlobalIlluminationReconstructionShader();
};

class Gaussian17TapHShader : public Shader<Gaussian17TapHShader, core::vector2df>, public TextureRead<Bilinear_Clamped_Filtered, Bilinear_Clamped_Filtered>
{
public:
    Gaussian17TapHShader();
};

class ComputeGaussian17TapHShader : public Shader<ComputeGaussian17TapHShader, core::vector2df>, public TextureRead<Neared_Clamped_Filtered, Neared_Clamped_Filtered>
{
public:
    GLuint TU_dest;
    ComputeGaussian17TapHShader();
};

class ComputeGaussian6HBlurShader : public Shader<ComputeGaussian6HBlurShader, core::vector2df, std::vector<float> >, public TextureRead<Bilinear_Clamped_Filtered>
{
public:
    GLuint TU_dest;
    ComputeGaussian6HBlurShader();
};

class ComputeShadowBlurHShader : public Shader<ComputeShadowBlurHShader, core::vector2df, std::vector<float> >, public TextureRead<Neared_Clamped_Filtered>
{
public:
    GLuint TU_dest;
    ComputeShadowBlurHShader();
};

class Gaussian6HBlurShader : public Shader<Gaussian6HBlurShader, core::vector2df, float>, public TextureRead<Bilinear_Clamped_Filtered>
{
public:
    Gaussian6HBlurShader();
};

class HorizontalBlurShader : public Shader<HorizontalBlurShader, core::vector2df>, public TextureRead<Bilinear_Clamped_Filtered>
{
public:
    HorizontalBlurShader();
};

class Gaussian3HBlurShader : public Shader<Gaussian3HBlurShader, core::vector2df>, public TextureRead<Bilinear_Clamped_Filtered>
{
public:
    Gaussian3HBlurShader();
};

class Gaussian17TapVShader : public Shader<Gaussian17TapVShader, core::vector2df>, public TextureRead<Bilinear_Clamped_Filtered, Bilinear_Clamped_Filtered>
{
public:
    Gaussian17TapVShader();
};

class ComputeGaussian17TapVShader : public Shader<ComputeGaussian17TapVShader, core::vector2df>, public TextureRead<Neared_Clamped_Filtered, Neared_Clamped_Filtered>
{
public:
    GLuint TU_dest;

    ComputeGaussian17TapVShader();
};

class ComputeGaussian6VBlurShader : public Shader<ComputeGaussian6VBlurShader, core::vector2df, std::vector<float> >, public TextureRead<Bilinear_Clamped_Filtered>
{
public:
    GLuint TU_dest;
    ComputeGaussian6VBlurShader();
};

class ComputeShadowBlurVShader : public Shader<ComputeShadowBlurVShader, core::vector2df, std::vector<float> >, public TextureRead<Neared_Clamped_Filtered>
{
public:
    GLuint TU_dest;
    ComputeShadowBlurVShader();
};

class Gaussian6VBlurShader : public Shader<Gaussian6VBlurShader, core::vector2df, float>, public TextureRead<Bilinear_Clamped_Filtered>
{
public:
    Gaussian6VBlurShader();
};

class Gaussian3VBlurShader : public Shader<Gaussian3VBlurShader, core::vector2df>, public TextureRead<Bilinear_Clamped_Filtered>
{
public:
    Gaussian3VBlurShader();
};

class PassThroughShader : public Shader<PassThroughShader, int, int>, public TextureRead<Bilinear_Filtered>
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

class LinearizeDepthShader : public Shader<LinearizeDepthShader, float, float>, public TextureRead<Bilinear_Filtered>
{
public:
    LinearizeDepthShader();
};

class LightspaceBoundingBoxShader : public Shader<LightspaceBoundingBoxShader, core::matrix4, float, float, float, float>, public TextureRead < Nearest_Filtered >
{
public:
    LightspaceBoundingBoxShader();
};

class ShadowMatrixesGenerationShader : public Shader <ShadowMatrixesGenerationShader, core::matrix4>
{
public:
    ShadowMatrixesGenerationShader();
};

class DepthHistogramShader : public Shader<DepthHistogramShader>, public TextureRead <Nearest_Filtered>
{
public:
    DepthHistogramShader();
};

class GlowShader : public Shader<GlowShader>, public TextureRead<Bilinear_Filtered>
{
public:
    GLuint vao;

    GlowShader();
};

class SSAOShader : public Shader<SSAOShader, float, float, float>, public TextureRead<Semi_trilinear>
{
public:
    SSAOShader();
};

class FogShader : public Shader<FogShader, float, core::vector3df>, public TextureRead<Nearest_Filtered>
{
public:
    FogShader();
};

class MotionBlurShader : public Shader<MotionBlurShader, core::matrix4, core::vector2df, float, float>, public TextureRead<Bilinear_Clamped_Filtered, Nearest_Filtered>
{
public:
    MotionBlurShader();
};

class GodFadeShader : public Shader<GodFadeShader, video::SColorf>, public TextureRead<Bilinear_Filtered>
{
public:
    GodFadeShader();
};

class GodRayShader : public Shader<GodRayShader, core::vector2df>, public TextureRead<Bilinear_Filtered>
{
public:
    GodRayShader();
};

class MLAAColorEdgeDetectionSHader : public Shader<MLAAColorEdgeDetectionSHader, core::vector2df>, public TextureRead<Nearest_Filtered>
{
public:
    MLAAColorEdgeDetectionSHader();
};

class MLAABlendWeightSHader : public Shader<MLAABlendWeightSHader, core::vector2df>, public TextureRead<Bilinear_Filtered, Nearest_Filtered>
{
public:
    MLAABlendWeightSHader();
};

class MLAAGatherSHader : public Shader<MLAAGatherSHader, core::vector2df>, public TextureRead<Nearest_Filtered, Nearest_Filtered>
{
public:
    MLAAGatherSHader();
};

}

namespace UIShader
{

class Primitive2DList : public Shader<Primitive2DList>, public TextureRead < Bilinear_Filtered >
{
public:
    Primitive2DList();
};

class TextureRectShader : public Shader<TextureRectShader, core::vector2df, core::vector2df, core::vector2df, core::vector2df>, public TextureRead<Bilinear_Filtered>
{
public:
    TextureRectShader();
};

class UniformColoredTextureRectShader : public Shader<UniformColoredTextureRectShader, core::vector2df, core::vector2df, core::vector2df, core::vector2df, video::SColor>, public TextureRead<Bilinear_Filtered>
{
public:
    UniformColoredTextureRectShader();
};

class ColoredTextureRectShader : public Shader<ColoredTextureRectShader, core::vector2df, core::vector2df, core::vector2df, core::vector2df>, public TextureRead<Bilinear_Filtered>
{
public:
    GLuint colorvbo;
    GLuint vao;

    ColoredTextureRectShader();
};

class ColoredRectShader : public Shader<ColoredRectShader, core::vector2df, core::vector2df, video::SColor>
{
public:
    ColoredRectShader();
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
public:
    Shaders();
    ~Shaders();

    video::E_MATERIAL_TYPE getShader(const ShaderType num) const;

    video::IShaderConstantSetCallBack * m_callbacks[ES_COUNT];

    void loadShaders();
    void killShaders();
private:
    void check(const int num) const;
    
    int m_shaders[ES_COUNT];
};

#undef ENUM
#undef STR
#undef FOREACH_SHADER

#endif
