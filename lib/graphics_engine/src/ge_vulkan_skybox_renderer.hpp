#ifndef HEADER_GE_VULKAN_SKYBOX_RENDERER_HPP
#define HEADER_GE_VULKAN_SKYBOX_RENDERER_HPP

#include "vulkan_wrapper.h"
#include <SColor.h>
#include <array>
#include <atomic>

namespace irr
{
    namespace scene { class ISceneNode; }
}

namespace GE
{
class GEVulkanArrayTexture;
class GEVulkanEnvironmentMap;

class GEVulkanSkyBoxRenderer
{
private:
    friend class GEVulkanEnvironmentMap;
    irr::scene::ISceneNode* m_skybox;

    GEVulkanArrayTexture *m_texture_cubemap, *m_diffuse_env_cubemap,
        *m_specular_env_cubemap, *m_dummy_env_cubemap;

    VkDescriptorSetLayout m_env_descriptor_layout;

    VkDescriptorPool m_descriptor_pool;

    std::array<VkDescriptorSet, 2> m_env_descriptor_set;

    std::atomic_bool m_skybox_loading, m_env_cubemap_loading;

    std::atomic<uint32_t> m_skytop_color;
public:
    // ------------------------------------------------------------------------
    GEVulkanSkyBoxRenderer();
    // ------------------------------------------------------------------------
    ~GEVulkanSkyBoxRenderer();
    // ------------------------------------------------------------------------
    void addSkyBox(irr::scene::ISceneNode* node);
    // ------------------------------------------------------------------------
    VkDescriptorSetLayout getEnvDescriptorSetLayout() const
                                            { return m_env_descriptor_layout; }
    // ------------------------------------------------------------------------
    const VkDescriptorSet* getEnvDescriptorSet() const;
    // ------------------------------------------------------------------------
    void reset()
    {
        while (m_skybox_loading.load());
        while (m_env_cubemap_loading.load());
        m_skybox = NULL;
    }
    // ------------------------------------------------------------------------
    irr::video::SColor getSkytopColor() const
    {
        irr::video::SColor c(0);
        if (m_skybox_loading.load() == true)
            return c;
        c.color = m_skytop_color.load();
        return c;
    }

};   // GEVulkanSkyBoxRenderer

}

#endif
