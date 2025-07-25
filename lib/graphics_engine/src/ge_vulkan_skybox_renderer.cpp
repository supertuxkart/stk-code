#include "ge_vulkan_skybox_renderer.hpp"

#include "ge_main.hpp"
#include "ge_vulkan_array_texture.hpp"
#include "ge_vulkan_driver.hpp"
#include "ge_vulkan_fbo_texture.hpp"
#include "ge_vulkan_environment_map.hpp"
#include "ge_vulkan_features.hpp"

#include <array>
#include <cstdint>
#include <stdexcept>

namespace GE
{
// ----------------------------------------------------------------------------
GEVulkanSkyBoxRenderer::GEVulkanSkyBoxRenderer()
                      : m_skybox(NULL), m_texture_cubemap(NULL),
                        m_diffuse_env_cubemap(NULL),
                        m_specular_env_cubemap(NULL),
                        m_dummy_env_cubemap(NULL),
                        m_descriptor_layout(VK_NULL_HANDLE),
                        m_env_descriptor_layout(VK_NULL_HANDLE),
                        m_descriptor_pool(VK_NULL_HANDLE),
                        m_skybox_loading(false), m_env_cubemap_loading(false),
                        m_skytop_color(0)
{
    m_dummy_env_cubemap = new GEVulkanArrayTexture(VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_VIEW_TYPE_CUBE, core::dimension2du(4, 4), 6,
        video::SColor(0));

    GEVulkanDriver* vk = getVKDriver();
    // m_descriptor_layout
    std::vector<VkDescriptorSetLayoutBinding> texture_layout_binding(2);
    texture_layout_binding[0].binding = 0;
    texture_layout_binding[0].descriptorCount = 1;
    texture_layout_binding[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    texture_layout_binding[0].pImmutableSamplers = NULL;
    texture_layout_binding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo setinfo = {};
    setinfo.flags = 0;
    setinfo.pNext = NULL;
    setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setinfo.pBindings = texture_layout_binding.data();
    setinfo.bindingCount = 1;
    if (vkCreateDescriptorSetLayout(vk->getDevice(), &setinfo,
        NULL, &m_descriptor_layout) != VK_SUCCESS)
    {
        throw std::runtime_error("vkCreateDescriptorSetLayout failed for "
            "GEVulkanSkyBoxRenderer::m_descriptor_layout");
    }

    texture_layout_binding[1] = texture_layout_binding[0];
    texture_layout_binding[1].binding = 1;
    setinfo.bindingCount = 2;
    if (vkCreateDescriptorSetLayout(vk->getDevice(), &setinfo,
        NULL, &m_env_descriptor_layout) != VK_SUCCESS)
    {
        throw std::runtime_error("vkCreateDescriptorSetLayout failed for "
            "GEVulkanSkyBoxRenderer::m_env_descriptor_layout");
    }

    // m_descriptor_pool
    VkDescriptorPoolSize pool_size;
    pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_size.descriptorCount = 1 + m_env_descriptor_set.size() * 2;

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = 0;
    pool_info.maxSets = 1 + m_env_descriptor_set.size();
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
            "GEVulkanSkyBoxRenderer::m_descriptor_set");
    }

    layouts.clear();
    layouts.resize(2, m_env_descriptor_layout);
    alloc_info.descriptorSetCount = layouts.size();
    alloc_info.pSetLayouts = layouts.data();
    if (vkAllocateDescriptorSets(vk->getDevice(), &alloc_info,
        m_env_descriptor_set.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("vkAllocateDescriptorSets failed for "
            "GEVulkanSkyBoxRenderer::m_env_descriptor_set");
    }

    std::array<VkDescriptorImageInfo, 2> info;
    info[0].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    info[0].sampler = vk->getSampler(GVS_SKYBOX);
    info[0].imageView = (VkImageView)m_dummy_env_cubemap->getTextureHandler();
    info[1] = info[0];

    VkWriteDescriptorSet write_descriptor_set = {};
    write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_descriptor_set.dstBinding = 0;
    write_descriptor_set.dstArrayElement = 0;
    write_descriptor_set.descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write_descriptor_set.descriptorCount = 2;
    write_descriptor_set.pBufferInfo = 0;
    write_descriptor_set.dstSet = m_env_descriptor_set[0];
    write_descriptor_set.pImageInfo = info.data();
    vkUpdateDescriptorSets(vk->getDevice(), 1, &write_descriptor_set, 0, NULL);
}   // init

// ----------------------------------------------------------------------------
GEVulkanSkyBoxRenderer::~GEVulkanSkyBoxRenderer()
{
    GEVulkanDriver* vk = getVKDriver();
    if (!vk)
        return;
    vk->waitIdle();

    while (m_skybox_loading.load());
    while (m_env_cubemap_loading.load());
    if (m_texture_cubemap != NULL)
        m_texture_cubemap->drop();
    if (m_diffuse_env_cubemap != NULL)
        m_diffuse_env_cubemap->drop();
    if (m_specular_env_cubemap != NULL)
        m_specular_env_cubemap->drop();
    if (m_dummy_env_cubemap != NULL)
        m_dummy_env_cubemap->drop();
    if (m_descriptor_pool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(vk->getDevice(), m_descriptor_pool, NULL);
    if (m_env_descriptor_layout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(vk->getDevice(), m_env_descriptor_layout,
            NULL);
    }
    if (m_descriptor_layout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(vk->getDevice(), m_descriptor_layout,
            NULL);
    }
}   // ~GEVulkanSkyBoxRenderer

// ----------------------------------------------------------------------------
void GEVulkanSkyBoxRenderer::addSkyBox(irr::scene::ISceneNode* skybox)
{
    if (skybox->getType() != irr::scene::ESNT_SKY_BOX)
        return;
    if (m_skybox == skybox)
        return;

    while (m_skybox_loading.load());
    while (m_env_cubemap_loading.load());
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

    class ImageManipulator
    {
        private:
            GEVulkanSkyBoxRenderer* m_sky;

            const bool m_srgb;
            // ----------------------------------------------------------------
            void updateDescriptor()
            {
                GEVulkanDriver* vk = getVKDriver();
                VkDescriptorImageInfo info;
                info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                info.sampler = vk->getSampler(GVS_SKYBOX);
                info.imageView = (VkImageView)m_sky->m_texture_cubemap
                    ->getImageView(m_srgb)->load();

                VkWriteDescriptorSet write_descriptor_set = {};
                write_descriptor_set.sType =
                    VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write_descriptor_set.dstBinding = 0;
                write_descriptor_set.dstArrayElement = 0;
                write_descriptor_set.descriptorType =
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                write_descriptor_set.descriptorCount = 1;
                write_descriptor_set.pBufferInfo = 0;
                write_descriptor_set.dstSet = m_sky->m_descriptor_set;
                write_descriptor_set.pImageInfo = &info;

                vkUpdateDescriptorSets(vk->getDevice(), 1,
                    &write_descriptor_set, 0, NULL);
                m_sky->m_skybox_loading.store(false);
            }
        public:
            // ----------------------------------------------------------------
            ImageManipulator(GEVulkanSkyBoxRenderer* sky, bool srgb)
                : m_sky(sky), m_srgb(srgb)
            {
                m_sky->m_skybox_loading.store(true);
            }
            // ----------------------------------------------------------------
            ~ImageManipulator()
            {
                if (m_sky->m_diffuse_env_cubemap != NULL)
                {
                    GEVulkanEnvironmentMap env(m_sky);
                    updateDescriptor();
                    env.load();
                }
                else
                    updateDescriptor();
            }
            // ----------------------------------------------------------------
            void swapPixels(video::IImage* img, unsigned idx)
            {
                if (!(idx == 2 || idx == 3))
                    return;
                if (idx == 2)
                {
                    video::IImage* pixel = getDriver()->createImage(
                        video::ECF_A8R8G8B8, core::dimension2du(1, 1));
                    img->copyToScaling(pixel);
                    m_sky->m_skytop_color.store(*(uint32_t*)pixel->lock());
                    pixel->drop();
                }
                unsigned width = img->getDimension().Width;
                uint8_t* tmp = new uint8_t[width * width * 4];
                uint32_t* tmp_array = (uint32_t*)tmp;
                uint32_t* img_data = (uint32_t*)img->lock();
                for (unsigned i = 0; i < width; i++)
                {
                    for (unsigned j = 0; j < width; j++)
                    {
                        tmp_array[j * width + i] =
                            img_data[i * width + (width - j - 1)];
                    }
                }
                uint8_t* u8_data = (uint8_t*)img->lock();
                delete [] u8_data;
                img->setMemory(tmp);
            }
    };

    GEVulkanDriver* vk = getVKDriver();
    bool srgb = vk->getRTTTexture() && vk->getRTTTexture()->isDeferredFBO();
    std::shared_ptr<ImageManipulator> image_manipulator =
        std::make_shared<ImageManipulator>(this, srgb);
    auto real_mani = [image_manipulator](video::IImage* img, unsigned idx)
    {
        image_manipulator->swapPixels(img, idx);
    };

    if (getGEConfig()->m_pbr && getGEConfig()->m_ibl &&
        GEVulkanFeatures::supportsComputeInMainQueue())
    {
        VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
        if (GEVulkanFeatures::supportsShaderStorageImageExtendedFormats())
            format = VK_FORMAT_A2B10G10R10_UNORM_PACK32;
        bool update_descriptor = false;
        if (m_diffuse_env_cubemap == NULL)
        {
            m_diffuse_env_cubemap =
                new GEVulkanArrayTexture(format, VK_IMAGE_VIEW_TYPE_CUBE,
                GEVulkanEnvironmentMap::getDiffuseEnvironmentMapSize(), 6,
                video::SColor(0));
            update_descriptor = true;
        }
        if (m_specular_env_cubemap == NULL)
        {
            m_specular_env_cubemap =
                new GEVulkanArrayTexture(format, VK_IMAGE_VIEW_TYPE_CUBE,
                GEVulkanEnvironmentMap::getSpecularEnvironmentMapSize(), 6,
                video::SColor(0));
            update_descriptor = true;
        }
        if (update_descriptor)
        {
            GEVulkanDriver* vk = getVKDriver();
            std::array<VkDescriptorImageInfo, 2> info;
            info[0].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            info[0].sampler = vk->getSampler(GVS_SKYBOX);
            info[0].imageView = (VkImageView)
                m_diffuse_env_cubemap->getTextureHandler();
            info[1] = info[0];
            info[1].imageView = (VkImageView)
                m_specular_env_cubemap->getTextureHandler();

            VkWriteDescriptorSet write_descriptor_set = {};
            write_descriptor_set.sType =
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_descriptor_set.dstBinding = 0;
            write_descriptor_set.dstArrayElement = 0;
            write_descriptor_set.descriptorType =
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            write_descriptor_set.descriptorCount = 2;
            write_descriptor_set.pBufferInfo = 0;
            write_descriptor_set.dstSet = m_env_descriptor_set[1];
            write_descriptor_set.pImageInfo = info.data();
            vkUpdateDescriptorSets(vk->getDevice(), 1, &write_descriptor_set,
                0, NULL);
        }
    }
    else
    {
        if (m_diffuse_env_cubemap != NULL)
        {
            m_diffuse_env_cubemap->drop();
            m_diffuse_env_cubemap = NULL;
        }
        if (m_specular_env_cubemap != NULL)
        {
            m_specular_env_cubemap->drop();
            m_specular_env_cubemap = NULL;
        }
    }

    if (m_texture_cubemap)
        m_texture_cubemap->drop();
    m_texture_cubemap = new GEVulkanArrayTexture(sky_tex,
        VK_IMAGE_VIEW_TYPE_CUBE, real_mani);
}   // addSkyBox

// ----------------------------------------------------------------------------
const VkDescriptorSet* GEVulkanSkyBoxRenderer::getEnvDescriptorSet() const
{
    if (m_diffuse_env_cubemap == NULL || m_skybox_loading.load() == true ||
        m_env_cubemap_loading.load() == true)
        return &m_env_descriptor_set[0];
    return &m_env_descriptor_set[1];
}   // getEnvDescriptorSet

}
