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
    m_callbacks[ES_COLOR_LEVELS] = new ColorLevelsProvider();
    m_callbacks[ES_BUBBLES] = new BubbleEffectProvider();
    m_callbacks[ES_RAIN] = new RainEffectProvider();
    m_callbacks[ES_MOTIONBLUR] = new MotionBlurProvider();
    m_callbacks[ES_GAUSSIAN3V] = m_callbacks[ES_GAUSSIAN3H] = new GaussianBlurProvider();
    m_callbacks[ES_MIPVIZ] = new MipVizProvider();
    m_callbacks[ES_COLORIZE] = new ColorizeProvider();
    m_callbacks[ES_GLOW] = new GlowProvider();
    m_callbacks[ES_OBJECTPASS] = new ObjectPassProvider();
    m_callbacks[ES_LIGHTBLEND] = new LightBlendProvider();
    m_callbacks[ES_POINTLIGHT] = new PointLightProvider();
    m_callbacks[ES_SUNLIGHT] = new SunLightProvider();
    m_callbacks[ES_BLOOM] = new BloomProvider();
    m_callbacks[ES_MLAA_COLOR1] = new MLAAColor1Provider();
    m_callbacks[ES_MLAA_BLEND2] = new MLAABlend2Provider();
    m_callbacks[ES_MLAA_NEIGH3] = new MLAANeigh3Provider();
    m_callbacks[ES_SSAO] = new SSAOProvider();
    m_callbacks[ES_GODRAY] = new GodRayProvider();
    m_callbacks[ES_SHADOWPASS] = new ShadowPassProvider();
    m_callbacks[ES_SHADOW_IMPORTANCE] = new ShadowImportanceProvider();
    m_callbacks[ES_COLLAPSE] = new CollapseProvider();
    m_callbacks[ES_BLOOM_POWER] = new BloomPowerProvider();
    m_callbacks[ES_MULTIPLY_ADD] = new MultiplyProvider();
    m_callbacks[ES_SHADOWGEN] = new ShadowGenProvider();
    m_callbacks[ES_CAUSTICS] = new CausticsProvider();
    m_callbacks[ES_DISPLACE] = new DisplaceProvider();
    m_callbacks[ES_PPDISPLACE] = new PPDisplaceProvider();
    m_callbacks[ES_FOG] = new FogProvider();

    for(s32 i=0 ; i < ES_COUNT ; i++)
        m_shaders[i] = -1;

    loadShaders();
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

    m_shaders[ES_SPHERE_MAP] = glslmat(dir + "objectpass_rimlit.vert", dir + "objectpass_spheremap.frag",
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

	m_shaders[ES_GAUSSIAN6H] = glslmat(dir + "pass.vert", dir + "gaussian6h.frag",
                                    m_callbacks[ES_GAUSSIAN3H], EMT_SOLID);
	m_shaders[ES_GAUSSIAN6V] = glslmat(dir + "pass.vert", dir + "gaussian6v.frag",
                                    m_callbacks[ES_GAUSSIAN3V], EMT_SOLID);

    m_shaders[ES_MIPVIZ] = glslmat(std::string(""), dir + "mipviz.frag",
                                    m_callbacks[ES_MIPVIZ], EMT_SOLID);

    m_shaders[ES_FLIP] = glslmat(std::string(""), dir + "flip.frag",
                                    0, EMT_SOLID);
    m_shaders[ES_FLIP_ADDITIVE] = glslmat(std::string(""), dir + "flip.frag",
                                    0, EMT_TRANSPARENT_ADD_COLOR);
                                    
    m_shaders[ES_COLOR_LEVELS] = glslmat(std::string(""), dir + "color_levels.frag",
                                    m_callbacks[ES_COLOR_LEVELS], EMT_SOLID);

    m_shaders[ES_BLOOM] = glslmat(std::string(""), dir + "bloom.frag",
                                    m_callbacks[ES_BLOOM], EMT_SOLID);

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

	m_shaders[ES_LIGHTBLEND] = glslmat(dir + "pass.vert", dir + "lightblend.frag",
                                    m_callbacks[ES_LIGHTBLEND], EMT_ONETEXTURE_BLEND);

	m_shaders[ES_POINTLIGHT] = glslmat(dir + "pass.vert", dir + "pointlight.frag",
                                    m_callbacks[ES_POINTLIGHT], EMT_ONETEXTURE_BLEND);

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

    m_shaders[ES_SSAO] = glsl(dir + "pass.vert", dir + "ssao.frag", m_callbacks[ES_SSAO]);

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
    m_shaders[ES_BLOOM_BLEND] = glslmat(std::string(""), dir + "bloomblend.frag",
                                    0, EMT_TRANSPARENT_ADD_COLOR);

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
    m_shaders[ES_PPDISPLACE] = glsl(std::string(""), dir + "ppdisplace.frag",
                                  m_callbacks[ES_PPDISPLACE]);

    m_shaders[ES_PASSFAR] = glsl(dir + "farplane.vert", dir + "colorize.frag",
                                  m_callbacks[ES_COLORIZE]);

	m_shaders[ES_FOG] = glslmat(dir + "pass.vert", dir + "fog.frag",
                                    m_callbacks[ES_FOG], EMT_ONETEXTURE_BLEND);

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
