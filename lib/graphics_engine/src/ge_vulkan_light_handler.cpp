#include "ge_vulkan_light_handler.hpp"

#include "ge_main.hpp"
#include "ge_occlusion_culling.hpp"
#include "ge_vulkan_driver.hpp"
#include "ge_vulkan_fbo_texture.hpp"
#include "ge_vulkan_skybox_renderer.hpp"

#include "ILightSceneNode.h"
#include "ISceneManager.h"
#include "IrrlichtDevice.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <iterator>

namespace GE
{
using namespace irr;
// ----------------------------------------------------------------------------
void GEVulkanLightHandler::prepare()
{
    m_buffer = {};
    m_lights.clear();
    m_fullscreen_light_count = 0;
    video::SColorf c = m_vk->getIrrlichtDevice()->getSceneManager()
        ->getAmbientLight();
    m_buffer.m_ambient_color.X = c.r * c.a;
    m_buffer.m_ambient_color.Y = c.g * c.a;
    m_buffer.m_ambient_color.Z = c.b * c.a;
    m_buffer.m_sun_scatter = 0.2f;
    m_buffer.m_sun_color = core::vector3df(0.75f, 0.75f, 0.75f);
    m_buffer.m_sun_angle_tan_half = 0.0022f;
    m_buffer.m_sun_direction = core::vector3df(0.15f, 0.2f, 1.0f).normalize();
    m_buffer.m_skytop_color.X = 0.325f;
    m_buffer.m_skytop_color.Y = 0.35f;
    m_buffer.m_skytop_color.Z = 0.375f;
}   // prepare

// ----------------------------------------------------------------------------
void GEVulkanLightHandler::generate(const irr::core::vector3df& cam_pos,
                                    GEVulkanSkyBoxRenderer* skybox)
{
    if (skybox)
    {
        irr::video::SColorf c(
            srgb255ToLinearFromSColor(skybox->getSkytopColor()));
        m_buffer.m_skytop_color.X = c.r;
        m_buffer.m_skytop_color.Y = c.g;
        m_buffer.m_skytop_color.Z = c.b;
    }
    if (m_lights.size() > MAX_RENDERING_LIGHT)
    {
        std::sort(m_lights.begin(), m_lights.end(),
            [cam_pos](GELight& a, GELight& b)
            {
                float al = a.m_position.getDistanceFromSQ(cam_pos);
                float bl = b.m_position.getDistanceFromSQ(cam_pos);
                return al < bl;
            });
        m_lights.resize(MAX_RENDERING_LIGHT);
    }
    if (m_lights.empty())
        return;

    GEVulkanFBOTexture* t =
        static_cast<GEVulkanDriver*>(getDriver())->getRTTTexture();
    if (t && t->isDeferredFBO())
    {
        auto i = std::partition(m_lights.begin(), m_lights.end(),
        [cam_pos](const GELight& l)
        {
            float radius_2 = l.m_radius * l.m_radius;
            float distance_2 = (cam_pos - l.m_position).getLengthSQ();
            return distance_2 <= radius_2;
        });
        m_fullscreen_light_count = std::distance(m_lights.begin(), i);
    }
    // Deferred fbo supports light culling using depth test
    if (hasOcclusionCulling() && (!t || !t->isDeferredFBO()))
    {
        auto l = m_lights.begin();
        auto rl = m_buffer.m_rendering_lights.begin();
        while (l != m_lights.end())
        {
            if (getOcclusionCulling()->isOccluded(cam_pos, l->m_position,
                l->m_radius))
            {
                l++;
                continue;
            }
            *rl = *l;
            l++;
            rl++;
        }
        m_buffer.m_light_count = rl - m_buffer.m_rendering_lights.begin();
    }
    else
    {
        std::copy(m_lights.begin(), m_lights.end(),
            m_buffer.m_rendering_lights.begin());
        m_buffer.m_light_count = m_lights.size();
    }
}   // generate

// ----------------------------------------------------------------------------
void GEVulkanLightHandler::addLightNode(irr::scene::ILightSceneNode* node)
{
    const video::SLight& l = node->getLightData();
    if (node->getLightType() == irr::video::ELT_DIRECTIONAL)
    {
        m_buffer.m_sun_color.X = l.DiffuseColor.r;
        m_buffer.m_sun_color.Y = l.DiffuseColor.g;
        m_buffer.m_sun_color.Z = l.DiffuseColor.b;
        m_buffer.m_sun_scatter = l.DiffuseColor.a;
        core::vector3df dir = l.Direction;
        m_buffer.m_sun_direction = -dir.normalize();
        m_buffer.m_sun_angle_tan_half = tanf(l.Radius * 0.5f);
    }
    else
    {
        GELight gl = {};
        gl.m_position = l.Position;
        gl.m_radius = l.Radius;
        gl.m_color.X = l.DiffuseColor.r * l.Attenuation.X;
        gl.m_color.Y = l.DiffuseColor.g * l.Attenuation.X;
        gl.m_color.Z = l.DiffuseColor.b * l.Attenuation.X;
        gl.m_inverse_range_squared = l.Attenuation.Y * l.Attenuation.Y;
        if (l.Type == irr::video::ELT_SPOT)
        {
            gl.m_direction.X = l.Direction.X;
            gl.m_direction.Y = l.Direction.Y;
            float cos_outer = cosf(l.OuterCone);
            gl.m_scale = 1.0f / std::max(cosf(l.InnerCone) - cos_outer, 1e-4f);
            gl.m_offset = -cos_outer * gl.m_scale;
            gl.m_scale *= l.Direction.Z > 0.f ? 1.f : -1.f;
        }
        m_lights.push_back(gl);
    }
}   // addLightNode

}
