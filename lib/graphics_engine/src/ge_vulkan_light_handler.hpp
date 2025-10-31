#ifndef HEADER_GE_VULKAN_LIGHT_HANDLER_HPP
#define HEADER_GE_VULKAN_LIGHT_HANDLER_HPP

#include "vector2d.h"
#include "vector3d.h"
#include "SColor.h"

#include <array>
#include <vector>

namespace irr
{
    namespace scene
    {
        class ILightSceneNode;
    }
}

namespace GE
{
class GEVulkanDriver;
class GEVulkanSkyBoxRenderer;
const irr::u32 MAX_RENDERING_LIGHT = 32;
struct GELight
{
    irr::core::vector3df m_position;
    irr::f32             m_radius;
    irr::core::vector3df m_color;
    irr::f32             m_inverse_range_squared;
    irr::core::vector2df m_direction;
    irr::f32             m_scale;
    irr::f32             m_offset;
};

struct GEGlobalLightBuffer
{
    irr::core::vector3df m_ambient_color;
    irr::f32             m_sun_scatter;
    irr::core::vector3df m_sun_color;
    irr::f32             m_sun_angle_tan_half;
    irr::core::vector3df m_sun_direction;
    irr::f32             m_fog_density;
    irr::video::SColorf  m_fog_color;
    irr::core::vector3df m_skytop_color;
    irr::u32             m_light_count;
    std::array<GELight, MAX_RENDERING_LIGHT> m_rendering_lights;
};

class GEVulkanLightHandler
{
private:
    GEVulkanDriver* m_vk;

    GEGlobalLightBuffer m_buffer;

    std::vector<GELight> m_lights;

    unsigned m_fullscreen_light_count;
public:
    // ------------------------------------------------------------------------
    GEVulkanLightHandler(GEVulkanDriver* vk)
    {
        m_vk = vk;
        prepare();
    }
    // ------------------------------------------------------------------------
    ~GEVulkanLightHandler()                                                  {}
    // ------------------------------------------------------------------------
    void prepare();
    // ------------------------------------------------------------------------
    void generate(const irr::core::vector3df& cam_pos,
                  GEVulkanSkyBoxRenderer* skybox);
    // ------------------------------------------------------------------------
    void addLightNode(irr::scene::ILightSceneNode* node);
    // ------------------------------------------------------------------------
    void* getData()                                       { return &m_buffer; }
    // ------------------------------------------------------------------------
    size_t getSize() const
    {
        return sizeof(GEGlobalLightBuffer) -
            (sizeof(GELight) * (MAX_RENDERING_LIGHT - m_buffer.m_light_count));
    }
    // ------------------------------------------------------------------------
    unsigned getLightCount() const           { return m_buffer.m_light_count; }
    // ------------------------------------------------------------------------
    unsigned getFullscreenLightCount() const
                                           { return m_fullscreen_light_count; }
};   // GEVulkanLightHandler

}

#endif
