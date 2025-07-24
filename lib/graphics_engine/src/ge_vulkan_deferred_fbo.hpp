#ifndef HEADER_GE_VULKAN_DEFERRED_FBO_HPP
#define HEADER_GE_VULKAN_DEFERRED_FBO_HPP

#include "ge_vulkan_fbo_texture.hpp"

#include <array>

namespace GE
{
enum GEVulkanDeferredFBOType : unsigned
{
    GVDFT_COLOR = 0,
    GVDFT_NORMAL,
    GVDFT_HDR,
    GVDFT_DISPLACE_MASK,
    GVDFT_DISPLACE_SSR,
    GVDFT_DISPLACE_COLOR,
    GVDFT_COUNT,
};

enum GEVulkanDeferredFBOPass : unsigned
{
    GVDFP_HDR = 0,
    GVDFP_CONVERT_COLOR,
    GVDFP_DISPLACE_MASK,
    GVDFP_DISPLACE_COLOR,
    GVDFP_COUNT,
};

class GEVulkanDeferredFBO : public GEVulkanFBOTexture
{
private:
    std::array<GEVulkanAttachmentTexture*, GVDFT_COUNT> m_attachments;

    std::array<VkDescriptorSetLayout, GVDFP_COUNT> m_descriptor_layout;

    std::array<VkDescriptorPool, GVDFP_COUNT> m_descriptor_pool;

    std::array<VkDescriptorSet, GVDFP_COUNT> m_descriptor_set;

    const bool m_swapchain_output;
    // ------------------------------------------------------------------------
    void initConvertColorDescriptor(GEVulkanDriver* vk);
    // ------------------------------------------------------------------------
    void initDisplaceDescriptor(GEVulkanDriver* vk);
    // ------------------------------------------------------------------------
    void createDisplacePasses();
public:
    // ------------------------------------------------------------------------
    GEVulkanDeferredFBO(GEVulkanDriver* vk, const core::dimension2d<u32>& size,
                        bool swapchain_output);
    // ------------------------------------------------------------------------
    virtual ~GEVulkanDeferredFBO();
    // ------------------------------------------------------------------------
    virtual void createRTT();
    // ------------------------------------------------------------------------
    virtual bool isDeferredFBO() const                         { return true; }
    // ------------------------------------------------------------------------
    virtual bool useSwapChainOutput() const      { return m_swapchain_output; }
    // ------------------------------------------------------------------------
    virtual unsigned getZeroClearCountForPass(unsigned pass) const
    {
        switch (pass)
        {
        case GVDFP_HDR:
        {
            unsigned count = 0;
            for (unsigned i = 0; i < m_attachments.size(); i++)
            {
                if (i == GVDFT_HDR)
                    break;
                GEVulkanAttachmentTexture* t = m_attachments[i];
                if (t)
                    count++;
            }
            return count;
        }
        case GVDFP_DISPLACE_MASK:
            return getAttachment<GVDFT_DISPLACE_SSR>() ? 2 : 1;
        case GVDFP_DISPLACE_COLOR:
            return 1;
        default:
            return GEVulkanFBOTexture::getZeroClearCountForPass(pass);
        }
    }
    // ------------------------------------------------------------------------
    virtual VkDescriptorSetLayout getDescriptorSetLayout(unsigned id) const
                                         { return m_descriptor_layout.at(id); }
    // ------------------------------------------------------------------------
    virtual const VkDescriptorSet* getDescriptorSet(unsigned id) const
                                           { return &m_descriptor_set.at(id); }
    // ------------------------------------------------------------------------
    template<unsigned AttachmentType>
    GEVulkanAttachmentTexture* getAttachment() const
    {
        return std::get<AttachmentType>(m_attachments);
    }
};   // GEVulkanDeferredFBO

}

#endif
