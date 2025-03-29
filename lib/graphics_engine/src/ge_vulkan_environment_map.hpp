#ifndef HEADER_GE_VULKAN_ENVIRONMENT_MAP_HPP
#define HEADER_GE_VULKAN_ENVIRONMENT_MAP_HPP

#include "ge_mipmap_generator.hpp"
#include "ge_vulkan_array_texture.hpp"
#include "ge_vulkan_texture.hpp"

namespace GE
{
class GEVulkanDriver;
class GEVulkanEnvironmentMap : public GEVulkanArrayTexture
{
protected:
    bool m_diffuse;

    virtual void reloadInternal(const std::vector<io::path>& list,
                                std::function<void(video::IImage*, unsigned)>
                                image_mani) override;
public:
    // ------------------------------------------------------------------------
    GEVulkanEnvironmentMap(const std::vector<io::path>& full_path_list,
                           VkImageViewType type,
                           std::function<void(video::IImage*, unsigned)>
                           image_mani = nullptr, bool diffuse = true);
    // ------------------------------------------------------------------------
    GEVulkanEnvironmentMap(const std::vector<GEVulkanTexture*>& textures,
                           VkImageViewType type,
                           std::function<void(video::IImage*, unsigned)>
                           image_mani = nullptr, bool diffuse = true);
    // ------------------------------------------------------------------------
    virtual ~GEVulkanEnvironmentMap()                                        {}
};   // GEVulkanArrayTexture

}

#endif
