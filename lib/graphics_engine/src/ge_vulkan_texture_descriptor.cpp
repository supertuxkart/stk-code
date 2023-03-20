#include "ge_vulkan_texture_descriptor.hpp"

#include "ge_main.hpp"
#include "ge_vulkan_driver.hpp"
#include "ge_vulkan_texture.hpp"

#include <algorithm>
#include <exception>
#include <stdexcept>

namespace GE
{
// ----------------------------------------------------------------------------
GEVulkanTextureDescriptor::GEVulkanTextureDescriptor(unsigned max_texture_list,
                                                     unsigned max_layer,
                                                     bool single_descriptor,
                                                     unsigned binding)
                         : m_max_texture_list(max_texture_list),
                           m_max_layer(max_layer), m_binding(binding)
{
    if (m_max_layer > _IRR_MATERIAL_MAX_TEXTURES_)
    {
        throw std::runtime_error(
            "Too large max_layer for GEVulkanTextureDescriptor");
    }

    m_vk = getVKDriver();

    // m_descriptor_set_layout
    std::vector<VkDescriptorSetLayoutBinding> texture_layout_binding;
    texture_layout_binding.resize(1);
    texture_layout_binding[0].binding = m_binding;
    texture_layout_binding[0].descriptorCount =
        single_descriptor ? m_max_texture_list * m_max_layer : 1;
    texture_layout_binding[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    texture_layout_binding[0].pImmutableSamplers = NULL;
    texture_layout_binding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    if (!single_descriptor)
    {
        texture_layout_binding.resize(m_max_layer, texture_layout_binding[0]);
        for (unsigned i = 1; i < m_max_layer; i++)
            texture_layout_binding[i].binding = m_binding + i;
    }

    VkDescriptorSetLayoutCreateInfo setinfo = {};
    setinfo.flags = 0;
    setinfo.pNext = NULL;
    setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setinfo.pBindings = texture_layout_binding.data();
    setinfo.bindingCount = texture_layout_binding.size();
    if (vkCreateDescriptorSetLayout(m_vk->getDevice(), &setinfo,
        NULL, &m_descriptor_set_layout) != VK_SUCCESS)
    {
        throw std::runtime_error("vkCreateDescriptorSetLayout failed for "
            "GEVulkanTextureDescriptor");
    }

    // m_descriptor_pool
    VkDescriptorPoolSize pool_size;
    pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_size.descriptorCount = m_max_texture_list * m_max_layer;

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = 0;
    pool_info.maxSets = single_descriptor ? 1 : m_max_texture_list;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = &pool_size;
    if (vkCreateDescriptorPool(m_vk->getDevice(), &pool_info, NULL,
        &m_descriptor_pool) != VK_SUCCESS)
    {
        throw std::runtime_error("vkCreateDescriptorPool failed for "
            "GEVulkanTextureDescriptor");
    }

    // m_descriptor_sets
    if (single_descriptor)
        m_descriptor_sets.resize(1);
    else
        m_descriptor_sets.resize(m_max_texture_list);
    std::vector<VkDescriptorSetLayout> layouts(m_descriptor_sets.size(),
        m_descriptor_set_layout);

    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = m_descriptor_pool;
    alloc_info.descriptorSetCount = layouts.size();
    alloc_info.pSetLayouts = layouts.data();

    if (vkAllocateDescriptorSets(m_vk->getDevice(), &alloc_info,
        m_descriptor_sets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("vkAllocateDescriptorSets failed for "
            "GEVulkanTextureDescriptor");
    }

    m_sampler_use = GVS_NEAREST;
    m_recreate_next_frame = false;
    m_needs_update_descriptor = false;

    GEVulkanTexture* tex = static_cast<GEVulkanTexture*>(
        m_vk->getWhiteTexture());
    m_white_image = tex->getImageView();
    tex = static_cast<GEVulkanTexture*>(m_vk->getTransparentTexture());
    m_transparent_image = tex->getImageView();
}   // GEVulkanTextureDescriptor

// ----------------------------------------------------------------------------
GEVulkanTextureDescriptor::~GEVulkanTextureDescriptor()
{
    vkDestroyDescriptorSetLayout(m_vk->getDevice(), m_descriptor_set_layout,
        NULL);
    vkDestroyDescriptorPool(m_vk->getDevice(), m_descriptor_pool, NULL);
}   // ~GEVulkanTextureDescriptor

// ----------------------------------------------------------------------------
void GEVulkanTextureDescriptor::updateDescriptor()
{
    if (!m_needs_update_descriptor)
        return;
    m_needs_update_descriptor = false;
    if (m_texture_list.empty())
        return;

    std::vector<VkDescriptorImageInfo> image_infos;
    image_infos.resize(m_texture_list.size() * m_max_layer);
    for (auto& p : m_texture_list)
    {
        const size_t max_size = std::min((size_t)m_max_layer, p.first.size());
        for (unsigned i = 0; i < max_size; i++)
        {
            VkDescriptorImageInfo info;
            info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            info.sampler = m_vk->getSampler(m_sampler_use);
            info.imageView = p.first[i].get()->load();
            if (info.imageView == VK_NULL_HANDLE)
                info.imageView = m_transparent_image.get()->load();
            image_infos[p.second * m_max_layer + i] = info;
        }
    }

    bool single_descriptor = (m_descriptor_sets.size() == 1);
    if (single_descriptor)
    {
        VkDescriptorImageInfo dummy_info;
        dummy_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        dummy_info.imageView = m_transparent_image.get()->load();
        dummy_info.sampler = m_vk->getSampler(m_sampler_use);
        image_infos.resize(m_max_texture_list * m_max_layer, dummy_info);
    }

    m_vk->waitIdle();
    if (single_descriptor)
    {
        VkWriteDescriptorSet write_descriptor_set = {};
        write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptor_set.dstBinding = m_binding;
        write_descriptor_set.dstArrayElement = 0;
        write_descriptor_set.descriptorType =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write_descriptor_set.descriptorCount = m_max_texture_list * m_max_layer;
        write_descriptor_set.pBufferInfo = 0;
        write_descriptor_set.dstSet = m_descriptor_sets[0];
        write_descriptor_set.pImageInfo = image_infos.data();

        vkUpdateDescriptorSets(m_vk->getDevice(), 1, &write_descriptor_set, 0,
            NULL);
    }
    else
    {
        std::vector<VkWriteDescriptorSet> all_sets;
        for (unsigned i = 0; i < image_infos.size(); i += m_max_layer)
        {
            const unsigned set_idx = i / m_max_layer;
            VkWriteDescriptorSet write_descriptor_set = {};
            write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_descriptor_set.dstBinding = m_binding;
            write_descriptor_set.dstArrayElement = 0;
            write_descriptor_set.descriptorType =
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            write_descriptor_set.descriptorCount = m_max_layer;
            write_descriptor_set.pBufferInfo = 0;
            write_descriptor_set.dstSet = m_descriptor_sets[set_idx];
            write_descriptor_set.pImageInfo = &image_infos[i];
            all_sets.push_back(write_descriptor_set);
        }
        vkUpdateDescriptorSets(m_vk->getDevice(), all_sets.size(),
            all_sets.data(), 0, NULL);
    }
}   // updateDescriptor

// ----------------------------------------------------------------------------
int GEVulkanTextureDescriptor::getTextureID(const irr::video::ITexture** list,
                                            const std::string& shader)
{
    TextureList key =
    {{
        m_white_image,
        m_transparent_image,
        m_transparent_image,
        m_transparent_image,
        m_transparent_image,
        m_transparent_image,
        m_transparent_image,
        m_transparent_image
    }};
    for (unsigned i = 0; i < m_max_layer; i++)
    {
        // Assume 0, 1 layer is srgb for pbr enabled currently
        if (list[i])
        {
            key[i] = static_cast<const GEVulkanTexture*>(
                list[i])->getImageView(getGEConfig()->m_pbr &&
                !shader.empty() && i <= 1 ? true : false);
        }
    }
    auto it = m_texture_list.find(key);
    if (it != m_texture_list.end())
        return it->second;
    else
    {
        int cur_id = m_texture_list.size();
        if (cur_id >= m_max_texture_list)
        {
            printf("Too many texture used in current frames\n");
            m_recreate_next_frame = true;
            return m_max_texture_list - 1;
        }

        m_texture_list[key] = cur_id;
        m_needs_update_descriptor = true;
        // Reset the list earlier if almost full
        if (cur_id > int((float)m_max_texture_list * 0.8f))
            m_recreate_next_frame = true;
        return cur_id;
    }
}   // getTextureID

}
