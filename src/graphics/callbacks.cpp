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

#include "graphics/callbacks.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/wind.hpp"
#include "guiengine/engine.hpp"
#include "modes/world.hpp"
#include "tracks/track.hpp"
#include "utils/helpers.hpp"

using namespace video;
using namespace core;

//-------------------------------------

void NormalMapProvider::OnSetConstants(IMaterialRendererServices *srv, int)
{
    if (!firstdone)
    {
        s32 texture = 0;
        srv->setPixelShaderConstant("texture", &texture, 1);

        s32 normaltex = 1;
        srv->setPixelShaderConstant("normalMap", &normaltex, 1);

        // We could calculate light direction as coming from the sun (then we'd need to
        // transform it into camera space). But I find that pretending light
        // comes from the camera gives good results

        firstdone = true;
    }
}

//-------------------------------------

void WaterShaderProvider::OnSetConstants(IMaterialRendererServices *srv, int)
{
    const float time = irr_driver->getDevice()->getTimer()->getTime() / 1000.0f;

    float strength = time;
    strength = 1.4f - fabsf(noise2d(strength / 30.0f + 133)) * 0.8f;

    m_dx_1 += GUIEngine::getLatestDt() * m_water_shader_speed_1 * strength;
    m_dy_1 += GUIEngine::getLatestDt() * m_water_shader_speed_1 * strength;

    m_dx_2 += GUIEngine::getLatestDt() * m_water_shader_speed_2 * strength;
    m_dy_2 -= GUIEngine::getLatestDt() * m_water_shader_speed_2 * strength;

    if (m_dx_1 > 1.0f) m_dx_1 -= 1.0f;
    if (m_dy_1 > 1.0f) m_dy_1 -= 1.0f;
    if (m_dx_2 > 1.0f) m_dx_2 -= 1.0f;
    if (m_dy_2 < 0.0f) m_dy_2 += 1.0f;

    const float d1[2] = { m_dx_1, m_dy_1 };
    const float d2[2] = { m_dx_2, m_dy_2 };

    srv->setVertexShaderConstant("delta1", d1, 2);
    srv->setVertexShaderConstant("delta2", d2, 2);

    const float speed = irr_driver->getDevice()->getTimer()->getTime() / m_speed;
    const float height = m_height * strength;

    srv->setVertexShaderConstant("height", &height, 1);
    srv->setVertexShaderConstant("speed", &speed, 1);
    srv->setVertexShaderConstant("waveLength", &m_length, 1);

    // Can't use the firstdone optimization, as the callback is shared
    //if (!firstdone)
    {
        s32 decaltex = 0;
        srv->setPixelShaderConstant("DecalTex", &decaltex, 1);

        s32 bumptex = 1;
        srv->setPixelShaderConstant("BumpTex1", &bumptex, 1);

        bumptex = 2;
        srv->setPixelShaderConstant("BumpTex2", &bumptex, 1);

        // Calculate light direction as coming from the sun.
        matrix4 normalm = srv->getVideoDriver()->getTransform(ETS_VIEW);
        normalm.makeInverse();
        normalm = normalm.getTransposed();
        vector3df tmp = m_sunpos;
        normalm.transformVect(tmp);
        tmp.normalize();

        const float lightdir[] = {tmp.X, tmp.Y, tmp.Z};
        srv->setVertexShaderConstant("lightdir", lightdir, 3);

        firstdone = true;
    }
}

//-------------------------------------

void GrassShaderProvider::OnSetConstants(IMaterialRendererServices *srv, int userData)
{
    IVideoDriver * const drv = srv->getVideoDriver();
    const core::vector3df pos = drv->getTransform(ETS_WORLD).getTranslation();
    const float time = irr_driver->getDevice()->getTimer()->getTime() / 1000.0f;

    float strength = (pos.X + pos.Y + pos.Z) * 1.2f + time * m_speed;
    strength = noise2d(strength / 10.0f) * m_amplitude * 5;
    // * 5 is to work with the existing amplitude values.

    // Pre-multiply on the cpu
    vector3df wind = irr_driver->getWind() * strength;

    srv->setVertexShaderConstant("windDir", &wind.X, 3);

    if (!firstdone)
    {
        s32 tex = 0;
        srv->setVertexShaderConstant("tex", &tex, 1);

        firstdone = true;
    }
}

//-------------------------------------

void ColorLevelsProvider::OnSetConstants(IMaterialRendererServices *srv, int userData)
{

    m_inlevel = World::getWorld()->getTrack()->getColorLevelIn();
    m_outlevel = World::getWorld()->getTrack()->getColorLevelOut();

    srv->setVertexShaderConstant("inlevel", &m_inlevel.X, 3);
    srv->setVertexShaderConstant("outlevel", &m_outlevel.X, 2);

    if (!firstdone)
    {
        s32 tex = 0;
        srv->setVertexShaderConstant("tex", &tex, 1);

        firstdone = true;
    }
}

//-------------------------------------

void SplattingProvider::OnSetConstants(IMaterialRendererServices *srv, int)
{
    if (!firstdone)
    {
        s32 tex_layout = 1;
        srv->setPixelShaderConstant("tex_layout", &tex_layout, 1);

        s32 tex_detail0 = 2;
        srv->setPixelShaderConstant("tex_detail0", &tex_detail0, 1);

        s32 tex_detail1 = 3;
        srv->setPixelShaderConstant("tex_detail1", &tex_detail1, 1);

        s32 tex_detail2 = 4;
        srv->setPixelShaderConstant("tex_detail2", &tex_detail2, 1);

        s32 tex_detail3 = 5;
        srv->setPixelShaderConstant("tex_detail3", &tex_detail3, 1);

        firstdone = true;
    }
}

//-------------------------------------

void BubbleEffectProvider::OnSetConstants(IMaterialRendererServices *srv, int)
{
    const float start = fabsf(mat.MaterialTypeParam2);
    const bool visible = mat.MaterialTypeParam2 > 0;
    const float time = irr_driver->getDevice()->getTimer()->getTime() / 1000.0f;
    float transparency;

    const float diff = (time - start) / 3.0f;

    if (visible)
    {
        transparency = diff;
    }
    else
    {
        transparency = 1.0f - diff;
    }

    transparency = clampf(transparency, 0, 1);

    srv->setVertexShaderConstant("time", &time, 1);
    srv->setVertexShaderConstant("transparency", &transparency, 1);
}

//-------------------------------------

void RainEffectProvider::OnSetConstants(IMaterialRendererServices *srv, int)
{
    const float screenw = (float)UserConfigParams::m_width;
    const float time = irr_driver->getDevice()->getTimer()->getTime() / 90.0f;
    const matrix4 viewm = srv->getVideoDriver()->getTransform(ETS_VIEW);
	matrix4 invproj = srv->getVideoDriver()->getTransform(ETS_PROJECTION);
	invproj.makeInverse();
    const vector3df campos = irr_driver->getSceneManager()->getActiveCamera()->getPosition();
	float screen[2] = { (float)UserConfigParams::m_width,
		(float)UserConfigParams::m_height };

    srv->setVertexShaderConstant("screenw", &screenw, 1);
    srv->setVertexShaderConstant("time", &time, 1);
    srv->setVertexShaderConstant("viewm", viewm.pointer(), 16);
    srv->setVertexShaderConstant("campos", &campos.X, 3);
	srv->setPixelShaderConstant("invproj", invproj.pointer(), 16);
	srv->setPixelShaderConstant("screen", screen, 2);
	s32 tex = 0;
	srv->setPixelShaderConstant("tex", &tex, 1);
	tex = 1;
	srv->setPixelShaderConstant("normals_and_depth", &tex, 1);
}

//-------------------------------------

void SnowEffectProvider::OnSetConstants(IMaterialRendererServices *srv, int)
{
    const float time = irr_driver->getDevice()->getTimer()->getTime() / 1000.0f;

    srv->setVertexShaderConstant("time", &time, 1);
}

//-------------------------------------

void MotionBlurProvider::OnSetConstants(IMaterialRendererServices *srv, int)
{
    // We need the maximum texture coordinates:
    float max_tex_height = m_maxheight[m_current_camera];
    srv->setPixelShaderConstant("max_tex_height", &max_tex_height, 1);

    // Scale the boost time to get a usable boost amount:
    float boost_amount = m_boost_time[m_current_camera] * 0.7f;

    // Especially for single screen the top of the screen is less blurred
    // in the fragment shader by multiplying the blurr factor by
    // (max_tex_height - texcoords.t), where max_tex_height is the maximum
    // texture coordinate (1.0 or 0.5). In split screen this factor is too
    // small (half the value compared with non-split screen), so we
    // multiply this by 2.
    if(Camera::getNumCameras() > 1)
        boost_amount *= 2.0f;

    srv->setPixelShaderConstant("boost_amount", &boost_amount, 1);
    srv->setPixelShaderConstant("center",
                                     &(m_center[m_current_camera].X), 2);
    srv->setPixelShaderConstant("direction",
                                     &(m_direction[m_current_camera].X), 2);

    // Use a radius of 0.15 when showing a single kart, otherwise (2-4 karts
    // on splitscreen) use only 0.75.
    float radius = Camera::getNumCameras()==1 ? 0.15f : 0.075f;
    srv->setPixelShaderConstant("mask_radius", &radius, 1);

    const int texunit = 0;
    srv->setPixelShaderConstant("color_buffer", &texunit, 1);
}

//-------------------------------------

void GaussianBlurProvider::OnSetConstants(IMaterialRendererServices *srv, int)
{
    srv->setVertexShaderConstant("pixel", m_pixel, 2);
}

//-------------------------------------

void MipVizProvider::OnSetConstants(IMaterialRendererServices *srv, int)
{
    const ITexture * const tex = mat.TextureLayer[0].Texture;

    const int notex = (mat.TextureLayer[0].Texture == NULL);
    srv->setVertexShaderConstant("notex", &notex, 1);
    if (!tex) return;

    const dimension2du size = tex->getSize();

    const float texsize[2] = {
        (float)size.Width,
        (float)size.Height
        };

    srv->setVertexShaderConstant("texsize", texsize, 2);
}

//-------------------------------------

void ColorizeProvider::OnSetConstants(IMaterialRendererServices *srv, int)
{
    srv->setVertexShaderConstant("col", m_color, 3);
}

//-------------------------------------

void GlowProvider::OnSetConstants(IMaterialRendererServices *srv, int)
{
    srv->setVertexShaderConstant("res", m_res, 2);
}

//-------------------------------------

void ObjectPassProvider::OnSetConstants(IMaterialRendererServices *srv, int)
{
    const int hastex = mat.TextureLayer[0].Texture != NULL;
    srv->setVertexShaderConstant("hastex", &hastex, 1);

    const int haslightmap = mat.TextureLayer[1].Texture != NULL;
    srv->setVertexShaderConstant("haslightmap", &haslightmap, 1);

    //if (!firstdone)
    // Can't use the firstdone optimization, as this callback is used for multiple shaders
    {
        int tex = 0;
        srv->setVertexShaderConstant("tex", &tex, 1);

        tex = 1;
        srv->setVertexShaderConstant("lighttex", &tex, 1);
    }
}

//-------------------------------------

void LightBlendProvider::OnSetConstants(IMaterialRendererServices *srv, int)
{
    const SColorf s = irr_driver->getSceneManager()->getAmbientLight();

    float ambient[3] = { s.r, s.g, s.b };
    srv->setVertexShaderConstant("ambient", ambient, 3);

    int tex = 0;
    srv->setVertexShaderConstant("diffuse", &tex, 1);
    tex = 1;
    srv->setVertexShaderConstant("specular", &tex, 1);
    tex = 2;
    srv->setVertexShaderConstant("ambient_occlusion", &tex, 1);
    tex = 3;
    srv->setVertexShaderConstant("specular_map", &tex, 1);
}

//-------------------------------------

void PointLightProvider::OnSetConstants(IMaterialRendererServices *srv, int)
{
    int lightcount = m_color.size() / 4;
    srv->setVertexShaderConstant("screen", m_screen, 2);
    srv->setVertexShaderConstant("spec", &m_specular, 1);
    srv->setVertexShaderConstant("invproj", m_invproj.pointer(), 16);
    srv->setVertexShaderConstant("energy[0]", m_energy.data(), m_energy.size());
    srv->setVertexShaderConstant("col[0]", m_color.data(), m_color.size());
    srv->setVertexShaderConstant("center[0]", m_pos.data(), m_pos.size());
    srv->setVertexShaderConstant("viewm", m_view.pointer(), 16);
    srv->setVertexShaderConstant("lightcount", &lightcount, 1);

    if (!firstdone)
    {
        int tex = 0;
        srv->setVertexShaderConstant("ntex", &tex, 1);

        tex = 1;
        srv->setVertexShaderConstant("dtex", &tex, 1);

        firstdone = true;
    }
}

//-------------------------------------

void SunLightProvider::OnSetConstants(IMaterialRendererServices *srv, int)
{
    const int hasclouds = World::getWorld()->getTrack()->hasClouds() &&
                          UserConfigParams::m_weather_effects;

    srv->setVertexShaderConstant("screen", m_screen, 2);
    srv->setVertexShaderConstant("col", m_color, 3);
    srv->setVertexShaderConstant("center", m_pos, 3);
    srv->setVertexShaderConstant("invproj", m_invproj.pointer(), 16);
    srv->setVertexShaderConstant("hasclouds", &hasclouds, 1);

    const float time = irr_driver->getDevice()->getTimer()->getTime() / 1000.0f;

    float strength = time;
    strength = fabsf(noise2d(strength / 10.0f)) * 0.003f;

    const vector3df winddir = irr_driver->getWind() * strength;
    m_wind[0] += winddir.X;
    m_wind[1] += winddir.Z;
    srv->setVertexShaderConstant("wind", m_wind, 2);

    if (UserConfigParams::m_shadows)
    {
        srv->setVertexShaderConstant("shadowmat", m_shadowmat.pointer(), 16);
    }

    // Can't use the firstdone optimization, as this callback is used for multiple shaders
    //if (!firstdone)
    {
        int tex = 0;
        srv->setVertexShaderConstant("ntex", &tex, 1);

        tex = 1;
        srv->setVertexShaderConstant("dtex", &tex, 1);

        tex = 2;
        srv->setVertexShaderConstant("cloudtex", &tex, 1);

        tex = 3;
        srv->setVertexShaderConstant("shadowtex", &tex, 1);

        tex = 4;
        srv->setVertexShaderConstant("warpx", &tex, 1);

        tex = 5;
        srv->setVertexShaderConstant("warpy", &tex, 1);

        const float shadowoffset = 1.0f / irr_driver->getRTT(RTT_SHADOW)->getSize().Width;
        srv->setVertexShaderConstant("shadowoffset", &shadowoffset, 1);

        firstdone = true;
    }
}

//-------------------------------------

void BloomProvider::OnSetConstants(IMaterialRendererServices *srv, int)
{
    srv->setVertexShaderConstant("low", &m_threshold, 1);
}

//-------------------------------------

void MLAAColor1Provider::OnSetConstants(IMaterialRendererServices *srv, int)
{
    if (!firstdone)
    {
        const float pixels[2] = {
            1.0f / UserConfigParams::m_width,
            1.0f / UserConfigParams::m_height
        };

        srv->setPixelShaderConstant("PIXEL_SIZE", pixels, 2);

        firstdone = true;
    }
}

//-------------------------------------

void MLAABlend2Provider::OnSetConstants(IMaterialRendererServices *srv, int)
{
    if (!firstdone)
    {
        const float pixels[2] = {
            1.0f / UserConfigParams::m_width,
            1.0f / UserConfigParams::m_height
        };

        srv->setPixelShaderConstant("PIXEL_SIZE", pixels, 2);


        int tex = 0;
        srv->setPixelShaderConstant("edgesMap", &tex, 1);

        tex = 1;
        srv->setPixelShaderConstant("areaMap", &tex, 1);


        firstdone = true;
    }
}

//-------------------------------------

void MLAANeigh3Provider::OnSetConstants(IMaterialRendererServices *srv, int)
{
    if (!firstdone)
    {
        const float pixels[2] = {
            1.0f / UserConfigParams::m_width,
            1.0f / UserConfigParams::m_height
        };

        srv->setPixelShaderConstant("PIXEL_SIZE", pixels, 2);


        int tex = 0;
        srv->setPixelShaderConstant("blendMap", &tex, 1);

        tex = 1;
        srv->setPixelShaderConstant("colorMap", &tex, 1);

        firstdone = true;
    }
}

//-------------------------------------

void SSAOProvider::OnSetConstants(IMaterialRendererServices *srv, int)
{
    srv->setPixelShaderConstant("invprojm", invprojm.pointer(), 16);
    srv->setPixelShaderConstant("projm", projm.pointer(), 16);
    srv->setPixelShaderConstant("samplePoints[0]", array, 64);

    if (!firstdone)
    {
        int tex = 0;
        srv->setPixelShaderConstant("normals_and_depth", &tex, 1);

        tex = 1;
        srv->setPixelShaderConstant("depth", &tex, 1);

        firstdone = true;
    }
}

//-------------------------------------

void GodRayProvider::OnSetConstants(IMaterialRendererServices *srv, int)
{
    srv->setPixelShaderConstant("sunpos", m_sunpos, 2);
}

//-------------------------------------

void ShadowPassProvider::OnSetConstants(IMaterialRendererServices *srv, int)
{
    const int hastex = mat.TextureLayer[0].Texture != NULL;
    srv->setVertexShaderConstant("hastex", &hastex, 1);

    int viz = irr_driver->getShadowViz();
    srv->setVertexShaderConstant("viz", &viz, 1);

    int wireframe = mat.Wireframe;
    srv->setVertexShaderConstant("wireframe", &wireframe, 1);

    float objectid = 0;
    if (hastex)
    {
        const stringc name = mat.TextureLayer[0].Texture->getName().getPath();
        objectid = shash8((const u8 *) name.c_str(), name.size()) / 255.0f;
    }
    srv->setVertexShaderConstant("objectid", &objectid, 1);

    //if (!firstdone)
    // Can't use the firstdone optimization, as this callback is used for multiple shaders
    {
        int tex = 0;
        srv->setVertexShaderConstant("tex", &tex, 1);

        tex = 1;
        srv->setVertexShaderConstant("warpx", &tex, 1);
        tex = 2;
        srv->setVertexShaderConstant("warpy", &tex, 1);

        firstdone = true;
    }
}

//-------------------------------------

void ShadowImportanceProvider::OnSetConstants(IMaterialRendererServices *srv, int)
{
    srv->setVertexShaderConstant("shadowmat", m_shadowmat.pointer(), 16);
    srv->setVertexShaderConstant("ipvmat", m_invprojview.pointer(), 16);

    srv->setVertexShaderConstant("campos", m_campos, 3);

    int low = UserConfigParams::m_shadows == 1;
    srv->setVertexShaderConstant("low", &low, 1);

    if (!firstdone)
    {
        int tex = 0;
        srv->setVertexShaderConstant("ntex", &tex, 1);

        tex = 1;
        srv->setVertexShaderConstant("dtex", &tex, 1);

        tex = 2;
        srv->setVertexShaderConstant("ctex", &tex, 1);

        firstdone = true;
    }
}

//-------------------------------------

void CollapseProvider::OnSetConstants(IMaterialRendererServices *srv, int)
{
    srv->setVertexShaderConstant("pixel", m_pixel, 2);
    srv->setVertexShaderConstant("multi", m_multi, 2);
    srv->setVertexShaderConstant("size", &m_size, 1);

    //if (!firstdone)
    // Can't use the firstdone optimization, as this callback is used for multiple shaders
    {
        int tex = 0;
        srv->setVertexShaderConstant("tex", &tex, 1);

        tex = 1;
        srv->setVertexShaderConstant("oldtex", &tex, 1);

        firstdone = true;
    }
}

//-------------------------------------

void BloomPowerProvider::OnSetConstants(IMaterialRendererServices *srv, int)
{
    srv->setVertexShaderConstant("power", &m_power, 1);
}

//-------------------------------------

void MultiplyProvider::OnSetConstants(IMaterialRendererServices *srv, int)
{
    if (!firstdone)
    {
        int tex = 0;
        srv->setVertexShaderConstant("tex1", &tex, 1);

        tex = 1;
        srv->setVertexShaderConstant("tex2", &tex, 1);

        firstdone = true;
    }
}

//-------------------------------------

void ShadowGenProvider::OnSetConstants(IMaterialRendererServices *srv, int)
{
    if (!firstdone)
    {
        int tex = 0;
        srv->setVertexShaderConstant("halft", &tex, 1);

        tex = 1;
        srv->setVertexShaderConstant("quarter", &tex, 1);

        tex = 2;
        srv->setVertexShaderConstant("eighth", &tex, 1);

        firstdone = true;
    }
}

//-------------------------------------

void CausticsProvider::OnSetConstants(IMaterialRendererServices *srv, int)
{
    const float time = irr_driver->getDevice()->getTimer()->getTime() / 1000.0f;
    const float speed = World::getWorld()->getTrack()->getCausticsSpeed();

    float strength = time;
    strength = fabsf(noise2d(strength / 10.0f)) * 0.006f + 0.001f;

    vector3df wind = irr_driver->getWind() * strength * speed;
    m_dir[0] += wind.X;
    m_dir[1] += wind.Z;

    strength = time * 0.56f + sinf(time);
    strength = fabsf(noise2d(0.0, strength / 6.0f)) * 0.0095f + 0.001f;

    wind = irr_driver->getWind() * strength * speed;
    wind.rotateXZBy(cosf(time));
    m_dir2[0] += wind.X;
    m_dir2[1] += wind.Z;

    srv->setVertexShaderConstant("dir", m_dir, 2);
    srv->setVertexShaderConstant("dir2", m_dir2, 2);

    if (!firstdone)
    {
        int tex = 0;
        srv->setVertexShaderConstant("tex", &tex, 1);

        tex = 1;
        srv->setVertexShaderConstant("caustictex", &tex, 1);

        firstdone = true;
    }
}

//-------------------------------------

void DisplaceProvider::OnSetConstants(IMaterialRendererServices *srv, int)
{
    const float time = irr_driver->getDevice()->getTimer()->getTime() / 1000.0f;
    const float speed = World::getWorld()->getTrack()->getDisplacementSpeed();

    float strength = time;
    strength = fabsf(noise2d(strength / 10.0f)) * 0.006f + 0.002f;

    vector3df wind = irr_driver->getWind() * strength * speed;
    m_dir[0] += wind.X;
    m_dir[1] += wind.Z;

    strength = time * 0.56f + sinf(time);
    strength = fabsf(noise2d(0.0, strength / 6.0f)) * 0.0095f + 0.0025f;

    wind = irr_driver->getWind() * strength * speed;
    wind.rotateXZBy(cosf(time));
    m_dir2[0] += wind.X;
    m_dir2[1] += wind.Z;

    srv->setVertexShaderConstant("dir", m_dir, 2);
    srv->setVertexShaderConstant("dir2", m_dir2, 2);

    srv->setVertexShaderConstant("screen", m_screen, 2);
}

//-------------------------------------

void PPDisplaceProvider::OnSetConstants(IMaterialRendererServices *srv, int)
{
    int viz = irr_driver->getDistortViz();
    srv->setPixelShaderConstant("viz", &viz, 1);

    if (!firstdone)
    {
        int tex = 0;
        srv->setPixelShaderConstant("tex", &tex, 1);

        tex = 1;
        srv->setPixelShaderConstant("dtex", &tex, 1);

        firstdone = true;
    }
}

//-------------------------------------

void FogProvider::OnSetConstants(IMaterialRendererServices *srv, int)
{
    const Track * const track = World::getWorld()->getTrack();

    // This function is only called once per frame - thus no need for setters.
    const float fogmax = track->getFogMax();
    const float startH = track->getFogStartHeight();
    const float endH = track->getFogEndHeight();
    const float start = track->getFogStart();
    const float end = track->getFogEnd();
    const SColor tmpcol = track->getFogColor();

    const float col[3] = { tmpcol.getRed() / 255.0f,
                           tmpcol.getGreen() / 255.0f,
                           tmpcol.getBlue() / 255.0f };

    srv->setPixelShaderConstant("fogmax", &fogmax, 1);
    srv->setPixelShaderConstant("startH", &startH, 1);
    srv->setPixelShaderConstant("endH", &endH, 1);
    srv->setPixelShaderConstant("start", &start, 1);
    srv->setPixelShaderConstant("end", &end, 1);
    srv->setPixelShaderConstant("col", col, 3);
    srv->setVertexShaderConstant("ipvmat", m_invprojview.pointer(), 16);
    srv->setVertexShaderConstant("campos", m_campos, 3);
}
