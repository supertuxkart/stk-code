#include "ge_vulkan_deferred_fbo.hpp"

#include "ge_main.hpp"
#include "ge_vulkan_attachment_texture.hpp"
#include "ge_vulkan_command_loader.hpp"
#include "ge_vulkan_driver.hpp"

#include <array>
#include <exception>
#include <stdexcept>

namespace GE
{
GEVulkanDeferredFBO::GEVulkanDeferredFBO(GEVulkanDriver* vk,
                                         const core::dimension2d<u32>& size,
                                         bool swapchain_output)
                   : GEVulkanFBOTexture(vk, size,
                     !(!vk->getSeparateRTTTexture() &&
                     getGEConfig()->m_auto_deferred_type == GADT_DISPLACE)),
                     m_swapchain_output(swapchain_output)
{
    m_attachments = {};
    m_descriptor_layout.fill(VK_NULL_HANDLE);
    m_descriptor_pool.fill(VK_NULL_HANDLE);
    m_descriptor_set.fill(VK_NULL_HANDLE);
    for (unsigned i = GVDFT_COLOR; i <= GVDFT_NORMAL; i++)
    {
        m_attachments[i] = new GEVulkanAttachmentTexture(vk, size,
            VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
            VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
            VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT);
    }
    std::vector<VkFormat> hdr_formats =
    {
        VK_FORMAT_B10G11R11_UFLOAT_PACK32,
        VK_FORMAT_R16G16B16A16_SFLOAT,
        VK_FORMAT_B8G8R8A8_UNORM
    };
    VkFormat hdr_format = vk->findSupportedFormat(hdr_formats,
        VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT |
        VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT);
    m_attachments[GVDFT_HDR] = new GEVulkanAttachmentTexture(vk, size,
        hdr_format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
        VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
        VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT);

    if (!vk->getSeparateRTTTexture() &&
        getGEConfig()->m_auto_deferred_type == GADT_DISPLACE)
    {
        std::vector<VkFormat> displace_mask_formats =
        {
            VK_FORMAT_R8G8_UNORM,
            VK_FORMAT_B8G8R8A8_UNORM
        };
        VkFormat displace_mask_format = vk->findSupportedFormat(
            displace_mask_formats, VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT);
        m_attachments[GVDFT_DISPLACE_MASK] = new GEVulkanAttachmentTexture(vk,
            size, displace_mask_format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
            VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

        if (getGEConfig()->m_screen_space_reflection_type != GSSRT_DISABLED)
        {
            m_attachments[GVDFT_DISPLACE_SSR] =
                new GEVulkanAttachmentTexture(vk, size,
                VK_FORMAT_B8G8R8A8_UNORM,
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
        }

        VkCommandBuffer command_buffer =
            GEVulkanCommandLoader::beginSingleTimeCommands();
        m_attachments[GVDFT_DISPLACE_MASK]->transitionImageLayout(
            command_buffer, VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        if (getAttachment<GVDFT_DISPLACE_SSR>())
        {
            getAttachment<GVDFT_DISPLACE_SSR>()->transitionImageLayout(
                command_buffer, VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
        GEVulkanCommandLoader::endSingleTimeCommands(command_buffer);

        m_attachments[GVDFT_DISPLACE_COLOR] = new GEVulkanAttachmentTexture(vk,
            size, VK_FORMAT_B8G8R8A8_UNORM,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT);
    }

    // m_descriptor_layout[GVDFP_HDR]
    std::array<VkDescriptorSetLayoutBinding, 3> texture_layout_binding = {};
    texture_layout_binding[0].binding = 0;
    texture_layout_binding[0].descriptorCount = 1;
    texture_layout_binding[0].descriptorType =
        VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    texture_layout_binding[0].pImmutableSamplers = NULL;
    texture_layout_binding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    texture_layout_binding[1] = texture_layout_binding[0];
    texture_layout_binding[1].binding = 1;
    texture_layout_binding[2] = texture_layout_binding[0];
    texture_layout_binding[2].binding = 2;

    VkDescriptorSetLayoutCreateInfo setinfo = {};
    setinfo.flags = 0;
    setinfo.pNext = NULL;
    setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setinfo.pBindings = texture_layout_binding.data();
    setinfo.bindingCount = texture_layout_binding.size();
    if (vkCreateDescriptorSetLayout(vk->getDevice(), &setinfo,
        NULL, &m_descriptor_layout[GVDFP_HDR]) != VK_SUCCESS)
    {
        throw std::runtime_error("vkCreateDescriptorSetLayout failed for "
            "GVDFP_HDR in GEVulkanDeferredFBO");
    }

    // m_descriptor_pool[GVDFP_HDR]
    VkDescriptorPoolSize pool_size;
    pool_size.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    pool_size.descriptorCount = texture_layout_binding.size();

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = 0;
    pool_info.maxSets = 1;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = &pool_size;
    if (vkCreateDescriptorPool(vk->getDevice(), &pool_info, NULL,
        &m_descriptor_pool[GVDFP_HDR]) != VK_SUCCESS)
    {
        throw std::runtime_error("vkCreateDescriptorPool failed for "
            "GVDFP_HDR in GEVulkanDeferredFBO");
    }

    // m_descriptor_set[GVDFP_HDR]
    std::vector<VkDescriptorSetLayout> layouts(1,
        m_descriptor_layout[GVDFP_HDR]);

    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = m_descriptor_pool[GVDFP_HDR];
    alloc_info.descriptorSetCount = layouts.size();
    alloc_info.pSetLayouts = layouts.data();

    if (vkAllocateDescriptorSets(vk->getDevice(), &alloc_info,
        &m_descriptor_set[GVDFP_HDR]) != VK_SUCCESS)
    {
        throw std::runtime_error("vkAllocateDescriptorSets failed for "
            "GVDFP_HDR in GEVulkanDeferredFBO");
    }

    std::array<VkDescriptorImageInfo, 3> image_infos = {};
    image_infos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_infos[0].imageView =
        (VkImageView)m_attachments[GVDFT_COLOR]->getTextureHandler();
    image_infos[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_infos[1].imageView =
        (VkImageView)m_attachments[GVDFT_NORMAL]->getTextureHandler();
    image_infos[2].imageLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    image_infos[2].imageView =
        (VkImageView)m_depth_texture->getTextureHandler();

    VkWriteDescriptorSet write_descriptor_set = {};
    write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_descriptor_set.dstBinding = 0;
    write_descriptor_set.dstArrayElement = 0;
    write_descriptor_set.descriptorType =
        VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    write_descriptor_set.descriptorCount = image_infos.size();
    write_descriptor_set.pBufferInfo = 0;
    write_descriptor_set.dstSet = m_descriptor_set[GVDFP_HDR];
    write_descriptor_set.pImageInfo = image_infos.data();

    vkUpdateDescriptorSets(vk->getDevice(), 1, &write_descriptor_set, 0,
        NULL);

    initConvertColorDescriptor(vk);
    if (getAttachment<GVDFT_DISPLACE_COLOR>())
        initDisplaceDescriptor(vk);
}   // GEVulkanDeferredFBO

// ----------------------------------------------------------------------------
GEVulkanDeferredFBO::~GEVulkanDeferredFBO()
{
    for (GEVulkanAttachmentTexture* t : m_attachments)
        delete t;
    for (VkDescriptorPool pool : m_descriptor_pool)
    {
        if (pool != VK_NULL_HANDLE)
            vkDestroyDescriptorPool(m_vk->getDevice(), pool, NULL);
    }
    for (VkDescriptorSetLayout descriptor_layout : m_descriptor_layout)
    {
        if (descriptor_layout != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorSetLayout(m_vk->getDevice(), descriptor_layout,
                NULL);
        }
    }
}   // ~GEVulkanDeferredFBO

// ----------------------------------------------------------------------------
void GEVulkanDeferredFBO::initConvertColorDescriptor(GEVulkanDriver* vk)
{
    // m_descriptor_layout[GVDFP_CONVERT_COLOR]
    std::array<VkDescriptorSetLayoutBinding, 1> texture_layout_binding = {};
    texture_layout_binding[0].binding = 0;
    texture_layout_binding[0].descriptorCount = 1;
    texture_layout_binding[0].descriptorType =
        VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    texture_layout_binding[0].pImmutableSamplers = NULL;
    texture_layout_binding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo setinfo = {};
    setinfo.flags = 0;
    setinfo.pNext = NULL;
    setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setinfo.pBindings = texture_layout_binding.data();
    setinfo.bindingCount = texture_layout_binding.size();
    if (vkCreateDescriptorSetLayout(vk->getDevice(), &setinfo,
        NULL, &m_descriptor_layout[GVDFP_CONVERT_COLOR]) != VK_SUCCESS)
    {
        throw std::runtime_error("vkCreateDescriptorSetLayout failed for "
            "GVDFP_CONVERT_COLOR in GEVulkanDeferredFBO");
    }

    // m_descriptor_pool[GVDFP_CONVERT_COLOR]
    VkDescriptorPoolSize pool_size;
    pool_size.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    pool_size.descriptorCount = texture_layout_binding.size();

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = 0;
    pool_info.maxSets = 1;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = &pool_size;
    if (vkCreateDescriptorPool(vk->getDevice(), &pool_info, NULL,
        &m_descriptor_pool[GVDFP_CONVERT_COLOR]) != VK_SUCCESS)
    {
        throw std::runtime_error("vkCreateDescriptorPool failed for "
            "GVDFP_CONVERT_COLOR in GEVulkanDeferredFBO");
    }

    // m_descriptor_set[GVDFP_CONVERT_COLOR]
    std::vector<VkDescriptorSetLayout> layouts(1,
        m_descriptor_layout[GVDFP_CONVERT_COLOR]);

    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = m_descriptor_pool[GVDFP_CONVERT_COLOR];
    alloc_info.descriptorSetCount = layouts.size();
    alloc_info.pSetLayouts = layouts.data();

    if (vkAllocateDescriptorSets(vk->getDevice(), &alloc_info,
        &m_descriptor_set[GVDFP_CONVERT_COLOR]) != VK_SUCCESS)
    {
        throw std::runtime_error("vkAllocateDescriptorSets failed for "
            "GVDFP_CONVERT_COLOR in GEVulkanDeferredFBO");
    }

    std::array<VkDescriptorImageInfo, 1> image_infos = {};
    image_infos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_infos[0].imageView =
        (VkImageView)m_attachments[GVDFT_HDR]->getTextureHandler();

    VkWriteDescriptorSet write_descriptor_set = {};
    write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_descriptor_set.dstBinding = 0;
    write_descriptor_set.dstArrayElement = 0;
    write_descriptor_set.descriptorType =
        VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    write_descriptor_set.descriptorCount = image_infos.size();
    write_descriptor_set.pBufferInfo = 0;
    write_descriptor_set.dstSet = m_descriptor_set[GVDFP_CONVERT_COLOR];
    write_descriptor_set.pImageInfo = image_infos.data();

    vkUpdateDescriptorSets(vk->getDevice(), 1, &write_descriptor_set, 0,
        NULL);
}   // initConvertColorDescriptor

// ----------------------------------------------------------------------------
void GEVulkanDeferredFBO::initDisplaceDescriptor(GEVulkanDriver* vk)
{
    // m_descriptor_layout[GVDFP_DISPLACE_COLOR]
    std::array<VkDescriptorSetLayoutBinding, 3> texture_layout_binding = {};
    texture_layout_binding[0].binding = 0;
    texture_layout_binding[0].descriptorCount = 1;
    texture_layout_binding[0].descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    texture_layout_binding[0].pImmutableSamplers = NULL;
    texture_layout_binding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    texture_layout_binding[1] = texture_layout_binding[0];
    texture_layout_binding[1].binding = 1;
    texture_layout_binding[2] = texture_layout_binding[0];
    texture_layout_binding[2].binding = 2;

    VkDescriptorSetLayoutCreateInfo setinfo = {};
    setinfo.flags = 0;
    setinfo.pNext = NULL;
    setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setinfo.pBindings = texture_layout_binding.data();
    setinfo.bindingCount = texture_layout_binding.size();
    if (vkCreateDescriptorSetLayout(vk->getDevice(), &setinfo,
        NULL, &m_descriptor_layout[GVDFP_DISPLACE_COLOR]) != VK_SUCCESS)
    {
        throw std::runtime_error("vkCreateDescriptorSetLayout failed for "
            "GVDFP_DISPLACE_COLOR in GEVulkanDeferredFBO");
    }

    int hiz_multi =
        getGEConfig()->m_screen_space_reflection_type <= GSSRT_FAST ? 2 : 1;
    // m_descriptor_pool[GVDFP_DISPLACE_COLOR]
    VkDescriptorPoolSize pool_size;
    pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_size.descriptorCount = texture_layout_binding.size() * hiz_multi;

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = 0;
    pool_info.maxSets = hiz_multi;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = &pool_size;
    if (vkCreateDescriptorPool(vk->getDevice(), &pool_info, NULL,
        &m_descriptor_pool[GVDFP_DISPLACE_COLOR]) != VK_SUCCESS)
    {
        throw std::runtime_error("vkCreateDescriptorPool failed for "
            "GVDFP_DISPLACE_COLOR in GEVulkanDeferredFBO");
    }

    // m_descriptor_set[GVDFP_DISPLACE_MASK + GVDFP_DISPLACE_COLOR]
    std::vector<VkDescriptorSetLayout> layouts(hiz_multi,
        m_descriptor_layout[GVDFP_DISPLACE_COLOR]);

    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = m_descriptor_pool[GVDFP_DISPLACE_COLOR];
    alloc_info.descriptorSetCount = layouts.size();
    alloc_info.pSetLayouts = layouts.data();

    if (vkAllocateDescriptorSets(vk->getDevice(), &alloc_info,
        hiz_multi == 1 ? &m_descriptor_set[GVDFP_DISPLACE_COLOR] :
        &m_descriptor_set[GVDFP_DISPLACE_MASK]) != VK_SUCCESS)
    {
        throw std::runtime_error("vkAllocateDescriptorSets failed for "
            "GVDFP_DISPLACE_MASK + GVDFP_DISPLACE_COLOR in "
            "GEVulkanDeferredFBO");
    }

    std::array<VkDescriptorImageInfo, texture_layout_binding.size()>
        image_infos = {};
    image_infos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_infos[0].imageView =
        (VkImageView)m_attachments[GVDFT_DISPLACE_MASK]->getTextureHandler();
    image_infos[0].sampler = m_vk->getSampler(GVS_NEAREST);
    image_infos[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_infos[1].imageView = m_attachments[GVDFT_DISPLACE_SSR] ?
        (VkImageView)m_attachments[GVDFT_DISPLACE_SSR]->getTextureHandler() :
        (VkImageView)m_vk->getTransparentTexture()->getTextureHandler();
    image_infos[1].sampler = m_vk->getSampler(GVS_NEAREST);
    image_infos[2].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_infos[2].imageView =
        (VkImageView)m_attachments[GVDFT_DISPLACE_COLOR]->getTextureHandler();
    image_infos[2].sampler = m_vk->getSampler(GVS_NEAREST);

    VkWriteDescriptorSet write_descriptor_set = {};
    write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_descriptor_set.dstBinding = 0;
    write_descriptor_set.dstArrayElement = 0;
    write_descriptor_set.descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write_descriptor_set.descriptorCount = image_infos.size();
    write_descriptor_set.pBufferInfo = 0;
    write_descriptor_set.dstSet = m_descriptor_set[GVDFP_DISPLACE_COLOR];
    write_descriptor_set.pImageInfo = image_infos.data();

    vkUpdateDescriptorSets(vk->getDevice(), 1, &write_descriptor_set, 0,
        NULL);

    if (hiz_multi == 1)
        return;
    image_infos[0] = image_infos[2];
    image_infos[1].imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    image_infos[1].imageView =
        (VkImageView)m_depth_texture->getTextureHandler();
    image_infos[1].sampler = m_vk->getSampler(GVS_SHADOW);
    image_infos[2].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_infos[2].imageView =
        (VkImageView)m_vk->getTransparentTexture()->getTextureHandler();
    write_descriptor_set.dstSet = m_descriptor_set[GVDFP_DISPLACE_MASK];
    image_infos[2].sampler = m_vk->getSampler(GVS_SKYBOX);

    vkUpdateDescriptorSets(vk->getDevice(), 1, &write_descriptor_set, 0,
        NULL);
}   // initDisplaceDescriptor

// ----------------------------------------------------------------------------
void GEVulkanDeferredFBO::createRTT()
{
    if (!useSwapChainOutput())
        createOutputImage();

    std::array<VkAttachmentDescription, 5> attachment_desc = {};
    // HDR attachment
    attachment_desc[0].format = m_attachments[GVDFT_HDR]->getInternalFormat();
    attachment_desc[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachment_desc[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment_desc[0].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_desc[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_desc[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_desc[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment_desc[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    // Depth attachment
    attachment_desc[1].format = m_depth_texture->getInternalFormat();
    attachment_desc[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachment_desc[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    if (getAttachment<GVDFT_DISPLACE_COLOR>())
        attachment_desc[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    else
        attachment_desc[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_desc[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_desc[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_desc[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    if (getAttachment<GVDFT_DISPLACE_COLOR>())
        attachment_desc[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    else
        attachment_desc[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    // Color / normal (mixed with pbr data) attachment
    attachment_desc[2].format = VK_FORMAT_B8G8R8A8_UNORM;
    attachment_desc[2].samples = VK_SAMPLE_COUNT_1_BIT;
    attachment_desc[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment_desc[2].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_desc[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_desc[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_desc[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment_desc[2].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    attachment_desc[3] = attachment_desc[2];
    // Output / swapchain attachment
    bool single_pass_swapchain =
        useSwapChainOutput() && !getAttachment<GVDFT_DISPLACE_COLOR>();
    attachment_desc[4] = attachment_desc[2];
    attachment_desc[4].format = single_pass_swapchain ?
        m_vk->getSwapChainImageFormat() : VK_FORMAT_B8G8R8A8_UNORM;
    attachment_desc[4].finalLayout = single_pass_swapchain ?
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR :
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    attachment_desc[4].storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    VkAttachmentReference hdr_reference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    VkAttachmentReference depth_reference = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
    std::array<VkAttachmentReference, 2> pbr_reference =
        {{
            { 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
            { 3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }
        }};

    std::array<VkSubpassDescription, 3> subpass_desc = {};
    subpass_desc[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_desc[0].colorAttachmentCount = pbr_reference.size();
    subpass_desc[0].pColorAttachments = pbr_reference.data();
    subpass_desc[0].pDepthStencilAttachment = &depth_reference;

    std::array<VkAttachmentReference, 3> input_reference =
        {{
            { 2, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
            { 3, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
            { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL },
        }};
    subpass_desc[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_desc[1].colorAttachmentCount = 1;
    subpass_desc[1].pColorAttachments = &hdr_reference;
    VkAttachmentReference depth_reference_read_only = depth_reference;
    depth_reference_read_only.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    subpass_desc[1].pDepthStencilAttachment = &depth_reference_read_only;
    subpass_desc[1].inputAttachmentCount = input_reference.size();
    subpass_desc[1].pInputAttachments = input_reference.data();

    VkAttachmentReference final_reference = { 4, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    subpass_desc[2].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_desc[2].colorAttachmentCount = 1;
    subpass_desc[2].pColorAttachments = &final_reference;
    VkAttachmentReference final_input_reference = { 0, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
    subpass_desc[2].inputAttachmentCount = 1;
    subpass_desc[2].pInputAttachments = &final_input_reference;
    subpass_desc[2].pDepthStencilAttachment = &depth_reference;

    // Create the actual render pass
    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = attachment_desc.size();
    render_pass_info.pAttachments = attachment_desc.data();
    render_pass_info.subpassCount = subpass_desc.size();
    render_pass_info.pSubpasses = subpass_desc.data();
    std::vector<VkSubpassDependency> dependencies(4);

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT |
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].dstSubpass = 1;
    dependencies[1].srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].srcAccessMask = 0;
    dependencies[1].dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[2].srcSubpass = 0;
    dependencies[2].dstSubpass = 1;
    dependencies[2].srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[2].dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[2].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[3].srcSubpass = 1;
    dependencies[3].dstSubpass = 2;
    dependencies[3].srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[3].dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[3].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[3].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[3].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    if (single_pass_swapchain)
    {
        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 2;
        dependency.srcStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        dependencies.push_back(dependency);
    }

    if (getAttachment<GVDFT_DISPLACE_COLOR>())
    {
        VkSubpassDependency dependency = {};
        dependency.srcSubpass = 2;
        dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
        dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        dependencies.push_back(dependency);
    }

    render_pass_info.dependencyCount = dependencies.size();
    render_pass_info.pDependencies = dependencies.data();

    m_rtt_render_pass.resize(1, VK_NULL_HANDLE);
    if (vkCreateRenderPass(m_vk->getDevice(), &render_pass_info, NULL,
        &m_rtt_render_pass[0]) != VK_SUCCESS)
        throw std::runtime_error("vkCreateRenderPass failed in createRTT");

    std::vector<std::array<VkImageView, attachment_desc.size()> > attachments(1);
    auto& sciv = m_vk->getSwapChainImageViews();
    if (single_pass_swapchain)
        attachments.resize(sciv.size());
    m_rtt_frame_buffer.resize(attachments.size(), VK_NULL_HANDLE);
    for (unsigned i = 0; i < attachments.size(); i++)
    {
        attachments[i] =
        {{
            (VkImageView)m_attachments[GVDFT_HDR]->getTextureHandler(),
            (VkImageView)m_depth_texture->getTextureHandler(),
            (VkImageView)m_attachments[GVDFT_COLOR]->getTextureHandler(),
            (VkImageView)m_attachments[GVDFT_NORMAL]->getTextureHandler(),
            VK_NULL_HANDLE
        }};
        if (getAttachment<GVDFT_DISPLACE_COLOR>())
        {
            attachments[i][4] = (VkImageView)
                getAttachment<GVDFT_DISPLACE_COLOR>()->getTextureHandler();
        }
        else if (useSwapChainOutput())
            attachments[i][4] = sciv[i];
        else
            attachments[i][4] = (VkImageView)getTextureHandler();

        VkFramebufferCreateInfo framebuffer_info = {};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = m_rtt_render_pass[0];
        framebuffer_info.attachmentCount = attachments[i].size();
        framebuffer_info.pAttachments = attachments[i].data();
        framebuffer_info.width = m_depth_texture->getSize().Width;
        framebuffer_info.height = m_depth_texture->getSize().Height;
        framebuffer_info.layers = 1;

        if (vkCreateFramebuffer(m_vk->getDevice(), &framebuffer_info,
            NULL, &m_rtt_frame_buffer[i]) != VK_SUCCESS)
            throw std::runtime_error("vkCreateFramebuffer failed in createRTT");
    }

    if (getAttachment<GVDFT_DISPLACE_COLOR>())
        createDisplacePasses();
}   // createRTT

// ----------------------------------------------------------------------------
void GEVulkanDeferredFBO::createDisplacePasses()
{
    m_rtt_render_pass.resize(GVDFP_COUNT, VK_NULL_HANDLE);

    // m_rtt_render_pass[GVDFP_DISPLACE_MASK]
    {
        std::vector<VkAttachmentDescription> attachment_desc(1);
        attachment_desc[0].format = m_attachments[GVDFT_DISPLACE_MASK]->getInternalFormat();
        attachment_desc[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_desc[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment_desc[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment_desc[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_desc[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_desc[0].initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        attachment_desc[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        if (getAttachment<GVDFT_DISPLACE_SSR>())
        {
            attachment_desc.push_back(attachment_desc[0]);
            attachment_desc.back().format =
                getAttachment<GVDFT_DISPLACE_SSR>()->getInternalFormat();
        }
        VkAttachmentDescription depth_desc = {};
        depth_desc.format = m_depth_texture->getInternalFormat();
        depth_desc.samples = VK_SAMPLE_COUNT_1_BIT;
        depth_desc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        depth_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depth_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depth_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        depth_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        attachment_desc.push_back(depth_desc);

        VkAttachmentReference depth_reference = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL };
        std::vector<VkAttachmentReference> color_references =
            {{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }};
        if (getAttachment<GVDFT_DISPLACE_SSR>())
        {
            color_references.push_back({ 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
            depth_reference.attachment = 2;
        }

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = color_references.size();
        subpass.pColorAttachments = color_references.data();
        subpass.pDepthStencilAttachment = &depth_reference;

        std::array<VkSubpassDependency, 1> dependencies = {};
        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
            VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo render_pass_info = {};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_info.attachmentCount = attachment_desc.size();
        render_pass_info.pAttachments = attachment_desc.data();
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &subpass;
        render_pass_info.dependencyCount = dependencies.size();
        render_pass_info.pDependencies = dependencies.data();

        if (vkCreateRenderPass(m_vk->getDevice(), &render_pass_info, NULL,
            &m_rtt_render_pass[GVDFP_DISPLACE_MASK]) != VK_SUCCESS)
            throw std::runtime_error("vkCreateRenderPass failed for GVDFP_DISPLACE_MASK");
    }

    // m_rtt_render_pass[GVDFP_DISPLACE_COLOR]
    {
        std::array<VkAttachmentDescription, 2> attachment_desc = {};
        attachment_desc[0].format = useSwapChainOutput() ?
            m_vk->getSwapChainImageFormat() : VK_FORMAT_B8G8R8A8_UNORM;
        attachment_desc[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_desc[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment_desc[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment_desc[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_desc[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_desc[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment_desc[0].finalLayout = useSwapChainOutput() ?
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        attachment_desc[1].format = m_depth_texture->getInternalFormat();
        attachment_desc[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_desc[1].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        attachment_desc[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_desc[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_desc[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment_desc[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        attachment_desc[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference color_reference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
        VkAttachmentReference depth_reference = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL };

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_reference;
        subpass.pDepthStencilAttachment = &depth_reference;

        std::array<VkSubpassDependency, 1> dependencies = {};
        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
            VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo render_pass_info = {};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_info.attachmentCount = attachment_desc.size();
        render_pass_info.pAttachments = attachment_desc.data();
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &subpass;
        render_pass_info.dependencyCount = dependencies.size();
        render_pass_info.pDependencies = dependencies.data();

        if (vkCreateRenderPass(m_vk->getDevice(), &render_pass_info, NULL,
            &m_rtt_render_pass[GVDFP_DISPLACE_COLOR]) != VK_SUCCESS)
            throw std::runtime_error("vkCreateRenderPass failed for GVDFP_DISPLACE_COLOR");
    }

    m_rtt_frame_buffer.resize(GVDFP_COUNT, VK_NULL_HANDLE);
    auto& sciv = m_vk->getSwapChainImageViews();
    if (useSwapChainOutput())
    {
        for (unsigned i = 0; i < sciv.size() - 1; i++)
            m_rtt_frame_buffer.push_back(VK_NULL_HANDLE);
    }

    // m_rtt_frame_buffer[GVDFP_DISPLACE_MASK]
    {
        std::vector<VkImageView> attachments =
        {
            (VkImageView)m_attachments[GVDFT_DISPLACE_MASK]->getTextureHandler()
        };
        if (getAttachment<GVDFT_DISPLACE_SSR>())
        {
            attachments.push_back((VkImageView)
                m_attachments[GVDFT_DISPLACE_SSR]->getTextureHandler());
        }
        attachments.push_back((VkImageView)m_depth_texture->getTextureHandler());

        VkFramebufferCreateInfo framebuffer_info = {};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = m_rtt_render_pass[GVDFP_DISPLACE_MASK];
        framebuffer_info.attachmentCount = attachments.size();
        framebuffer_info.pAttachments = attachments.data();
        framebuffer_info.width = m_depth_texture->getSize().Width;
        framebuffer_info.height = m_depth_texture->getSize().Height;
        framebuffer_info.layers = 1;

        if (vkCreateFramebuffer(m_vk->getDevice(), &framebuffer_info, NULL,
            &m_rtt_frame_buffer[GVDFP_DISPLACE_MASK]) != VK_SUCCESS)
            throw std::runtime_error("vkCreateFramebuffer failed for GVDFP_DISPLACE_MASK");
    }

    // m_rtt_frame_buffer[GVDFP_DISPLACE_COLOR]
    for (unsigned i = GVDFP_DISPLACE_COLOR; i < m_rtt_frame_buffer.size(); i++)
    {
        std::array<VkImageView, 2> attachments =
        {{
            useSwapChainOutput() ? sciv[i - GVDFP_DISPLACE_COLOR] : (VkImageView)getTextureHandler(),
            (VkImageView)m_depth_texture->getTextureHandler()
        }};

        VkFramebufferCreateInfo framebuffer_info = {};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = m_rtt_render_pass[GVDFP_DISPLACE_COLOR];
        framebuffer_info.attachmentCount = attachments.size();
        framebuffer_info.pAttachments = attachments.data();
        framebuffer_info.width = m_depth_texture->getSize().Width;
        framebuffer_info.height = m_depth_texture->getSize().Height;
        framebuffer_info.layers = 1;

        if (vkCreateFramebuffer(m_vk->getDevice(), &framebuffer_info, NULL,
            &m_rtt_frame_buffer[i]) != VK_SUCCESS)
            throw std::runtime_error("vkCreateFramebuffer failed for GVDFP_DISPLACE_COLOR");
    }
}   // createDisplacePasses

}
