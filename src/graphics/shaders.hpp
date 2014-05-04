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

typedef unsigned int    GLuint;
using namespace irr;
class ParticleSystemProxy;

class SharedObject
{
public:
    static GLuint billboardvbo;
    static GLuint cubevbo, cubeindexes;
    static GLuint ViewProjectionMatrixesUBO;
};
namespace MeshShader
{
class ObjectPass1Shader
{
public:
    static GLuint Program;
    static GLuint attrib_position, attrib_texcoord, attrib_normal;
    static GLuint uniform_MM, uniform_IMM, uniform_tex;

    static void init();
    static void setUniforms(const core::matrix4 &ModelMatrix, const core::matrix4 &InverseModelMatrix, unsigned TU_tex);
};

class ObjectRefPass1Shader
{
public:
    static GLuint Program;
    static GLuint attrib_position, attrib_normal, attrib_texcoord;
    static GLuint uniform_MM, uniform_TM, uniform_IMM, uniform_tex;

    static void init();
    static void setUniforms(const core::matrix4 &ModelMatrix, const core::matrix4 &InverseModelMatrix, const core::matrix4 &TextureMatrix, unsigned TU_texture);
};

class GrassPass1Shader
{
public:
    static GLuint Program;
    static GLuint attrib_position, attrib_texcoord, attrib_normal, attrib_color;
    static GLuint uniform_MVP, uniform_TIMV, uniform_tex, uniform_windDir;

    static void init();
    static void setUniforms(const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView, const core::vector3df &windDirection, unsigned TU_tex);
};

class NormalMapShader
{
public:
    static GLuint Program;
    static GLuint attrib_position, attrib_texcoord, attrib_tangent, attrib_bitangent;
    static GLuint uniform_MM, uniform_IMM, uniform_normalMap, uniform_DiffuseForAlpha;

    static void init();
    static void setUniforms(const core::matrix4 &ModelMatrix, const core::matrix4 &InverseModelMatrix, unsigned TU_normalMap, unsigned TU_uniform_DiffuseForAlpha);
};

class InstancedObjectPass1Shader
{
public:
    static GLuint Program;
    static GLuint attrib_position, attrib_normal, attrib_texcoord, attrib_origin, attrib_orientation, attrib_scale;
    static GLuint uniform_tex;

    static void init();
    static void setUniforms(unsigned TU_tex);
};

class InstancedObjectRefPass1Shader
{
public:
    static GLuint Program;
    static GLuint attrib_position, attrib_normal, attrib_texcoord, attrib_origin, attrib_orientation, attrib_scale;
    static GLuint uniform_tex;

    static void init();
    static void setUniforms(unsigned TU_tex);
};

class InstancedGrassPass1Shader
{
public:
    static GLuint Program;
    static GLuint attrib_position, attrib_normal, attrib_origin, attrib_orientation, attrib_scale, attrib_color, attrib_texcoord;
    static GLuint uniform_windDir, uniform_tex;

    static void init();
    static void setUniforms(const core::vector3df &windDir, unsigned TU_tex);
};

class ObjectPass2Shader
{
public:
    static GLuint Program;
    static GLuint attrib_position, attrib_texcoord;
    static GLuint uniform_MM, uniform_TM, uniform_screen, uniform_ambient;
    static GLuint TU_Albedo;

    static void init();
    static void setUniforms(const core::matrix4 &ModelMatrix, const core::matrix4 &TextureMatrix);
};

class InstancedObjectPass2Shader
{
public:
    static GLuint Program;
    static GLuint attrib_position, attrib_texcoord, attrib_origin, attrib_orientation, attrib_scale;
    static GLuint uniform_VP, uniform_TM, uniform_screen, uniform_ambient;
    static GLuint TU_Albedo;

    static void init();
    static void setUniforms(const core::matrix4 &ViewProjectionMatrix, const core::matrix4 &TextureMatrix);
};

class InstancedObjectRefPass2Shader
{
public:
    static GLuint Program;
    static GLuint attrib_position, attrib_texcoord, attrib_origin, attrib_orientation, attrib_scale;
    static GLuint uniform_VP, uniform_TM, uniform_screen, uniform_ambient;
    static GLuint TU_Albedo;

    static void init();
    static void setUniforms(const core::matrix4 &ViewProjectionMatrix, const core::matrix4 &TextureMatrix);
};

class DetailledObjectPass2Shader
{
public:
    static GLuint Program;
    static GLuint attrib_position, attrib_texcoord, attrib_second_texcoord;
    static GLuint uniform_MM, uniform_screen, uniform_ambient;
    static GLuint TU_Albedo, TU_detail;

    static void init();
    static void setUniforms(const core::matrix4 &ModelMatrix);
};

class ObjectRimLimitShader
{
public:
    static GLuint Program;
    static GLuint attrib_position, attrib_normal, attrib_texcoord;
    static GLuint uniform_MM, uniform_IMM, uniform_TM, uniform_screen, uniform_ambient;
    static GLuint TU_Albedo;

    static void init();
    static void setUniforms(const core::matrix4 &ModelMatrix, const core::matrix4 &InverseModelMatrix, const core::matrix4 &TextureMatrix);
};

class UntexturedObjectShader
{
public:
    static GLuint Program;
    static GLuint attrib_position, attrib_color;
    static GLuint uniform_MM, uniform_screen, uniform_ambient;

    static void init();
    static void setUniforms(const core::matrix4 &ModelMatrix);
};

class ObjectUnlitShader
{
public:
    static GLuint Program;
    static GLuint attrib_position, attrib_texcoord;
    static GLuint uniform_MM;
    static GLuint TU_tex;

    static void init();
    static void setUniforms(const core::matrix4 &ModelMatrix);
};

class ObjectRefPass2Shader
{
public:
    static GLuint Program;
    static GLuint attrib_position, attrib_texcoord;
    static GLuint uniform_MM, uniform_TM, uniform_screen, uniform_ambient;
    static GLuint TU_Albedo;

    static void init();
    static void setUniforms(const core::matrix4 &ModelMatrix, const core::matrix4 &TextureMatrix);
};

class GrassPass2Shader
{
public:
    static GLuint Program;
    static GLuint attrib_position, attrib_texcoord, attrib_color;
    static GLuint uniform_MVP, uniform_screen, uniform_ambient, uniform_windDir;
    static GLuint TU_Albedo;

    static void init();
    static void setUniforms(const core::matrix4 &ModelViewProjectionMatrix, const core::vector3df &windDirection);
};

class InstancedGrassPass2Shader
{
public:
    static GLuint Program;
    static GLuint attrib_position, attrib_texcoord, attrib_normal, attrib_color, attrib_origin, attrib_orientation, attrib_scale;
    static GLuint uniform_VP, uniform_TM, uniform_IVM, uniform_screen, uniform_ambient, uniform_windDir, uniform_invproj, uniform_SunDir;
    static GLuint TU_Albedo, TU_dtex;

    static void init();
    static void setUniforms(const core::matrix4 &ViewProjectionMatrix, const core::matrix4 &InverseViewMatrix, const core::matrix4 &invproj, const core::vector3df &windDirection, const core::vector3df &SunDir);
};

class SphereMapShader
{
public:
    static GLuint Program;
    static GLuint attrib_position, attrib_normal;
    static GLuint uniform_MM, uniform_IMM, uniform_screen;
    static GLuint TU_tex;

    static void init();
    static void setUniforms(const core::matrix4 &ModelMatrix, const core::matrix4 &InverseModelMatrix, const core::vector2df& screen);
};

class SplattingShader
{
public:
    static GLuint Program;
    static GLuint attrib_position, attrib_texcoord, attrib_second_texcoord;
    static GLuint uniform_MM, uniform_screen, uniform_ambient;
    static GLuint TU_tex_layout, TU_tex_detail0, TU_tex_detail1, TU_tex_detail2, TU_tex_detail3;

    static void init();
    static void setUniforms(const core::matrix4 &ModelMatrix);
};

class CausticsShader
{
public:
    static GLuint Program;
    static GLuint attrib_position, attrib_texcoord;
    static GLuint uniform_MVP, uniform_dir, uniform_dir2, uniform_screen, uniform_ambient;
    static GLuint TU_Albedo, TU_caustictex;

    static void init();
    static void setUniforms(const core::matrix4 &ModelViewProjectionMatrix, const core::vector2df &dir, const core::vector2df &dir2, const core::vector2df &screen);
};

class BubbleShader
{
public:
    static GLuint Program;
    static GLuint attrib_position, attrib_texcoord;
    static GLuint uniform_MVP, uniform_tex, uniform_time, uniform_transparency;

    static void init();
    static void setUniforms(const core::matrix4 &ModelViewProjectionMatrix, unsigned TU_tex, float time, float transparency);
};

class TransparentShader
{
public:
    static GLuint Program;
    static GLuint attrib_position, attrib_texcoord, attrib_color;
    static GLuint uniform_MVP, uniform_TM, uniform_tex;

    static void init();
    static void setUniforms(const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TextureMatrix, unsigned TU_tex);
};

class TransparentFogShader
{
public:
    static GLuint Program;
    static GLuint attrib_position, attrib_texcoord, attrib_color;
    static GLuint uniform_MVP, uniform_TM, uniform_tex, uniform_fogmax, uniform_startH, uniform_endH, uniform_start, uniform_end, uniform_col, uniform_screen, uniform_ipvmat;

    static void init();
    static void setUniforms(const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TextureMatrix, const core::matrix4 &ipvmat, float fogmax, float startH, float endH, float start, float end, const core::vector3df &col, const core::vector3df &campos, unsigned TU_tex);
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
    static GLuint attrib_position;
    static GLuint uniform_MM, uniform_col;

    static void init();
    static void setUniforms(const core::matrix4 &ModelMatrix, float r, float g, float b);
};

class ShadowShader
{
public:
    static GLuint Program;
    static GLuint attrib_position;
    static GLuint uniform_MM, uniform_ViewProjectionMatrixesUBO;

    static void init();
    static void setUniforms(const core::matrix4 &ModelMatrix);
};

class InstancedShadowShader
{
public:
    static GLuint Program;
    static GLuint attrib_position, attrib_origin, attrib_orientation, attrib_scale;

    static void init();
    static void setUniforms();
};

class RefShadowShader
{
public:
    static GLuint Program;
    static GLuint attrib_position, attrib_texcoord;
    static GLuint uniform_MM, uniform_tex;

    static void init();
    static void setUniforms(const core::matrix4 &ModelMatrix, unsigned TU_tex);
};

class InstancedRefShadowShader
{
public:
    static GLuint Program;
    static GLuint attrib_position, attrib_texcoord, attrib_origin, attrib_orientation, attrib_scale;
    static GLuint uniform_tex;

    static void init();
    static void setUniforms(unsigned TU_tex);
};

class GrassShadowShader
{
public:
    static GLuint Program;
    static GLuint attrib_position, attrib_texcoord, attrib_color;
    static GLuint uniform_MVP, uniform_tex, uniform_windDir;

    static void init();
    static void setUniforms(const core::matrix4 &ModelViewProjectionMatrix, const core::vector3df &windDirection, unsigned TU_tex);
};

class DisplaceMaskShader
{
public:
    static GLuint Program;
    static GLuint attrib_position;
    static GLuint uniform_MVP;

    static void init();
    static void setUniforms(const core::matrix4 &ModelViewProjectionMatrix);
};

class DisplaceShader
{
public:
    static GLuint Program;
    static GLuint attrib_position, attrib_texcoord, attrib_second_texcoord;
    static GLuint uniform_MVP, uniform_MV, uniform_displacement_tex, uniform_mask_tex, uniform_color_tex, uniform_screen, uniform_dir, uniform_dir2;

    static void init();
    static void setUniforms(const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &ModelViewMatrix, const core::vector2df &dir, const core::vector2df &dir2, const core::vector2df &screen, unsigned TU_displacement_tex, unsigned TU_mask_tex, unsigned TU_color_tex);
};

class SkyboxShader
{
public:
    static GLuint Program;
    static GLuint attrib_position;
    static GLuint uniform_MM, uniform_tex, uniform_screen;
    static GLuint cubevao;

    static void init();
    static void setUniforms(const core::matrix4 &ModelMatrix, const core::vector2df &screen, unsigned TU_tex);
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
        float padding;
    };


    class PointLightShader
    {
    public:
        static GLuint Program;
        static GLuint attrib_Position, attrib_Energy, attrib_Color;
        static GLuint uniform_ntex, uniform_dtex, uniform_spec, uniform_screen;
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
    static GLuint uniform_matrix, uniform_viewmatrix, uniform_tex, uniform_dtex, uniform_screen, uniform_invproj, uniform_color_from, uniform_color_to;

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
    static GLuint uniform_matrix, uniform_viewmatrix, uniform_tex, uniform_dtex, uniform_screen, uniform_invproj;

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
    static GLuint uniform_texture;
    static GLuint vao;

    static void init();
    static void setUniforms(unsigned TU_tex);
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
    static GLuint uniform_tex, uniform_depth, uniform_screen, uniform_invproj;
    static GLuint vao;

    static void init();
    static void setUniforms(const core::matrix4 &invproj, const core::vector2df &screen, unsigned TU_tex, unsigned TU_depth);
};

class ColorLevelShader
{
public:
    static GLuint Program;
    static GLuint uniform_tex, uniform_invprojm, uniform_dtex, uniform_inlevel, uniform_outlevel;
    static GLuint vao;

    static void init();
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

class PenumbraHShader
{
public:
    static GLuint Program;
    static GLuint uniform_tex, uniform_pixel;
    static GLuint vao;

    static void init();
    static void setUniforms(const core::vector2df &pixels, GLuint TU_tex);
};

class PenumbraVShader
{
public:
    static GLuint Program;
    static GLuint uniform_tex, uniform_pixel;
    static GLuint vao;

    static void init();
    static void setUniforms(const core::vector2df &pixels, GLuint TU_tex);
};

class ShadowGenShader
{
public:
    static GLuint Program;
    static GLuint uniform_halft, uniform_quarter, uniform_height;
    static GLuint vao;

    static void init();
    static void setUniforms(GLuint TU_halft, GLuint TU_quarter, GLuint TU_height);
};

class PassThroughShader
{
public:
    static GLuint Program;
    static GLuint uniform_texture;
    static GLuint vao;

    static void init();
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
    static void setUniforms(unsigned TU_ntex, unsigned TU_dtex, unsigned TU_noise);
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
    static GLuint uniform_boost_amount, uniform_color_buffer, uniform_center, uniform_direction, uniform_mask_radius, uniform_max_tex_height;
    static GLuint vao;

    static void init();
    static void setUniforms(float boost_amount, const core::vector2df &center, const core::vector2df &direction, float mask_radius, float max_tex_height, unsigned TU_cb);
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

class LogLuminanceShader
{
public:
    static GLuint Program;
    static GLuint uniform_tex;
    static GLuint vao;

    static void init();
    static void setUniforms(unsigned TU_tex);
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
    ACT(ES_RAIN) \
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
    ACT(ES_SHADOWPASS) \
    ACT(ES_SHADOW_IMPORTANCE) \
    ACT(ES_COLLAPSE) \
    ACT(ES_SHADOW_WARPH) \
    ACT(ES_SHADOW_WARPV) \
    ACT(ES_MULTIPLY_ADD) \
    ACT(ES_PENUMBRAH) \
    ACT(ES_PENUMBRAV) \
    ACT(ES_SHADOWGEN) \
    ACT(ES_CAUSTICS) \
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
