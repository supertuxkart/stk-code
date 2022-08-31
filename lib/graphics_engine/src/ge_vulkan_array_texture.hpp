#ifndef HEADER_GE_VULKAN_ARRAY_TEXTURE_HPP
#define HEADER_GE_VULKAN_ARRAY_TEXTURE_HPP

#include "ge_vulkan_texture.hpp"

namespace GE
{
class GEVulkanDriver;
class GEVulkanArrayTexture : public GEVulkanTexture
{
private:
    void reloadInternal(const std::vector<io::path>& list,
                        std::function<void(video::IImage*, unsigned)>
                        image_mani);
public:
    // ------------------------------------------------------------------------
    GEVulkanArrayTexture(const std::vector<io::path>& full_path_list,
                         VkImageViewType type,
                         std::function<void(video::IImage*, unsigned)>
                         image_mani = nullptr);
    // ------------------------------------------------------------------------
    GEVulkanArrayTexture(const std::vector<GEVulkanTexture*>& textures,
                         VkImageViewType type,
                         std::function<void(video::IImage*, unsigned)>
                         image_mani = nullptr);
    // ------------------------------------------------------------------------
    virtual ~GEVulkanArrayTexture()                                          {}
    // ------------------------------------------------------------------------
    virtual void* lock(video::E_TEXTURE_LOCK_MODE mode =
                       video::ETLM_READ_WRITE, u32 mipmap_level = 0)
                                                               { return NULL; }
    // ------------------------------------------------------------------------
    virtual void unlock()                                                    {}
    // ------------------------------------------------------------------------
    virtual video::E_DRIVER_TYPE getDriverType() const
                                                  { return video::EDT_VULKAN; }
    // ------------------------------------------------------------------------
    virtual video::ECOLOR_FORMAT getColorFormat() const
                                                { return video::ECF_A8R8G8B8; }
    // ------------------------------------------------------------------------
    virtual u32 getPitch() const                                  { return 0; }
    // ------------------------------------------------------------------------
    virtual bool hasMipMaps() const                           { return false; }
    // ------------------------------------------------------------------------
    virtual void regenerateMipMapLevels(void* mipmap_data = NULL)            {}
    // ------------------------------------------------------------------------
    virtual void reload()                                                    {}
    // ------------------------------------------------------------------------
    virtual void updateTexture(void* data, irr::video::ECOLOR_FORMAT format,
                               u32 w, u32 h, u32 x, u32 y)                   {}
};   // GEVulkanArrayTexture

}

#endif
