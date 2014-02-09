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

typedef unsigned int	GLuint;
using namespace irr;

namespace MeshShader
{
class ObjectPass1Shader
{
public:
	static GLuint Program;
	static GLuint attrib_position, attrib_normal;
	static GLuint uniform_MVP, uniform_TIMV;

	static void init();
	static void setUniforms(const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView);
};

class ObjectRefPass1Shader
{
public:
	static GLuint Program;
	static GLuint attrib_position, attrib_normal, attrib_texcoord;
	static GLuint uniform_MVP, uniform_TIMV, uniform_tex;

	static void init();
	static void setUniforms(const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView, unsigned TU_texture);
};

class ObjectPass2Shader
{
public:
	static GLuint Program;
	static GLuint attrib_position, attrib_texcoord;
	static GLuint uniform_MVP, uniform_Albedo, uniform_DiffuseMap, uniform_SpecularMap, uniform_SSAO, uniform_screen, uniform_ambient;

	static void init();
	static void setUniforms(const core::matrix4 &ModelViewProjectionMatrix, unsigned TU_Albedo, unsigned TU_DiffuseMap, unsigned TU_SpecularMap, unsigned TU_SSAO);
};

class DetailledObjectPass2Shader
{
public:
	static GLuint Program;
	static GLuint attrib_position, attrib_texcoord, attrib_second_texcoord;
	static GLuint uniform_MVP, uniform_Albedo, uniform_Detail, uniform_DiffuseMap, uniform_SpecularMap, uniform_SSAO, uniform_screen, uniform_ambient;

	static void init();
	static void setUniforms(const core::matrix4 &ModelViewProjectionMatrix, unsigned TU_Albedo, unsigned TU_detail, unsigned TU_DiffuseMap, unsigned TU_SpecularMap, unsigned TU_SSAO);
};

class ObjectRimLimitShader
{
public:
	static GLuint Program;
	static GLuint attrib_position, attrib_normal, attrib_texcoord;
	static GLuint uniform_MVP, uniform_TIMV, uniform_Albedo, uniform_DiffuseMap, uniform_SpecularMap, uniform_SSAO,	uniform_screen,	uniform_ambient;

	static void init();
	static void setUniforms(const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView, unsigned TU_Albedo, unsigned TU_DiffuseMap, unsigned TU_SpecularMap, unsigned TU_SSAO);
};

class UntexturedObjectShader
{
public:
	static GLuint Program;
	static GLuint attrib_position, attrib_color;
	static GLuint uniform_MVP, uniform_DiffuseMap, uniform_SpecularMap, uniform_SSAO, uniform_screen, uniform_ambient;

	static void init();
	static void setUniforms(const core::matrix4 &ModelViewProjectionMatrix, unsigned TU_DiffuseMap, unsigned TU_SpecularMap, unsigned TU_SSAO);
};

class ObjectUnlitShader
{
public:
	static GLuint Program;
	static GLuint attrib_position, attrib_texcoord;
	static GLuint uniform_MVP, uniform_tex;

	static void init();
	static void setUniforms(const core::matrix4 &ModelViewProjectionMatrix, unsigned TU_tex);
};

class ObjectRefPass2Shader
{
public:
	static GLuint Program;
	static GLuint attrib_position, attrib_texcoord;
	static GLuint uniform_MVP, uniform_Albedo, uniform_DiffuseMap, uniform_SpecularMap, uniform_SSAO, uniform_screen, uniform_ambient;

	static void init();
	static void setUniforms(const core::matrix4 &ModelViewProjectionMatrix, unsigned TU_Albedo, unsigned TU_DiffuseMap, unsigned TU_SpecularMap, unsigned TU_SSAO);
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

class GrassPass2Shader
{
public:
	static GLuint Program;
	static GLuint attrib_position, attrib_texcoord, attrib_color;
	static GLuint uniform_MVP, uniform_Albedo, uniform_DiffuseMap, uniform_SpecularMap, uniform_SSAO, uniform_screen, uniform_ambient, uniform_windDir;

	static void init();
	static void setUniforms(const core::matrix4 &ModelViewProjectionMatrix, const core::vector3df &windDirection, unsigned TU_Albedo, unsigned TU_DiffuseMap, unsigned TU_SpecularMap, unsigned TU_SSAO);
};

class NormalMapShader
{
public:
	static GLuint Program;
	static GLuint attrib_position, attrib_texcoord, attrib_tangent, attrib_bitangent;
	static GLuint uniform_MVP, uniform_TIMV, uniform_normalMap;

	static void init();
	static void setUniforms(const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView, unsigned TU_normalMap);
};

class SphereMapShader
{
public:
	static GLuint Program;
	static GLuint attrib_position, attrib_normal;
	static GLuint uniform_MVP, uniform_TIMV, uniform_tex;

	static void init();
	static void setUniforms(const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView, unsigned TU_tex);
};

class SplattingShader
{
public:
	static GLuint Program;
	static GLuint attrib_position, attrib_texcoord, attrib_second_texcoord;
	static GLuint uniform_MVP, uniform_tex_layout, uniform_tex_detail0, uniform_tex_detail1, uniform_tex_detail2, uniform_tex_detail3, uniform_DiffuseMap, uniform_SpecularMap, uniform_SSAO, uniform_screen, uniform_ambient;

	static void init();
	static void setUniforms(const core::matrix4 &ModelViewProjectionMatrix, unsigned TU_tex_layout, unsigned TU_tex_detail0, unsigned TU_tex_detail1, unsigned TU_tex_detail2, unsigned TU_tex_detail3, unsigned TU_DiffuseMap, unsigned TU_SpecularMap, unsigned TU_SSAO);
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
	static GLuint attrib_position, attrib_texcoord;
	static GLuint uniform_MVP, uniform_tex;

	static void init();
	static void setUniforms(const core::matrix4 &ModelViewProjectionMatrix, unsigned TU_tex);
};

class TransparentFogShader
{
public:
    static GLuint Program;
    static GLuint attrib_position, attrib_texcoord;
    static GLuint uniform_MVP, uniform_tex, uniform_fogmax, uniform_startH, uniform_endH, uniform_start, uniform_end, uniform_col, uniform_screen, uniform_ipvmat;

    static void init();
    static void setUniforms(const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &ipvmat, float fogmax, float startH, float endH, float start, float end, const core::vector3df &col, const core::vector3df &campos, unsigned TU_tex);
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
	static GLuint uniform_MVP, uniform_col;

	static void init();
	static void setUniforms(const core::matrix4 &ModelViewProjectionMatrix, float r, float g, float b);
};

class ShadowShader
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
	static GLuint uniform_MVP, uniform_MV, uniform_tex, uniform_dir, uniform_dir2;

	static void init();
	static void setUniforms(const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &ModelViewMatrix, float dirX, float dirY, float dir2X, float dir2Y, unsigned TU_tex);
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
	static GLuint uniform_matrix, uniform_viewmatrix, uniform_tex, uniform_dtex, uniform_screen, uniform_invproj;

	static void init();
	static void setUniforms(const core::matrix4 &ViewMatrix, const core::matrix4 &ProjMatrix, const core::matrix4 InvProjMatrix, float width, float height, unsigned TU_tex, unsigned TU_normal_and_depth);
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
	static GLuint uniform_texture, uniform_low;
	static GLuint vao;

	static void init();
};

class BloomBlendShader
{
public:
	static GLuint Program;
	static GLuint uniform_texture, uniform_low;
	static GLuint vao;

	static void init();
};

class PPDisplaceShader
{
public:
	static GLuint Program;
	static GLuint uniform_tex, uniform_dtex, uniform_viz;
	static GLuint vao;

	static void init();
};

class ColorLevelShader
{
public:
	static GLuint Program;
	static GLuint uniform_tex, uniform_inlevel, uniform_outlevel;
	static GLuint vao;

	static void init();
};

class PointLightShader
{
public:
	static GLuint Program;
	static GLuint uniform_ntex, uniform_dtex, uniform_center, uniform_col, uniform_energy, uniform_spec, uniform_invproj, uniform_viewm;
	static GLuint vao;

	static void init();
	static void setUniforms(const core::matrix4 &InvProjMatrix, const core::matrix4 &ViewMatrix, const std::vector<float> &positions, const std::vector<float> &colors, const std::vector<float> &energy, unsigned spec, unsigned TU_ntex, unsigned TU_dtex);
};

class SunLightShader
{
public:
	static GLuint Program;
	static GLuint uniform_ntex, uniform_dtex, uniform_direction, uniform_col, uniform_invproj;
	static GLuint vao;

	static void init();
	static void setUniforms(const core::vector3df &direction, const core::matrix4 &InvProjMatrix, float r, float g, float b, unsigned TU_ntex, unsigned TU_dtex);
};

class ShadowedSunLightShader
{
public:
    static GLuint Program;
    static GLuint uniform_ntex, uniform_dtex, uniform_shadowtex, uniform_shadowmat, uniform_direction, uniform_col, uniform_invproj;
    static GLuint vao;

    static void init();
    static void setUniforms(const core::matrix4 &shadowmat, const core::vector3df &direction, const core::matrix4 &InvProjMatrix, float r, float g, float b, unsigned TU_ntex, unsigned TU_dtex, unsigned TU_shadowtex);
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
	static GLuint uniform_ntex, uniform_dtex, uniform_noise_texture, uniform_invprojm, uniform_projm, uniform_samplePoints;
	static GLuint vao;
	static float SSAOSamples[64];
	
	static void init();
	static void setUniforms(const core::matrix4& projm, const core::matrix4 &invprojm, unsigned TU_ntex, unsigned TU_dtex, unsigned TU_noise);
};

class FogShader
{
public:
	static GLuint Program;
	static GLuint uniform_tex, uniform_fogmax, uniform_startH, uniform_endH, uniform_start, uniform_end, uniform_col, uniform_ipvmat;
	static GLuint vao;

	static void init();
	static void setUniforms(const core::matrix4 &ipvmat, float fogmax, float startH, float endH, float start, float end, const core::vector3df &col, unsigned TU_ntex);
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
    ACT(ES_COLORIZE_REF) \
	ACT(ES_OBJECT_UNLIT) \
    ACT(ES_OBJECTPASS) \
    ACT(ES_OBJECTPASS_REF) \
    ACT(ES_SUNLIGHT) \
    ACT(ES_SUNLIGHT_SHADOW) \
    ACT(ES_OBJECTPASS_RIMLIT) \
    ACT(ES_MLAA_COLOR1) \
    ACT(ES_MLAA_BLEND2) \
    ACT(ES_MLAA_NEIGH3) \
    ACT(ES_GODFADE) \
    ACT(ES_GODRAY) \
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
