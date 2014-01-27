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

#define SHADER_NAMES

#include "graphics/callbacks.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/shaders.hpp"
#include "io/file_manager.hpp"
#include "utils/log.hpp"
#include "graphics/glwrap.hpp"
#include <assert.h>
#include <IGPUProgrammingServices.h>

using namespace video;

Shaders::Shaders()
{
    // Callbacks
    memset(m_callbacks, 0, sizeof(m_callbacks));

    m_callbacks[ES_SKYBOX] = new SkyboxProvider();
    m_callbacks[ES_WATER] = new WaterShaderProvider();
    m_callbacks[ES_GRASS] = new GrassShaderProvider();
    m_callbacks[ES_BUBBLES] = new BubbleEffectProvider();
    m_callbacks[ES_MOTIONBLUR] = new MotionBlurProvider();
    m_callbacks[ES_GAUSSIAN3V] = m_callbacks[ES_GAUSSIAN3H] = new GaussianBlurProvider();
    m_callbacks[ES_MIPVIZ] = new MipVizProvider();
    m_callbacks[ES_COLORIZE] = new ColorizeProvider();
    m_callbacks[ES_GLOW] = new GlowProvider();
    m_callbacks[ES_OBJECTPASS] = new ObjectPassProvider();
    m_callbacks[ES_SUNLIGHT] = new SunLightProvider();
    m_callbacks[ES_MLAA_COLOR1] = new MLAAColor1Provider();
    m_callbacks[ES_MLAA_BLEND2] = new MLAABlend2Provider();
    m_callbacks[ES_MLAA_NEIGH3] = new MLAANeigh3Provider();
    m_callbacks[ES_GODRAY] = new GodRayProvider();
    m_callbacks[ES_SHADOWPASS] = new ShadowPassProvider();
    m_callbacks[ES_SHADOW_IMPORTANCE] = new ShadowImportanceProvider();
    m_callbacks[ES_COLLAPSE] = new CollapseProvider();
    m_callbacks[ES_MULTIPLY_ADD] = new MultiplyProvider();
    m_callbacks[ES_SHADOWGEN] = new ShadowGenProvider();
    m_callbacks[ES_CAUSTICS] = new CausticsProvider();
    m_callbacks[ES_DISPLACE] = new DisplaceProvider();

    for(s32 i=0 ; i < ES_COUNT ; i++)
        m_shaders[i] = -1;

    loadShaders();
}

GLuint quad_vbo = 0;

static void initQuadVBO()
{
	initGL();
	const float quad_vertex[] = {
		-1., -1., 0., 0., // UpperLeft
		-1., 1., 0., 1., // LowerLeft
		1., -1., 1., 0., // UpperRight
		1., 1., 1., 1., // LowerRight
	};
	glGenBuffers(1, &quad_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
	glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(float), quad_vertex, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Shaders::loadShaders()
{
    const std::string &dir = file_manager->getAsset(FileManager::SHADER, "");

    IGPUProgrammingServices * const gpu = irr_driver->getVideoDriver()->getGPUProgrammingServices();

    #define glsl(a, b, c) gpu->addHighLevelShaderMaterialFromFiles((a).c_str(), (b).c_str(), (IShaderConstantSetCallBack*) c)
    #define glslmat(a, b, c, d) gpu->addHighLevelShaderMaterialFromFiles((a).c_str(), (b).c_str(), (IShaderConstantSetCallBack*) c, d)
    #define glsl_noinput(a, b) gpu->addHighLevelShaderMaterialFromFiles((a).c_str(), (b).c_str(), (IShaderConstantSetCallBack*) 0)

    // Save previous shaders (used in case some shaders don't compile)
    int saved_shaders[ES_COUNT];
    memcpy(saved_shaders, m_shaders, sizeof(m_shaders));

    // Ok, go
    m_shaders[ES_NORMAL_MAP] = glslmat(dir + "normalmap.vert", dir + "normalmap.frag",
                                       m_callbacks[ES_NORMAL_MAP], EMT_SOLID_2_LAYER);

    m_shaders[ES_NORMAL_MAP_LIGHTMAP] = glslmat(dir + "normalmap.vert", dir + "normalmap.frag",
                                             m_callbacks[ES_NORMAL_MAP_LIGHTMAP], EMT_SOLID_2_LAYER);

    m_shaders[ES_SKYBOX] = glslmat(dir + "skybox.vert", dir + "skybox.frag",
                                   m_callbacks[ES_SKYBOX], EMT_TRANSPARENT_ALPHA_CHANNEL);

    m_shaders[ES_SPLATTING] = glslmat(dir + "splatting.vert", dir + "splatting.frag",
                                   m_callbacks[ES_SPLATTING], EMT_SOLID);

    m_shaders[ES_WATER] = glslmat(dir + "water.vert", dir + "water.frag",
                                  m_callbacks[ES_WATER], EMT_TRANSPARENT_ALPHA_CHANNEL);
    m_shaders[ES_WATER_SURFACE] = glsl(dir + "water.vert", dir + "pass.frag",
                                  m_callbacks[ES_WATER]);

    m_shaders[ES_SPHERE_MAP] = glslmat(dir + "objectpass.vert", dir + "objectpass_spheremap.frag",
                                       m_callbacks[ES_OBJECTPASS], EMT_SOLID);

	m_shaders[ES_GRASS] = glslmat(dir + "grass.vert", dir + "grass.frag",
                                  m_callbacks[ES_GRASS], EMT_TRANSPARENT_ALPHA_CHANNEL);
	m_shaders[ES_GRASS_REF] = glslmat(dir + "grass.vert", dir + "grass.frag",
                                  m_callbacks[ES_GRASS], EMT_TRANSPARENT_ALPHA_CHANNEL_REF);

    m_shaders[ES_BUBBLES] = glslmat(dir + "bubble.vert", dir + "bubble.frag",
                                    m_callbacks[ES_BUBBLES], EMT_TRANSPARENT_ALPHA_CHANNEL);

    m_shaders[ES_RAIN] = glslmat(dir + "rain.vert", dir + "rain.frag",
                                    m_callbacks[ES_RAIN], EMT_TRANSPARENT_ALPHA_CHANNEL);

    m_shaders[ES_MOTIONBLUR] = glsl(std::string(""), dir + "motion_blur.frag",
                                    m_callbacks[ES_MOTIONBLUR]);

	m_shaders[ES_GAUSSIAN3H] = glslmat(dir + "pass.vert", dir + "gaussian3h.frag",
                                    m_callbacks[ES_GAUSSIAN3H], EMT_SOLID);
	m_shaders[ES_GAUSSIAN3V] = glslmat(dir + "pass.vert", dir + "gaussian3v.frag",
                                    m_callbacks[ES_GAUSSIAN3V], EMT_SOLID);

    m_shaders[ES_MIPVIZ] = glslmat(std::string(""), dir + "mipviz.frag",
                                    m_callbacks[ES_MIPVIZ], EMT_SOLID);

    m_shaders[ES_COLORIZE] = glslmat(std::string(""), dir + "colorize.frag",
                                    m_callbacks[ES_COLORIZE], EMT_SOLID);
    m_shaders[ES_COLORIZE_REF] = glslmat(std::string(""), dir + "colorize_ref.frag",
                                    m_callbacks[ES_COLORIZE], EMT_SOLID);

    m_shaders[ES_GLOW] = glslmat(std::string(""), dir + "glow.frag",
                                    m_callbacks[ES_GLOW], EMT_TRANSPARENT_ALPHA_CHANNEL);

    m_shaders[ES_OBJECTPASS] = glslmat(dir + "objectpass.vert", dir + "objectpass.frag",
                                    m_callbacks[ES_OBJECTPASS], EMT_SOLID);
	m_shaders[ES_OBJECT_UNLIT] = glslmat(dir + "objectpass.vert", dir + "objectpass.frag",
									m_callbacks[ES_OBJECTPASS], EMT_SOLID);
    m_shaders[ES_OBJECTPASS_REF] = glslmat(dir + "objectpass.vert", dir + "objectpass_ref.frag",
                                    m_callbacks[ES_OBJECTPASS], EMT_SOLID);
    m_shaders[ES_OBJECTPASS_RIMLIT] = glslmat(dir + "objectpass_rimlit.vert", dir + "objectpass_rimlit.frag",
                                    m_callbacks[ES_OBJECTPASS], EMT_SOLID);

    m_shaders[ES_SUNLIGHT] = glslmat(std::string(""), dir + "sunlight.frag",
                                    m_callbacks[ES_SUNLIGHT], EMT_SOLID);
    m_shaders[ES_SUNLIGHT_SHADOW] = glslmat(dir + "pass.vert", dir + "sunlightshadow.frag",
                                    m_callbacks[ES_SUNLIGHT], EMT_SOLID);

    m_shaders[ES_MLAA_COLOR1] = glsl(dir + "mlaa_offset.vert", dir + "mlaa_color1.frag",
                                    m_callbacks[ES_MLAA_COLOR1]);
    m_shaders[ES_MLAA_BLEND2] = glsl(dir + "pass.vert", dir + "mlaa_blend2.frag",
                                    m_callbacks[ES_MLAA_BLEND2]);
    m_shaders[ES_MLAA_NEIGH3] = glsl(dir + "mlaa_offset.vert", dir + "mlaa_neigh3.frag",
                                    m_callbacks[ES_MLAA_NEIGH3]);

    m_shaders[ES_GODFADE] = glsl(std::string(""), dir + "godfade.frag", m_callbacks[ES_COLORIZE]);
    m_shaders[ES_GODRAY] = glsl(std::string(""), dir + "godray.frag", m_callbacks[ES_GODRAY]);

    m_shaders[ES_SHADOWPASS] = glsl(dir + "shadowpass.vert", dir + "shadowpass.frag",
                                    m_callbacks[ES_SHADOWPASS]);

    m_shaders[ES_SHADOW_IMPORTANCE] = glsl(dir + "shadowimportance.vert",
                                           dir + "shadowimportance.frag",
                                    m_callbacks[ES_SHADOW_IMPORTANCE]);

    m_shaders[ES_COLLAPSE] = glsl(std::string(""), dir + "collapse.frag",
                                    m_callbacks[ES_COLLAPSE]);
    m_shaders[ES_SHADOW_WARPH] = glsl(std::string(""), dir + "shadowwarph.frag",
                                    m_callbacks[ES_COLLAPSE]);
    m_shaders[ES_SHADOW_WARPV] = glsl(std::string(""), dir + "shadowwarpv.frag",
                                    m_callbacks[ES_COLLAPSE]);

    m_shaders[ES_MULTIPLY_ADD] = glslmat(std::string(""), dir + "multiply.frag",
                                    m_callbacks[ES_MULTIPLY_ADD], EMT_ONETEXTURE_BLEND);

    m_shaders[ES_PENUMBRAH] = glslmat(std::string(""), dir + "penumbrah.frag",
                                    m_callbacks[ES_GAUSSIAN3H], EMT_SOLID);
    m_shaders[ES_PENUMBRAV] = glslmat(std::string(""), dir + "penumbrav.frag",
                                    m_callbacks[ES_GAUSSIAN3H], EMT_SOLID);
    m_shaders[ES_SHADOWGEN] = glslmat(std::string(""), dir + "shadowgen.frag",
                                    m_callbacks[ES_SHADOWGEN], EMT_SOLID);

    m_shaders[ES_CAUSTICS] = glslmat(std::string(""), dir + "caustics.frag",
                                    m_callbacks[ES_CAUSTICS], EMT_TRANSPARENT_ALPHA_CHANNEL);

    m_shaders[ES_DISPLACE] = glsl(dir + "displace.vert", dir + "displace.frag",
                                  m_callbacks[ES_DISPLACE]);

    m_shaders[ES_PASSFAR] = glsl(dir + "farplane.vert", dir + "colorize.frag",
                                  m_callbacks[ES_COLORIZE]);

    // Check that all successfully loaded
    for (s32 i = 0; i < ES_COUNT; i++) {

        // Old Intel Windows drivers fail here.
        // It's an artist option, so not necessary to play.
        if (i == ES_MIPVIZ)
            continue;

        check(i);
    }

    #undef glsl
    #undef glslmat
    #undef glsl_noinput

    // In case we're reloading and a shader didn't compile: keep the previous, working one
    for(s32 i=0 ; i < ES_COUNT ; i++)
    {
        if(m_shaders[i] == -1)
            m_shaders[i] = saved_shaders[i];
    }

	initGL();
	initQuadVBO();
	FullScreenShader::BloomBlendShader::init();
	FullScreenShader::BloomShader::init();
	FullScreenShader::ColorLevelShader::init();
	FullScreenShader::FogShader::init();
	FullScreenShader::Gaussian3HBlurShader::init();
	FullScreenShader::Gaussian3VBlurShader::init();
	FullScreenShader::Gaussian6HBlurShader::init();
	FullScreenShader::Gaussian6VBlurShader::init();
	FullScreenShader::GlowShader::init();
	FullScreenShader::LightBlendShader::init();
	FullScreenShader::PassThroughShader::init();
	FullScreenShader::PointLightShader::init();
	FullScreenShader::PPDisplaceShader::init();
	FullScreenShader::SSAOShader::init();
	FullScreenShader::SunLightShader::init();
	MeshShader::ColorizeShader::init();
	MeshShader::NormalMapShader::init();
	MeshShader::ObjectPass1Shader::init();
	MeshShader::ObjectRefPass1Shader::init();
	MeshShader::ObjectPass2Shader::init();
	MeshShader::DetailledObjectPass2Shader::init();
	MeshShader::ObjectRimLimitShader::init();
	MeshShader::UntexturedObjectShader::init();
	MeshShader::ObjectRefPass2Shader::init();
	MeshShader::ObjectUnlitShader::init();
	MeshShader::SphereMapShader::init();
	MeshShader::SplattingShader::init();
	MeshShader::GrassPass1Shader::init();
	MeshShader::GrassPass2Shader::init();
	MeshShader::BubbleShader::init();
	MeshShader::TransparentShader::init();
	MeshShader::BillboardShader::init();
	MeshShader::DisplaceShader::init();
	ParticleShader::FlipParticleRender::init();
	ParticleShader::HeightmapSimulationShader::init();
	ParticleShader::SimpleParticleRender::init();
	ParticleShader::SimpleSimulationShader::init();
}

Shaders::~Shaders()
{
    u32 i;
    for (i = 0; i < ES_COUNT; i++)
    {
        if (i == ES_GAUSSIAN3V || !m_callbacks[i]) continue;
        delete m_callbacks[i];
    }
}

E_MATERIAL_TYPE Shaders::getShader(const ShaderType num) const
{
    assert(num < ES_COUNT);

    return (E_MATERIAL_TYPE) m_shaders[num];
}

void Shaders::check(const int num) const
{
    if (m_shaders[num] == -1)
    {
        Log::fatal("shaders", "Shader %s failed to load. Update your drivers, if the issue "
                              "persists, report a bug to us.", shader_names[num] + 3);
    }
}

namespace MeshShader
{
	GLuint ObjectPass1Shader::Program;
	GLuint ObjectPass1Shader::attrib_position;
	GLuint ObjectPass1Shader::attrib_normal;
	GLuint ObjectPass1Shader::uniform_MVP;
	GLuint ObjectPass1Shader::uniform_TIMV;

	void ObjectPass1Shader::init()
	{
		Program = LoadProgram(file_manager->getAsset("shaders/object_pass1.vert").c_str(), file_manager->getAsset("shaders/object_pass1.frag").c_str());
		attrib_position = glGetAttribLocation(Program, "Position");
		attrib_normal = glGetAttribLocation(Program, "Normal");
		uniform_MVP = glGetUniformLocation(Program, "ModelViewProjectionMatrix");
		uniform_TIMV = glGetUniformLocation(Program, "TransposeInverseModelView");
	}

	void ObjectPass1Shader::setUniforms(const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView)
	{
		glUniformMatrix4fv(uniform_MVP, 1, GL_FALSE, ModelViewProjectionMatrix.pointer());
		glUniformMatrix4fv(uniform_TIMV, 1, GL_FALSE, TransposeInverseModelView.pointer());
	}

	GLuint ObjectRefPass1Shader::Program;
	GLuint ObjectRefPass1Shader::attrib_position;
	GLuint ObjectRefPass1Shader::attrib_normal;
	GLuint ObjectRefPass1Shader::attrib_texcoord;
	GLuint ObjectRefPass1Shader::uniform_MVP;
	GLuint ObjectRefPass1Shader::uniform_TIMV;
	GLuint ObjectRefPass1Shader::uniform_tex;

	void ObjectRefPass1Shader::init()
	{
		Program = LoadProgram(file_manager->getAsset("shaders/objectref_pass1.vert").c_str(), file_manager->getAsset("shaders/objectref_pass1.frag").c_str());
		attrib_position = glGetAttribLocation(Program, "Position");
		attrib_normal = glGetAttribLocation(Program, "Normal");
		attrib_texcoord = glGetAttribLocation(Program, "Texcoord");
		uniform_MVP = glGetUniformLocation(Program, "ModelViewProjectionMatrix");
		uniform_TIMV = glGetUniformLocation(Program, "TransposeInverseModelView");
		uniform_tex = glGetUniformLocation(Program, "tex");
	}

	void ObjectRefPass1Shader::setUniforms(const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView, unsigned TU_tex)
	{
		glUniformMatrix4fv(uniform_MVP, 1, GL_FALSE, ModelViewProjectionMatrix.pointer());
		glUniformMatrix4fv(uniform_TIMV, 1, GL_FALSE, TransposeInverseModelView.pointer());
		glUniform1i(uniform_tex, TU_tex);
	}

	GLuint ObjectPass2Shader::Program;
	GLuint ObjectPass2Shader::attrib_position;
	GLuint ObjectPass2Shader::attrib_texcoord;
	GLuint ObjectPass2Shader::uniform_MVP;
	GLuint ObjectPass2Shader::uniform_Albedo;
	GLuint ObjectPass2Shader::uniform_DiffuseMap;
	GLuint ObjectPass2Shader::uniform_SpecularMap;
	GLuint ObjectPass2Shader::uniform_SSAO;
	GLuint ObjectPass2Shader::uniform_screen;
	GLuint ObjectPass2Shader::uniform_ambient;

	void ObjectPass2Shader::init()
	{
		Program = LoadProgram(file_manager->getAsset("shaders/object_pass2.vert").c_str(), file_manager->getAsset("shaders/object_pass2.frag").c_str());
		attrib_position = glGetAttribLocation(Program, "Position");
		attrib_texcoord = glGetAttribLocation(Program, "Texcoord");
		uniform_MVP = glGetUniformLocation(Program, "ModelViewProjectionMatrix");
		uniform_Albedo = glGetUniformLocation(Program, "Albedo");
		uniform_DiffuseMap = glGetUniformLocation(Program, "DiffuseMap");
		uniform_SpecularMap = glGetUniformLocation(Program, "SpecularMap");
		uniform_SSAO = glGetUniformLocation(Program, "SSAO");
		uniform_screen = glGetUniformLocation(Program, "screen");
		uniform_ambient = glGetUniformLocation(Program, "ambient");
	}

	void ObjectPass2Shader::setUniforms(const core::matrix4 &ModelViewProjectionMatrix, unsigned TU_Albedo, unsigned TU_DiffuseMap, unsigned TU_SpecularMap, unsigned TU_SSAO)
	{
		glUniformMatrix4fv(uniform_MVP, 1, GL_FALSE, ModelViewProjectionMatrix.pointer());
		glUniform1i(uniform_Albedo, TU_Albedo);
		glUniform1i(uniform_DiffuseMap, TU_DiffuseMap);
		glUniform1i(uniform_SpecularMap, TU_SpecularMap);
		glUniform1i(uniform_SSAO, TU_SSAO);
		glUniform2f(uniform_screen, UserConfigParams::m_width, UserConfigParams::m_height);
		const video::SColorf s = irr_driver->getSceneManager()->getAmbientLight();
		glUniform3f(uniform_ambient, s.r, s.g, s.b);
	}

	GLuint DetailledObjectPass2Shader::Program;
	GLuint DetailledObjectPass2Shader::attrib_position;
	GLuint DetailledObjectPass2Shader::attrib_texcoord;
	GLuint DetailledObjectPass2Shader::attrib_second_texcoord;
	GLuint DetailledObjectPass2Shader::uniform_MVP;
	GLuint DetailledObjectPass2Shader::uniform_Albedo;
	GLuint DetailledObjectPass2Shader::uniform_Detail;
	GLuint DetailledObjectPass2Shader::uniform_DiffuseMap;
	GLuint DetailledObjectPass2Shader::uniform_SpecularMap;
	GLuint DetailledObjectPass2Shader::uniform_SSAO;
	GLuint DetailledObjectPass2Shader::uniform_screen;
	GLuint DetailledObjectPass2Shader::uniform_ambient;

	void DetailledObjectPass2Shader::init()
	{
		Program = LoadProgram(file_manager->getAsset("shaders/object_pass2.vert").c_str(), file_manager->getAsset("shaders/detailledobject_pass2.frag").c_str());
		attrib_position = glGetAttribLocation(Program, "Position");
		attrib_texcoord = glGetAttribLocation(Program, "Texcoord");
		attrib_second_texcoord = glGetAttribLocation(Program, "SecondTexcoord");
		uniform_MVP = glGetUniformLocation(Program, "ModelViewProjectionMatrix");
		uniform_Albedo = glGetUniformLocation(Program, "Albedo");
		uniform_Detail = glGetUniformLocation(Program, "Detail");
		uniform_DiffuseMap = glGetUniformLocation(Program, "DiffuseMap");
		uniform_SpecularMap = glGetUniformLocation(Program, "SpecularMap");
		uniform_SSAO = glGetUniformLocation(Program, "SSAO");
		uniform_screen = glGetUniformLocation(Program, "screen");
		uniform_ambient = glGetUniformLocation(Program, "ambient");
	}

	void DetailledObjectPass2Shader::setUniforms(const core::matrix4 &ModelViewProjectionMatrix, unsigned TU_Albedo, unsigned TU_detail, unsigned TU_DiffuseMap, unsigned TU_SpecularMap, unsigned TU_SSAO)
	{
		glUniformMatrix4fv(uniform_MVP, 1, GL_FALSE, ModelViewProjectionMatrix.pointer());
		glUniform1i(uniform_Albedo, TU_Albedo);
		glUniform1i(uniform_Detail, TU_detail);
		glUniform1i(uniform_DiffuseMap, TU_DiffuseMap);
		glUniform1i(uniform_SpecularMap, TU_SpecularMap);
		glUniform1i(uniform_SSAO, TU_SSAO);
		glUniform2f(uniform_screen, UserConfigParams::m_width, UserConfigParams::m_height);
		const video::SColorf s = irr_driver->getSceneManager()->getAmbientLight();
		glUniform3f(uniform_ambient, s.r, s.g, s.b);
	}

	GLuint ObjectUnlitShader::Program;
	GLuint ObjectUnlitShader::attrib_position;
	GLuint ObjectUnlitShader::attrib_texcoord;
	GLuint ObjectUnlitShader::uniform_MVP;
	GLuint ObjectUnlitShader::uniform_tex;

	void ObjectUnlitShader::init()
	{
		Program = LoadProgram(file_manager->getAsset("shaders/object_pass2.vert").c_str(), file_manager->getAsset("shaders/object_unlit.frag").c_str());
		attrib_position = glGetAttribLocation(Program, "Position");
		attrib_texcoord = glGetAttribLocation(Program, "Texcoord");
		uniform_MVP = glGetUniformLocation(Program, "ModelViewProjectionMatrix");
		uniform_tex = glGetUniformLocation(Program, "tex");
	}

	void ObjectUnlitShader::setUniforms(const core::matrix4 &ModelViewProjectionMatrix, unsigned TU_tex)
	{
		glUniformMatrix4fv(uniform_MVP, 1, GL_FALSE, ModelViewProjectionMatrix.pointer());
		glUniform1i(uniform_tex, TU_tex);
	}

	GLuint ObjectRimLimitShader::Program;
	GLuint ObjectRimLimitShader::attrib_position;
	GLuint ObjectRimLimitShader::attrib_texcoord;
	GLuint ObjectRimLimitShader::attrib_normal;
	GLuint ObjectRimLimitShader::uniform_MVP;
	GLuint ObjectRimLimitShader::uniform_TIMV;
	GLuint ObjectRimLimitShader::uniform_Albedo;
	GLuint ObjectRimLimitShader::uniform_DiffuseMap;
	GLuint ObjectRimLimitShader::uniform_SpecularMap;
	GLuint ObjectRimLimitShader::uniform_SSAO;
	GLuint ObjectRimLimitShader::uniform_screen;
	GLuint ObjectRimLimitShader::uniform_ambient;

	void ObjectRimLimitShader::init()
	{
		Program = LoadProgram(file_manager->getAsset("shaders/objectpass_rimlit.vert").c_str(), file_manager->getAsset("shaders/objectpass_rimlit.frag").c_str());
		attrib_position = glGetAttribLocation(Program, "Position");
		attrib_texcoord = glGetAttribLocation(Program, "Texcoord");
		attrib_normal = glGetAttribLocation(Program, "Normal");
		uniform_MVP = glGetUniformLocation(Program, "ModelViewProjectionMatrix");
		uniform_TIMV = glGetUniformLocation(Program, "TransposeInverseModelView");
		uniform_Albedo = glGetUniformLocation(Program, "Albedo");
		uniform_DiffuseMap = glGetUniformLocation(Program, "DiffuseMap");
		uniform_SpecularMap = glGetUniformLocation(Program, "SpecularMap");
		uniform_SSAO = glGetUniformLocation(Program, "SSAO");
		uniform_screen = glGetUniformLocation(Program, "screen");
		uniform_ambient = glGetUniformLocation(Program, "ambient");
	}

	void ObjectRimLimitShader::setUniforms(const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView, unsigned TU_Albedo, unsigned TU_DiffuseMap, unsigned TU_SpecularMap, unsigned TU_SSAO)
	{
		glUniformMatrix4fv(uniform_MVP, 1, GL_FALSE, ModelViewProjectionMatrix.pointer());
		glUniformMatrix4fv(uniform_TIMV, 1, GL_FALSE, TransposeInverseModelView.pointer());
		glUniform1i(uniform_Albedo, TU_Albedo);
		glUniform1i(uniform_DiffuseMap, TU_DiffuseMap);
		glUniform1i(uniform_SpecularMap, TU_SpecularMap);
		glUniform1i(uniform_SSAO, TU_SSAO);
		glUniform2f(uniform_screen, UserConfigParams::m_width, UserConfigParams::m_height);
		const video::SColorf s = irr_driver->getSceneManager()->getAmbientLight();
		glUniform3f(uniform_ambient, s.r, s.g, s.b);
	}

	GLuint UntexturedObjectShader::Program;
	GLuint UntexturedObjectShader::attrib_position;
	GLuint UntexturedObjectShader::attrib_color;
	GLuint UntexturedObjectShader::uniform_MVP;
	GLuint UntexturedObjectShader::uniform_DiffuseMap;
	GLuint UntexturedObjectShader::uniform_SpecularMap;
	GLuint UntexturedObjectShader::uniform_SSAO;
	GLuint UntexturedObjectShader::uniform_screen;
	GLuint UntexturedObjectShader::uniform_ambient;

	void UntexturedObjectShader::init()
	{
	  Program = LoadProgram(file_manager->getAsset("shaders/untextured_object.vert").c_str(), file_manager->getAsset("shaders/untextured_object.frag").c_str());
	  attrib_position = glGetAttribLocation(Program, "Position");
	  attrib_color = glGetAttribLocation(Program, "Color");
	  uniform_MVP = glGetUniformLocation(Program, "ModelViewProjectionMatrix");
	  uniform_DiffuseMap = glGetUniformLocation(Program, "DiffuseMap");
	  uniform_SpecularMap = glGetUniformLocation(Program, "SpecularMap");
	  uniform_SSAO = glGetUniformLocation(Program, "SSAO");
	  uniform_screen = glGetUniformLocation(Program, "screen");
	  uniform_ambient = glGetUniformLocation(Program, "ambient");
	}

	void UntexturedObjectShader::setUniforms(const core::matrix4 &ModelViewProjectionMatrix, unsigned TU_DiffuseMap, unsigned TU_SpecularMap, unsigned TU_SSAO)
	{
	  glUniformMatrix4fv(uniform_MVP, 1, GL_FALSE, ModelViewProjectionMatrix.pointer());
	  glUniform1i(uniform_DiffuseMap, TU_DiffuseMap);
	  glUniform1i(uniform_SpecularMap, TU_SpecularMap);
	  glUniform1i(uniform_SSAO, TU_SSAO);
	  glUniform2f(uniform_screen, UserConfigParams::m_width, UserConfigParams::m_height);
	  const video::SColorf s = irr_driver->getSceneManager()->getAmbientLight();
	  glUniform3f(uniform_ambient, s.r, s.g, s.b);
	}


	GLuint ObjectRefPass2Shader::Program;
	GLuint ObjectRefPass2Shader::attrib_position;
	GLuint ObjectRefPass2Shader::attrib_texcoord;
	GLuint ObjectRefPass2Shader::uniform_MVP;
	GLuint ObjectRefPass2Shader::uniform_Albedo;
	GLuint ObjectRefPass2Shader::uniform_DiffuseMap;
	GLuint ObjectRefPass2Shader::uniform_SpecularMap;
	GLuint ObjectRefPass2Shader::uniform_SSAO;
	GLuint ObjectRefPass2Shader::uniform_screen;
	GLuint ObjectRefPass2Shader::uniform_ambient;

	void ObjectRefPass2Shader::init()
	{
		initGL();
		Program = LoadProgram(file_manager->getAsset("shaders/object_pass2.vert").c_str(), file_manager->getAsset("shaders/objectref_pass2.frag").c_str());
		attrib_position = glGetAttribLocation(Program, "Position");
		attrib_texcoord = glGetAttribLocation(Program, "Texcoord");
		uniform_MVP = glGetUniformLocation(Program, "ModelViewProjectionMatrix");
		uniform_Albedo = glGetUniformLocation(Program, "Albedo");
		uniform_DiffuseMap = glGetUniformLocation(Program, "DiffuseMap");
		uniform_SpecularMap = glGetUniformLocation(Program, "SpecularMap");
		uniform_SSAO = glGetUniformLocation(Program, "SSAO");
		uniform_screen = glGetUniformLocation(Program, "screen");
		uniform_ambient = glGetUniformLocation(Program, "ambient");
	}

	void ObjectRefPass2Shader::setUniforms(const core::matrix4 &ModelViewProjectionMatrix, unsigned TU_Albedo, unsigned TU_DiffuseMap, unsigned TU_SpecularMap, unsigned TU_SSAO)
	{
		glUniformMatrix4fv(uniform_MVP, 1, GL_FALSE, ModelViewProjectionMatrix.pointer());
		glUniform1i(uniform_Albedo, TU_Albedo);
		glUniform1i(uniform_DiffuseMap, TU_DiffuseMap);
		glUniform1i(uniform_SpecularMap, TU_SpecularMap);
		glUniform1i(uniform_SSAO, TU_SSAO);
		glUniform2f(uniform_screen, UserConfigParams::m_width, UserConfigParams::m_height);
		const video::SColorf s = irr_driver->getSceneManager()->getAmbientLight();
		glUniform3f(uniform_ambient, s.r, s.g, s.b);
	}

	GLuint GrassPass1Shader::Program;
	GLuint GrassPass1Shader::attrib_position;
	GLuint GrassPass1Shader::attrib_texcoord;
	GLuint GrassPass1Shader::attrib_normal;
	GLuint GrassPass1Shader::attrib_color;
	GLuint GrassPass1Shader::uniform_MVP;
	GLuint GrassPass1Shader::uniform_TIMV;
	GLuint GrassPass1Shader::uniform_tex;
	GLuint GrassPass1Shader::uniform_windDir;

	void GrassPass1Shader::init()
	{
		Program = LoadProgram(file_manager->getAsset("shaders/grass_pass1.vert").c_str(), file_manager->getAsset("shaders/objectref_pass1.frag").c_str());
		attrib_position = glGetAttribLocation(Program, "Position");
		attrib_texcoord = glGetAttribLocation(Program, "Texcoord");
		attrib_normal = glGetAttribLocation(Program, "Normal");
		attrib_color = glGetAttribLocation(Program, "Color");
		uniform_MVP = glGetUniformLocation(Program, "ModelViewProjectionMatrix");
		uniform_TIMV = glGetUniformLocation(Program, "TransposeInverseModelView");
		uniform_tex = glGetUniformLocation(Program, "tex");
		uniform_windDir = glGetUniformLocation(Program, "windDir");
	}

	void GrassPass1Shader::setUniforms(const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView, const core::vector3df &windDirection, unsigned TU_tex)
	{
		glUniformMatrix4fv(uniform_MVP, 1, GL_FALSE, ModelViewProjectionMatrix.pointer());
		glUniformMatrix4fv(uniform_TIMV, 1, GL_FALSE, TransposeInverseModelView.pointer());
		glUniform3f(uniform_windDir, windDirection.X, windDirection.Y, windDirection.Z);
		glUniform1i(uniform_tex, TU_tex);
	}

	GLuint GrassPass2Shader::Program;
	GLuint GrassPass2Shader::attrib_position;
	GLuint GrassPass2Shader::attrib_texcoord;
	GLuint GrassPass2Shader::attrib_color;
	GLuint GrassPass2Shader::uniform_MVP;
	GLuint GrassPass2Shader::uniform_Albedo;
	GLuint GrassPass2Shader::uniform_DiffuseMap;
	GLuint GrassPass2Shader::uniform_SpecularMap;
	GLuint GrassPass2Shader::uniform_SSAO;
	GLuint GrassPass2Shader::uniform_screen;
	GLuint GrassPass2Shader::uniform_ambient;
	GLuint GrassPass2Shader::uniform_windDir;

	void GrassPass2Shader::init()
	{
		Program = LoadProgram(file_manager->getAsset("shaders/grass_pass2.vert").c_str(), file_manager->getAsset("shaders/objectref_pass2.frag").c_str());
		attrib_position = glGetAttribLocation(Program, "Position");
		attrib_texcoord = glGetAttribLocation(Program, "Texcoord");
		attrib_color = glGetAttribLocation(Program, "Color");
		uniform_MVP = glGetUniformLocation(Program, "ModelViewProjectionMatrix");
		uniform_Albedo = glGetUniformLocation(Program, "Albedo");
		uniform_DiffuseMap = glGetUniformLocation(Program, "DiffuseMap");
		uniform_SpecularMap = glGetUniformLocation(Program, "SpecularMap");
		uniform_SSAO = glGetUniformLocation(Program, "SSAO");
		uniform_screen = glGetUniformLocation(Program, "screen");
		uniform_ambient = glGetUniformLocation(Program, "ambient");
		uniform_windDir = glGetUniformLocation(Program, "windDir");
	}

	void GrassPass2Shader::setUniforms(const core::matrix4 &ModelViewProjectionMatrix, const core::vector3df &windDirection, unsigned TU_Albedo, unsigned TU_DiffuseMap, unsigned TU_SpecularMap, unsigned TU_SSAO)
	{
		glUniformMatrix4fv(uniform_MVP, 1, GL_FALSE, ModelViewProjectionMatrix.pointer());
		glUniform1i(uniform_Albedo, TU_Albedo);
		glUniform1i(uniform_DiffuseMap, TU_DiffuseMap);
		glUniform1i(uniform_SpecularMap, TU_SpecularMap);
		glUniform1i(uniform_SSAO, TU_SSAO);
		glUniform2f(uniform_screen, UserConfigParams::m_width, UserConfigParams::m_height);
		const video::SColorf s = irr_driver->getSceneManager()->getAmbientLight();
		glUniform3f(uniform_ambient, s.r, s.g, s.b);
		glUniform3f(uniform_windDir, windDirection.X, windDirection.Y, windDirection.Z);
	}

	GLuint NormalMapShader::Program;
	GLuint NormalMapShader::attrib_position;
	GLuint NormalMapShader::attrib_texcoord;
	GLuint NormalMapShader::attrib_tangent;
	GLuint NormalMapShader::attrib_bitangent;
	GLuint NormalMapShader::uniform_MVP;
	GLuint NormalMapShader::uniform_TIMV;
	GLuint NormalMapShader::uniform_normalMap;

	void NormalMapShader::init()
	{
		Program = LoadProgram(file_manager->getAsset("shaders/normalmap.vert").c_str(), file_manager->getAsset("shaders/normalmap.frag").c_str());
		attrib_position = glGetAttribLocation(Program, "Position");
		attrib_texcoord = glGetAttribLocation(Program, "Texcoord");
		attrib_tangent = glGetAttribLocation(Program, "Tangent");
		attrib_bitangent = glGetAttribLocation(Program, "Bitangent");
		uniform_MVP = glGetUniformLocation(Program, "ModelViewProjectionMatrix");
		uniform_TIMV = glGetUniformLocation(Program, "TransposeInverseModelView");
		uniform_normalMap = glGetUniformLocation(Program, "normalMap");
	}

	void NormalMapShader::setUniforms(const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView, unsigned TU_normalMap)
	{
		glUniformMatrix4fv(uniform_MVP, 1, GL_FALSE, ModelViewProjectionMatrix.pointer());
		glUniformMatrix4fv(uniform_TIMV, 1, GL_FALSE, TransposeInverseModelView.pointer());
		glUniform1i(uniform_normalMap, TU_normalMap);
	}

	GLuint SphereMapShader::Program;
	GLuint SphereMapShader::attrib_position;
	GLuint SphereMapShader::attrib_normal;
	GLuint SphereMapShader::uniform_MVP;
	GLuint SphereMapShader::uniform_TIMV;
	GLuint SphereMapShader::uniform_tex;

	void SphereMapShader::init()
	{
		Program = LoadProgram(file_manager->getAsset("shaders/object_pass1.vert").c_str(), file_manager->getAsset("shaders/objectpass_spheremap.frag").c_str());
		attrib_position = glGetAttribLocation(Program, "Position");
		attrib_normal = glGetAttribLocation(Program, "Normal");
		uniform_MVP = glGetUniformLocation(Program, "ModelViewProjectionMatrix");
		uniform_TIMV = glGetUniformLocation(Program, "TransposeInverseModelView");
		uniform_tex = glGetUniformLocation(Program, "tex");
	}

	void SphereMapShader::setUniforms(const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView, unsigned TU_tex)
	{
		glUniformMatrix4fv(uniform_MVP, 1, GL_FALSE, ModelViewProjectionMatrix.pointer());
		glUniformMatrix4fv(uniform_TIMV, 1, GL_FALSE, TransposeInverseModelView.pointer());
		glUniform1i(uniform_tex, TU_tex);
	}

	GLuint SplattingShader::Program;
	GLuint SplattingShader::attrib_position;
	GLuint SplattingShader::attrib_texcoord;
	GLuint SplattingShader::attrib_second_texcoord;
	GLuint SplattingShader::uniform_MVP;
	GLuint SplattingShader::uniform_tex_layout;
	GLuint SplattingShader::uniform_tex_detail0;
	GLuint SplattingShader::uniform_tex_detail1;
	GLuint SplattingShader::uniform_tex_detail2;
	GLuint SplattingShader::uniform_tex_detail3;
	GLuint SplattingShader::uniform_DiffuseMap;
	GLuint SplattingShader::uniform_SpecularMap;
	GLuint SplattingShader::uniform_SSAO;
	GLuint SplattingShader::uniform_screen;
	GLuint SplattingShader::uniform_ambient;

	void SplattingShader::init()
	{
		Program = LoadProgram(file_manager->getAsset("shaders/splatting.vert").c_str(), file_manager->getAsset("shaders/splatting.frag").c_str());
		attrib_position = glGetAttribLocation(Program, "Position");
		attrib_texcoord = glGetAttribLocation(Program, "Texcoord");
		attrib_second_texcoord = glGetAttribLocation(Program, "SecondTexcoord");
		uniform_MVP = glGetUniformLocation(Program, "ModelViewProjectionMatrix");
		uniform_tex_layout = glGetUniformLocation(Program, "tex_layout");
		uniform_tex_detail0 = glGetUniformLocation(Program, "tex_detail0");
		uniform_tex_detail1 = glGetUniformLocation(Program, "tex_detail1");
		uniform_tex_detail2 = glGetUniformLocation(Program, "tex_detail2");
		uniform_tex_detail3 = glGetUniformLocation(Program, "tex_detail3");
		uniform_DiffuseMap = glGetUniformLocation(Program, "DiffuseMap");
		uniform_SpecularMap = glGetUniformLocation(Program, "SpecularMap");
		uniform_SSAO = glGetUniformLocation(Program, "SSAO");
		uniform_screen = glGetUniformLocation(Program, "screen");
		uniform_ambient = glGetUniformLocation(Program, "ambient");
	}

	void SplattingShader::setUniforms(const core::matrix4 &ModelViewProjectionMatrix, unsigned TU_tex_layout, unsigned TU_tex_detail0, unsigned TU_tex_detail1, unsigned TU_tex_detail2, unsigned TU_tex_detail3, unsigned TU_DiffuseMap, unsigned TU_SpecularMap, unsigned TU_SSAO)
	{
		glUniformMatrix4fv(uniform_MVP, 1, GL_FALSE, ModelViewProjectionMatrix.pointer());
		glUniform1i(uniform_tex_layout, TU_tex_layout);
		glUniform1i(uniform_tex_detail0, TU_tex_detail0);
		glUniform1i(uniform_tex_detail1, TU_tex_detail1);
		glUniform1i(uniform_tex_detail2, TU_tex_detail2);
		glUniform1i(uniform_tex_detail3, TU_tex_detail3);
		glUniform1i(uniform_DiffuseMap, TU_DiffuseMap);
		glUniform1i(uniform_SpecularMap, TU_SpecularMap);
		glUniform1i(uniform_SSAO, TU_SSAO);
		glUniform2f(uniform_screen, UserConfigParams::m_width, UserConfigParams::m_height);
		const video::SColorf s = irr_driver->getSceneManager()->getAmbientLight();
		glUniform3f(uniform_ambient, s.r, s.g, s.b);
	}

	GLuint BubbleShader::Program;
	GLuint BubbleShader::attrib_position;
	GLuint BubbleShader::attrib_texcoord;
	GLuint BubbleShader::uniform_MVP;
	GLuint BubbleShader::uniform_tex;
	GLuint BubbleShader::uniform_time;
	GLuint BubbleShader::uniform_transparency;

	void BubbleShader::init()
	{
		Program = LoadProgram(file_manager->getAsset("shaders/bubble.vert").c_str(), file_manager->getAsset("shaders/bubble.frag").c_str());
		attrib_position = glGetAttribLocation(Program, "Position");
		attrib_texcoord = glGetAttribLocation(Program, "Texcoord");
		uniform_MVP = glGetUniformLocation(Program, "ModelViewProjectionMatrix");
		uniform_tex = glGetUniformLocation(Program, "tex");
		uniform_time = glGetUniformLocation(Program, "time");
		uniform_transparency = glGetUniformLocation(Program, "transparency");
	}
	void BubbleShader::setUniforms(const core::matrix4 &ModelViewProjectionMatrix, unsigned TU_tex, float time, float transparency)
	{
		glUniformMatrix4fv(uniform_MVP, 1, GL_FALSE, ModelViewProjectionMatrix.pointer());
		glUniform1i(uniform_tex, TU_tex);
		glUniform1f(uniform_time, time);
		glUniform1f(uniform_transparency, transparency);
	}

	GLuint TransparentShader::Program;
	GLuint TransparentShader::attrib_position;
	GLuint TransparentShader::attrib_texcoord;
	GLuint TransparentShader::uniform_MVP;
	GLuint TransparentShader::uniform_tex;

	void TransparentShader::init()
	{
		Program = LoadProgram(file_manager->getAsset("shaders/transparent.vert").c_str(), file_manager->getAsset("shaders/transparent.frag").c_str());
		attrib_position = glGetAttribLocation(Program, "Position");
		attrib_texcoord = glGetAttribLocation(Program, "Texcoord");
		uniform_MVP = glGetUniformLocation(Program, "ModelViewProjectionMatrix");
		uniform_tex = glGetUniformLocation(Program, "tex");
	}

	void TransparentShader::setUniforms(const core::matrix4 &ModelViewProjectionMatrix, unsigned TU_tex)
	{
		glUniformMatrix4fv(uniform_MVP, 1, GL_FALSE, ModelViewProjectionMatrix.pointer());
		glUniform1i(uniform_tex, TU_tex);
	}
	
	GLuint BillboardShader::Program;
	GLuint BillboardShader::attrib_corner;
	GLuint BillboardShader::attrib_texcoord;
	GLuint BillboardShader::uniform_MV;
	GLuint BillboardShader::uniform_P;
	GLuint BillboardShader::uniform_tex;
	GLuint BillboardShader::uniform_Position;
	GLuint BillboardShader::uniform_Size;

	void BillboardShader::init()
	{
		Program = LoadProgram(file_manager->getAsset("shaders/billboard.vert").c_str(), file_manager->getAsset("shaders/billboard.frag").c_str());
		attrib_corner = glGetAttribLocation(Program, "Corner");
		attrib_texcoord = glGetAttribLocation(Program, "Texcoord");
		uniform_MV = glGetUniformLocation(Program, "ModelViewMatrix");
		uniform_P = glGetUniformLocation(Program, "ProjectionMatrix");
		uniform_Position = glGetUniformLocation(Program, "Position");
		uniform_Size = glGetUniformLocation(Program, "Size");
		uniform_tex = glGetUniformLocation(Program, "tex");
		printf("TUTex is %d, Texcoord is %d\n", uniform_tex, attrib_texcoord);
	}

	void BillboardShader::setUniforms(const core::matrix4 &ModelViewMatrix, const core::matrix4 &ProjectionMatrix, const core::vector3df &Position, const core::dimension2d<float> &size, unsigned TU_tex)
	{
		glUniformMatrix4fv(uniform_MV, 1, GL_FALSE, ModelViewMatrix.pointer());
		glUniformMatrix4fv(uniform_P, 1, GL_FALSE, ProjectionMatrix.pointer());
		glUniform3f(uniform_Position, Position.X, Position.Y, Position.Z);
		glUniform2f(uniform_Size, size.Width, size.Height);
		glUniform1i(uniform_tex, TU_tex);
	}

	GLuint ColorizeShader::Program;
	GLuint ColorizeShader::attrib_position;
	GLuint ColorizeShader::uniform_MVP;
	GLuint ColorizeShader::uniform_col;

	void ColorizeShader::init()
	{
		Program = LoadProgram(file_manager->getAsset("shaders/object_pass2.vert").c_str(), file_manager->getAsset("shaders/colorize.frag").c_str());
		attrib_position = glGetAttribLocation(Program, "Position");
		uniform_MVP = glGetUniformLocation(Program, "ModelViewProjectionMatrix");
		uniform_col = glGetUniformLocation(Program, "col");
	}

	void ColorizeShader::setUniforms(const core::matrix4 &ModelViewProjectionMatrix, float r, float g, float b)
	{
		glUniformMatrix4fv(uniform_MVP, 1, GL_FALSE, ModelViewProjectionMatrix.pointer());
		glUniform3f(uniform_col, r, g, b);
	}

	GLuint DisplaceShader::Program;
	GLuint DisplaceShader::attrib_position;
	GLuint DisplaceShader::attrib_texcoord;
	GLuint DisplaceShader::attrib_second_texcoord;
	GLuint DisplaceShader::uniform_MVP;
	GLuint DisplaceShader::uniform_MV;
	GLuint DisplaceShader::uniform_tex;
	GLuint DisplaceShader::uniform_dir;
	GLuint DisplaceShader::uniform_dir2;

	void DisplaceShader::init()
	{
		Program = LoadProgram(file_manager->getAsset("shaders/displace.vert").c_str(), file_manager->getAsset("shaders/displace.frag").c_str());
		attrib_position = glGetAttribLocation(Program, "Position");
		attrib_texcoord = glGetAttribLocation(Program, "Texcoord");
		attrib_second_texcoord = glGetAttribLocation(Program, "SecondTexcoord");
		uniform_MVP = glGetUniformLocation(Program, "ModelViewProjectionMatrix");
		uniform_MV = glGetUniformLocation(Program, "ModelViewMatrix");
		uniform_tex = glGetUniformLocation(Program, "tex");
		uniform_dir = glGetUniformLocation(Program, "dir");
		uniform_dir2 = glGetUniformLocation(Program, "dir2");
	}

	void DisplaceShader::setUniforms(const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &ModelViewMatrix, float dirX, float dirY, float dir2X, float dir2Y, unsigned TU_tex)
	{
		glUniformMatrix4fv(uniform_MVP, 1, GL_FALSE, ModelViewProjectionMatrix.pointer());
		glUniformMatrix4fv(uniform_MV, 1, GL_FALSE, ModelViewMatrix.pointer());
		glUniform2f(uniform_dir, dirX, dirY);
		glUniform2f(uniform_dir2, dir2X, dir2Y);
		glUniform1i(uniform_tex, TU_tex);
	}
}


namespace ParticleShader
{
	GLuint SimpleSimulationShader::Program;
	GLuint SimpleSimulationShader::attrib_position;
	GLuint SimpleSimulationShader::attrib_velocity;
	GLuint SimpleSimulationShader::attrib_lifetime;
	GLuint SimpleSimulationShader::attrib_initial_position;
	GLuint SimpleSimulationShader::attrib_initial_velocity;
	GLuint SimpleSimulationShader::attrib_initial_lifetime;
	GLuint SimpleSimulationShader::attrib_size;
	GLuint SimpleSimulationShader::attrib_initial_size;
	GLuint SimpleSimulationShader::uniform_sourcematrix;
	GLuint SimpleSimulationShader::uniform_dt;
	GLuint SimpleSimulationShader::uniform_level;
	GLuint SimpleSimulationShader::uniform_size_increase_factor;

	void SimpleSimulationShader::init()
	{
		const char *varyings[] = {
			"new_particle_position",
			"new_lifetime",
			"new_particle_velocity",
			"new_size",
		};
		Program = LoadTFBProgram(file_manager->getAsset("shaders/pointemitter.vert").c_str(), varyings, 4);

		uniform_dt = glGetUniformLocation(Program, "dt");
		uniform_sourcematrix = glGetUniformLocation(Program, "sourcematrix");
		uniform_level = glGetUniformLocation(Program, "level");
		uniform_size_increase_factor = glGetUniformLocation(Program, "size_increase_factor");

		attrib_position = glGetAttribLocation(Program, "particle_position");
		attrib_lifetime = glGetAttribLocation(Program, "lifetime");
		attrib_velocity = glGetAttribLocation(Program, "particle_velocity");
		attrib_size = glGetAttribLocation(Program, "size");
		attrib_initial_position = glGetAttribLocation(Program, "particle_position_initial");
		attrib_initial_lifetime = glGetAttribLocation(Program, "lifetime_initial");
		attrib_initial_velocity = glGetAttribLocation(Program, "particle_velocity_initial");
		attrib_initial_size = glGetAttribLocation(Program, "size_initial");
	}

	GLuint HeightmapSimulationShader::Program;
	GLuint HeightmapSimulationShader::attrib_position;
	GLuint HeightmapSimulationShader::attrib_velocity;
	GLuint HeightmapSimulationShader::attrib_lifetime;
	GLuint HeightmapSimulationShader::attrib_initial_position;
	GLuint HeightmapSimulationShader::attrib_initial_velocity;
	GLuint HeightmapSimulationShader::attrib_initial_lifetime;
	GLuint HeightmapSimulationShader::attrib_size;
	GLuint HeightmapSimulationShader::attrib_initial_size;
	GLuint HeightmapSimulationShader::uniform_sourcematrix;
	GLuint HeightmapSimulationShader::uniform_dt;
	GLuint HeightmapSimulationShader::uniform_level;
	GLuint HeightmapSimulationShader::uniform_size_increase_factor;
	GLuint HeightmapSimulationShader::uniform_track_x;
	GLuint HeightmapSimulationShader::uniform_track_z;
	GLuint HeightmapSimulationShader::uniform_track_x_len;
	GLuint HeightmapSimulationShader::uniform_track_z_len;
	GLuint HeightmapSimulationShader::uniform_heightmap;

	void HeightmapSimulationShader::init()
	{
		const char *varyings[] = {
			"new_particle_position",
			"new_lifetime",
			"new_particle_velocity",
			"new_size",
		};
		Program = LoadTFBProgram(file_manager->getAsset("shaders/particlesimheightmap.vert").c_str(), varyings, 4);

		uniform_dt = glGetUniformLocation(Program, "dt");
		uniform_sourcematrix = glGetUniformLocation(Program, "sourcematrix");
		uniform_level = glGetUniformLocation(Program, "level");
		uniform_size_increase_factor = glGetUniformLocation(Program, "size_increase_factor");

		attrib_position = glGetAttribLocation(Program, "particle_position");
		attrib_lifetime = glGetAttribLocation(Program, "lifetime");
		attrib_velocity = glGetAttribLocation(Program, "particle_velocity");
		attrib_size = glGetAttribLocation(Program, "size");
		attrib_initial_position = glGetAttribLocation(Program, "particle_position_initial");
		attrib_initial_lifetime = glGetAttribLocation(Program, "lifetime_initial");
		attrib_initial_velocity = glGetAttribLocation(Program, "particle_velocity_initial");
		attrib_initial_size = glGetAttribLocation(Program, "size_initial");

		uniform_heightmap = glGetUniformLocation(Program, "heightmap");
		uniform_track_x = glGetUniformLocation(Program, "track_x");
		uniform_track_x_len = glGetUniformLocation(Program, "track_x_len");
		uniform_track_z = glGetUniformLocation(Program, "track_z");
		uniform_track_z_len = glGetUniformLocation(Program, "track_z_len");
	}

	GLuint SimpleParticleRender::Program;
	GLuint SimpleParticleRender::attrib_pos;
	GLuint SimpleParticleRender::attrib_lf;
	GLuint SimpleParticleRender::attrib_quadcorner;
	GLuint SimpleParticleRender::attrib_texcoord;
	GLuint SimpleParticleRender::attrib_sz;
	GLuint SimpleParticleRender::uniform_matrix;
	GLuint SimpleParticleRender::uniform_viewmatrix;
	GLuint SimpleParticleRender::uniform_tex;
	GLuint SimpleParticleRender::uniform_normal_and_depths;
	GLuint SimpleParticleRender::uniform_screen;
	GLuint SimpleParticleRender::uniform_invproj;
	
	void SimpleParticleRender::init()
	{
		Program = LoadProgram(file_manager->getAsset("shaders/particle.vert").c_str(), file_manager->getAsset("shaders/particle.frag").c_str());
		attrib_pos = glGetAttribLocation(Program, "position");
		attrib_sz = glGetAttribLocation(Program, "size");
		attrib_lf = glGetAttribLocation(Program, "lifetime");
		attrib_quadcorner = glGetAttribLocation(Program, "quadcorner");
		attrib_texcoord = glGetAttribLocation(Program, "texcoord");


		uniform_matrix = glGetUniformLocation(Program, "ProjectionMatrix");
		uniform_viewmatrix = glGetUniformLocation(Program, "ViewMatrix");
		uniform_tex = glGetUniformLocation(Program, "tex");
		uniform_invproj = glGetUniformLocation(Program, "invproj");
		uniform_screen = glGetUniformLocation(Program, "screen");
		uniform_normal_and_depths = glGetUniformLocation(Program, "normals_and_depth");
	}

	void SimpleParticleRender::setUniforms(const core::matrix4 &ViewMatrix, const core::matrix4 &ProjMatrix, const core::matrix4 InvProjMatrix, float width, float height, unsigned TU_tex, unsigned TU_normal_and_depth)
	{
		glUniformMatrix4fv(uniform_invproj, 1, GL_FALSE, InvProjMatrix.pointer());
		glUniform2f(uniform_screen, width, height);
		glUniformMatrix4fv(uniform_matrix, 1, GL_FALSE, irr_driver->getProjMatrix().pointer());
		glUniformMatrix4fv(uniform_viewmatrix, 1, GL_FALSE, irr_driver->getViewMatrix().pointer());
		glUniform1i(uniform_tex, TU_tex);
		glUniform1i(uniform_normal_and_depths, TU_normal_and_depth);
	}

	GLuint FlipParticleRender::Program;
	GLuint FlipParticleRender::attrib_pos;
	GLuint FlipParticleRender::attrib_lf;
	GLuint FlipParticleRender::attrib_quadcorner;
	GLuint FlipParticleRender::attrib_texcoord;
	GLuint FlipParticleRender::attrib_sz;
	GLuint FlipParticleRender::attrib_rotationvec;
	GLuint FlipParticleRender::attrib_anglespeed;
	GLuint FlipParticleRender::uniform_matrix;
	GLuint FlipParticleRender::uniform_viewmatrix;
	GLuint FlipParticleRender::uniform_tex;
	GLuint FlipParticleRender::uniform_normal_and_depths;
	GLuint FlipParticleRender::uniform_screen;
	GLuint FlipParticleRender::uniform_invproj;

	void FlipParticleRender::init()
	{
		Program = LoadProgram(file_manager->getAsset("shaders/flipparticle.vert").c_str(), file_manager->getAsset("shaders/particle.frag").c_str());
		attrib_pos = glGetAttribLocation(Program, "position");
		attrib_sz = glGetAttribLocation(Program, "size");
		attrib_lf = glGetAttribLocation(Program, "lifetime");
		attrib_quadcorner = glGetAttribLocation(Program, "quadcorner");
		attrib_texcoord = glGetAttribLocation(Program, "texcoord");
		attrib_anglespeed = glGetAttribLocation(Program, "anglespeed");
		attrib_rotationvec = glGetAttribLocation(Program, "rotationvec");

		uniform_matrix = glGetUniformLocation(Program, "ProjectionMatrix");
		uniform_viewmatrix = glGetUniformLocation(Program, "ViewMatrix");
		uniform_tex = glGetUniformLocation(Program, "tex");
		uniform_invproj = glGetUniformLocation(Program, "invproj");
		uniform_screen = glGetUniformLocation(Program, "screen");
		uniform_normal_and_depths = glGetUniformLocation(Program, "normals_and_depth");
	}

	void FlipParticleRender::setUniforms(const core::matrix4 &ViewMatrix, const core::matrix4 &ProjMatrix, const core::matrix4 InvProjMatrix, float width, float height, unsigned TU_tex, unsigned TU_normal_and_depth)
	{
		glUniformMatrix4fv(uniform_invproj, 1, GL_FALSE, InvProjMatrix.pointer());
		glUniform2f(uniform_screen, width, height);
		glUniformMatrix4fv(uniform_matrix, 1, GL_FALSE, irr_driver->getProjMatrix().pointer());
		glUniformMatrix4fv(uniform_viewmatrix, 1, GL_FALSE, irr_driver->getViewMatrix().pointer());
		glUniform1i(uniform_tex, TU_tex);
		glUniform1i(uniform_normal_and_depths, TU_normal_and_depth);
	}
}

static GLuint createVAO(GLuint Program)
{
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	GLuint attrib_position = glGetAttribLocation(Program, "Position");
	GLuint attrib_texcoord = glGetAttribLocation(Program, "Texcoord");
	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
	glEnableVertexAttribArray(attrib_position);
	glEnableVertexAttribArray(attrib_texcoord);
	glVertexAttribPointer(attrib_position, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glVertexAttribPointer(attrib_texcoord, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid*)(2 * sizeof(float)));
	glBindVertexArray(0);
	return vao;
}

namespace FullScreenShader
{
	GLuint BloomShader::Program;
	GLuint BloomShader::uniform_texture;
	GLuint BloomShader::uniform_low;
	GLuint BloomShader::vao;
	void BloomShader::init()
	{
		Program = LoadProgram(file_manager->getAsset("shaders/screenquad.vert").c_str(), file_manager->getAsset("shaders/bloom.frag").c_str());
		uniform_texture = glGetUniformLocation(Program, "tex");
		uniform_low = glGetUniformLocation(Program, "low");
		vao = createVAO(Program);
	}

	GLuint BloomBlendShader::Program;
	GLuint BloomBlendShader::uniform_texture;
	GLuint BloomBlendShader::uniform_low;
	GLuint BloomBlendShader::vao;
	void BloomBlendShader::init()
	{
		Program = LoadProgram(file_manager->getAsset("shaders/screenquad.vert").c_str(), file_manager->getAsset("shaders/bloomblend.frag").c_str());
		uniform_texture = glGetUniformLocation(Program, "tex");
		vao = createVAO(Program);
	}

	GLuint PPDisplaceShader::Program;
	GLuint PPDisplaceShader::uniform_tex;
	GLuint PPDisplaceShader::uniform_dtex;
	GLuint PPDisplaceShader::uniform_viz;
	GLuint PPDisplaceShader::vao;
	void PPDisplaceShader::init()
	{
		Program = LoadProgram(file_manager->getAsset("shaders/screenquad.vert").c_str(), file_manager->getAsset("shaders/ppdisplace.frag").c_str());
		uniform_tex = glGetUniformLocation(Program, "tex");
		uniform_dtex = glGetUniformLocation(Program, "dtex");
		uniform_viz = glGetUniformLocation(Program, "viz");
		vao = createVAO(Program);
	}

	GLuint ColorLevelShader::Program;
	GLuint ColorLevelShader::uniform_tex;
	GLuint ColorLevelShader::uniform_inlevel;
	GLuint ColorLevelShader::uniform_outlevel;
	GLuint ColorLevelShader::vao;
	void ColorLevelShader::init()
	{
		Program = LoadProgram(file_manager->getAsset("shaders/screenquad.vert").c_str(), file_manager->getAsset("shaders/color_levels.frag").c_str());
		uniform_tex = glGetUniformLocation(Program, "tex");
		uniform_inlevel = glGetUniformLocation(Program, "inlevel");
		uniform_outlevel = glGetUniformLocation(Program, "outlevel");
		vao = createVAO(Program);
	}

	GLuint PointLightShader::Program;
	GLuint PointLightShader::uniform_ntex;
	GLuint PointLightShader::uniform_center;
	GLuint PointLightShader::uniform_col;
	GLuint PointLightShader::uniform_energy;
	GLuint PointLightShader::uniform_spec;
	GLuint PointLightShader::uniform_invproj;
	GLuint PointLightShader::uniform_viewm;
	GLuint PointLightShader::vao;

	void PointLightShader::init()
	{
		Program = LoadProgram(file_manager->getAsset("shaders/screenquad.vert").c_str(), file_manager->getAsset("shaders/pointlight.frag").c_str());
		uniform_ntex = glGetUniformLocation(Program, "ntex");
		uniform_center = glGetUniformLocation(Program, "center[0]");
		uniform_col = glGetUniformLocation(Program, "col[0]");
		uniform_energy = glGetUniformLocation(Program, "energy[0]");
		uniform_spec = glGetUniformLocation(Program, "spec");
		uniform_invproj = glGetUniformLocation(Program, "invproj");
		uniform_viewm = glGetUniformLocation(Program, "viewm");
		vao = createVAO(Program);
	}

	void PointLightShader::setUniforms(const core::matrix4 &InvProjMatrix, const core::matrix4 &ViewMatrix, const std::vector<float> &positions, const std::vector<float> &colors, const std::vector<float> &energy, unsigned spec, unsigned TU_ntex)
	{
		glUniform4fv(FullScreenShader::PointLightShader::uniform_center, 16, positions.data());
		glUniform4fv(FullScreenShader::PointLightShader::uniform_col, 16, colors.data());
		glUniform1fv(FullScreenShader::PointLightShader::uniform_energy, 16, energy.data());
		glUniform1f(FullScreenShader::PointLightShader::uniform_spec, 200);
		glUniformMatrix4fv(FullScreenShader::PointLightShader::uniform_invproj, 1, GL_FALSE, InvProjMatrix.pointer());
		glUniformMatrix4fv(FullScreenShader::PointLightShader::uniform_viewm, 1, GL_FALSE, ViewMatrix.pointer());

		glUniform1i(FullScreenShader::PointLightShader::uniform_ntex, TU_ntex);
	}

	GLuint SunLightShader::Program;
	GLuint SunLightShader::uniform_ntex;
	GLuint SunLightShader::uniform_direction;
	GLuint SunLightShader::uniform_col;
	GLuint SunLightShader::uniform_invproj;
	GLuint SunLightShader::vao;

	void SunLightShader::init()
	{
		Program = LoadProgram(file_manager->getAsset("shaders/screenquad.vert").c_str(), file_manager->getAsset("shaders/sunlight.frag").c_str());
		uniform_ntex = glGetUniformLocation(Program, "ntex");
		uniform_direction = glGetUniformLocation(Program, "direction");
		uniform_col = glGetUniformLocation(Program, "col");
		uniform_invproj = glGetUniformLocation(Program, "invproj");
		vao = createVAO(Program);
	}

	void SunLightShader::setUniforms(const core::vector3df &direction, const core::matrix4 &InvProjMatrix, float r, float g, float b, unsigned TU_ntex)
	{
		glUniformMatrix4fv(uniform_invproj, 1, GL_FALSE, InvProjMatrix.pointer());
		glUniform3f(uniform_direction, direction.X, direction.Y, direction.Z);
		glUniform3f(uniform_col, r, g, b);
		glUniform1i(uniform_ntex, TU_ntex);
	}

	GLuint LightBlendShader::Program;
	GLuint LightBlendShader::uniform_diffuse;
	GLuint LightBlendShader::uniform_specular;
	GLuint LightBlendShader::uniform_ambient_occlusion;
	GLuint LightBlendShader::uniform_specular_map;
	GLuint LightBlendShader::uniform_ambient;
	GLuint LightBlendShader::vao;
	void LightBlendShader::init()
	{
		Program = LoadProgram(file_manager->getAsset("shaders/screenquad.vert").c_str(), file_manager->getAsset("shaders/lightblend.frag").c_str());
		uniform_diffuse = glGetUniformLocation(Program, "diffuse");
		uniform_specular = glGetUniformLocation(Program, "specular");
		uniform_ambient_occlusion = glGetUniformLocation(Program, "ambient_occlusion");
		uniform_specular_map = glGetUniformLocation(Program, "specular_map");
		uniform_ambient = glGetUniformLocation(Program, "ambient");
		vao = createVAO(Program);
	}

	GLuint Gaussian6HBlurShader::Program;
	GLuint Gaussian6HBlurShader::uniform_tex;
	GLuint Gaussian6HBlurShader::uniform_pixel;
	GLuint Gaussian6HBlurShader::vao;
	void Gaussian6HBlurShader::init()
	{
		Program = LoadProgram(file_manager->getAsset("shaders/screenquad.vert").c_str(), file_manager->getAsset("shaders/gaussian6h.frag").c_str());
		uniform_tex = glGetUniformLocation(Program, "tex");
		uniform_pixel = glGetUniformLocation(Program, "pixel");
		vao = createVAO(Program);
	}

	GLuint Gaussian3HBlurShader::Program;
	GLuint Gaussian3HBlurShader::uniform_tex;
	GLuint Gaussian3HBlurShader::uniform_pixel;
	GLuint Gaussian3HBlurShader::vao;
	void Gaussian3HBlurShader::init()
	{
		Program = LoadProgram(file_manager->getAsset("shaders/screenquad.vert").c_str(), file_manager->getAsset("shaders/gaussian3h.frag").c_str());
		uniform_tex = glGetUniformLocation(Program, "tex");
		uniform_pixel = glGetUniformLocation(Program, "pixel");
		vao = createVAO(Program);
	}

	GLuint Gaussian6VBlurShader::Program;
	GLuint Gaussian6VBlurShader::uniform_tex;
	GLuint Gaussian6VBlurShader::uniform_pixel;
	GLuint Gaussian6VBlurShader::vao;
	void Gaussian6VBlurShader::init()
	{
		Program = LoadProgram(file_manager->getAsset("shaders/screenquad.vert").c_str(), file_manager->getAsset("shaders/gaussian6v.frag").c_str());
		uniform_tex = glGetUniformLocation(Program, "tex");
		uniform_pixel = glGetUniformLocation(Program, "pixel");
		vao = createVAO(Program);
	}

	GLuint Gaussian3VBlurShader::Program;
	GLuint Gaussian3VBlurShader::uniform_tex;
	GLuint Gaussian3VBlurShader::uniform_pixel;
	GLuint Gaussian3VBlurShader::vao;
	void Gaussian3VBlurShader::init()
	{
		Program = LoadProgram(file_manager->getAsset("shaders/screenquad.vert").c_str(), file_manager->getAsset("shaders/gaussian3v.frag").c_str());
		uniform_tex = glGetUniformLocation(Program, "tex");
		uniform_pixel = glGetUniformLocation(Program, "pixel");
		vao = createVAO(Program);
	}

	GLuint PassThroughShader::Program;
	GLuint PassThroughShader::uniform_texture;
	GLuint PassThroughShader::vao;
	void PassThroughShader::init()
	{
		Program = LoadProgram(file_manager->getAsset("shaders/screenquad.vert").c_str(), file_manager->getAsset("shaders/texturedquad.frag").c_str());
		uniform_texture = glGetUniformLocation(Program, "texture");
		vao = createVAO(Program);
	}

	GLuint GlowShader::Program;
	GLuint GlowShader::uniform_tex;
	GLuint GlowShader::vao;
	void GlowShader::init()
	{
		Program = LoadProgram(file_manager->getAsset("shaders/screenquad.vert").c_str(), file_manager->getAsset("shaders/glow.frag").c_str());
		uniform_tex = glGetUniformLocation(Program, "tex");
		vao = createVAO(Program);
	}

	GLuint SSAOShader::Program;
	GLuint SSAOShader::uniform_normals_and_depth;
	GLuint SSAOShader::uniform_noise_texture;
	GLuint SSAOShader::uniform_invprojm;
	GLuint SSAOShader::uniform_projm;
	GLuint SSAOShader::uniform_samplePoints;
	GLuint SSAOShader::vao;
	float SSAOShader::SSAOSamples[64];
	void SSAOShader::init()
	{
		Program = LoadProgram(file_manager->getAsset("shaders/screenquad.vert").c_str(), file_manager->getAsset("shaders/ssao.frag").c_str());
		uniform_normals_and_depth = glGetUniformLocation(Program, "normals_and_depth");
		uniform_noise_texture = glGetUniformLocation(Program, "noise_texture");
		uniform_invprojm = glGetUniformLocation(Program, "invprojm");
		uniform_projm = glGetUniformLocation(Program, "projm");
		uniform_samplePoints = glGetUniformLocation(Program, "samplePoints[0]");
		vao = createVAO(Program);

		// SSAOSamples[4 * i] and SSAOSamples[4 * i + 1] can be negative

		SSAOSamples[0] = 0.135061;
		SSAOSamples[1] = 0.207948;
		SSAOSamples[2] = 0.968770;
		SSAOSamples[3] = 0.983032;

		SSAOSamples[4] = 0.273456;
		SSAOSamples[5] = -0.805390;
		SSAOSamples[6] = 0.525898;
		SSAOSamples[7] = 0.942808;

		SSAOSamples[8] = 0.443450;
		SSAOSamples[9] = -0.803786;
		SSAOSamples[10] = 0.396585;
		SSAOSamples[11] = 0.007996;

		SSAOSamples[12] = 0.742420;
		SSAOSamples[13] = -0.620072;
		SSAOSamples[14] = 0.253621;
		SSAOSamples[15] = 0.284829;

		SSAOSamples[16] = 0.892464;
		SSAOSamples[17] = 0.046221;
		SSAOSamples[18] = 0.448744;
		SSAOSamples[19] = 0.753655;

		SSAOSamples[20] = 0.830350;
		SSAOSamples[21] = -0.043593;
		SSAOSamples[22] = 0.555535;
		SSAOSamples[23] = 0.357463;

		SSAOSamples[24] = -0.600612;
		SSAOSamples[25] = -0.536421;
		SSAOSamples[26] = 0.592889;
		SSAOSamples[27] = 0.670583;

		SSAOSamples[28] = -0.280658;
		SSAOSamples[29] = 0.674894;
		SSAOSamples[30] = 0.682458;
		SSAOSamples[31] = 0.553362;

		SSAOSamples[32] = -0.654493;
		SSAOSamples[33] = -0.140866;
		SSAOSamples[34] = 0.742830;
		SSAOSamples[35] = 0.699820;

		SSAOSamples[36] = 0.114730;
		SSAOSamples[37] = 0.873130;
		SSAOSamples[38] = 0.473794;
		SSAOSamples[39] = 0.483901;

		SSAOSamples[40] = 0.699167;
		SSAOSamples[41] = 0.632210;
		SSAOSamples[42] = 0.333879;
		SSAOSamples[43] = 0.010956;

		SSAOSamples[44] = 0.904603;
		SSAOSamples[45] = 0.393410;
		SSAOSamples[46] = 0.164080;
		SSAOSamples[47] = 0.780297;

		SSAOSamples[48] = 0.631662;
		SSAOSamples[49] = -0.405195;
		SSAOSamples[50] = 0.660924;
		SSAOSamples[51] = 0.865596;

		SSAOSamples[52] = -0.195668;
		SSAOSamples[53] = 0.629185;
		SSAOSamples[54] = 0.752223;
		SSAOSamples[55] = 0.019013;

		SSAOSamples[56] = -0.511316;
		SSAOSamples[57] = 0.635504;
		SSAOSamples[58] = 0.578524;
		SSAOSamples[59] = 0.605457;

		SSAOSamples[60] = -0.898843;
		SSAOSamples[61] = 0.067382;
		SSAOSamples[62] = 0.433061;
		SSAOSamples[63] = 0.772942;

		// Generate another random distribution, if needed
/*		for (unsigned i = 0; i < 16; i++) {
			// Use double to avoid denorm and get a true uniform distribution
			// Generate z component between [0.1; 1] to avoid being too close from surface
			double z = rand();
			z /= RAND_MAX;
			z = 0.1 + 0.9 * z;

			// Now generate x,y on the unit circle
			double x = rand();
			x /= RAND_MAX;
			x = 2 * x - 1;
			double y = rand();
			y /= RAND_MAX;
			y = 2 * y - 1;
			double xynorm = sqrt(x * x + y * y);
			x /= xynorm;
			y /= xynorm;
			// Now resize x,y so that norm(x,y,z) is one
			x *= sqrt(1. - z * z);
			y *= sqrt(1. - z * z);

			// Norm factor
			double w = rand();
			w /= RAND_MAX;
			SSAOSamples[4 * i] = (float)x;
			SSAOSamples[4 * i + 1] = (float)y;
			SSAOSamples[4 * i + 2] = (float)z;
			SSAOSamples[4 * i + 3] = (float)w;
		}*/
	}

	GLuint FogShader::Program;
	GLuint FogShader::uniform_tex;
	GLuint FogShader::uniform_fogmax;
	GLuint FogShader::uniform_startH;
	GLuint FogShader::uniform_endH;
	GLuint FogShader::uniform_start;
	GLuint FogShader::uniform_end;
	GLuint FogShader::uniform_col;
	GLuint FogShader::uniform_campos;
	GLuint FogShader::uniform_ipvmat;
	GLuint FogShader::vao;
	void FogShader::init()
	{
		Program = LoadProgram(file_manager->getAsset("shaders/screenquad.vert").c_str(), file_manager->getAsset("shaders/fog.frag").c_str());
		uniform_tex = glGetUniformLocation(Program, "tex");
		uniform_fogmax = glGetUniformLocation(Program, "fogmax");
		uniform_startH = glGetUniformLocation(Program, "startH");
		uniform_endH = glGetUniformLocation(Program, "endH");
		uniform_start = glGetUniformLocation(Program, "start");
		uniform_end = glGetUniformLocation(Program, "end");
		uniform_col = glGetUniformLocation(Program, "col");
		uniform_campos = glGetUniformLocation(Program, "campos");
		uniform_ipvmat = glGetUniformLocation(Program, "ipvmat");
		vao = createVAO(Program);
	}
}
