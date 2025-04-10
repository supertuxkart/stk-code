#include "ge_vulkan_skybox_renderer.hpp"

#include "ge_main.hpp"
#include "ge_vulkan_array_texture.hpp"
#include "ge_vulkan_driver.hpp"

#include <array>
#include <cstdint>
#include <stdexcept>

namespace GE
{
// ----------------------------------------------------------------------------
GEVulkanSkyBoxRenderer::GEVulkanSkyBoxRenderer()
{
    m_skybox = NULL;
    m_texture_cubemap = NULL;

    GEVulkanDriver* vk = getVKDriver();
    // m_descriptor_layout
    VkDescriptorSetLayoutBinding texture_layout_binding;
    texture_layout_binding.binding = 0;
    texture_layout_binding.descriptorCount = 1;
    texture_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    texture_layout_binding.pImmutableSamplers = NULL;
    texture_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo setinfo = {};
    setinfo.flags = 0;
    setinfo.pNext = NULL;
    setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setinfo.pBindings = &texture_layout_binding;
    setinfo.bindingCount = 1;
    if (vkCreateDescriptorSetLayout(vk->getDevice(), &setinfo,
        NULL, &m_descriptor_layout) != VK_SUCCESS)
    {
        throw std::runtime_error("vkCreateDescriptorSetLayout failed for "
            "GEVulkanSkyBoxRenderer");
    }

    // m_descriptor_pool
    VkDescriptorPoolSize pool_size;
    pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_size.descriptorCount = 1;

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = 0;
    pool_info.maxSets = 1;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = &pool_size;
    if (vkCreateDescriptorPool(vk->getDevice(), &pool_info, NULL,
        &m_descriptor_pool) != VK_SUCCESS)
    {
        throw std::runtime_error("vkCreateDescriptorPool failed for "
            "GEVulkanSkyBoxRenderer");
    }

    // m_descriptor_set
    std::vector<VkDescriptorSetLayout> layouts(1, m_descriptor_layout);

    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = m_descriptor_pool;
    alloc_info.descriptorSetCount = layouts.size();
    alloc_info.pSetLayouts = layouts.data();

    if (vkAllocateDescriptorSets(vk->getDevice(), &alloc_info,
        &m_descriptor_set) != VK_SUCCESS)
    {
        throw std::runtime_error("vkAllocateDescriptorSets failed for "
            "GEVulkanSkyBoxRenderer");
    }
}   // init

// ----------------------------------------------------------------------------
GEVulkanSkyBoxRenderer::~GEVulkanSkyBoxRenderer()
{
    GEVulkanDriver* vk = getVKDriver();
    if (!vk)
        return;
    vk->waitIdle();

    if (m_texture_cubemap != NULL)
        m_texture_cubemap->drop();
    vkDestroyDescriptorPool(vk->getDevice(), m_descriptor_pool, NULL);
    vkDestroyDescriptorSetLayout(vk->getDevice(), m_descriptor_layout,
        NULL);
}   // ~GEVulkanSkyBoxRenderer

// ----------------------------------------------------------------------------
void GEVulkanSkyBoxRenderer::addSkyBox(irr::scene::ISceneNode* skybox)
{
    if (skybox->getType() != irr::scene::ESNT_SKY_BOX)
        return;
    if (m_skybox == skybox)
        return;

    m_skybox = skybox;
    std::vector<GEVulkanTexture*> sky_tex;
    std::array<int, 6> order = {{ 1, 3, 4, 5, 2, 0}};

    for (unsigned i = 0; i < 6; i++)
    {
        video::ITexture* tex = skybox->getMaterial(order[i]).getTexture(0);
        if (!tex)
            return;
        sky_tex.push_back(static_cast<GEVulkanTexture*>(tex));
    }

    auto swap_pixels = [](video::IImage* img, unsigned idx)
    {
        if (!(idx == 2 || idx == 3))
            return;
        unsigned width = img->getDimension().Width;
        uint8_t* tmp = new uint8_t[width * width * 4];
        uint32_t* tmp_array = (uint32_t*)tmp;
        uint32_t* img_data = (uint32_t*)img->lock();
        for (unsigned i = 0; i < width; i++)
        {
            for (unsigned j = 0; j < width; j++)
                tmp_array[j * width + i] = img_data[i * width + (width - j - 1)];
        }
        uint8_t* u8_data = (uint8_t*)img->lock();
        delete [] u8_data;
        img->setMemory(tmp);
    };
    if (m_texture_cubemap)
        m_texture_cubemap->drop();
    m_texture_cubemap = new GEVulkanArrayTexture(sky_tex,
        VK_IMAGE_VIEW_TYPE_CUBE, swap_pixels);

    GEVulkanDriver* vk = getVKDriver();
    VkDescriptorImageInfo info;
    info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    info.sampler = vk->getSampler(GVS_SKYBOX);
    info.imageView = (VkImageView)m_texture_cubemap->getTextureHandler();

    VkWriteDescriptorSet write_descriptor_set = {};
    write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_descriptor_set.dstBinding = 0;
    write_descriptor_set.dstArrayElement = 0;
    write_descriptor_set.descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write_descriptor_set.descriptorCount = 1;
    write_descriptor_set.pBufferInfo = 0;
    write_descriptor_set.dstSet = m_descriptor_set;
    write_descriptor_set.pImageInfo = &info;

    vkUpdateDescriptorSets(vk->getDevice(), 1, &write_descriptor_set, 0,
        NULL);
}   // addSkyBox

}
