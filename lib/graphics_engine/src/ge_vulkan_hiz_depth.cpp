#include "ge_vulkan_hiz_depth.hpp"

#include "ge_main.hpp"
#include "ge_vulkan_array_texture.hpp"
#include "ge_vulkan_attachment_texture.hpp"
#include "ge_vulkan_camera_scene_node.hpp"
#include "ge_vulkan_deferred_fbo.hpp"
#include "ge_vulkan_driver.hpp"
#include "ge_vulkan_shader_manager.hpp"

#include <algorithm>
#include <stdexcept>

namespace GE
{
// ----------------------------------------------------------------------------
GEVulkanHiZDepth::GEVulkanHiZDepth(GEVulkanDriver* vk)
                : m_vk(vk), m_hiz_depth(NULL),
                  m_descriptor_layout(VK_NULL_HANDLE),
                  m_pipeline_layout(VK_NULL_HANDLE),
                  m_pipeline(VK_NULL_HANDLE),
                  m_descriptor_pool(VK_NULL_HANDLE),
                  m_rendering_descriptor_pool(VK_NULL_HANDLE),
                  m_rendering_descriptor_set(VK_NULL_HANDLE)
{
}   // GEVulkanHiZDepth

// ----------------------------------------------------------------------------
GEVulkanHiZDepth::~GEVulkanHiZDepth()
{
    destroy();
}   // ~GEVulkanHiZDepth

// ----------------------------------------------------------------------------
void GEVulkanHiZDepth::prepare(GEVulkanCameraSceneNode* cam)
{
    irr::core::recti hiz_size(irr::core::position2di(
        cam->getUBOData()->m_viewport.UpperLeftCorner.X,
        cam->getUBOData()->m_viewport.UpperLeftCorner.Y),
        irr::core::dimension2du(
        cam->getUBOData()->m_viewport.LowerRightCorner.X,
        cam->getUBOData()->m_viewport.LowerRightCorner.Y));
    if (m_hiz_size != hiz_size)
    {
        m_hiz_size = hiz_size;
        destroy();
        init();
    }
}   // prepare

// ----------------------------------------------------------------------------
void GEVulkanHiZDepth::init()
{
    m_hiz_depth = new GEVulkanArrayTexture(VK_FORMAT_R32_SFLOAT,
        VK_IMAGE_VIEW_TYPE_2D,
        irr::core::dimension2du(m_hiz_size.getWidth(), m_hiz_size.getHeight()),
        1, irr::video::SColor(0), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {};
    // Input depth buffer
    bindings[0].binding = 0;
    bindings[0].descriptorCount = 1;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    // Output storage image
    bindings[1].binding = 1;
    bindings[1].descriptorCount = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutCreateInfo layout_info = {};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = bindings.size();
    layout_info.pBindings = bindings.data();

    VkDevice device = m_vk->getDevice();
    if (vkCreateDescriptorSetLayout(device, &layout_info, NULL,
        &m_descriptor_layout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create descriptor set layout for "
            "HiZ depth");
    }

    VkPushConstantRange push_constant = {};
    push_constant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    push_constant.offset = 0;
    push_constant.size = sizeof(uint32_t) * 3;

    VkPipelineLayoutCreateInfo pipeline_layout_info = {};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &m_descriptor_layout;
    pipeline_layout_info.pushConstantRangeCount = 1;
    pipeline_layout_info.pPushConstantRanges = &push_constant;

    if (vkCreatePipelineLayout(device, &pipeline_layout_info, NULL,
        &m_pipeline_layout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline layout for HiZ "
            "depth");
    }

    VkComputePipelineCreateInfo pipeline_info = {};
    pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipeline_info.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipeline_info.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    pipeline_info.stage.module = GEVulkanShaderManager::getShader("hiz_depth.comp");
    pipeline_info.stage.pName = "main";
    pipeline_info.layout = m_pipeline_layout;

    if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1,
        &pipeline_info, NULL, &m_pipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create compute pipeline for HiZ "
            "depth");
    }

    // Create descriptor pool
    const uint32_t mip_levels = m_hiz_depth->getMipmapLevels();
    std::array<VkDescriptorPoolSize, 2> pool_sizes = {};
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[0].descriptorCount = mip_levels;
    pool_sizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    pool_sizes[1].descriptorCount = mip_levels;

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.maxSets = mip_levels;
    pool_info.poolSizeCount = pool_sizes.size();
    pool_info.pPoolSizes = pool_sizes.data();

    if (vkCreateDescriptorPool(device, &pool_info, NULL,
        &m_descriptor_pool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create descriptor pool for HiZ depth");
    }

    // Create descriptor sets for each mip level
    m_descriptor_sets.resize(mip_levels);
    std::vector<VkDescriptorSetLayout> layouts(mip_levels,
        m_descriptor_layout);

    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = m_descriptor_pool;
    alloc_info.descriptorSetCount = m_descriptor_sets.size();
    alloc_info.pSetLayouts = layouts.data();

    if (vkAllocateDescriptorSets(device, &alloc_info,
        m_descriptor_sets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate descriptor sets for HiZ "
            "depth");
    }

    // Create image views for each mip level
    m_hiz_views.resize(mip_levels);
    for (uint32_t i = 0; i < m_hiz_views.size(); i++)
    {
        VkImageViewCreateInfo view_info = {};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = m_hiz_depth->getImage();
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = m_hiz_depth->getInternalFormat();
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_info.subresourceRange.baseMipLevel = i;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &view_info, NULL,
            &m_hiz_views[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create image view for HiZ "
                "depth");
        }
    }

    GEVulkanDeferredFBO* dfbo =
        static_cast<GEVulkanDeferredFBO*>(m_vk->getRTTTexture());
    for (uint32_t i = 0; i < mip_levels; i++)
    {
        VkDescriptorImageInfo input_info = {};
        input_info.imageLayout = i == 0 ?
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL :
            VK_IMAGE_LAYOUT_GENERAL;
        input_info.imageView = i == 0 ?
            (VkImageView)dfbo->getDepthTexture()->getTextureHandler() :
            (VkImageView)m_hiz_depth->getTextureHandler();
        input_info.sampler = m_vk->getSampler(GVS_SKYBOX);

        VkDescriptorImageInfo output_info = {};
        output_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        output_info.imageView = m_hiz_views[i];

        std::array<VkWriteDescriptorSet, 2> descriptor_writes = {};
        descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[0].dstSet = m_descriptor_sets[i];
        descriptor_writes[0].dstBinding = 0;
        descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_writes[0].descriptorCount = 1;
        descriptor_writes[0].pImageInfo = &input_info;

        descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[1].dstSet = m_descriptor_sets[i];
        descriptor_writes[1].dstBinding = 1;
        descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        descriptor_writes[1].descriptorCount = 1;
        descriptor_writes[1].pImageInfo = &output_info;

        vkUpdateDescriptorSets(m_vk->getDevice(), descriptor_writes.size(),
            descriptor_writes.data(), 0, NULL);
    }
    loadRenderingDescriptor();
}   // init

// ----------------------------------------------------------------------------
void GEVulkanHiZDepth::loadRenderingDescriptor()
{
    const size_t displace_binding_count = 3;
    VkDescriptorPoolSize pool_size;
    pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_size.descriptorCount = displace_binding_count;

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = 0;
    pool_info.maxSets = 1;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = &pool_size;
    if (vkCreateDescriptorPool(m_vk->getDevice(), &pool_info, NULL,
        &m_rendering_descriptor_pool) != VK_SUCCESS)
    {
        throw std::runtime_error("vkCreateDescriptorPool failed for "
            "m_rendering_descriptor_pool in GEVulkanHiZDepth");
    }

    GEVulkanDeferredFBO* dfbo =
        static_cast<GEVulkanDeferredFBO*>(m_vk->getRTTTexture());
    std::vector<VkDescriptorSetLayout> layouts(1,
        dfbo->getDescriptorSetLayout(GVDFP_DISPLACE_COLOR));

    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = m_rendering_descriptor_pool;
    alloc_info.descriptorSetCount = layouts.size();
    alloc_info.pSetLayouts = layouts.data();

    if (vkAllocateDescriptorSets(m_vk->getDevice(), &alloc_info,
        &m_rendering_descriptor_set) != VK_SUCCESS)
    {
        throw std::runtime_error("vkAllocateDescriptorSets failed for "
            "m_rendering_descriptor_set in GEVulkanHiZDepth");
    }

    std::array<VkDescriptorImageInfo, displace_binding_count> image_infos = {};
    image_infos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_infos[0].imageView =
        (VkImageView)
        dfbo->getAttachment<GVDFT_DISPLACE_COLOR>()->getTextureHandler();
    image_infos[0].sampler = m_vk->getSampler(GVS_NEAREST);
    image_infos[1].imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    image_infos[1].imageView =
        (VkImageView)dfbo->getDepthTexture()->getTextureHandler();
    image_infos[1].sampler = m_vk->getSampler(GVS_SHADOW);
    image_infos[2].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_infos[2].imageView =
        (VkImageView)m_hiz_depth->getTextureHandler();
    image_infos[2].sampler = m_vk->getSampler(GVS_SKYBOX);

    VkWriteDescriptorSet write_descriptor_set = {};
    write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_descriptor_set.dstBinding = 0;
    write_descriptor_set.dstArrayElement = 0;
    write_descriptor_set.descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write_descriptor_set.descriptorCount = image_infos.size();
    write_descriptor_set.pBufferInfo = 0;
    write_descriptor_set.dstSet = m_rendering_descriptor_set;
    write_descriptor_set.pImageInfo = image_infos.data();

    vkUpdateDescriptorSets(m_vk->getDevice(), 1, &write_descriptor_set, 0,
        NULL);
}   // loadRenderingDescriptor

// ----------------------------------------------------------------------------
void GEVulkanHiZDepth::destroy()
{
    delete m_hiz_depth;
    m_hiz_depth = NULL;
    VkDevice device = m_vk->getDevice();
    if (m_pipeline != VK_NULL_HANDLE)
        vkDestroyPipeline(device, m_pipeline, NULL);
    m_pipeline = VK_NULL_HANDLE;
    if (m_pipeline_layout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(device, m_pipeline_layout, NULL);
    m_pipeline_layout = VK_NULL_HANDLE;
    if (m_rendering_descriptor_pool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(device, m_rendering_descriptor_pool, NULL);
    m_rendering_descriptor_pool = VK_NULL_HANDLE;
    m_rendering_descriptor_set = VK_NULL_HANDLE;
    if (m_descriptor_pool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(device, m_descriptor_pool, NULL);
    m_descriptor_pool = VK_NULL_HANDLE;
    if (m_descriptor_layout != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(device, m_descriptor_layout, NULL);
    m_descriptor_layout = VK_NULL_HANDLE;
    for (VkImageView view : m_hiz_views)
        vkDestroyImageView(device, view, NULL);
    m_hiz_views.clear();
    m_descriptor_sets.clear();
}   // destroy

// ----------------------------------------------------------------------------
void GEVulkanHiZDepth::generate(VkCommandBuffer cmd)
{
    const uint32_t mip_levels = m_hiz_depth->getMipmapLevels();

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_hiz_depth->getImage();
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mip_levels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(cmd,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        0, 0, NULL, 0, NULL, 1, &barrier);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline);

    vkCmdPushConstants(cmd, m_pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT,
        0, sizeof(uint32_t) * 2, &m_hiz_size.UpperLeftCorner.X);

    for (uint32_t i = 0; i < mip_levels; i++)
    {
        // Bind descriptor set and push mip level constant
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
            m_pipeline_layout, 0, 1, &m_descriptor_sets[i], 0, NULL);

        vkCmdPushConstants(cmd, m_pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT,
            sizeof(uint32_t) * 2, sizeof(uint32_t), &i);

        // Calculate dispatch size based on current mip level dimensions
        const uint32_t width = m_hiz_depth->getSize().Width >> i;
        const uint32_t height = m_hiz_depth->getSize().Height >> i;
        const uint32_t group_size_x = std::max(1u, (width + 15) / 16);
        const uint32_t group_size_y = std::max(1u, (height + 15) / 16);

        vkCmdDispatch(cmd, group_size_x, group_size_y, 1);

        // Add memory barrier between mip level generations
        if (i < mip_levels - 1)
        {
            barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            barrier.subresourceRange.baseMipLevel = i;
            barrier.subresourceRange.levelCount = 1;

            vkCmdPipelineBarrier(cmd,
                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                0, 0, NULL, 0, NULL, 1, &barrier);
        }
    }

    barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mip_levels;

    vkCmdPipelineBarrier(cmd,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0, 0, NULL, 0, NULL, 1, &barrier);
}   // generate

}
