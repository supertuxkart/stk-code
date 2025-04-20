#ifndef HEADER_GE_VULKAN_SKYBOX_RENDERER_HPP
#define HEADER_GE_VULKAN_SKYBOX_RENDERER_HPP

#include "vulkan_wrapper.h"
#include <atomic>

namespace irr
{
    namespace scene { class ISceneNode; }
}

namespace GE
{
class GEVulkanArrayTexture;

class GEVulkanSkyBoxRenderer
{
private:
    irr::scene::ISceneNode* m_skybox;

    GEVulkanArrayTexture* m_texture_cubemap;

    VkDescriptorSetLayout m_descriptor_layout;

    VkDescriptorPool m_descriptor_pool;

    VkDescriptorSet m_descriptor_set;

    std::atomic_bool m_skybox_loading;
public:
    // ------------------------------------------------------------------------
    GEVulkanSkyBoxRenderer();
    // ------------------------------------------------------------------------
    ~GEVulkanSkyBoxRenderer();
    // ------------------------------------------------------------------------
    void addSkyBox(irr::scene::ISceneNode* node);
    // ------------------------------------------------------------------------
    VkDescriptorSetLayout getDescriptorSetLayout() const
                                                { return m_descriptor_layout; }
    // ------------------------------------------------------------------------
    const VkDescriptorSet* getDescriptorSet() const
    {
        if (m_skybox_loading.load() == true)
            return NULL;
        return &m_descriptor_set;
    }

};   // GEVulkanSkyBoxRenderer

}

#endif
