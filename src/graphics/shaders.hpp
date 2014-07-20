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
void glUniform2fWraper(GLuint a, float b, float c);
void glUniform1fWrapper(GLuint, float);

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
    static void setUniformsHelper(const std::vector<GLuint> &uniforms, float f, Args... arg)
    {
        glUniform1fWrapper(uniforms[N], f);
        setUniformsHelper<N + 1>(uniforms, arg...);
    }

};

void bypassUBO(GLuint Program);
GLuint getUniformLocation(GLuint program, const char* name);

template<typename... Args>
class ShaderHelper
{
protected:
    std::vector<GLuint> uniforms;

    void AssignUniforms(const char* name)
    {
        uniforms.push_back(getUniformLocation(Program, name));
    }

    template<typename... T>
    void AssignUniforms(const char* name, T... rest)
    {
        uniforms.push_back(getUniformLocation(Program, name));
        AssignUniforms(rest...);
    }

public:
    GLuint Program;

    void setUniforms(const Args & ... args) const
    {
        if (UserConfigParams::m_ubo_disabled)
            bypassUBO(Program);
        UniformHelper::setUniformsHelper(uniforms, args...);
    }
};

template<typename T, typename... Args>
class ShaderHelperSingleton : public Singleton<T>
{
protected:
    std::vector<GLuint> uniforms;
    
    void AssignUniforms(const char* name)
    {
        uniforms.push_back(getUniformLocation(Program, name));
    }

    template<typename... U>
    void AssignUniforms(const char* name, U... rest)
    {
        uniforms.push_back(getUniformLocation(Program, name));
        AssignUniforms(rest...);
    }

public:
    friend class Singleton<class ObjectPass1Shader>;
    GLuint Program;

    void setUniforms(const Args & ... args) const
    {
        if (UserConfigParams::m_ubo_disabled)
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

class InstancedObjectPass1Shader : public ShaderHelper<>
{
public:
    GLuint TU_tex;

    InstancedObjectPass1Shader();
};

extern InstancedObjectPass1Shader *InstancedObjectPass1ShaderInstance;

class InstancedObjectRefPass1Shader : public ShaderHelper<>
{
public:
    GLuint TU_tex;

    InstancedObjectRefPass1Shader();
};

extern InstancedObjectRefPass1Shader *InstancedObjectRefPass1ShaderInstance;

class InstancedGrassPass1Shader : public ShaderHelper<core::vector3df>
{
public:
    GLuint TU_tex;

    InstancedGrassPass1Shader();
};

extern InstancedGrassPass1Shader *InstancedGrassPass1ShaderInstance;

class ObjectPass2Shader : public ShaderHelperSingleton<ObjectPass2Shader, core::matrix4, core::matrix4, video::SColorf>
{
public:
    GLuint TU_Albedo;

    ObjectPass2Shader();
};

class InstancedObjectPass2Shader : public ShaderHelper<video::SColorf>
{
public:
    GLuint TU_Albedo;

    InstancedObjectPass2Shader();
};

extern InstancedObjectPass2Shader *InstancedObjectPass2ShaderInstance;

class InstancedObjectRefPass2Shader : public ShaderHelper<video::SColorf>
{
public:
    GLuint TU_Albedo;

    InstancedObjectRefPass2Shader();
};

extern InstancedObjectRefPass2Shader *InstancedObjectRefPass2ShaderInstance;

class DetailledObjectPass2Shader : public ShaderHelperSingleton<DetailledObjectPass2Shader, core::matrix4, video::SColorf>
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

class ObjectRefPass2Shader : public ShaderHelperSingleton<ObjectRefPass2Shader, core::matrix4, core::matrix4, video::SColorf>
{
public:
    GLuint TU_Albedo;

    ObjectRefPass2Shader();
};

class GrassPass2Shader : public ShaderHelperSingleton<GrassPass2Shader, core::matrix4, core::vector3df, video::SColorf>
{
public:
    GLuint TU_Albedo;

    GrassPass2Shader();
};

class InstancedGrassPass2Shader : public ShaderHelper<core::vector3df, core::vector3df, video::SColorf>
{
public:
    GLuint TU_Albedo, TU_dtex;

    InstancedGrassPass2Shader();
};

extern InstancedGrassPass2Shader *InstancedGrassPass2ShaderInstance;

class SphereMapShader : public ShaderHelperSingleton<SphereMapShader, core::matrix4, core::matrix4, video::SColorf>
{
public:
    GLuint TU_tex;

    SphereMapShader();
};

class SplattingShader : public ShaderHelperSingleton<SplattingShader, core::matrix4, video::SColorf>
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

class BillboardShader
{
public:
    static GLuint Program;
    static GLuint attrib_corner, attrib_texcoord;
    static GLuint uniform_MV, uniform_P, uniform_tex, uniform_Position, uniform_Size;

    static void init();
    static void setUniforms(const core::matrix4 &ModelViewMatrix, const core::matrix4 &ProjectionMatrix, const core::vector3df &Position, const core::dimension2d<float> &size, unsigned TU_tex);
};


class ColorizeShader
{
public:
    static GLuint Program;
    static GLuint uniform_MM, uniform_col;

    static void init();
    static void setUniforms(const core::matrix4 &ModelMatrix, float r, float g, float b);
};

class ShadowShader : public ShaderHelper<core::matrix4>
{
public:
    ShadowShader();
};

extern ShadowShader *ShadowShaderInstance;

class RSMShader
{
public:
    static GLuint Program;
    static GLuint uniform_MM, uniform_RSMMatrix;
    static GLuint TU_tex;

    static void init();
    static void setUniforms(const core::matrix4 &RSMMatrix, const core::matrix4 &ModelMatrix);
};

class InstancedShadowShader : public ShaderHelper<>
{
public:
    InstancedShadowShader();
};

extern InstancedShadowShader *InstancedShadowShaderInstance;

class RefShadowShader : public ShaderHelper<core::matrix4>
{
public:
    GLuint TU_tex;

    RefShadowShader();
};

extern RefShadowShader *RefShadowShaderInstance;

class InstancedRefShadowShader : public ShaderHelper<>
{
public:
    GLuint TU_tex;

    InstancedRefShadowShader();
};

extern InstancedRefShadowShader *InstancedRefShadowShaderInstance;

class GrassShadowShader : public ShaderHelper<core::matrix4, core::vector3df>
{
public:
    GLuint TU_tex;
    GrassShadowShader();
};

extern GrassShadowShader *GrassShadowShaderInstance;

class InstancedGrassShadowShader : public ShaderHelper<core::vector3df>
{
public:
    GLuint TU_tex;
    InstancedGrassShadowShader();
};

extern InstancedGrassShadowShader *InstancedGrassShadowShaderInstance;

class DisplaceMaskShader : public ShaderHelper<core::matrix4>
{
public:
    DisplaceMaskShader();
};

extern DisplaceMaskShader *DisplaceMaskShaderInstance;

class DisplaceShader : public ShaderHelper<core::matrix4, core::vector2df, core::vector2df>
{
public:
    GLuint TU_displacement_tex, TU_mask_tex, TU_color_tex, TU_tex;

    DisplaceShader();
};

extern DisplaceShader *DisplaceShaderInstance;

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

class BloomShader
{
public:
    static GLuint Program;
    static GLuint uniform_texture;
    static GLuint vao;

    static void init();
    static void setUniforms(unsigned TU_tex);
};

class BloomBlendShader
{
public:
    static GLuint Program;
    static GLuint uniform_tex_128, uniform_tex_256, uniform_tex_512;
    static GLuint vao;

    static void init();
    static void setUniforms(unsigned TU_tex_128, unsigned TU_tex_256, unsigned TU_tex_512);
};

class ToneMapShader
{
public:
    static GLuint Program;
    static GLuint uniform_tex, uniform_logluminancetex, uniform_exposure, uniform_lwhite;
    static GLuint vao;

    static void init();
    static void setUniforms(float exposure, float Lwhite, unsigned TU_tex, unsigned TU_logluminance);
};

class DepthOfFieldShader
{
public:
    static GLuint Program;
    static GLuint uniform_tex, uniform_depth;
    static GLuint vao;

    static void init();
    static void setUniforms(unsigned TU_tex, unsigned TU_depth);
};

class SunLightShader
{
public:
    static GLuint Program;
    static GLuint uniform_ntex, uniform_dtex, uniform_direction, uniform_col;
    static GLuint vao;

    static void init();
    static void setUniforms(const core::vector3df &direction, float r, float g, float b, unsigned TU_ntex, unsigned TU_dtex);
};

class DiffuseEnvMapShader
{
public:
    static GLuint Program;
    static GLuint uniform_ntex, uniform_TVM, uniform_blueLmn, uniform_greenLmn, uniform_redLmn;
    static GLuint vao;

    static void init();
    static void setUniforms(const core::matrix4 &TransposeViewMatrix, const float *blueSHCoeff, const float *greenSHCoeff, const float *redSHCoeff, unsigned TU_ntex);
};

class ShadowedSunLightShader
{
public:
    static GLuint Program;
    static GLuint uniform_ntex, uniform_dtex, uniform_shadowtex, uniform_direction, uniform_col;
    static GLuint vao;

    static void init();
    static void setUniforms(const core::vector3df &direction, float r, float g, float b, unsigned TU_ntex, unsigned TU_dtex, unsigned TU_shadowtex);
};

class ShadowedSunLightDebugShader
{
public:
    static GLuint Program;
    static GLuint uniform_ntex, uniform_dtex, uniform_shadowtex, uniform_direction, uniform_col;
    static GLuint vao;

    static void init();
    static void setUniforms(const core::vector3df &direction, float r, float g, float b, unsigned TU_ntex, unsigned TU_dtex, unsigned TU_shadowtex);
};

class RadianceHintsConstructionShader
{
public:
    static GLuint Program;
    static GLuint uniform_ctex, uniform_ntex, uniform_dtex, uniform_extents, uniform_RHMatrix, uniform_RSMMatrix;
    static GLuint vao;

    static void init();
    static void setUniforms(const core::matrix4 &RSMMatrix, const core::matrix4 &RHMatrix, const core::vector3df &extents, unsigned TU_ctex, unsigned TU_ntex, unsigned TU_dtex);
};

class RHDebug
{
public:
    static GLuint Program;
    static GLuint uniform_extents, uniform_SHR, uniform_SHG, uniform_SHB, uniform_RHMatrix;

    static void init();
    static void setUniforms(const core::matrix4 &RHMatrix, const core::vector3df &extents, unsigned TU_SHR, unsigned TU_SHG, unsigned TU_SHB);
};

class GlobalIlluminationReconstructionShader
{
public:
    static GLuint Program;
    static GLuint uniform_ntex, uniform_dtex, uniform_extents, uniform_SHR, uniform_SHG, uniform_SHB, uniform_RHMatrix, uniform_InvRHMatrix;
    static GLuint vao;

    static void init();
    static void setUniforms(const core::matrix4 &RHMatrix, const core::matrix4 &InvRHMatrix, const core::vector3df &extents, unsigned TU_ntex, unsigned TU_dtex, unsigned TU_SHR, unsigned TU_SHG, unsigned TU_SHB);
};

class Gaussian17TapHShader
{
public:
    static GLuint Program;
    static GLuint uniform_tex, uniform_depth, uniform_pixel;
    static GLuint vao;

    static void init();
};

class ComputeGaussian17TapHShader
{
public:
    static GLuint Program;
    static GLuint uniform_source, uniform_depth, uniform_dest;

    static void init();
};

class Gaussian6HBlurShader
{
public:
    static GLuint Program;
    static GLuint uniform_tex, uniform_pixel;
    static GLuint vao;

    static void init();
};

class Gaussian3HBlurShader
{
public:
    static GLuint Program;
    static GLuint uniform_tex, uniform_pixel;
    static GLuint vao;

    static void init();
};

class Gaussian17TapVShader
{
public:
    static GLuint Program;
    static GLuint uniform_tex, uniform_depth, uniform_pixel;
    static GLuint vao;

    static void init();
};

class ComputeGaussian17TapVShader
{
public:
    static GLuint Program;
    static GLuint uniform_source, uniform_depth, uniform_dest;

    static void init();
};


class Gaussian6VBlurShader
{
public:
    static GLuint Program;
    static GLuint uniform_tex, uniform_pixel;
    static GLuint vao;

    static void init();
};

class Gaussian3VBlurShader
{
public:
    static GLuint Program;
    static GLuint uniform_tex, uniform_pixel;
    static GLuint vao;

    static void init();
};

class PassThroughShader
{
public:
    static GLuint Program;
    static GLuint uniform_texture;
    static GLuint vao;

    static void init();
};

class LayerPassThroughShader
{
public:
    static GLuint Program;
    static GLuint uniform_layer, uniform_texture;
    static GLuint vao;

    static void init();
};

class LinearizeDepthShader
{
public:
    static GLuint Program;
    static GLuint uniform_zn, uniform_zf, uniform_texture;
    static GLuint vao;

    static void init();
    static void setUniforms(float zn, float zf, unsigned TU_tex);
};

class GlowShader
{
public:
    static GLuint Program;
    static GLuint uniform_tex;
    static GLuint vao;

    static void init();
};

class SSAOShader
{
public:
    static GLuint Program;
    static GLuint uniform_ntex, uniform_dtex, uniform_noise_texture, uniform_samplePoints;
    static GLuint vao;
    static float SSAOSamples[64];
    
    static void init();
    static void setUniforms(const core::vector2df &screen, unsigned TU_dtex, unsigned TU_noise);
};

class FogShader
{
public:
    static GLuint Program;
    static GLuint uniform_tex, uniform_fogmax, uniform_startH, uniform_endH, uniform_start, uniform_end, uniform_col;
    static GLuint vao;

    static void init();
    static void setUniforms(float fogmax, float startH, float endH, float start, float end, const core::vector3df &col, unsigned TU_ntex);
};

class MotionBlurShader
{
public:
    static GLuint Program;
    static GLuint uniform_boost_amount, uniform_color_buffer, uniform_dtex, uniform_previous_viewproj, uniform_center, uniform_mask_radius;
    static GLuint vao;

    static void init();
    static void setUniforms(float boost_amount, const core::matrix4 &previousVP, const core::vector2df &center,  float mask_radius, unsigned TU_cb, unsigned TU_dtex);
};

class GodFadeShader
{
public:
    static GLuint Program;
    static GLuint uniform_tex, uniform_col;
    static GLuint vao;

    static void init();
    static void setUniforms(const video::SColor &col, unsigned TU_tex);
};

class GodRayShader
{
public:
    static GLuint Program;
    static GLuint uniform_tex, uniform_sunpos;
    static GLuint vao;

    static void init();
    static void setUniforms(const core::vector2df &sunpos, unsigned TU_tex);
};

class MLAAColorEdgeDetectionSHader
{
public:
    static GLuint Program;
    static GLuint uniform_colorMapG, uniform_PIXEL_SIZE;
    static GLuint vao;

    static void init();
    static void setUniforms(const core::vector2df &PIXEL_SIZE, unsigned TU_colorMapG);
};

class MLAABlendWeightSHader
{
public:
    static GLuint Program;
    static GLuint uniform_PIXEL_SIZE, uniform_edgesMap, uniform_areaMap;

    static GLuint vao;

    static void init();
    static void setUniforms(const core::vector2df &PIXEL_SIZE, unsigned TU_edgesMap, unsigned TU_areaMap);

};

class MLAAGatherSHader
{
public:
    static GLuint Program;
    static GLuint uniform_PIXEL_SIZE, uniform_colorMap, uniform_blendMap;
    static GLuint vao;

    static void init();
    static void setUniforms(const core::vector2df &PIXEL_SIZE, unsigned TU_colormap, unsigned TU_blendmap);
};

}

namespace UIShader
{
class TextureRectShader
{
public:
    static GLuint Program;
    static GLuint attrib_position, attrib_texcoord;
    static GLuint uniform_tex, uniform_center, uniform_size, uniform_texcenter, uniform_texsize;
    static GLuint vao;

    static void init();
    static void setUniforms(float center_pos_x, float center_pos_y, float width, float height, float tex_center_pos_x, float tex_center_pos_y, float tex_width, float tex_height, unsigned TU_tex);
};

class UniformColoredTextureRectShader
{
public:
    static GLuint Program;
    static GLuint attrib_position, attrib_texcoord;
    static GLuint uniform_tex, uniform_color, uniform_center, uniform_size, uniform_texcenter, uniform_texsize;
    static GLuint vao;

    static void init();
    static void setUniforms(float center_pos_x, float center_pos_y, float width, float height, float tex_center_pos_x, float tex_center_pos_y, float tex_width, float tex_height, const video::SColor &color, unsigned TU_tex);
};

class ColoredTextureRectShader
{
public:
    static GLuint Program;
    static GLuint attrib_position, attrib_texcoord, attrib_color;
    static GLuint uniform_tex, uniform_center, uniform_size, uniform_texcenter, uniform_texsize;
    static GLuint colorvbo;
    static GLuint vao;

    static void init();
    static void setUniforms(float center_pos_x, float center_pos_y, float width, float height, float tex_center_pos_x, float tex_center_pos_y, float tex_width, float tex_height, unsigned TU_tex);
};

class ColoredRectShader
{
public:
    static GLuint Program;
    static GLuint attrib_position;
    static GLuint uniform_center, uniform_size, uniform_color;
    static GLuint vao;

    static void init();
    static void setUniforms(float center_pos_x, float center_pos_y, float width, float height, const video::SColor &color);
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
