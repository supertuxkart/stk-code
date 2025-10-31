#ifndef HEADER_GE_VULKAN_ARRAY_TEXTURE_HPP
#define HEADER_GE_VULKAN_ARRAY_TEXTURE_HPP

#include "ge_vulkan_texture.hpp"

namespace GE
{
class GEMipmapGenerator;
class GEVulkanDriver;
class GEVulkanArrayTexture : public GEVulkanTexture
{
private:

    class ThreadLoader
    {
    private:
        GEVulkanArrayTexture* m_texture;

        std::vector<video::IImage*> m_images;

        std::vector<GEMipmapGenerator*> m_mipmaps;

        std::vector<io::path> m_list;

        core::dimension2du m_max_size;

        std::function<void(video::IImage*, unsigned)> m_image_mani;

        video::SColor m_unicolor;

        VkImageLayout m_first_layout;
    public:
        // --------------------------------------------------------------------
        ThreadLoader(GEVulkanArrayTexture* texture,
                     const std::vector<io::path>& list,
                     std::function<void(video::IImage*, unsigned)> image_mani,
                     video::SColor unicolor = video::SColor(),
                     VkImageLayout first_layout = VK_IMAGE_LAYOUT_UNDEFINED);
        // --------------------------------------------------------------------
        ~ThreadLoader();
        // --------------------------------------------------------------------
        void load(unsigned layer);
    };
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
    GEVulkanArrayTexture(VkFormat internal_format,
                         VkImageViewType type, const core::dimension2du& size,
                         unsigned layer_count, video::SColor unicolor,
                         VkImageLayout first_layout = VK_IMAGE_LAYOUT_UNDEFINED);
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
