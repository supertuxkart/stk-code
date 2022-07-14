#include "ge_vulkan_2d_renderer.hpp"

#include "ge_main.hpp"
#include "ge_vulkan_driver.hpp"
#include "ge_vulkan_dynamic_buffer.hpp"
#include "ge_vulkan_features.hpp"
#include "ge_vulkan_shader_manager.hpp"

#include <algorithm>
#include <cstddef>
#include <map>
#include <string>
#include <stdexcept>
#include <utility>

#include "rect.h"
#include "vector2d.h"
#include "SColor.h"

namespace GE
{
// ============================================================================
namespace GEVulkan2dRenderer
{
using namespace irr;
// ============================================================================
GEVulkanDriver* g_vk;

VkPipelineLayout g_pipeline_layout = VK_NULL_HANDLE;
VkPipeline g_graphics_pipeline = VK_NULL_HANDLE;

VkDescriptorPool g_descriptor_pool = VK_NULL_HANDLE;
VkDescriptorSetLayout g_descriptor_set_layout = VK_NULL_HANDLE;

GEVulkanDynamicBuffer* g_tris_buffer = NULL;

std::vector<VkDescriptorSet> g_descriptor_sets;

struct Tri
{
    core::vector2df pos;
    video::SColor color;
    core::vector2df uv;
    int sampler_idx;
};

std::map<const irr::video::ITexture*, unsigned> g_tex_map;
std::vector<irr::core::recti> g_tris_clip;
std::vector<Tri> g_tris_queue;
std::vector<uint16_t> g_tris_index_queue;
}   // GEVulkan2dRenderer

// ============================================================================
void GEVulkan2dRenderer::init(GEVulkanDriver* vk)
{
    g_vk = vk;
    createDescriptorSetLayout();
    createPipelineLayout();
    createGraphicsPipeline();
    createTrisBuffers();
    createDescriptorPool();
    createDescriptorSets();
}   // init

// ----------------------------------------------------------------------------
void GEVulkan2dRenderer::destroy()
{
    delete g_tris_buffer;
    g_tris_buffer = NULL;

    if (!g_vk)
        return;
    vkDestroyDescriptorSetLayout(g_vk->getDevice(), g_descriptor_set_layout,
        NULL);
    vkDestroyDescriptorPool(g_vk->getDevice(), g_descriptor_pool, NULL);
    vkDestroyPipeline(g_vk->getDevice(), g_graphics_pipeline, NULL);
    vkDestroyPipelineLayout(g_vk->getDevice(), g_pipeline_layout, NULL);
    g_descriptor_sets.clear();
}   // destroy

// ----------------------------------------------------------------------------
void GEVulkan2dRenderer::createDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding sampler_layout_binding = {};
    sampler_layout_binding.binding = 0;
    sampler_layout_binding.descriptorCount =
        GEVulkanFeatures::supportsBindTexturesAtOnce() ?
        GEVulkanShaderManager::getSamplerSize() : 1;
    sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler_layout_binding.pImmutableSamplers = NULL;
    sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 1> bindings =
    {
        sampler_layout_binding
    };

    VkDescriptorSetLayoutCreateInfo layout_info = {};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = (uint32_t)(bindings.size());
    layout_info.pBindings = &bindings[0];

    VkResult result = vkCreateDescriptorSetLayout(g_vk->getDevice(), &layout_info,
        NULL, &g_descriptor_set_layout);

    if (result != VK_SUCCESS)
        throw std::runtime_error("vkCreateDescriptorSetLayout failed");
}   // createDescriptorSetLayout

// ----------------------------------------------------------------------------
void GEVulkan2dRenderer::createPipelineLayout()
{
    VkPipelineLayoutCreateInfo pipeline_layout_info = {};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &g_descriptor_set_layout;

    VkResult result = vkCreatePipelineLayout(g_vk->getDevice(), &pipeline_layout_info,
        nullptr, &g_pipeline_layout);

    if (result != VK_SUCCESS)
        throw std::runtime_error("vkCreatePipelineLayout failed");
}   // createPipelineLayout

// ----------------------------------------------------------------------------
void GEVulkan2dRenderer::createGraphicsPipeline()
{
    VkPipelineShaderStageCreateInfo vert_shader_stage_info = {};
    vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_shader_stage_info.module = GEVulkanShaderManager::getShader("2d_render.vert");
    vert_shader_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo frag_shader_stage_info = {};
    frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_shader_stage_info.module = GEVulkanShaderManager::getShader("2d_render.frag");
    frag_shader_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo shader_stages[] =
    {
        vert_shader_stage_info,
        frag_shader_stage_info
    };

    VkVertexInputBindingDescription binding_description = {};
    binding_description.binding = 0;
    binding_description.stride = sizeof(Tri);
    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::array<VkVertexInputAttributeDescription, 4> attribute_descriptions = {};
    attribute_descriptions[0].binding = 0;
    attribute_descriptions[0].location = 0;
    attribute_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attribute_descriptions[0].offset = offsetof(Tri, pos);
    attribute_descriptions[1].binding = 0;
    attribute_descriptions[1].location = 1;
    attribute_descriptions[1].format = VK_FORMAT_R8G8B8A8_UNORM;
    attribute_descriptions[1].offset = offsetof(Tri, color);
    attribute_descriptions[2].binding = 0;
    attribute_descriptions[2].location = 2;
    attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attribute_descriptions[2].offset = offsetof(Tri, uv);
    attribute_descriptions[3].binding = 0;
    attribute_descriptions[3].location = 3;
    attribute_descriptions[3].format = VK_FORMAT_R32_SINT;
    attribute_descriptions[3].offset = offsetof(Tri, sampler_idx);

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.vertexAttributeDescriptionCount = (uint32_t)(attribute_descriptions.size());
    vertex_input_info.pVertexBindingDescriptions = &binding_description;
    vertex_input_info.pVertexAttributeDescriptions = &attribute_descriptions[0];

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)g_vk->getSwapChainExtent().width;
    viewport.height = (float)g_vk->getSwapChainExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = g_vk->getSwapChainExtent();

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
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depth_stencil = {};
    depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = VK_FALSE;
    depth_stencil.depthWriteEnable = VK_FALSE;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState color_blend_attachment = {};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_TRUE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

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
    {
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_VIEWPORT
    };
    VkPipelineDynamicStateCreateInfo dynamic_state_info = {};
    dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    dynamic_state_info.dynamicStateCount = dynamic_state.size(),
    dynamic_state_info.pDynamicStates = dynamic_state.data();

    VkGraphicsPipelineCreateInfo pipeline_info = {};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pDepthStencilState = &depth_stencil;
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.pDynamicState = &dynamic_state_info;
    pipeline_info.layout = g_pipeline_layout;
    pipeline_info.renderPass = g_vk->getRenderPass();
    pipeline_info.subpass = 0;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

    VkResult result = vkCreateGraphicsPipelines(g_vk->getDevice(),
        VK_NULL_HANDLE, 1, &pipeline_info, NULL, &g_graphics_pipeline);

    if (result != VK_SUCCESS)
        throw std::runtime_error("vkCreateGraphicsPipelines failed");
}   // createGraphicsPipeline

// ----------------------------------------------------------------------------
void GEVulkan2dRenderer::createTrisBuffers()
{
    g_tris_buffer = new GEVulkanDynamicBuffer(GVDBT_SYSTEM_RAM,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        12000);
}   // createTrisBuffers

// ----------------------------------------------------------------------------
void GEVulkan2dRenderer::createDescriptorPool()
{
    uint32_t descriptor_count = g_vk->getMaxFrameInFlight();
    if (!GEVulkanFeatures::supportsBindTexturesAtOnce())
        descriptor_count *= GEVulkanShaderManager::getSamplerSize();

    std::array<VkDescriptorPoolSize, 1> pool_sizes = {};
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[0].descriptorCount = GEVulkanShaderManager::getSamplerSize() *
        g_vk->getMaxFrameInFlight();

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = (uint32_t)(pool_sizes.size());
    pool_info.pPoolSizes = &pool_sizes[0];
    pool_info.maxSets = descriptor_count;

    if (vkCreateDescriptorPool(g_vk->getDevice(), &pool_info, NULL,
        &g_descriptor_pool) != VK_SUCCESS)
        throw std::runtime_error("createDescriptorPool failed");
}   // createDescriptorPool

// ----------------------------------------------------------------------------
void GEVulkan2dRenderer::createDescriptorSets()
{
    unsigned set_size = g_vk->getMaxFrameInFlight();
    if (!GEVulkanFeatures::supportsBindTexturesAtOnce())
        set_size *= GEVulkanShaderManager::getSamplerSize();
    g_descriptor_sets.resize(set_size);
    std::vector<VkDescriptorSetLayout> layouts(g_descriptor_sets.size(),
        g_descriptor_set_layout);

    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = g_descriptor_pool;
    alloc_info.descriptorSetCount = (uint32_t)(layouts.size());
    alloc_info.pSetLayouts = layouts.data();

    if (vkAllocateDescriptorSets(g_vk->getDevice(), &alloc_info,
        g_descriptor_sets.data()) != VK_SUCCESS)
        throw std::runtime_error("vkAllocateDescriptorSets failed");
}   // createDescriptorSets

// ----------------------------------------------------------------------------
void GEVulkan2dRenderer::uploadTrisBuffers()
{
    if (g_tex_map.empty())
        return;

    g_tris_buffer->setCurrentData(
    {
        { (void*)g_tris_queue.data(), g_tris_queue.size() * sizeof(Tri) },
        { (void*)g_tris_index_queue.data(), g_tris_index_queue.size() * sizeof(uint16_t) }
    });
}   // uploadTrisBuffers

// ----------------------------------------------------------------------------
void GEVulkan2dRenderer::render()
{
    if (g_tex_map.empty())
        return;

    std::vector<std::pair<const irr::video::ITexture*, unsigned> > tex_map;
    for (auto& tex : g_tex_map)
        tex_map.emplace_back(tex.first, (unsigned)tex.second);

    std::sort(tex_map.begin(), tex_map.end(),
        [](const std::pair<const irr::video::ITexture*, unsigned>& a,
        const std::pair<const irr::video::ITexture*, unsigned>& b)
        {
            return a.second < b.second;
        });

    std::vector<VkDescriptorImageInfo> image_infos;
    for (auto& tex : tex_map)
    {
        VkDescriptorImageInfo image_info;
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.imageView = (VkImageView)tex.first->getTextureHandler();
        image_info.sampler = g_vk->getSampler(GVS_2D_RENDER);
        image_infos.push_back(image_info);
        if (image_infos.size() >= GEVulkanShaderManager::getSamplerSize())
            break;
    }

    if (GEVulkanFeatures::supportsBindTexturesAtOnce())
    {
        image_infos.resize(GEVulkanShaderManager::getSamplerSize(), image_infos[0]);

        VkWriteDescriptorSet write_descriptor_set = {};
        write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptor_set.dstBinding = 0;
        write_descriptor_set.dstArrayElement = 0;
        write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write_descriptor_set.descriptorCount = GEVulkanShaderManager::getSamplerSize();
        write_descriptor_set.pBufferInfo = 0;
        write_descriptor_set.dstSet = g_descriptor_sets[g_vk->getCurrentFrame()];
        write_descriptor_set.pImageInfo = image_infos.data();

        vkUpdateDescriptorSets(g_vk->getDevice(), 1, &write_descriptor_set, 0,
            NULL);
    }
    else
    {
        for (unsigned i = 0; i < image_infos.size(); i++)
        {
            VkWriteDescriptorSet write_descriptor_set = {};
            write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_descriptor_set.dstBinding = 0;
            write_descriptor_set.dstArrayElement = 0;
            write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            write_descriptor_set.descriptorCount = 1;
            write_descriptor_set.pBufferInfo = 0;
            const unsigned set_idx = g_vk->getCurrentFrame() * GEVulkanShaderManager::getSamplerSize() + i;
            write_descriptor_set.dstSet = g_descriptor_sets[set_idx];
            write_descriptor_set.pImageInfo = &image_infos[i];

            vkUpdateDescriptorSets(g_vk->getDevice(), 1, &write_descriptor_set,
                0, NULL);
        }
    }

    VkDeviceSize offsets[] = {0};
    VkBuffer buffer = VK_NULL_HANDLE;
    unsigned idx = 0;
    unsigned idx_count = 0;
    int sampler_idx = 0;
    core::recti clip;

    buffer = g_tris_buffer->getCurrentBuffer();
    if (buffer == VK_NULL_HANDLE)
        goto end;

    vkCmdBindPipeline(g_vk->getCurrentCommandBuffer(),
        VK_PIPELINE_BIND_POINT_GRAPHICS, g_graphics_pipeline);

    vkCmdBindVertexBuffers(g_vk->getCurrentCommandBuffer(), 0, 1,
        &buffer, offsets);

    vkCmdBindIndexBuffer(g_vk->getCurrentCommandBuffer(), buffer,
        g_tris_queue.size() * sizeof(Tri), VK_INDEX_TYPE_UINT16);

    VkViewport vp;
    vp.x = g_vk->getViewPort().UpperLeftCorner.X;
    vp.y = g_vk->getViewPort().UpperLeftCorner.Y;
    vp.width = g_vk->getViewPort().getWidth();
    vp.height = g_vk->getViewPort().getHeight();
    vp.minDepth = 0;
    vp.maxDepth = 1.0f;
    g_vk->getRotatedViewport(&vp);
    vkCmdSetViewport(g_vk->getCurrentCommandBuffer(), 0, 1, &vp);

    if (GEVulkanFeatures::supportsBindTexturesAtOnce())
    {
        vkCmdBindDescriptorSets(g_vk->getCurrentCommandBuffer(),
            VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipeline_layout, 0, 1,
            &g_descriptor_sets[g_vk->getCurrentFrame()], 0, NULL);
    }

    sampler_idx = g_tris_queue[0].sampler_idx;
    clip = g_tris_clip[0];
    for (; idx < g_tris_index_queue.size(); idx += 3)
    {
        Tri& cur_tri = g_tris_queue[g_tris_index_queue[idx]];
        int cur_sampler_idx = cur_tri.sampler_idx;
        if (GEVulkanFeatures::supportsDifferentTexturePerDraw())
            cur_sampler_idx = g_tris_queue[0].sampler_idx;
        const core::recti& cur_clip = g_tris_clip[g_tris_index_queue[idx]];
        if (cur_sampler_idx != sampler_idx || cur_clip != clip)
        {
            if (!GEVulkanFeatures::supportsBindTexturesAtOnce())
            {
                const unsigned set_idx = g_vk->getCurrentFrame() * GEVulkanShaderManager::getSamplerSize() + sampler_idx;
                vkCmdBindDescriptorSets(g_vk->getCurrentCommandBuffer(),
                    VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipeline_layout, 0, 1,
                    &g_descriptor_sets[set_idx], 0, NULL);
            }

            VkRect2D scissor;
            scissor.offset.x = clip.UpperLeftCorner.X;
            scissor.offset.y = clip.UpperLeftCorner.Y;
            scissor.extent.width = clip.getWidth();
            scissor.extent.height = clip.getHeight();
            g_vk->getRotatedRect2D(&scissor);
            vkCmdSetScissor(g_vk->getCurrentCommandBuffer(), 0, 1, &scissor);

            vkCmdDrawIndexed(g_vk->getCurrentCommandBuffer(), idx_count, 1,
                idx - idx_count, 0, 0);
            sampler_idx = cur_sampler_idx;
            clip = cur_clip;
            idx_count = 3;
        }
        else
        {
            idx_count += 3;
        }
    }
    if (!GEVulkanFeatures::supportsBindTexturesAtOnce())
    {
        const unsigned set_idx = g_vk->getCurrentFrame() * GEVulkanShaderManager::getSamplerSize() + sampler_idx;
        vkCmdBindDescriptorSets(g_vk->getCurrentCommandBuffer(),
            VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipeline_layout, 0, 1,
            &g_descriptor_sets[set_idx], 0, NULL);
    }

    VkRect2D scissor;
    scissor.offset.x = clip.UpperLeftCorner.X;
    scissor.offset.y = clip.UpperLeftCorner.Y;
    scissor.extent.width = clip.getWidth();
    scissor.extent.height = clip.getHeight();
    g_vk->getRotatedRect2D(&scissor);
    vkCmdSetScissor(g_vk->getCurrentCommandBuffer(), 0, 1, &scissor);

    vkCmdDrawIndexed(g_vk->getCurrentCommandBuffer(), idx_count, 1,
        idx - idx_count, 0, 0);

end:
    clear();
}   // render

// ----------------------------------------------------------------------------
void GEVulkan2dRenderer::clear()
{
    g_tex_map.clear();
    g_tris_queue.clear();
    g_tris_index_queue.clear();
    g_tris_clip.clear();
}   // clear

// ----------------------------------------------------------------------------
void GEVulkan2dRenderer::addVerticesIndices(irr::video::S3DVertex* vertices,
                                            unsigned vertices_count,
                                            uint16_t* indices,
                                            unsigned indices_count,
                                            const irr::video::ITexture* t)
{
    if (g_tex_map.find(t) == g_tex_map.end())
    {
        unsigned pre_size = g_tex_map.size();
        g_tex_map[t] = pre_size;
    }
    int sampler_idx = g_tex_map.at(t);
    // In STK we rarely use more than 256 textures per (2d) draw call
    if (sampler_idx >= (int)GEVulkanShaderManager::getSamplerSize())
        sampler_idx = GEVulkanShaderManager::getSamplerSize() - 1;
    uint16_t last_index = (uint16_t)g_tris_queue.size();
    if (last_index + vertices_count > 65535)
        return;
    for (unsigned idx = 0; idx < vertices_count; idx++)
    {
        Tri t;
        const S3DVertex& vertex = vertices[idx];
        t.pos = core::vector2df(
            vertex.Pos.X / g_vk->getCurrentRenderTargetSize().Width,
            vertex.Pos.Y / g_vk->getCurrentRenderTargetSize().Height);
        t.pos = t.pos * 2.0f;
        t.pos -= 1.0f;
        core::vector3df position = core::vector3df(t.pos.X, t.pos.Y, 0);
        g_vk->getPreRotationMatrix().transformVect(position);
        t.pos = core::vector2df(position.X, position.Y);
        t.color = vertex.Color;
        t.uv = vertex.TCoords;
        t.sampler_idx = sampler_idx;
        g_tris_queue.push_back(t);
        g_tris_clip.push_back(g_vk->getCurrentClip());
    }
    const core::recti& fclip = g_vk->getFullscreenClip();
    for (unsigned idx = 0; idx < indices_count * 3; idx += 3)
    {
        g_tris_index_queue.push_back(last_index + indices[idx]);
        g_tris_index_queue.push_back(last_index + indices[idx + 1]);
        g_tris_index_queue.push_back(last_index + indices[idx + 2]);
        const core::recti& cur_clip = g_tris_clip[last_index + indices[idx]];
        const core::vector3df& pos_1 = vertices[indices[idx]].Pos;
        const core::vector3df& pos_2 = vertices[indices[idx + 1]].Pos;
        const core::vector3df& pos_3 = vertices[indices[idx + 2]].Pos;
        if (GEVulkanFeatures::supportsDifferentTexturePerDraw() &&
            fclip != cur_clip &&
            cur_clip.isPointInside(core::position2di(pos_1.X, pos_1.Y)) &&
            cur_clip.isPointInside(core::position2di(pos_2.X, pos_2.Y)) &&
            cur_clip.isPointInside(core::position2di(pos_3.X, pos_3.Y)))
        {
            g_tris_clip[last_index + indices[idx]] = fclip;
            g_tris_clip[last_index + indices[idx + 1]] = fclip;
            g_tris_clip[last_index + indices[idx + 2]] = fclip;
        }
    }
}   // addVerticesIndices

}
