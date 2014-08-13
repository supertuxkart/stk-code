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
#include <vector>
#include "config/user_config.hpp"
#include "utils/singleton.hpp"

typedef unsigned int    GLuint;
using namespace irr;
class ParticleSystemProxy;

class SharedObject
{
public:
    static GLuint billboardvbo;
    static GLuint cubevbo, cubeindexes, frustrumvbo, frustrumindexes;
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

void glUniformMatrix4fvWraper(GLuint, size_t, unsigned, const float *mat);
void glUniform3fWraper(GLuint, float, float, float);
void glUniform4iWraper(GLuint, int, int, int, int);
void glUniform2fWraper(GLuint a, float b, float c);
void glUniform1fWrapper(GLuint, float);
void glUniform1iWrapper(GLuint, int);
bool needsUBO();

struct UniformHelper
{
    template<unsigned N = 0>
    static void setUniformsHelper(const std::vector<GLuint> &uniforms)
    {
    }

    template<unsigned N = 0, typename... Args>
    static void setUniformsHelper(const std::vector<GLuint> &uniforms, const core::matrix4 &mat, Args... arg)
    {
#ifndef GL_FALSE
#define GL_FALSE 0
#endif
        glUniformMatrix4fvWraper(uniforms[N], 1, GL_FALSE, mat.pointer());
        setUniformsHelper<N + 1>(uniforms, arg...);
    }

    template<unsigned N = 0, typename... Args>
    static void setUniformsHelper(const std::vector<GLuint> &uniforms, const video::SColorf &col, Args... arg)
    {
        glUniform3fWraper(uniforms[N], col.r, col.g, col.b);
        setUniformsHelper<N + 1>(uniforms, arg...);
    }

    template<unsigned N = 0, typename... Args>
    static void setUniformsHelper(const std::vector<GLuint> &uniforms, const video::SColor &col, Args... arg)
    {
        glUniform4iWraper(uniforms[N], col.getRed(), col.getGreen(), col.getBlue(), col.getAlpha());
        setUniformsHelper<N + 1>(uniforms, arg...);
    }

    template<unsigned N = 0, typename... Args>
    static void setUniformsHelper(const std::vector<GLuint> &uniforms, const core::vector3df &v, Args... arg)
    {
        glUniform3fWraper(uniforms[N], v.X, v.Y, v.Z);
        setUniformsHelper<N + 1>(uniforms, arg...);
    }


    template<unsigned N = 0, typename... Args>
    static void setUniformsHelper(const std::vector<GLuint> &uniforms, const core::vector2df &v, Args... arg)
    {
        glUniform2fWraper(uniforms[N], v.X, v.Y);
        setUniformsHelper<N + 1>(uniforms, arg...);
    }

    template<unsigned N = 0, typename... Args>
    static void setUniformsHelper(const std::vector<GLuint> &uniforms, const core::dimension2df &v, Args... arg)
    {
        glUniform2fWraper(uniforms[N], v.Width, v.Height);
        setUniformsHelper<N + 1>(uniforms, arg...);
    }

    template<unsigned N = 0, typename... Args>
    static void setUniformsHelper(const std::vector<GLuint> &uniforms, float f, Args... arg)
    {
        glUniform1fWrapper(uniforms[N], f);
        setUniformsHelper<N + 1>(uniforms, arg...);
    }

    template<unsigned N = 0, typename... Args>
    static void setUniformsHelper(const std::vector<GLuint> &uniforms, int f, Args... arg)
    {
        glUniform1iWrapper(uniforms[N], f);
        setUniformsHelper<N + 1>(uniforms, arg...);
    }

};

void bypassUBO(GLuint Program);
GLuint getUniformLocation(GLuint program, const char* name);

template<typename T, typename... Args>
class ShaderHelperSingleton : public Singleton<T>
{
protected:
    std::vector<GLuint> uniforms;
    
    void AssignUniforms_impl()
    {
    }

    template<typename... U>
    void AssignUniforms_impl(const char* name, U... rest)
    {
        uniforms.push_back(getUniformLocation(Program, name));
        AssignUniforms_impl(rest...);
    }

    template<typename... U>
    void AssignUniforms(U... rest)
    {
        static_assert(sizeof...(rest) == sizeof...(Args), "Count of Uniform's name mismatch");
        AssignUniforms_impl(rest...);
    }

public:
    GLuint Program;

    void setUniforms(const Args & ... args) const
    {
        if (needsUBO())
            bypassUBO(Program);
        UniformHelper::setUniformsHelper(uniforms, args...);
    }
};

namespace MeshShader
{
class ObjectPass1Shader : public ShaderHelperSingleton<ObjectPass1Shader, core::matrix4, core::matrix4>
{
public:
    GLuint TU_tex;
    ObjectPass1Shader();
};

class ObjectRefPass1Shader : public ShaderHelperSingleton<ObjectRefPass1Shader, core::matrix4, core::matrix4, core::matrix4>
{
public:
    GLuint TU_tex;
    ObjectRefPass1Shader();
};

class GrassPass1Shader : public ShaderHelperSingleton<GrassPass1Shader, core::matrix4, core::matrix4, core::vector3df>
{
public:
    GLuint TU_tex;

    GrassPass1Shader();
};

class NormalMapShader : public ShaderHelperSingleton<NormalMapShader, core::matrix4, core::matrix4>
{
public:
    GLuint TU_normalmap, TU_glossy;
    NormalMapShader();
};

class InstancedObjectPass1Shader : public ShaderHelperSingleton<InstancedObjectPass1Shader>
{
public:
    GLuint TU_tex;

    InstancedObjectPass1Shader();
};

class InstancedObjectRefPass1Shader : public ShaderHelperSingleton<InstancedObjectRefPass1Shader>
{
public:
    GLuint TU_tex;

    InstancedObjectRefPass1Shader();
};

class InstancedGrassPass1Shader : public ShaderHelperSingleton<InstancedGrassPass1Shader, core::vector3df>
{
public:
    GLuint TU_tex;

    InstancedGrassPass1Shader();
};

class InstancedNormalMapShader : public ShaderHelperSingleton<InstancedNormalMapShader>
{
public:
    GLuint TU_glossy, TU_normalmap;

    InstancedNormalMapShader();
};

class ObjectPass2Shader : public ShaderHelperSingleton<ObjectPass2Shader, core::matrix4, core::matrix4>
{
public:
    GLuint TU_Albedo;

    ObjectPass2Shader();
};

class InstancedObjectPass2Shader : public ShaderHelperSingleton<InstancedObjectPass2Shader>
{
public:
    GLuint TU_Albedo;

    InstancedObjectPass2Shader();
};

class InstancedObjectRefPass2Shader : public ShaderHelperSingleton<InstancedObjectRefPass2Shader>
{
public:
    GLuint TU_Albedo;

    InstancedObjectRefPass2Shader();
};

class DetailledObjectPass2Shader : public ShaderHelperSingleton<DetailledObjectPass2Shader, core::matrix4>
{
public:
    GLuint TU_Albedo, TU_detail;

    DetailledObjectPass2Shader();
};

class ObjectUnlitShader : public ShaderHelperSingleton<ObjectUnlitShader, core::matrix4>
{
public:
    GLuint TU_tex;

    ObjectUnlitShader();
};

class ObjectRefPass2Shader : public ShaderHelperSingleton<ObjectRefPass2Shader, core::matrix4, core::matrix4>
{
public:
    GLuint TU_Albedo;

    ObjectRefPass2Shader();
};

class GrassPass2Shader : public ShaderHelperSingleton<GrassPass2Shader, core::matrix4, core::vector3df>
{
public:
    GLuint TU_Albedo;

    GrassPass2Shader();
};

class InstancedGrassPass2Shader : public ShaderHelperSingleton<InstancedGrassPass2Shader, core::vector3df, core::vector3df>
{
public:
    GLuint TU_Albedo, TU_dtex;

    InstancedGrassPass2Shader();
};

class SphereMapShader : public ShaderHelperSingleton<SphereMapShader, core::matrix4, core::matrix4>
{
public:
    GLuint TU_tex;

    SphereMapShader();
};

class SplattingShader : public ShaderHelperSingleton<SplattingShader, core::matrix4>
{
public:
    GLuint TU_tex_layout, TU_tex_detail0, TU_tex_detail1, TU_tex_detail2, TU_tex_detail3;

    SplattingShader();
};

class BubbleShader
{
public:
    static GLuint Program;
    static GLuint uniform_MVP, uniform_tex, uniform_time, uniform_transparency;

    static void init();
    static void setUniforms(const core::matrix4 &ModelViewProjectionMatrix, unsigned TU_tex, float time, float transparency);
};

class TransparentShader : public ShaderHelperSingleton<TransparentShader, core::matrix4, core::matrix4>
{
public:
    GLuint TU_tex;

    TransparentShader();
};

class TransparentFogShader : public ShaderHelperSingleton<TransparentFogShader, core::matrix4, core::matrix4, float, float, float, float, float, video::SColorf>
{
public:
    GLuint TU_tex;

    TransparentFogShader();
};

class BillboardShader : public ShaderHelperSingleton<BillboardShader, core::matrix4, core::matrix4, core::vector3df, core::dimension2df>
{
public:
    GLuint TU_tex;

    BillboardShader();
};


class ColorizeShader : public ShaderHelperSingleton<ColorizeShader, core::matrix4, video::SColorf>
{
public:
    ColorizeShader();
};

class ShadowShader : public ShaderHelperSingleton<ShadowShader, core::matrix4>
{
public:
    ShadowShader();
};

class RSMShader : public ShaderHelperSingleton<RSMShader, core::matrix4, core::matrix4, core::matrix4>
{
public:
    GLuint TU_tex;

    RSMShader();
};

class SplattingRSMShader : public ShaderHelperSingleton<SplattingRSMShader, core::matrix4, core::matrix4>
{
public:
    GLuint TU_layout, TU_detail0, TU_detail1, TU_detail2, TU_detail3;

    SplattingRSMShader();
};

class InstancedShadowShader : public ShaderHelperSingleton<InstancedShadowShader>
{
public:
    InstancedShadowShader();
};

class RefShadowShader : public ShaderHelperSingleton<RefShadowShader, core::matrix4>
{
public:
    GLuint TU_tex;
    RefShadowShader();
};

class InstancedRefShadowShader : public ShaderHelperSingleton<InstancedRefShadowShader>
{
public:
    GLuint TU_tex;
    InstancedRefShadowShader();
};

class GrassShadowShader : public ShaderHelperSingleton<GrassShadowShader, core::matrix4, core::vector3df>
{
public:
    GLuint TU_tex;
    GrassShadowShader();
};

class InstancedGrassShadowShader : public ShaderHelperSingleton<InstancedGrassShadowShader, core::vector3df>
{
public:
    GLuint TU_tex;
    InstancedGrassShadowShader();
};

class DisplaceMaskShader : public ShaderHelperSingleton<DisplaceMaskShader, core::matrix4>
{
public:
    DisplaceMaskShader();
};

class DisplaceShader : public ShaderHelperSingleton<DisplaceShader, core::matrix4, core::vector2df, core::vector2df>
{
public:
    GLuint TU_displacement_tex, TU_mask_tex, TU_color_tex, TU_tex;

    DisplaceShader();
};

class SkyboxShader
{
public:
    static GLuint Program;
    static GLuint attrib_position;
    static GLuint uniform_MM, uniform_tex;
    static GLuint cubevao;

    static void init();
    static void setUniforms(const core::matrix4 &ModelMatrix, const core::vector2df &screen, unsigned TU_tex);
};

class NormalVisualizer : public ShaderHelperSingleton<NormalVisualizer, core::matrix4, core::matrix4, video::SColor>
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


    class PointLightShader
    {
    public:
        static GLuint Program;
        static GLuint attrib_Position, attrib_Energy, attrib_Color, attrib_Radius;
        static GLuint uniform_ntex, uniform_dtex, uniform_spec;
        static GLuint vbo;
        static GLuint vao;

        static void init();
        static void setUniforms(const core::vector2df &screen, unsigned spec, unsigned TU_ntex, unsigned TU_dtex);
    };
}

namespace ParticleShader
{

class SimpleSimulationShader
{
public:
    static GLuint Program;
    static GLuint attrib_position, attrib_velocity, attrib_lifetime, attrib_initial_position, attrib_initial_velocity, attrib_initial_lifetime, attrib_size, attrib_initial_size;
    static GLuint uniform_sourcematrix, uniform_dt, uniform_level, uniform_size_increase_factor;

    static void init();
};



class HeightmapSimulationShader
{
public:
    static GLuint Program;
    static GLuint attrib_position, attrib_velocity, attrib_lifetime, attrib_initial_position, attrib_initial_velocity, attrib_initial_lifetime, attrib_size, attrib_initial_size;
    static GLuint uniform_sourcematrix, uniform_dt, uniform_level, uniform_size_increase_factor;
    static GLuint uniform_track_x, uniform_track_z, uniform_track_x_len, uniform_track_z_len, uniform_heightmap;

    static void init();
};

class SimpleParticleRender
{
public:
    static GLuint Program;
    static GLuint attrib_pos, attrib_lf, attrib_quadcorner, attrib_texcoord, attrib_sz;
    static GLuint uniform_matrix, uniform_viewmatrix, uniform_tex, uniform_dtex, uniform_invproj, uniform_color_from, uniform_color_to;

    static void init();
    static void setUniforms(const core::matrix4 &ViewMatrix, const core::matrix4 &ProjMatrix,
                            const core::matrix4 InvProjMatrix, float width, float height, unsigned TU_tex,
                            unsigned TU_normal_and_depth, const ParticleSystemProxy* particle_system);
};

class FlipParticleRender
{
public:
    static GLuint Program;
    static GLuint attrib_pos, attrib_lf, attrib_quadcorner, attrib_texcoord, attrib_sz, attrib_rotationvec, attrib_anglespeed;
    static GLuint uniform_matrix, uniform_viewmatrix, uniform_tex, uniform_dtex, uniform_invproj;

    static void init();
    static void setUniforms(const core::matrix4 &ViewMatrix, const core::matrix4 &ProjMatrix, const core::matrix4 InvProjMatrix, float width, float height, unsigned TU_tex, unsigned TU_normal_and_depth);
};
}

namespace FullScreenShader
{

class BloomShader : public ShaderHelperSingleton<BloomShader>
{
public:
    GLuint TU_tex;

    BloomShader();
};

class BloomBlendShader : public ShaderHelperSingleton<BloomBlendShader>
{
public:
    GLuint TU_tex_128, TU_tex_256, TU_tex_512;

    BloomBlendShader();
};

class ToneMapShader : public ShaderHelperSingleton<ToneMapShader>
{
public:
    GLuint TU_tex;

    ToneMapShader();
};

class DepthOfFieldShader : public ShaderHelperSingleton<DepthOfFieldShader>
{
public:
    GLuint TU_tex, TU_depth;

    DepthOfFieldShader();
};

class SunLightShader : public ShaderHelperSingleton<SunLightShader, core::vector3df, video::SColorf>
{
public:
    GLuint TU_ntex, TU_dtex;

    SunLightShader();
};

class DiffuseEnvMapShader
{
public:
    static GLuint Program;
    static GLuint uniform_ntex, uniform_TVM, uniform_blueLmn, uniform_greenLmn, uniform_redLmn;

    static void init();
    static void setUniforms(const core::matrix4 &TransposeViewMatrix, const float *blueSHCoeff, const float *greenSHCoeff, const float *redSHCoeff, unsigned TU_ntex);
};

class ShadowedSunLightShader : public ShaderHelperSingleton<ShadowedSunLightShader, core::vector3df, video::SColorf>
{
public:
    GLuint TU_ntex, TU_dtex, TU_shadowtex;

    ShadowedSunLightShader();
};

class RadianceHintsConstructionShader : public ShaderHelperSingleton<RadianceHintsConstructionShader, core::matrix4, core::matrix4, core::vector3df>
{
public:
    GLuint TU_ctex, TU_ntex, TU_dtex;

    RadianceHintsConstructionShader();
};

class RHDebug : public ShaderHelperSingleton<RHDebug, core::matrix4, core::vector3df>
{
public:
    GLuint TU_SHR, TU_SHG, TU_SHB;

    RHDebug();
};

class GlobalIlluminationReconstructionShader : public ShaderHelperSingleton<GlobalIlluminationReconstructionShader, core::matrix4, core::matrix4, core::vector3df>
{
public:
    GLuint TU_ntex, TU_dtex, TU_SHR, TU_SHG, TU_SHB, uniform_RHMatrix;

    GlobalIlluminationReconstructionShader();
};

class Gaussian17TapHShader : public ShaderHelperSingleton<Gaussian17TapHShader, core::vector2df>
{
public:
    GLuint TU_tex, TU_depth;

    Gaussian17TapHShader();
};

class ComputeGaussian17TapHShader : public ShaderHelperSingleton<ComputeGaussian17TapHShader>
{
public:
    GLuint TU_source, TU_dest, TU_depth;
    ComputeGaussian17TapHShader();
};

class Gaussian6HBlurShader : public ShaderHelperSingleton<Gaussian6HBlurShader, core::vector2df>
{
public:
    GLuint TU_tex;

    Gaussian6HBlurShader();
};

class Gaussian3HBlurShader : public ShaderHelperSingleton<Gaussian3HBlurShader, core::vector2df>
{
public:
    GLuint TU_tex;

    Gaussian3HBlurShader();
};

class Gaussian17TapVShader : public ShaderHelperSingleton<Gaussian17TapVShader, core::vector2df>
{
public:
    GLuint TU_tex, TU_depth;

    Gaussian17TapVShader();
};

class ComputeGaussian17TapVShader : public ShaderHelperSingleton<ComputeGaussian17TapVShader>
{
public:
    GLuint TU_source, TU_depth, TU_dest;

    ComputeGaussian17TapVShader();
};


class Gaussian6VBlurShader : public ShaderHelperSingleton<Gaussian6VBlurShader, core::vector2df>
{
public:
    GLuint TU_tex;

    Gaussian6VBlurShader();
};

class Gaussian3VBlurShader : public ShaderHelperSingleton<Gaussian3VBlurShader, core::vector2df>
{
public:
    GLuint TU_tex;

    Gaussian3VBlurShader();
};

class PassThroughShader : public ShaderHelperSingleton<PassThroughShader>
{
public:
    GLuint TU_tex;
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

class LinearizeDepthShader : public ShaderHelperSingleton<LinearizeDepthShader, float, float>
{
public:
    GLuint TU_tex;

    LinearizeDepthShader();
};

class GlowShader : public ShaderHelperSingleton<GlowShader>
{
public:
    GLuint TU_tex;
    GLuint vao;

    GlowShader();
};

class SSAOShader : public ShaderHelperSingleton<SSAOShader, float, float, float>
{
public:
    GLuint TU_dtex;

    SSAOShader();
};

class FogShader : public ShaderHelperSingleton<FogShader, float, float, float, float, float, core::vector3df>
{
public:
    GLuint TU_tex;

    FogShader();
};

class MotionBlurShader : public ShaderHelperSingleton<MotionBlurShader, core::matrix4, core::vector2df, float, float>
{
public:
    GLuint TU_cb, TU_dtex;

    MotionBlurShader();
};

class GodFadeShader : public ShaderHelperSingleton<GodFadeShader, video::SColorf>
{
public:
    GLuint TU_tex;
    GLuint vao;

    GodFadeShader();
};

class GodRayShader : public ShaderHelperSingleton<GodRayShader, core::vector2df>
{
public:
    GLuint TU_tex;
    GLuint vao;

    GodRayShader();
};

class MLAAColorEdgeDetectionSHader : public ShaderHelperSingleton<MLAAColorEdgeDetectionSHader, core::vector2df>
{
public:
    GLuint TU_colorMapG;
    GLuint vao;

    MLAAColorEdgeDetectionSHader();
};

class MLAABlendWeightSHader : public ShaderHelperSingleton<MLAABlendWeightSHader, core::vector2df>
{
public:
    GLuint TU_edgesMap, TU_areaMap;
    GLuint vao;

    MLAABlendWeightSHader();
};

class MLAAGatherSHader : public ShaderHelperSingleton<MLAAGatherSHader, core::vector2df>
{
public:
    GLuint TU_colorMap, TU_blendMap;
    GLuint vao;

    MLAAGatherSHader();
};

}

namespace UIShader
{
class TextureRectShader : public ShaderHelperSingleton<TextureRectShader, core::vector2df, core::vector2df, core::vector2df, core::vector2df>
{
public:
    GLuint TU_tex;

    TextureRectShader();
};

class UniformColoredTextureRectShader : public ShaderHelperSingleton<UniformColoredTextureRectShader, core::vector2df, core::vector2df, core::vector2df, core::vector2df, video::SColor>
{
public:
    GLuint TU_tex;

    UniformColoredTextureRectShader();
};

class ColoredTextureRectShader : public ShaderHelperSingleton<ColoredTextureRectShader, core::vector2df, core::vector2df, core::vector2df, core::vector2df>
{
public:
    GLuint TU_tex;
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
    ACT(ES_BUBBLES) \
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
private:
    void check(const int num) const;
    
    int m_shaders[ES_COUNT];
};

#undef ENUM
#undef STR
#undef FOREACH_SHADER

#endif
