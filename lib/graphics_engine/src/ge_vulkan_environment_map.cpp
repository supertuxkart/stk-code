#include "ge_vulkan_environment_map.hpp"

#include "ge_main.hpp"
#include "ge_vulkan_array_texture.hpp"
#include "ge_vulkan_command_loader.hpp"
#include "ge_vulkan_driver.hpp"
#include "ge_vulkan_shader_manager.hpp"
#include "ge_vulkan_skybox_renderer.hpp"

#include <algorithm>
#include <array>

namespace GE
{
// ----------------------------------------------------------------------------
GEVulkanEnvironmentMap::GEVulkanEnvironmentMap(GEVulkanSkyBoxRenderer* skybox)
                      : m_skybox(skybox)
{
    m_skybox->m_env_cubemap_loading.store(true);
}   // GEVulkanEnvironmentMap

// ----------------------------------------------------------------------------
GEVulkanEnvironmentMap::~GEVulkanEnvironmentMap()
{
    m_skybox->m_env_cubemap_loading.store(false);
}   // ~GEVulkanEnvironmentMap

// ----------------------------------------------------------------------------
void GEVulkanEnvironmentMap::load()
{
    struct PushConstants
    {
        int m_size;
        int m_sample_count;
        int m_level;
        int m_total_mipmaps;
    };
    PushConstants pc;
    VkPushConstantRange push_constant = {};
    push_constant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    push_constant.offset = 0;
    push_constant.size = sizeof(PushConstants);

    VkPipelineLayoutCreateInfo pipeline_layout_info = {};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pushConstantRangeCount = 1;
    pipeline_layout_info.pPushConstantRanges = &push_constant;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {};
    bindings[0].binding = 0;
    bindings[0].descriptorCount = 1;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    bindings[1].binding = 1;
    bindings[1].descriptorCount = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutCreateInfo setinfo = {};
    setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setinfo.pBindings = bindings.data();
    setinfo.bindingCount = bindings.size();

    GEVulkanDriver* vk = getVKDriver();
    VkDescriptorSetLayout layout = VK_NULL_HANDLE;
    VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> descriptor_sets;
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    VkPipeline diffuse_pipeline = VK_NULL_HANDLE;
    VkPipeline specular_pipeline = VK_NULL_HANDLE;

    auto levels = [](GEVulkanArrayTexture* t)
    {
        std::vector<unsigned> l;
        l.push_back(t->getSize().Width);
        unsigned width = l.back();
        while (true)
        {
            width = width < 2 ? 1 : width >> 1;
            l.push_back(width);
            if (width == 1)
                break;
        }
        return l;
    };
    GEVulkanArrayTexture* texture_cubemap = m_skybox->m_texture_cubemap;
    GEVulkanArrayTexture* diffuse_env_cubemap =
        m_skybox->m_diffuse_env_cubemap;
    GEVulkanArrayTexture* specular_env_cubemap =
        m_skybox->m_specular_env_cubemap;
    std::vector<unsigned> diffuse_levels = levels(diffuse_env_cubemap);
    std::vector<unsigned> specular_levels = levels(specular_env_cubemap);
    unsigned total_levels = diffuse_levels.size() + specular_levels.size();
    std::array<VkDescriptorPoolSize, 2> pool_sizes =
    {{
        {
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, total_levels
        },
        {
            VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, total_levels
        }
    }};
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.maxSets = total_levels;
    pool_info.poolSizeCount = pool_sizes.size();
    pool_info.pPoolSizes = pool_sizes.data();

    std::vector<VkDescriptorSetLayout> data_layouts;
    VkDescriptorSetAllocateInfo alloc_info = {};

    VkComputePipelineCreateInfo compute_info = {};
    compute_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    compute_info.stage.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    compute_info.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    compute_info.stage.pName = "main";

    VkCommandBuffer cmd = VK_NULL_HANDLE;

    std::vector<VkDescriptorImageInfo> image_infos;
    std::vector<VkImageView> image_views;
    for (unsigned i = 0; i < diffuse_levels.size(); i++)
    {
        VkImageViewCreateInfo view_info = {};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = diffuse_env_cubemap->getImage();
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        view_info.format = diffuse_env_cubemap->getInternalFormat();
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_info.subresourceRange.baseMipLevel = i;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 6;
        VkImageView view = VK_NULL_HANDLE;
        if (vkCreateImageView(vk->getDevice(), &view_info, NULL, &view)
            != VK_SUCCESS)
        {
            printf("vkCreateImageView failed for "
                "GEVulkanEnvironmentMap::load");
            goto destroy;
        }
        image_views.push_back(view);
    }
    for (unsigned i = 0; i < specular_levels.size(); i++)
    {
        VkImageViewCreateInfo view_info = {};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = specular_env_cubemap->getImage();
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        view_info.format = specular_env_cubemap->getInternalFormat();
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_info.subresourceRange.baseMipLevel = i;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 6;
        VkImageView view = VK_NULL_HANDLE;
        if (vkCreateImageView(vk->getDevice(), &view_info, NULL, &view)
            != VK_SUCCESS)
        {
            printf("vkCreateImageView failed for "
                "GEVulkanEnvironmentMap::load");
            goto destroy;
        }
        image_views.push_back(view);
    }

    if (vkCreateDescriptorSetLayout(vk->getDevice(), &setinfo, NULL,
        &layout) != VK_SUCCESS)
    {
        printf("vkCreateDescriptorSetLayout failed for "
            "GEVulkanEnvironmentMap::load");
        goto destroy;
    }

    if (vkCreateDescriptorPool(vk->getDevice(), &pool_info, NULL,
        &descriptor_pool) != VK_SUCCESS)
    {
        printf("createDescriptorPool for GEVulkanEnvironmentMap::load");
        goto destroy;
    }

    data_layouts.resize(total_levels, layout);
    descriptor_sets.resize(total_levels);
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = descriptor_pool;
    alloc_info.descriptorSetCount = data_layouts.size();
    alloc_info.pSetLayouts = data_layouts.data();
    if (vkAllocateDescriptorSets(vk->getDevice(), &alloc_info,
        descriptor_sets.data()) != VK_SUCCESS)
    {
        printf("vkAllocateDescriptorSets failed for "
            "GEVulkanEnvironmentMap::load");
        goto destroy;
    }

    for (VkImageView view : image_views)
    {
        VkDescriptorImageInfo info = {};
        info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        info.sampler = vk->getSampler(GVS_SKYBOX);
        info.imageView = texture_cubemap->getImageView(true/*srgb*/)->load();
        image_infos.push_back(info);
        info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        info.imageView = view;
        image_infos.push_back(info);
    }

    for (unsigned i = 0; i < total_levels; i++)
    {
        std::array<VkWriteDescriptorSet, 2> write_descriptor_set = {};
        write_descriptor_set[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptor_set[0].dstSet = descriptor_sets[i];
        write_descriptor_set[0].dstBinding = 0;
        write_descriptor_set[0].descriptorType =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write_descriptor_set[0].descriptorCount = 1;
        write_descriptor_set[0].pImageInfo = &image_infos[i * 2];

        write_descriptor_set[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptor_set[1].dstSet = descriptor_sets[i];
        write_descriptor_set[1].dstBinding = 1;
        write_descriptor_set[1].descriptorType =
            VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        write_descriptor_set[1].descriptorCount = 1;
        write_descriptor_set[1].pImageInfo = &image_infos[i * 2 + 1];

        vkUpdateDescriptorSets(vk->getDevice(), write_descriptor_set.size(),
            write_descriptor_set.data(), 0, NULL);
    }

    pipeline_layout_info.pSetLayouts = &layout;
    if (vkCreatePipelineLayout(vk->getDevice(), &pipeline_layout_info, NULL,
        &pipeline_layout) != VK_SUCCESS)
    {
        printf("vkCreatePipelineLayout failed for "
            "GEVulkanEnvironmentMap::load");
        goto destroy;
    }

    compute_info.stage.module =
        GEVulkanShaderManager::getShader("diffuse_irradiance.comp");
    compute_info.layout = pipeline_layout;
    if (vkCreateComputePipelines(vk->getDevice(), VK_NULL_HANDLE, 1,
        &compute_info, NULL, &diffuse_pipeline) != VK_SUCCESS)
    {
        printf("vkCreateComputePipelines failed for "
            "GEVulkanEnvironmentMap::load");
        goto destroy;
    }

    compute_info.stage.module =
        GEVulkanShaderManager::getShader("specular_prefilter.comp");
    if (vkCreateComputePipelines(vk->getDevice(), VK_NULL_HANDLE, 1,
        &compute_info, NULL, &specular_pipeline) != VK_SUCCESS)
    {
        printf("vkCreateComputePipelines failed for "
            "GEVulkanEnvironmentMap::load");
        goto destroy;
    }

    cmd = GEVulkanCommandLoader::beginSingleTimeCommands();
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, diffuse_pipeline);
    for (unsigned i = 0; i < diffuse_levels.size(); i++)
    {
        PushConstants pc;
        pc.m_size = diffuse_levels[i];
        pc.m_sample_count = getDiffuseEnvironmentMapSampleCount();
        pc.m_level = i;
        pc.m_total_mipmaps = diffuse_levels.size();

        vkCmdPushConstants(cmd, pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT,
            0, sizeof(PushConstants), &pc);

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
            pipeline_layout, 0, 1, &descriptor_sets[i], 0, NULL);

        // Calculate dispatch size (ceil(size/16))
        uint32_t dispatch_size = (pc.m_size + 15) / 16;
        vkCmdDispatch(cmd, dispatch_size, dispatch_size, 6);
    }

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, specular_pipeline);
    for (unsigned i = 0; i < specular_levels.size(); i++)
    {
        const unsigned offset = diffuse_levels.size();
        auto next_power_of_2 = [](unsigned v)
        {
            v--;
            v |= v >> 1;
            v |= v >> 2;
            v |= v >> 4;
            v |= v >> 8;
            v |= v >> 16;
            v++;
            return v;
        };
        PushConstants pc;
        pc.m_size = specular_levels[i];
        // Calculate sample count: start with size / 2, minimum 16
        pc.m_sample_count = std::max(16u,
            next_power_of_2(specular_levels[i] / 2));
        pc.m_level = i;
        pc.m_total_mipmaps = specular_levels.size();

        vkCmdPushConstants(cmd, pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT,
            0, sizeof(PushConstants), &pc);

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
            pipeline_layout, 0, 1, &descriptor_sets[i + offset], 0, NULL);

        uint32_t dispatch_size = (pc.m_size + 15) / 16;
        vkCmdDispatch(cmd, dispatch_size, dispatch_size, 6);
    }

destroy:
    if (cmd != VK_NULL_HANDLE)
        GEVulkanCommandLoader::endSingleTimeCommands(cmd);
    if (specular_pipeline != VK_NULL_HANDLE)
        vkDestroyPipeline(vk->getDevice(), specular_pipeline, NULL);
    if (diffuse_pipeline != VK_NULL_HANDLE)
        vkDestroyPipeline(vk->getDevice(), diffuse_pipeline, NULL);
    if (pipeline_layout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(vk->getDevice(), pipeline_layout, NULL);
    if (descriptor_pool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(vk->getDevice(), descriptor_pool, NULL);
    if (layout != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(vk->getDevice(), layout, NULL);
    for (VkImageView view : image_views)
        vkDestroyImageView(vk->getDevice(), view, NULL);
}   // load

}
