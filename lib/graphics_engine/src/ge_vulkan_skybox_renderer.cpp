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
bool g_updated_texture_descriptor = false;

VkDescriptorSetLayout g_descriptor_layout = VK_NULL_HANDLE;
VkDescriptorPool g_descriptor_pool = VK_NULL_HANDLE;
VkDescriptorSet g_descriptor_set = VK_NULL_HANDLE;
VkPipelineLayout g_pipeline_layout = VK_NULL_HANDLE;
VkPipeline g_graphics_pipeline = VK_NULL_HANDLE;
}   // GEVulkanSkyBoxRenderer

// ============================================================================
void GEVulkanSkyBoxRenderer::init()
{
    g_skybox = NULL;
    g_render_skybox.clear();
    g_descriptor_layout = VK_NULL_HANDLE;
    g_descriptor_pool = VK_NULL_HANDLE;
    g_descriptor_set = VK_NULL_HANDLE;
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
    g_skybox = NULL;
    g_render_skybox.clear();
    g_updated_texture_descriptor = true;
}   // destroy

// ----------------------------------------------------------------------------
void GEVulkanSkyBoxRenderer::render(VkCommandBuffer cmd,
                                    GEVulkanCameraSceneNode* cam)
{
    if (g_render_skybox.find(cam) == g_render_skybox.end() ||
        !g_render_skybox.at(cam))
        return;
    g_render_skybox.at(cam) = false;

    if (!g_updated_texture_descriptor)
    {
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
    }

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
    g_texture_cubemap = new GEVulkanArrayTexture(sky_tex,
        VK_IMAGE_VIEW_TYPE_CUBE, swap_pixels);
    g_updated_texture_descriptor = false;

    g_skybox = skybox;
    g_render_skybox[cam] = true;
    GEVulkanDriver* vk = getVKDriver();

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

}
