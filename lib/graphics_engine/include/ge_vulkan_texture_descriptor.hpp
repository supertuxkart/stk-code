#ifndef HEADER_GE_VULKAN_TEXTURE_DESCRIPTOR_HPP
#define HEADER_GE_VULKAN_TEXTURE_DESCRIPTOR_HPP

#include "vulkan_wrapper.h"

#include "IrrCompileConfig.h"
namespace irr
{
    namespace video { class ITexture; }
}

#include <array>
#include <atomic>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace GE
{
class GEVulkanDriver;
enum GEVulkanSampler : unsigned;

class GEVulkanTextureDescriptor
{
    typedef std::array<std::shared_ptr<std::atomic<VkImageView> >,
        _IRR_MATERIAL_MAX_TEXTURES_> TextureList;

    std::map<TextureList, int> m_texture_list;

    std::shared_ptr<std::atomic<VkImageView> > m_white_image;

    std::shared_ptr<std::atomic<VkImageView> > m_transparent_image;

    VkDescriptorSetLayout m_descriptor_set_layout;

    VkDescriptorPool m_descriptor_pool;

    std::vector<VkDescriptorSet> m_descriptor_sets;

    const unsigned m_max_texture_list;

    const unsigned m_max_layer;

    const unsigned m_binding;

    GEVulkanSampler m_sampler_use;

    GEVulkanDriver* m_vk;

    bool m_recreate_next_frame;

    bool m_needs_update_descriptor;
public:
    // ------------------------------------------------------------------------
    GEVulkanTextureDescriptor(unsigned max_texture_list, unsigned max_layer,
        bool single_descriptor, unsigned binding = 0);
    // ------------------------------------------------------------------------
    ~GEVulkanTextureDescriptor();
    // ------------------------------------------------------------------------
    void clear()
    {
        m_texture_list.clear();
        m_needs_update_descriptor = true;
        m_recreate_next_frame = false;
    }
    // ------------------------------------------------------------------------
    void handleDeletedTextures()
    {
        bool has_deleted_image_view = false;
        for (auto& p : m_texture_list)
        {
            for (auto& t : p.first)
            {
                if (t.get()->load() == VK_NULL_HANDLE)
                {
                    has_deleted_image_view = true;
                    break;
                }
            }
        }
        if (has_deleted_image_view || m_recreate_next_frame)
            clear();
    }
    // ------------------------------------------------------------------------
    int getTextureID(const irr::video::ITexture** list,
                     const std::string& shader = std::string());
    // ------------------------------------------------------------------------
    void setSamplerUse(GEVulkanSampler sampler)
    {
        if (m_sampler_use == sampler)
            return;
        m_sampler_use = sampler;
        m_needs_update_descriptor = true;
    }
    // ------------------------------------------------------------------------
    void updateDescriptor();
    // ------------------------------------------------------------------------
    unsigned getMaxTextureList() const           { return m_max_texture_list; }
    // ------------------------------------------------------------------------
    unsigned getMaxLayer() const                        { return m_max_layer; }
    // ------------------------------------------------------------------------
    VkDescriptorSetLayout* getDescriptorSetLayout()
                                           { return &m_descriptor_set_layout; }
    // ------------------------------------------------------------------------
    VkDescriptorSet* getDescriptorSet()
                                           { return m_descriptor_sets.data(); }
};   // GEVulkanTextureDescriptor

}

#endif
