//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 SuperTuxKart-Team
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

#include "graphics/camera.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/wind.hpp"
#include "guiengine/engine.hpp"
#include "modes/world.hpp"
#include "tracks/track.hpp"
#include "utils/helpers.hpp"

using namespace video;
using namespace core;

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

void DisplaceProvider::OnSetConstants(IMaterialRendererServices *srv, int)
{

}

void DisplaceProvider::update()
{
    if (!Track::getCurrentTrack()) return;

    const float time = irr_driver->getDevice()->getTimer()->getTime() / 1000.0f;
    const float speed = Track::getCurrentTrack()->getDisplacementSpeed();

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
}
