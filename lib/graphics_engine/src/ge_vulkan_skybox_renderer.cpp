#include "ge_vulkan_skybox_renderer.hpp"

#include "ge_main.hpp"
#include "ge_vma.hpp"
#include "ge_vulkan_array_texture.hpp"
#include "ge_vulkan_camera_scene_node.hpp"
#include "ge_vulkan_driver.hpp"
#include "ge_vulkan_features.hpp"
#include "ge_vulkan_fbo_texture.hpp"
#include "ge_vulkan_shader_manager.hpp"

#include <array>
#include <cstdint>
#include <stdexcept>
#include <unordered_map>
#include <stdexcept>

namespace GE
{
namespace GEVulkanSkyBoxRenderer
{
// ============================================================================
irr::scene::ISceneNode* g_skybox = NULL;
std::unordered_map<GEVulkanCameraSceneNode*, bool> g_render_skybox;

GEVulkanArrayTexture* g_texture_cubemap = NULL;
GEVulkanArrayTexture* g_diffuse_env_cubemap = NULL;
GEVulkanArrayTexture* g_specular_env_cubemap = NULL;
bool g_updated_texture_descriptor = false;

VkDescriptorSetLayout g_descriptor_layout = VK_NULL_HANDLE;
VkDescriptorPool g_descriptor_pool = VK_NULL_HANDLE;
VkDescriptorSet g_descriptor_set = VK_NULL_HANDLE;
VkPipelineLayout g_pipeline_layout = VK_NULL_HANDLE;
VkPipeline g_graphics_pipeline = VK_NULL_HANDLE;

VkDescriptorSetLayout g_descriptor_layout_env_map = VK_NULL_HANDLE;
VkDescriptorPool g_descriptor_pool_env_map = VK_NULL_HANDLE;
VkDescriptorSet g_descriptor_set_env_map = VK_NULL_HANDLE;

void updateDescriptorSet();
}   // GEVulkanSkyBoxRenderer

// ============================================================================
void GEVulkanSkyBoxRenderer::init()
{
    g_skybox = NULL;
    g_render_skybox.clear();
    g_descriptor_layout = VK_NULL_HANDLE;
    g_descriptor_pool = VK_NULL_HANDLE;
    g_descriptor_set = VK_NULL_HANDLE;
    g_descriptor_layout_env_map = VK_NULL_HANDLE;
    g_descriptor_pool_env_map = VK_NULL_HANDLE;
    g_descriptor_set_env_map = VK_NULL_HANDLE;
    g_pipeline_layout = VK_NULL_HANDLE;
    g_graphics_pipeline = VK_NULL_HANDLE;
    g_updated_texture_descriptor = true;
}   // init

// ----------------------------------------------------------------------------
void GEVulkanSkyBoxRenderer::destroy()
{
    GEVulkanDriver* vk = getVKDriver();
    if (!vk)
        return;
    vk->waitIdle();

    if (g_texture_cubemap != NULL)
    {
        g_texture_cubemap->drop();
        g_texture_cubemap = NULL;
    }

    if (g_diffuse_env_cubemap != NULL)
    {
        g_diffuse_env_cubemap->drop();
        g_diffuse_env_cubemap = NULL;
    }

    if (g_specular_env_cubemap != NULL)
    {
        g_specular_env_cubemap->drop();
        g_specular_env_cubemap = NULL;
    }

    if (g_graphics_pipeline != VK_NULL_HANDLE)
        vkDestroyPipeline(vk->getDevice(), g_graphics_pipeline, NULL);
    g_graphics_pipeline = VK_NULL_HANDLE;

    if (g_pipeline_layout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(vk->getDevice(), g_pipeline_layout, NULL);
    g_pipeline_layout = VK_NULL_HANDLE;

    if (g_descriptor_pool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(vk->getDevice(), g_descriptor_pool, NULL);
    g_descriptor_pool = VK_NULL_HANDLE;

    if (g_descriptor_layout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(vk->getDevice(), g_descriptor_layout,
            NULL);
    }
    g_descriptor_layout = VK_NULL_HANDLE;

    g_descriptor_set = VK_NULL_HANDLE;

    {
        if (g_descriptor_pool_env_map != VK_NULL_HANDLE)
            vkDestroyDescriptorPool(vk->getDevice(), g_descriptor_pool_env_map, NULL);
        g_descriptor_pool_env_map = VK_NULL_HANDLE;

        if (g_descriptor_layout_env_map != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorSetLayout(vk->getDevice(), g_descriptor_layout_env_map,
                NULL);
        }
        g_descriptor_layout_env_map = VK_NULL_HANDLE;

        g_descriptor_set_env_map = VK_NULL_HANDLE;
    }
    g_skybox = NULL;
    g_render_skybox.clear();
    g_updated_texture_descriptor = true;
}   // destroy

// ----------------------------------------------------------------------------
void GEVulkanSkyBoxRenderer::updateDescriptorSet()
{
    if (g_updated_texture_descriptor)
    {   
        return;
    }
    g_updated_texture_descriptor = true;

    GEVulkanDriver* vk = getVKDriver();
    VkDescriptorImageInfo info;
    info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    info.sampler = vk->getSampler(GVS_SKYBOX);
    info.imageView = (VkImageView)g_texture_cubemap->getTextureHandler();

    VkWriteDescriptorSet write_descriptor_set = {};
    write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_descriptor_set.dstBinding = 0;
    write_descriptor_set.dstArrayElement = 0;
    write_descriptor_set.descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write_descriptor_set.descriptorCount = 1;
    write_descriptor_set.pBufferInfo = 0;
    write_descriptor_set.dstSet = g_descriptor_set;
    write_descriptor_set.pImageInfo = &info;

    vkUpdateDescriptorSets(vk->getDevice(), 1, &write_descriptor_set, 0,
        NULL);

    std::vector<VkWriteDescriptorSet> data_set;

    VkDescriptorImageInfo info_diffuse;
    info_diffuse.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    info_diffuse.sampler = vk->getSampler(GVS_SKYBOX);
    info_diffuse.imageView = *g_diffuse_env_cubemap->getImageView(true);

    write_descriptor_set.dstBinding = 0;
    write_descriptor_set.dstSet = g_descriptor_set_env_map;
    write_descriptor_set.pImageInfo = &info_diffuse;
    data_set.push_back(write_descriptor_set);

    VkDescriptorImageInfo info_specular;
    info_specular.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    info_specular.sampler = vk->getSampler(GVS_SKYBOX);
    info_specular.imageView = *g_specular_env_cubemap->getImageView(true);

    write_descriptor_set.dstBinding = 1;
    write_descriptor_set.dstSet = g_descriptor_set_env_map;
    write_descriptor_set.pImageInfo = &info_specular;
    data_set.push_back(write_descriptor_set);

    vkUpdateDescriptorSets(vk->getDevice(), data_set.size(), data_set.data(), 0,
        NULL);
}

// ----------------------------------------------------------------------------
void GEVulkanSkyBoxRenderer::render(VkCommandBuffer cmd,
                                    GEVulkanCameraSceneNode* cam)
{
    if (g_render_skybox.find(cam) == g_render_skybox.end() ||
        !g_render_skybox.at(cam))
        return;
    g_render_skybox.at(cam) = false;

    updateDescriptorSet();

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g_graphics_pipeline);

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
        g_pipeline_layout, 0, 1, &g_descriptor_set, 0, NULL);

    vkCmdPushConstants(cmd, g_pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0,
        sizeof(irr::core::matrix4),
        cam->getUBOData()->m_inverse_projection_view_matrix.pointer());

    vkCmdDraw(cmd, 3, 1, 0, 0);
}   // render

// ----------------------------------------------------------------------------
void GEVulkanSkyBoxRenderer::addSkyBox(GEVulkanCameraSceneNode* cam,
                                       irr::scene::ISceneNode* skybox)
{
    if (skybox->getType() != irr::scene::ESNT_SKY_BOX)
        return;
    if (g_skybox == skybox)
    {
        g_render_skybox[cam] = true;
        return;
    }

    destroy();
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

    using irr::core::PI;
    using irr::core::vector2df;
    using irr::core::vector2di;
    using irr::core::vector3df;
    using irr::video::SColorf;
    using irr::core::clamp;

    auto hammersly2d = [](int i, int n)->vector2df
    {
        uint32_t bits = i;
        bits = (bits << 16u) | (bits >> 16u);
        bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
        bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
        bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
        bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
        return vector2df(float(i) / float(n), 
                         float(bits) * 2.3283064365386963e-10);
    };
    auto texture_lod = [](const std::vector<std::vector<GEImageLevel> > &cubemap,
                         vector3df dir, float lod) -> SColorf
    {
        auto sample_lod = [](const std::vector<GEImageLevel>& mipmap,
                             vector2df uv, float lod) -> SColorf
        {
            auto sample = [](const GEImageLevel& level, vector2df uv) -> SColorf
            {
                uv = vector2df(
                    clamp<float>(uv.X * (level.m_dim.Width - 2.0f) + 1.0f, 1.0f, -1.0f + level.m_dim.Width),
                    clamp<float>(uv.Y * (level.m_dim.Height - 2.0f) + 1.0f, 1.0f, -1.0f + level.m_dim.Height));

                vector2di point = vector2di(uv.X - 0.5f, uv.Y - 0.5f);
                vector2df lerp = vector2df(uv.X - (0.5f + point.X), uv.Y - (0.5f + point.Y));
                
                SColorf texel0 = 
                    (((SColorf*)level.m_data)[point.Y * level.m_dim.Width + point.X]);
                SColorf texel1 = 
                    (((SColorf*)level.m_data)[(point.Y + 1) * level.m_dim.Width + point.X]);
                texel0 = texel1.getInterpolated(texel0, lerp.Y);
                SColorf texel2 = 
                    (((SColorf*)level.m_data)[point.Y * level.m_dim.Width + (point.X + 1)]);
                SColorf texel3 = 
                    (((SColorf*)level.m_data)[(point.Y + 1) * level.m_dim.Width + (point.X + 1)]);
                texel2 = texel3.getInterpolated(texel2, lerp.Y);
                texel0 = texel2.getInterpolated(texel0, lerp.X);
                return texel0;
            };
            lod = std::min(lod, float(mipmap.size() - 1));
            float level;
            float lerp = modff(lod, &level);
            SColorf lod0 = sample(mipmap[level], uv);
            if (lerp == 0.0) return lod0;
            SColorf lod1 = sample(mipmap[level + 1], uv);
            return lod0.getInterpolated(lod1, lerp);
        };
        float ax = abs(dir.X), ay = abs(dir.Y), az = abs(dir.Z);
        if (ax > ay && ax > az)
        {
            return dir.X > 0.0 ? sample_lod(cubemap[0], vector2df(-dir.Z / ax * 0.5 + 0.5, -dir.Y / ax * 0.5 + 0.5), lod)
                               : sample_lod(cubemap[1], vector2df(dir.Z / ax * 0.5 + 0.5, -dir.Y / ax * 0.5 + 0.5), lod);
        }
        else if (ay > ax && ay > az)
        {
            return dir.Y > 0.0 ? sample_lod(cubemap[2], vector2df(dir.X / ay * 0.5 + 0.5, dir.Z / ay * 0.5 + 0.5), lod)
                               : sample_lod(cubemap[3], vector2df(-dir.X / ay * 0.5 + 0.5, dir.Z / ay * 0.5 + 0.5), lod);
        }
        else
        {
            return dir.Z > 0.0 ? sample_lod(cubemap[4], vector2df(dir.X / az * 0.5 + 0.5, -dir.Y / az * 0.5 + 0.5), lod)
                               : sample_lod(cubemap[5], vector2df(-dir.X / az * 0.5 + 0.5, -dir.Y / az * 0.5 + 0.5), lod);
        }
    };
    auto mipmap_preprocess = [](const std::vector<std::vector<GEImageLevel> > &src) -> std::vector<std::vector<GEImageLevel> > 
    {
        std::vector<std::vector<GEImageLevel> > src2;
        src2.resize(6);

        
        return src2;
    };
    auto diffuse_presample = [hammersly2d, texture_lod, mipmap_preprocess]
                            (const std::vector<GEImageLevel> &dst, 
                             const std::vector<std::vector<GEImageLevel> > &src,
                             int tex_index)
    {
        float lodbias = 0.;
        for (int i = 0; i < src[tex_index].size(); i++)
        {
            if (src[tex_index][i].m_dim.Width > dst[0].m_dim.Width) lodbias += 1.0;
            else break;
        }
        std::vector<std::vector<GEImageLevel> > src2;
        std::vector<std::vector<std::vector<SColorf> > > buff; 
        auto fill_buf = [](GEImageLevel dst, int x1, int y1, GEImageLevel src, int x2, int y2)
        {
            int ind1 = dst.m_dim.Width * y1 + x1;
            int ind2 = src.m_dim.Width * y2 + x2;
            SColorf *conv = (SColorf*) dst.m_data;
            uint32_t color = ((uint32_t*) src.m_data)[ind2];
            conv[ind1] = SColorf(color);
            conv[ind1].b = powf(conv[ind1].b, 2.2f);
            conv[ind1].g = powf(conv[ind1].g, 2.2f);
            conv[ind1].r = powf(conv[ind1].r, 2.2f);
            conv[ind1].a = powf(conv[ind1].a, 2.2f);
        };
        auto fill_corner = [](GEImageLevel dst, int xt, int yt, int x1, int y1, int x2, int y2, int x3, int y3)
        {
            int ind1 = dst.m_dim.Width * y1 + x1;
            int ind2 = dst.m_dim.Width * y2 + x2;
            int ind3 = dst.m_dim.Width * y3 + x3;
            int indt = dst.m_dim.Width * yt + xt;
            SColorf *conv = (SColorf*) dst.m_data;
            conv[indt] = conv[ind1].getInterpolated(conv[ind2].getInterpolated(conv[ind2], 0.5f), 1.f / 3.f);
        };
        
        src2.resize(6);
        buff.resize(6);
        
        for (int i = 0; i < 6; i++)
        {
            src2[i].resize(src[i].size());
            buff[i].resize(src[i].size());
            for (int j = 0; j < src[i].size(); j++)
            {
                src2[i][j].m_dim.Width = src[i][j].m_dim.Width + 2;
                src2[i][j].m_dim.Height = src[i][j].m_dim.Height + 2;
                src2[i][j].m_size = src2[i][j].m_dim.Width * src2[i][j].m_dim.Height;

                buff[i][j].resize(src2[i][j].m_size);
                src2[i][j].m_data = buff[i][j].data();

                for (int u = 0; u < src[i][j].m_dim.Width; u++)
                for (int v = 0; v < src[i][j].m_dim.Height; v++)
                {
                    fill_buf(src2[i][j], u + 1, v + 1, src[i][j], u, v);
                }
            }
        }
        const int horizon[4] = { 0, 5, 1, 4 };
        for (int h = 0; h < 4; h++)
        {
            int i = horizon[h];
            int r = horizon[(h + 1) % 4];
            for (int j = 0; j < src2[i].size(); j++)
            {
                for (int k = 0; k < src[i][j].m_dim.Height; k++)
                {
                    fill_buf(src2[r][j], 0, k + 1, src[i][j], src[i][j].m_dim.Width - 1, k);
                    fill_buf(src2[i][j], src2[i][j].m_dim.Width - 1, k + 1, src[r][j], 0, k);
                }
                for (int k = 0; k < src[i][j].m_dim.Width; k++)
                {
                    int x1[4] = { (int) src[2][j].m_dim.Width - 1, (int) src[2][j].m_dim.Width - k - 1, 0, k };
                    int y1[4] = { (int) src[2][j].m_dim.Height - k - 1, 0, k, (int) src[2][j].m_dim.Height - 1 };
                    int x2[4] = { (int) src2[2][j].m_dim.Width - 1, (int) src2[2][j].m_dim.Width - k - 2, 0, k + 1 };
                    int y2[4] = { (int) src2[2][j].m_dim.Height - k - 2, 0,  k + 1, (int) src2[2][j].m_dim.Height - 1 };
                    
                    int l = (h + 2) % 4;
                    fill_buf(src2[i][j], k + 1, src[i][j].m_dim.Height - 1, src[3][j], x1[l], y1[h]);
                    fill_buf(src2[i][j], k + 1, 0,                          src[2][j], x1[h], y1[h]);
                    fill_buf(src2[3][j], x2[l], y2[h], src[i][j], k, src[i][j].m_dim.Height - 1);
                    fill_buf(src2[2][j], x2[h], y2[h], src[i][j], k, 0);
                }
            }
        }
        for (int i = 0; i < 6; i++)
        {
            for (int j = 0; j < src2[i].size(); j++)
            {
                int w = src2[i][j].m_dim.Width;
                int h = src2[i][j].m_dim.Height;
                fill_corner(src2[i][j], 0, 0, 0, 1, 1, 0, 1, 1);
                fill_corner(src2[i][j], 0, h - 1, 0, h - 2, 1, h - 1, 1, h - 2);
                fill_corner(src2[i][j], w - 1, 0, w - 1, 1, w - 2, 0, w - 2, 1);
                fill_corner(src2[i][j], w - 1, h - 1, w - 1, h - 2, w - 2, h - 1, w - 2, h - 2);
            }
        }
        const int sample_count = 1024;

        for (int level = 0; level < dst.size(); level++)
        {
            std::array<vector3df, sample_count> local_space_dirs;
            std::array<float, sample_count> lods;
            for (int i = 0; i < sample_count; i++)
            {
                // Lambertian
                vector2df xi = hammersly2d(i, sample_count);
                float cos_theta = sqrtf(1.0 - xi.Y);
                float sin_theta = sqrtf(xi.Y);
                float phi = 2.0 * PI * xi.X;
                float pdf = cos_theta / PI;
                local_space_dirs[i] = vector3df(
                    sin_theta * cosf(phi),
                    sin_theta * sinf(phi),
                    cos_theta).normalize();
                lods[i] = 0.5f * log2(6.0 * dst[level].m_dim.Width
                                          * dst[level].m_dim.Height
                                          / (sample_count * pdf)) + lodbias;
            }
            for (int i = 0; i < dst[level].m_dim.Width * dst[level].m_dim.Height; i++)
            {
                vector2df uv = vector2df(
                    (0.5f + i % dst[level].m_dim.Width) / dst[level].m_dim.Width  * 2.0f - 1.0f,
                    (0.5f + i / dst[level].m_dim.Width) / dst[level].m_dim.Height * 2.0f - 1.0f);
                vector3df scan = vector3df();
                switch(tex_index)
                {
                    case 0: scan = vector3df( 1.f,  uv.Y, -uv.X); break;
                    case 1: scan = vector3df(-1.f,  uv.Y,  uv.X); break;
                    case 2: scan = vector3df( uv.X, -1.f,  uv.Y); break;
                    case 3: scan = vector3df( uv.X,  1.f, -uv.Y); break;
                    case 4: scan = vector3df( uv.X, uv.Y,  1.f ); break;
                    case 5: scan = vector3df(-uv.X, uv.Y, -1.f ); break;
                }
                vector3df dir = scan.normalize(); // Normal
                dir.Y = -dir.Y;
                vector3df bitangent = vector3df(0.0, 1.0, 0.0);
                float ndotup = dir.dotProduct(vector3df(0.0, 1.0, 0.0));
                if (1.0 - abs(ndotup) < 0.00001)
                {
                    bitangent = ndotup > 0.0 ? vector3df(0.0, 0.0, 1.0)
                                            : vector3df(0.0, 0.0, -1.0);
                }
                vector3df tangent = bitangent.crossProduct(dir).normalize();
                bitangent         = dir.crossProduct(tangent);

                SColorf color = SColorf();
                float weight = 0.;
            
                for (int i = 0; i < sample_count; i++)
                {
                    // TBN * local_space_dir
                    vector3df ldir = local_space_dirs[i];
                    float lod = lods[i];

                    vector3df world_dir = vector3df(
                        tangent.X * ldir.X + bitangent.X * ldir.Y + dir.X * ldir.Z,
                        tangent.Y * ldir.X + bitangent.Y * ldir.Y + dir.Y * ldir.Z,
                        tangent.Z * ldir.X + bitangent.Z * ldir.Y + dir.Z * ldir.Z
                    );

                    SColorf samp = texture_lod(src2, world_dir, lod);
                    color.a += samp.a * irr::core::clamp(world_dir.Y * 2.f + 0.2f, 0.f, 1.f);
                    color.r += samp.r * irr::core::clamp(world_dir.Y * 2.f + 0.2f, 0.f, 1.f);
                    color.g += samp.g * irr::core::clamp(world_dir.Y * 2.f + 0.2f, 0.f, 1.f);
                    color.b += samp.b * irr::core::clamp(world_dir.Y * 2.f + 0.2f, 0.f, 1.f);
                    weight += 1.0f;
                }
                color.a /= weight;
                color.r /= weight;
                color.g /= weight;
                color.b /= weight;
                ((uint32_t*)dst[level].m_data)[i]
                    = ((uint32_t)(powf(color.b, 0.455f) * 255.))
                    + ((uint32_t)(powf(color.g, 0.455f) * 255.) << 8)
                    + ((uint32_t)(powf(color.r, 0.455f) * 255.) << 16)
                    + ((uint32_t)(powf(color.a, 0.455f) * 255.) << 24);
            }
        }
    };
    auto specular_presample = [hammersly2d, texture_lod, mipmap_preprocess]
                              (const std::vector<GEImageLevel> &dst, 
                               const std::vector<std::vector<GEImageLevel> > &src,
                               int tex_index)
    {
        float lodbias = 0.;
        for (int i = 0; i < src[tex_index].size(); i++)
        {
            if (src[tex_index][i].m_dim.Width > dst[0].m_dim.Width) lodbias += 1.0;
            else break;
        }

        std::vector<std::vector<GEImageLevel> > src2;
        std::vector<std::vector<std::vector<SColorf> > > buff; 
        auto fill_buf = [](GEImageLevel dst, int x1, int y1, GEImageLevel src, int x2, int y2)
        {
            int ind1 = dst.m_dim.Width * y1 + x1;
            int ind2 = src.m_dim.Width * y2 + x2;
            SColorf *conv = (SColorf*) dst.m_data;
            uint32_t color = ((uint32_t*) src.m_data)[ind2];
            conv[ind1] = SColorf(color);
            conv[ind1].b = powf(conv[ind1].b, 2.2f);
            conv[ind1].g = powf(conv[ind1].g, 2.2f);
            conv[ind1].r = powf(conv[ind1].r, 2.2f);
            conv[ind1].a = powf(conv[ind1].a, 2.2f);
        };
        auto fill_corner = [](GEImageLevel dst, int xt, int yt, int x1, int y1, int x2, int y2, int x3, int y3)
        {
            int ind1 = dst.m_dim.Width * y1 + x1;
            int ind2 = dst.m_dim.Width * y2 + x2;
            int ind3 = dst.m_dim.Width * y3 + x3;
            int indt = dst.m_dim.Width * yt + xt;
            SColorf *conv = (SColorf*) dst.m_data;
            conv[indt] = conv[ind1].getInterpolated(conv[ind2].getInterpolated(conv[ind2], 0.5f), 1.f / 3.f);
        };
        
        src2.resize(6);
        buff.resize(6);
        
        for (int i = 0; i < 6; i++)
        {
            src2[i].resize(src[i].size());
            buff[i].resize(src[i].size());
            for (int j = 0; j < src[i].size(); j++)
            {
                src2[i][j].m_dim.Width = src[i][j].m_dim.Width + 2;
                src2[i][j].m_dim.Height = src[i][j].m_dim.Height + 2;
                src2[i][j].m_size = src2[i][j].m_dim.Width * src2[i][j].m_dim.Height;

                buff[i][j].resize(src2[i][j].m_size);
                src2[i][j].m_data = buff[i][j].data();

                for (int u = 0; u < src[i][j].m_dim.Width; u++)
                for (int v = 0; v < src[i][j].m_dim.Height; v++)
                {
                    fill_buf(src2[i][j], u + 1, v + 1, src[i][j], u, v);
                }
            }
        }
        const int horizon[4] = { 0, 5, 1, 4 };
        for (int h = 0; h < 4; h++)
        {
            int i = horizon[h];
            int r = horizon[(h + 1) % 4];
            for (int j = 0; j < src2[i].size(); j++)
            {
                for (int k = 0; k < src[i][j].m_dim.Height; k++)
                {
                    fill_buf(src2[r][j], 0, k + 1, src[i][j], src[i][j].m_dim.Width - 1, k);
                    fill_buf(src2[i][j], src2[i][j].m_dim.Width - 1, k + 1, src[r][j], 0, k);
                }
                for (int k = 0; k < src[i][j].m_dim.Width; k++)
                {
                    int x1[4] = { (int) src[2][j].m_dim.Width - 1, (int) src[2][j].m_dim.Width - k - 1, 0, k };
                    int y1[4] = { (int) src[2][j].m_dim.Height - k - 1, 0, k, (int) src[2][j].m_dim.Height - 1 };
                    int x2[4] = { (int) src2[2][j].m_dim.Width - 1, (int) src2[2][j].m_dim.Width - k - 2, 0, k + 1 };
                    int y2[4] = { (int) src2[2][j].m_dim.Height - k - 2, 0,  k + 1, (int) src2[2][j].m_dim.Height - 1 };
                    
                    int l = (h + 2) % 4;
                    fill_buf(src2[i][j], k + 1, src[i][j].m_dim.Height - 1, src[3][j], x1[l], y1[h]);
                    fill_buf(src2[i][j], k + 1, 0,                          src[2][j], x1[h], y1[h]);
                    fill_buf(src2[3][j], x2[l], y2[h], src[i][j], k, src[i][j].m_dim.Height - 1);
                    fill_buf(src2[2][j], x2[h], y2[h], src[i][j], k, 0);
                }
            }
        }
        for (int i = 0; i < 6; i++)
        {
            for (int j = 0; j < src2[i].size(); j++)
            {
                int w = src2[i][j].m_dim.Width;
                int h = src2[i][j].m_dim.Height;
                fill_corner(src2[i][j], 0, 0, 0, 1, 1, 0, 1, 1);
                fill_corner(src2[i][j], 0, h - 1, 0, h - 2, 1, h - 1, 1, h - 2);
                fill_corner(src2[i][j], w - 1, 0, w - 1, 1, w - 2, 0, w - 2, 1);
                fill_corner(src2[i][j], w - 1, h - 1, w - 1, h - 2, w - 2, h - 1, w - 2, h - 2);
            }
        }

        const int sample_count = 32;

        for (int level = 0; level < dst.size(); level++)
        {
            std::array<vector3df, sample_count> local_space_dirs;
            std::array<float, sample_count> lods;

            for (int i = 0; i < sample_count; i++)
            {
                // GGX
                vector2df xi = hammersly2d(i, sample_count);
                float roughness = 1.0f * level / (dst.size() - 1);
                float alpha = roughness * roughness;
                float cos_theta = clamp<float>(sqrtf((1.0 - xi.Y) / (1.0 + (alpha * alpha - 1.0) * xi.Y)), 0.0, 1.0);
                float sin_theta = sqrtf(1.0 - cos_theta * cos_theta);
                float phi = 2.0 * PI * xi.X;
                float ggx_a = cos_theta * alpha;
                float pdf = alpha / (1.0 - cos_theta * cos_theta + ggx_a * ggx_a);
                pdf = pdf * pdf / PI / 4.0;  // GGX + Jacobian
                local_space_dirs[i] = vector3df(
                    sin_theta * cosf(phi),
                    sin_theta * sinf(phi),
                    cos_theta).normalize();
                lods[i] = 0.5f * log2(6.0 * dst[level].m_dim.Width
                                          * dst[level].m_dim.Height
                                          / (sample_count * pdf)) + lodbias;
            }
            for (int i = 0; i < dst[level].m_dim.Width * dst[level].m_dim.Height; i++)
            {
                vector2df uv = vector2df(
                    (0.5f + i % dst[level].m_dim.Width) / dst[level].m_dim.Width  * 2.0f - 1.0f,
                    (0.5f + i / dst[level].m_dim.Width) / dst[level].m_dim.Height * 2.0f - 1.0f);
                vector3df scan = vector3df();
                switch(tex_index)
                {
                    case 0: scan = vector3df( 1.f,  uv.Y, -uv.X); break;
                    case 1: scan = vector3df(-1.f,  uv.Y,  uv.X); break;
                    case 2: scan = vector3df( uv.X, -1.f,  uv.Y); break;
                    case 3: scan = vector3df( uv.X,  1.f, -uv.Y); break;
                    case 4: scan = vector3df( uv.X, uv.Y,  1.f); break;
                    case 5: scan = vector3df(-uv.X, uv.Y, -1.f); break;
                }
                vector3df dir = scan.normalize(); // Normal
                dir.Y = -dir.Y;

                if (level == 0)
                {
                    SColorf color = texture_lod(src2, dir, lodbias);

                    ((uint32_t*)dst[level].m_data)[i]
                        = ((uint32_t)(color.b * irr::core::clamp(dir.Y * 2.f, 0.f, 1.f) * 255.))
                        + ((uint32_t)(color.g * irr::core::clamp(dir.Y * 2.f, 0.f, 1.f) * 255.) << 8)
                        + ((uint32_t)(color.r * irr::core::clamp(dir.Y * 2.f, 0.f, 1.f) * 255.) << 16)
                        + ((uint32_t)(color.a * irr::core::clamp(dir.Y * 2.f, 0.f, 1.f) * 255.) << 24);
                    continue;
                }
                
                
                vector3df bitangent = vector3df(0.0, 1.0, 0.0);
                float ndotup = dir.dotProduct(vector3df(0.0, 1.0, 0.0));
                if (1.0 - abs(ndotup) < 0.00001)
                {
                    bitangent = ndotup > 0.0 ? vector3df(0.0, 0.0, 1.0)
                                             : vector3df(0.0, 0.0, -1.0);
                }
                vector3df tangent = bitangent.crossProduct(dir).normalize();
                bitangent         = dir.crossProduct(tangent);

                SColorf color = SColorf();
                float weight = 1e-8;

                for (int i = 0; i < sample_count; i++)
                {
                    // TBN * local_space_dir
                    vector3df ldir = local_space_dirs[i];
                    float lod = lods[i];

                    vector3df world_dir = vector3df(
                        tangent.X * ldir.X + bitangent.X * ldir.Y + dir.X * ldir.Z,
                        tangent.Y * ldir.X + bitangent.Y * ldir.Y + dir.Y * ldir.Z,
                        tangent.Z * ldir.X + bitangent.Z * ldir.Y + dir.Z * ldir.Z
                    );
                    
                    // Reflect
                    vector3df light = -dir;
                    light -= 2.0f * world_dir * light.dotProduct(world_dir);
                    light = light.normalize();
                    float NdotL = dir.dotProduct(light);
                    
                    if (NdotL > 0.0)
                    {
                        SColorf samp = texture_lod(src2, light, lod);
                        color.a += samp.a * irr::core::clamp(light.Y * 2.f, 0.f, 1.f) * NdotL;
                        color.r += samp.r * irr::core::clamp(light.Y * 2.f, 0.f, 1.f) * NdotL;
                        color.g += samp.g * irr::core::clamp(light.Y * 2.f, 0.f, 1.f) * NdotL;
                        color.b += samp.b * irr::core::clamp(light.Y * 2.f, 0.f, 1.f) * NdotL;
                        weight += NdotL;
                    }
                }
                color.a /= weight;
                color.r /= weight;
                color.g /= weight;
                color.b /= weight;
                ((uint32_t*)dst[level].m_data)[i]
                    = ((uint32_t)(powf(color.b, 0.455f) * 255.))
                    + ((uint32_t)(powf(color.g, 0.455f) * 255.) << 8)
                    + ((uint32_t)(powf(color.r, 0.455f) * 255.) << 16)
                    + ((uint32_t)(powf(color.a, 0.455f) * 255.) << 24);
            }
        }
    };
    g_texture_cubemap = new GEVulkanArrayTexture(sky_tex,
        VK_IMAGE_VIEW_TYPE_CUBE, swap_pixels);

    g_diffuse_env_cubemap = new GEVulkanArrayTexture(sky_tex,
        VK_IMAGE_VIEW_TYPE_CUBE, swap_pixels, diffuse_presample, 
        irr::core::dimension2d<irr::u32>(32, 32));
    g_specular_env_cubemap = new GEVulkanArrayTexture(sky_tex,
        VK_IMAGE_VIEW_TYPE_CUBE, swap_pixels, specular_presample, 
        irr::core::dimension2d<irr::u32>(256, 256));
    
    g_updated_texture_descriptor = false;

    g_skybox = skybox;
    g_render_skybox[cam] = true;
    GEVulkanDriver* vk = getVKDriver();

    // Environment map
    {
        std::vector<VkDescriptorSetLayoutBinding> layout_list;

        VkDescriptorSetLayoutBinding texture_layout_binding;
        texture_layout_binding.binding = 0;
        texture_layout_binding.descriptorCount = 1;
        texture_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        texture_layout_binding.pImmutableSamplers = NULL;
        texture_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        layout_list.push_back(texture_layout_binding);

        texture_layout_binding.binding = 1;
        texture_layout_binding.descriptorCount = 1;
        texture_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        texture_layout_binding.pImmutableSamplers = NULL;
        texture_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        layout_list.push_back(texture_layout_binding);

        VkDescriptorSetLayoutCreateInfo setinfo = {};
        setinfo.flags = 0;
        setinfo.pNext = NULL;
        setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        setinfo.pBindings = layout_list.data();
        setinfo.bindingCount = layout_list.size();
        if (vkCreateDescriptorSetLayout(vk->getDevice(), &setinfo,
            NULL, &g_descriptor_layout_env_map) != VK_SUCCESS)
        {
            throw std::runtime_error("vkCreateDescriptorSetLayout failed for "
                "addSkyBox");
        }

        VkDescriptorPoolSize pool_size;
        pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        pool_size.descriptorCount = 2;

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = 0;
        pool_info.maxSets = 1;
        pool_info.poolSizeCount = 1;
        pool_info.pPoolSizes = &pool_size;
        if (vkCreateDescriptorPool(vk->getDevice(), &pool_info, NULL,
            &g_descriptor_pool_env_map) != VK_SUCCESS)
        {
            throw std::runtime_error("vkCreateDescriptorPool failed for "
                "addSkyBox");
        }

        std::vector<VkDescriptorSetLayout> layouts(1, g_descriptor_layout_env_map);

        VkDescriptorSetAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = g_descriptor_pool_env_map;
        alloc_info.descriptorSetCount = layouts.size();
        alloc_info.pSetLayouts = layouts.data();

        if (vkAllocateDescriptorSets(vk->getDevice(), &alloc_info,
            &g_descriptor_set_env_map) != VK_SUCCESS)
        {
            throw std::runtime_error("vkAllocateDescriptorSets failed for "
                "addSkyBox");
        }
    } // Environment map

    // g_descriptor_layout
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
        NULL, &g_descriptor_layout) != VK_SUCCESS)
    {
        throw std::runtime_error("vkCreateDescriptorSetLayout failed for "
            "addSkyBox");
    }

    // g_descriptor_pool
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
        &g_descriptor_pool) != VK_SUCCESS)
    {
        throw std::runtime_error("vkCreateDescriptorPool failed for "
            "addSkyBox");
    }

    // g_descriptor_set
    std::vector<VkDescriptorSetLayout> layouts(1, g_descriptor_layout);

    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = g_descriptor_pool;
    alloc_info.descriptorSetCount = layouts.size();
    alloc_info.pSetLayouts = layouts.data();

    if (vkAllocateDescriptorSets(vk->getDevice(), &alloc_info,
        &g_descriptor_set) != VK_SUCCESS)
    {
        throw std::runtime_error("vkAllocateDescriptorSets failed for "
            "addSkyBox");
    }


    // g_pipeline_layout
    VkPipelineLayoutCreateInfo pipeline_layout_info = {};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &g_descriptor_layout;

    VkPushConstantRange push_constant;
    push_constant.offset = 0;
    push_constant.size = sizeof(irr::core::matrix4);
    push_constant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    pipeline_layout_info.pPushConstantRanges = &push_constant;
    pipeline_layout_info.pushConstantRangeCount = 1;

    if (vkCreatePipelineLayout(vk->getDevice(), &pipeline_layout_info,
        NULL, &g_pipeline_layout) != VK_SUCCESS)
    {
        throw std::runtime_error("vkCreatePipelineLayout failed for "
            "addSkyBox");
    }

    // g_graphics_pipeline
    VkPipelineShaderStageCreateInfo vert_shader_stage_info = {};
    vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_shader_stage_info.module = GEVulkanShaderManager::getShader("fullscreen_quad.vert");
    vert_shader_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo frag_shader_stage_info = {};
    frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_shader_stage_info.module = GEVulkanShaderManager::getShader("skybox.frag");
    frag_shader_stage_info.pName = "main";

    std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages =
    {{
        vert_shader_stage_info,
        frag_shader_stage_info
    }};

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 0;
    vertex_input_info.vertexAttributeDescriptionCount = 0;
    vertex_input_info.pVertexBindingDescriptions = NULL;
    vertex_input_info.pVertexAttributeDescriptions = NULL;

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)vk->getSwapChainExtent().width;
    viewport.height = (float)vk->getSwapChainExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = vk->getSwapChainExtent();

    VkPipelineViewportStateCreateInfo viewport_state = {};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = &viewport;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depth_stencil = {};
    depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_FALSE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_EQUAL;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState color_blend_attachment = {};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo color_blending = {};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.logicOp = VK_LOGIC_OP_COPY;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_blend_attachment;
    color_blending.blendConstants[0] = 0.0f;
    color_blending.blendConstants[1] = 0.0f;
    color_blending.blendConstants[2] = 0.0f;
    color_blending.blendConstants[3] = 0.0f;

    std::array<VkDynamicState, 2> dynamic_state =
    {{
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_VIEWPORT
    }};

    VkPipelineDynamicStateCreateInfo dynamic_state_info = {};
    dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    dynamic_state_info.dynamicStateCount = dynamic_state.size(),
    dynamic_state_info.pDynamicStates = dynamic_state.data();

    VkGraphicsPipelineCreateInfo pipeline_info = {};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = shader_stages.size();
    pipeline_info.pStages = shader_stages.data();
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pDepthStencilState = &depth_stencil;
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.pDynamicState = &dynamic_state_info;
    pipeline_info.layout = g_pipeline_layout;
    pipeline_info.renderPass = vk->getRTTTexture() ?
        vk->getRTTTexture()->getRTTRenderPass() : vk->getRenderPass();
    pipeline_info.subpass = 0;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(vk->getDevice(), VK_NULL_HANDLE,
        1, &pipeline_info, NULL, &g_graphics_pipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("vkCreateGraphicsPipelines failed for "
            "addSkyBox");
    }

}   // addSkyBox

// ----------------------------------------------------------------------------
VkDescriptorSet *GEVulkanSkyBoxRenderer::getEnvMapDescriptor()
{
    updateDescriptorSet();

    return &g_descriptor_set_env_map;
}

// ----------------------------------------------------------------------------
VkDescriptorSetLayout *GEVulkanSkyBoxRenderer::getEnvMapDescriptorLayout()
{
    return &g_descriptor_layout_env_map;
}

}
