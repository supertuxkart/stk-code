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
	static GLuint uniform_MVP, uniform_TIMV, uniform_Albedo, uniform_DiffuseMap, uniform_SpecularMap, uniform_SSAO, uniform_screen, uniform_ambient;

	static void init();
	static void setUniforms(const core::matrix4 &ModelViewProjectionMatrix, unsigned TU_Albedo, unsigned TU_DiffuseMap, unsigned TU_SpecularMap, unsigned TU_SSAO);
};

class ObjectRefPass2Shader
{
public:
	static GLuint Program;
	static GLuint attrib_position, attrib_texcoord;
	static GLuint uniform_MVP, uniform_TIMV, uniform_Albedo, uniform_DiffuseMap, uniform_SpecularMap, uniform_SSAO, uniform_screen, uniform_ambient;

	static void init();
	static void setUniforms(const core::matrix4 &ModelViewProjectionMatrix, unsigned TU_Albedo, unsigned TU_DiffuseMap, unsigned TU_SpecularMap, unsigned TU_SSAO);
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

class ColorizeShader
{
public:
	static GLuint Program;
	static GLuint attrib_position;
	static GLuint uniform_MVP, uniform_col;

	static void init();
	static void setUniforms(const core::matrix4 &ModelViewProjectionMatrix, float r, float g, float b);
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
	static GLuint uniform_ntex, uniform_center, uniform_col, uniform_energy, uniform_spec, uniform_invproj, uniform_viewm;
	static GLuint vao;

	static void init();
};

class LightBlendShader
{
public:
	static GLuint Program;
	static GLuint uniform_diffuse, uniform_specular, uniform_ambient_occlusion, uniform_specular_map, uniform_ambient;
	static GLuint vao;

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
	static GLuint uniform_normals_and_depth, uniform_noise_texture, uniform_invprojm, uniform_projm, uniform_samplePoints;
	static GLuint vao;
	static float SSAOSamples[64];
	
	static void init();
};

class FogShader
{
public:
	static GLuint Program;
	static GLuint uniform_tex, uniform_fogmax, uniform_startH, uniform_endH, uniform_start, uniform_end, uniform_col, uniform_campos, uniform_ipvmat;
	static GLuint vao;

	static void init();
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
    ACT(ES_GLOW) \
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
