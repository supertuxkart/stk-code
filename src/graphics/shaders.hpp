//  SuperTuxKart - a fun racing game with go-kart
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

#include <IShaderConstantSetCallBack.h>
#include <IMeshSceneNode.h>
#include "config/user_config.hpp"

#include "shaders_util.hpp"

using namespace irr;
class ParticleSystemProxy;

class SharedObject
{
public:
    static GLuint billboardvbo;
    static GLuint cubevbo, cubeindexes, frustrumvbo, frustrumindexes, ParticleQuadVBO;
    static GLuint ViewProjectionMatrixesUBO;
    static GLuint FullScreenQuadVAO;
    static GLuint UIVAO;
};

namespace UtilShader
{
class ColoredLine
{
public:
    static GLuint Program;
    static GLuint uniform_color;
    static GLuint vao, vbo;

    static void init();
    static void setUniforms(const irr::video::SColor &);
};
}


namespace MeshShader
{
class ObjectPass1Shader : public ShaderHelperSingleton<ObjectPass1Shader, core::matrix4, core::matrix4>, public TextureRead<Trilinear_Anisotropic_Filtered>
{
public:
    ObjectPass1Shader();
};

class ObjectRefPass1Shader : public ShaderHelperSingleton<ObjectRefPass1Shader, core::matrix4, core::matrix4, core::matrix4>, public TextureRead<Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    ObjectRefPass1Shader();
};

class GrassPass1Shader : public ShaderHelperSingleton<GrassPass1Shader, core::matrix4, core::matrix4, core::vector3df>, public TextureRead<Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    GrassPass1Shader();
};

class NormalMapShader : public ShaderHelperSingleton<NormalMapShader, core::matrix4, core::matrix4>, public TextureRead<Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    NormalMapShader();
};

class InstancedObjectPass1Shader : public ShaderHelperSingleton<InstancedObjectPass1Shader>, public TextureRead<Trilinear_Anisotropic_Filtered>
{
public:
    InstancedObjectPass1Shader();
};

class InstancedObjectRefPass1Shader : public ShaderHelperSingleton<InstancedObjectRefPass1Shader>, public TextureRead<Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    InstancedObjectRefPass1Shader();
};

class InstancedGrassPass1Shader : public ShaderHelperSingleton<InstancedGrassPass1Shader, core::vector3df>, public TextureRead<Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    InstancedGrassPass1Shader();
};

class InstancedNormalMapShader : public ShaderHelperSingleton<InstancedNormalMapShader>, public TextureRead<Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    InstancedNormalMapShader();
};

class ObjectPass2Shader : public ShaderHelperSingleton<ObjectPass2Shader, core::matrix4, core::matrix4>, public TextureRead<Nearest_Filtered, Nearest_Filtered, Bilinear_Filtered, Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    ObjectPass2Shader();
};

class InstancedObjectPass2Shader : public ShaderHelperSingleton<InstancedObjectPass2Shader>, public TextureRead<Nearest_Filtered, Nearest_Filtered, Bilinear_Filtered, Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    InstancedObjectPass2Shader();
};

class InstancedObjectRefPass2Shader : public ShaderHelperSingleton<InstancedObjectRefPass2Shader>, public TextureRead<Nearest_Filtered, Nearest_Filtered, Bilinear_Filtered, Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    InstancedObjectRefPass2Shader();
};

class DetailledObjectPass2Shader : public ShaderHelperSingleton<DetailledObjectPass2Shader, core::matrix4>, public TextureRead<Nearest_Filtered, Nearest_Filtered, Bilinear_Filtered, Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    DetailledObjectPass2Shader();
};

class InstancedDetailledObjectPass2Shader : public ShaderHelperSingleton<InstancedDetailledObjectPass2Shader>, public TextureRead<Nearest_Filtered, Nearest_Filtered, Bilinear_Filtered, Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    InstancedDetailledObjectPass2Shader();
};

class ObjectUnlitShader : public ShaderHelperSingleton<ObjectUnlitShader, core::matrix4, core::matrix4>, public TextureRead<Nearest_Filtered, Nearest_Filtered, Bilinear_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    ObjectUnlitShader();
};

class InstancedObjectUnlitShader : public ShaderHelperSingleton<InstancedObjectUnlitShader>, public TextureRead<Nearest_Filtered, Nearest_Filtered, Bilinear_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    InstancedObjectUnlitShader();
};

class ObjectRefPass2Shader : public ShaderHelperSingleton<ObjectRefPass2Shader, core::matrix4, core::matrix4>, public TextureRead<Nearest_Filtered, Nearest_Filtered, Bilinear_Filtered, Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    ObjectRefPass2Shader();
};

class GrassPass2Shader : public ShaderHelperSingleton<GrassPass2Shader, core::matrix4, core::vector3df>, public TextureRead<Nearest_Filtered, Nearest_Filtered, Bilinear_Filtered, Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    GrassPass2Shader();
};

class InstancedGrassPass2Shader : public ShaderHelperSingleton<InstancedGrassPass2Shader, core::vector3df, core::vector3df>, public TextureRead<Nearest_Filtered, Nearest_Filtered, Bilinear_Filtered, Nearest_Filtered, Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    InstancedGrassPass2Shader();
};

class SphereMapShader : public ShaderHelperSingleton<SphereMapShader, core::matrix4, core::matrix4>, public TextureRead<Nearest_Filtered, Nearest_Filtered, Bilinear_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    SphereMapShader();
};

class InstancedSphereMapShader : public ShaderHelperSingleton<InstancedSphereMapShader>, public TextureRead<Nearest_Filtered, Nearest_Filtered, Bilinear_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    InstancedSphereMapShader();
};

class SplattingShader : public ShaderHelperSingleton<SplattingShader, core::matrix4>, public TextureRead<Nearest_Filtered, Nearest_Filtered, Bilinear_Filtered, Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    SplattingShader();
};

class TransparentShader : public ShaderHelperSingleton<TransparentShader, core::matrix4, core::matrix4>, public TextureRead<Trilinear_Anisotropic_Filtered>
{
public:
    TransparentShader();
};

class TransparentFogShader : public ShaderHelperSingleton<TransparentFogShader, core::matrix4, core::matrix4, float, float, float, float, float, video::SColorf>, public TextureRead<Trilinear_Anisotropic_Filtered>
{
public:
    TransparentFogShader();
};

class BillboardShader : public ShaderHelperSingleton<BillboardShader, core::matrix4, core::matrix4, core::vector3df, core::dimension2df>, public TextureRead<Trilinear_Anisotropic_Filtered>
{
public:
    BillboardShader();
};


class ColorizeShader : public ShaderHelperSingleton<ColorizeShader, core::matrix4, video::SColorf>
{
public:
    ColorizeShader();
};

class InstancedColorizeShader : public ShaderHelperSingleton<InstancedColorizeShader>
{
public:
    InstancedColorizeShader();
};

class ShadowShader : public ShaderHelperSingleton<ShadowShader, int, core::matrix4>, public TextureRead<>
{
public:
    ShadowShader();
};

class RSMShader : public ShaderHelperSingleton<RSMShader, core::matrix4, core::matrix4, core::matrix4>, public TextureRead<Trilinear_Anisotropic_Filtered>
{
public:
    RSMShader();
};

class InstancedRSMShader : public ShaderHelperSingleton<InstancedRSMShader, core::matrix4>, public TextureRead<Trilinear_Anisotropic_Filtered>
{
public:
    InstancedRSMShader();
};

class SplattingRSMShader : public ShaderHelperSingleton<SplattingRSMShader, core::matrix4, core::matrix4>,
    public TextureRead<Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    SplattingRSMShader();
};

class InstancedShadowShader : public ShaderHelperSingleton<InstancedShadowShader, int>, public TextureRead<>
{
public:
    InstancedShadowShader();
};

class RefShadowShader : public ShaderHelperSingleton<RefShadowShader, int, core::matrix4>, public TextureRead<Trilinear_Anisotropic_Filtered>
{
public:
    RefShadowShader();
};

class InstancedRefShadowShader : public ShaderHelperSingleton<InstancedRefShadowShader, int>, public TextureRead<Trilinear_Anisotropic_Filtered>
{
public:
    InstancedRefShadowShader();
};

class GrassShadowShader : public ShaderHelperSingleton<GrassShadowShader, int, core::matrix4, core::vector3df>, public TextureRead<Trilinear_Anisotropic_Filtered>
{
public:
    GrassShadowShader();
};

class InstancedGrassShadowShader : public ShaderHelperSingleton<InstancedGrassShadowShader, int, core::vector3df>, public TextureRead<Trilinear_Anisotropic_Filtered>
{
public:
    InstancedGrassShadowShader();
};

class DisplaceMaskShader : public ShaderHelperSingleton<DisplaceMaskShader, core::matrix4>
{
public:
    DisplaceMaskShader();
};

class DisplaceShader : public ShaderHelperSingleton<DisplaceShader, core::matrix4, core::vector2df, core::vector2df>, public TextureRead<Bilinear_Filtered, Bilinear_Filtered, Bilinear_Filtered, Trilinear_Anisotropic_Filtered>
{
public:
    DisplaceShader();
};

class SkyboxShader : public ShaderHelperSingleton<SkyboxShader, core::matrix4>, public TextureRead<Trilinear_cubemap>
{
public:
    SkyboxShader();
    GLuint cubevao;
};

class NormalVisualizer : public ShaderHelperSingleton<NormalVisualizer, video::SColor>
{
public:
    NormalVisualizer();
};

class ViewFrustrumShader
{
public:
    static GLuint Program;
    static GLuint attrib_position;
    static GLuint uniform_color, uniform_idx;
    static GLuint frustrumvao;

    static void init();
    static void setUniforms(const video::SColor &color, unsigned idx);
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


    class PointLightShader : public ShaderHelperSingleton<PointLightShader>, public TextureRead<Nearest_Filtered, Nearest_Filtered>
    {
    public:
        GLuint vbo;
        GLuint vao;
        PointLightShader();
    };
}

namespace ParticleShader
{

class SimpleSimulationShader : public ShaderHelperSingleton<SimpleSimulationShader, core::matrix4, int, int, float>
{
public:
    SimpleSimulationShader();
};



class HeightmapSimulationShader : public ShaderHelperSingleton<HeightmapSimulationShader, core::matrix4, int, int, float, float, float, float, float>
{
public:
    GLuint TU_heightmap;

    HeightmapSimulationShader();
};

class SimpleParticleRender : public ShaderHelperSingleton<SimpleParticleRender, video::SColorf, video::SColorf>, public TextureRead<Trilinear_Anisotropic_Filtered, Nearest_Filtered>
{
public:
    SimpleParticleRender();
};

class FlipParticleRender : public ShaderHelperSingleton<FlipParticleRender>, public TextureRead<Trilinear_Anisotropic_Filtered, Nearest_Filtered>
{
public:
    FlipParticleRender();
};
}

namespace FullScreenShader
{

class BloomShader : public ShaderHelperSingleton<BloomShader>, public TextureRead<Nearest_Filtered>
{
public:
    BloomShader();
};

class BloomBlendShader : public ShaderHelperSingleton<BloomBlendShader>, public TextureRead<Bilinear_Filtered, Bilinear_Filtered, Bilinear_Filtered>
{
public:
    BloomBlendShader();
};

class ToneMapShader : public ShaderHelperSingleton<ToneMapShader>, public TextureRead<Nearest_Filtered>
{
public:
    ToneMapShader();
};

class DepthOfFieldShader : public ShaderHelperSingleton<DepthOfFieldShader>, public TextureRead<Bilinear_Filtered, Nearest_Filtered>
{
public:
    DepthOfFieldShader();
};

class SunLightShader : public ShaderHelperSingleton<SunLightShader, core::vector3df, video::SColorf>, public TextureRead<Nearest_Filtered, Nearest_Filtered>
{
public:
    SunLightShader();
};

class EnvMapShader : public ShaderHelperSingleton<EnvMapShader, core::matrix4, std::vector<float>, std::vector<float>, std::vector<float> >, public TextureRead<Nearest_Filtered, Nearest_Filtered, Trilinear_cubemap>
{
public:
    EnvMapShader();
};

class ShadowedSunLightShader : public ShaderHelperSingleton<ShadowedSunLightShader, core::vector3df, video::SColorf>, public TextureRead<Nearest_Filtered, Nearest_Filtered, Shadow_Sampler>
{
public:
    ShadowedSunLightShader();
};

class RadianceHintsConstructionShader : public ShaderHelperSingleton<RadianceHintsConstructionShader, core::matrix4, core::matrix4, core::vector3df, video::SColorf>, public TextureRead<Bilinear_Filtered, Bilinear_Filtered, Bilinear_Filtered>
{
public:
    RadianceHintsConstructionShader();
};

// Workaround for a bug found in kepler nvidia linux and fermi nvidia windows
class NVWorkaroundRadianceHintsConstructionShader : public ShaderHelperSingleton<NVWorkaroundRadianceHintsConstructionShader, core::matrix4, core::matrix4, core::vector3df, int, video::SColorf>, public TextureRead<Bilinear_Filtered, Bilinear_Filtered, Bilinear_Filtered>
{
public:
    NVWorkaroundRadianceHintsConstructionShader();
};

class RHDebug : public ShaderHelperSingleton<RHDebug, core::matrix4, core::vector3df>
{
public:
    GLuint TU_SHR, TU_SHG, TU_SHB;

    RHDebug();
};

class GlobalIlluminationReconstructionShader : public ShaderHelperSingleton<GlobalIlluminationReconstructionShader, core::matrix4, core::matrix4, core::vector3df>,
    public TextureRead<Nearest_Filtered, Nearest_Filtered, Volume_Linear_Filtered, Volume_Linear_Filtered, Volume_Linear_Filtered>
{
public:
    GlobalIlluminationReconstructionShader();
};

class Gaussian17TapHShader : public ShaderHelperSingleton<Gaussian17TapHShader, core::vector2df>, public TextureRead<Bilinear_Clamped_Filtered, Bilinear_Clamped_Filtered>
{
public:
    Gaussian17TapHShader();
};

class ComputeGaussian17TapHShader : public ShaderHelperSingleton<ComputeGaussian17TapHShader, core::vector2df>, public TextureRead<Neared_Clamped_Filtered, Neared_Clamped_Filtered>
{
public:
    GLuint TU_dest;
    ComputeGaussian17TapHShader();
};

class Gaussian6HBlurShader : public ShaderHelperSingleton<Gaussian6HBlurShader, core::vector2df>, public TextureRead<Bilinear_Clamped_Filtered>
{
public:
    Gaussian6HBlurShader();
};

class Gaussian3HBlurShader : public ShaderHelperSingleton<Gaussian3HBlurShader, core::vector2df>, public TextureRead<Bilinear_Clamped_Filtered>
{
public:
    Gaussian3HBlurShader();
};

class Gaussian17TapVShader : public ShaderHelperSingleton<Gaussian17TapVShader, core::vector2df>, public TextureRead<Bilinear_Clamped_Filtered, Bilinear_Clamped_Filtered>
{
public:
    Gaussian17TapVShader();
};

class ComputeGaussian17TapVShader : public ShaderHelperSingleton<ComputeGaussian17TapVShader, core::vector2df>, public TextureRead<Neared_Clamped_Filtered, Neared_Clamped_Filtered>
{
public:
    GLuint TU_dest;

    ComputeGaussian17TapVShader();
};


class Gaussian6VBlurShader : public ShaderHelperSingleton<Gaussian6VBlurShader, core::vector2df>, public TextureRead<Bilinear_Clamped_Filtered>
{
public:
    Gaussian6VBlurShader();
};

class Gaussian3VBlurShader : public ShaderHelperSingleton<Gaussian3VBlurShader, core::vector2df>, public TextureRead<Bilinear_Clamped_Filtered>
{
public:
    Gaussian3VBlurShader();
};

class PassThroughShader : public ShaderHelperSingleton<PassThroughShader>, public TextureRead<Bilinear_Filtered>
{
public:
    GLuint vao;

    PassThroughShader();
};

class LayerPassThroughShader : public ShaderHelperSingleton<LayerPassThroughShader, int>
{
public:
    GLuint TU_texture;
    GLuint vao;

    LayerPassThroughShader();
};

class LinearizeDepthShader : public ShaderHelperSingleton<LinearizeDepthShader, float, float>, public TextureRead<Bilinear_Filtered>
{
public:
    LinearizeDepthShader();
};

class GlowShader : public ShaderHelperSingleton<GlowShader>, public TextureRead<Bilinear_Filtered>
{
public:
    GLuint vao;

    GlowShader();
};

class SSAOShader : public ShaderHelperSingleton<SSAOShader, float, float, float>, public TextureRead<Semi_trilinear>
{
public:
    SSAOShader();
};

class FogShader : public ShaderHelperSingleton<FogShader, float, float, float, float, float, core::vector3df>, public TextureRead<Nearest_Filtered>
{
public:
    FogShader();
};

class MotionBlurShader : public ShaderHelperSingleton<MotionBlurShader, core::matrix4, core::vector2df, float, float>, public TextureRead<Bilinear_Clamped_Filtered, Nearest_Filtered>
{
public:
    MotionBlurShader();
};

class GodFadeShader : public ShaderHelperSingleton<GodFadeShader, video::SColorf>, public TextureRead<Bilinear_Filtered>
{
public:
    GLuint vao;

    GodFadeShader();
};

class GodRayShader : public ShaderHelperSingleton<GodRayShader, core::vector2df>, public TextureRead<Bilinear_Filtered>
{
public:
    GLuint vao;

    GodRayShader();
};

class MLAAColorEdgeDetectionSHader : public ShaderHelperSingleton<MLAAColorEdgeDetectionSHader, core::vector2df>, public TextureRead<Nearest_Filtered>
{
public:
    GLuint vao;

    MLAAColorEdgeDetectionSHader();
};

class MLAABlendWeightSHader : public ShaderHelperSingleton<MLAABlendWeightSHader, core::vector2df>, public TextureRead<Bilinear_Filtered, Nearest_Filtered>
{
public:
    GLuint vao;

    MLAABlendWeightSHader();
};

class MLAAGatherSHader : public ShaderHelperSingleton<MLAAGatherSHader, core::vector2df>, public TextureRead<Nearest_Filtered, Nearest_Filtered>
{
public:
    GLuint vao;

    MLAAGatherSHader();
};

}

namespace UIShader
{

class Primitive2DList : public ShaderHelperSingleton<Primitive2DList>, public TextureRead < Bilinear_Filtered >
{
public:
    Primitive2DList();
};

class TextureRectShader : public ShaderHelperSingleton<TextureRectShader, core::vector2df, core::vector2df, core::vector2df, core::vector2df>, public TextureRead<Bilinear_Filtered>
{
public:
    TextureRectShader();
};

class UniformColoredTextureRectShader : public ShaderHelperSingleton<UniformColoredTextureRectShader, core::vector2df, core::vector2df, core::vector2df, core::vector2df, video::SColor>, public TextureRead<Bilinear_Filtered>
{
public:
    UniformColoredTextureRectShader();
};

class ColoredTextureRectShader : public ShaderHelperSingleton<ColoredTextureRectShader, core::vector2df, core::vector2df, core::vector2df, core::vector2df>, public TextureRead<Bilinear_Filtered>
{
public:
    GLuint colorvbo;
    GLuint vao;

    ColoredTextureRectShader();
};

class ColoredRectShader : public ShaderHelperSingleton<ColoredRectShader, core::vector2df, core::vector2df, video::SColor>
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
    ACT(ES_COLORIZE) \
    ACT(ES_OBJECT_UNLIT) \
    ACT(ES_OBJECTPASS) \
    ACT(ES_OBJECTPASS_REF) \
    ACT(ES_SUNLIGHT) \
    ACT(ES_OBJECTPASS_RIMLIT) \
    ACT(ES_DISPLACE) \
    ACT(ES_PASSFAR) \

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
