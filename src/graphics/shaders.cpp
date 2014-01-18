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

#include <assert.h>
#include <IGPUProgrammingServices.h>

using namespace video;

Shaders::Shaders()
{
    // Callbacks
    memset(m_callbacks, 0, sizeof(m_callbacks));

    m_callbacks[ES_NORMAL_MAP_LIGHTMAP] = new NormalMapProvider(true);
    m_callbacks[ES_NORMAL_MAP] = new NormalMapProvider(false);
    m_callbacks[ES_SKYBOX] = new SkyboxProvider();
    m_callbacks[ES_SPLATTING] = new SplattingProvider();
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
    m_callbacks[ES_BLOOM_POWER] = new BloomPowerProvider();
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

    m_shaders[ES_PASS] = glslmat(std::string(""), dir + "pass.frag",
                                    0, EMT_SOLID);
    m_shaders[ES_PASS_ADDITIVE] = glslmat(std::string(""), dir + "pass.frag",
                                    0, EMT_TRANSPARENT_ADD_COLOR);

    m_shaders[ES_GLOW] = glslmat(std::string(""), dir + "glow.frag",
                                    m_callbacks[ES_GLOW], EMT_TRANSPARENT_ALPHA_CHANNEL);

    m_shaders[ES_OBJECTPASS] = glslmat(dir + "objectpass.vert", dir + "objectpass.frag",
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

    m_shaders[ES_BLOOM_POWER] = glsl(std::string(""), dir + "bloompower.frag",
                                    m_callbacks[ES_BLOOM_POWER]);

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
	GLuint SSAOShader::uniform_invprojm;
	GLuint SSAOShader::uniform_projm;
	GLuint SSAOShader::uniform_samplePoints;
	GLuint SSAOShader::vao;
	float SSAOShader::SSAOSamples[64];
	void SSAOShader::init()
	{
		Program = LoadProgram(file_manager->getAsset("shaders/screenquad.vert").c_str(), file_manager->getAsset("shaders/ssao.frag").c_str());
		uniform_normals_and_depth = glGetUniformLocation(Program, "normals_and_depth");
		uniform_invprojm = glGetUniformLocation(Program, "invprojm");
		uniform_projm = glGetUniformLocation(Program, "projm");
		uniform_samplePoints = glGetUniformLocation(Program, "samplePoints[0]");
		vao = createVAO(Program);

		for (unsigned i = 0; i < 16; i++) {
			// Generate x/y component between -1 and 1
			// Use double to avoid denorm and get a true uniform distribution
			double x = rand();
			x /= RAND_MAX;
			x = 2 * x - 1;
			double y = rand();
			y /= RAND_MAX;
			y = 2 * y - 1;

			// compute z so that norm (x,y,z) is one
			double z = sqrt(x * x + y * y);
			// Norm factor
			double w = rand();
			w /= RAND_MAX;
			SSAOSamples[4 * i] = (float)x;
			SSAOSamples[4 * i + 1] = (float)y;
			SSAOSamples[4 * i + 2] = (float)z;
			SSAOSamples[4 * i + 3] = (float)w;
		}
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