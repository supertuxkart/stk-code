#include "ge_vulkan_draw_call.hpp"

#include "ge_culling_tool.hpp"
#include "ge_main.hpp"
#include "ge_spm.hpp"
#include "ge_spm_buffer.hpp"
#include "ge_vulkan_animated_mesh_scene_node.hpp"
#include "ge_vulkan_camera_scene_node.hpp"
#include "ge_vulkan_driver.hpp"
#include "ge_vulkan_dynamic_buffer.hpp"
#include "ge_vulkan_fbo_texture.hpp"
#include "ge_vulkan_features.hpp"
#include "ge_vulkan_mesh_cache.hpp"
#include "ge_vulkan_mesh_scene_node.hpp"
#include "ge_vulkan_shader_manager.hpp"
#include "ge_vulkan_texture_descriptor.hpp"

#include <algorithm>
#include <limits>

namespace GE
{
// ============================================================================
ObjectData::ObjectData(irr::scene::ISceneNode* node, int material_id,
                       int skinning_offset)
{
    memcpy(m_mat_1, node->getAbsoluteTransformation().pointer(),
        sizeof(irr::core::matrix4));
    m_skinning_offset = skinning_offset;
    m_material_id = material_id;
    m_texture_trans[0] = 0.0f;
    m_texture_trans[1] = 0.0f;
}   // ObjectData

// ----------------------------------------------------------------------------
GEVulkanDrawCall::GEVulkanDrawCall()
{
    m_culling_tool = new GECullingTool;
    m_dynamic_data = NULL;
    m_object_data_padded_size = 0;
    m_skinning_data_padded_size = 0;
    m_data_padding = NULL;
    const VkPhysicalDeviceLimits& limit =
         getVKDriver()->getPhysicalDeviceProperties().limits;
    const size_t ubo_padding = limit.minUniformBufferOffsetAlignment;
    const size_t sbo_padding = limit.minStorageBufferOffsetAlignment;
    size_t padding = std::max(
    {
        ubo_padding, sbo_padding, sizeof(irr::core::matrix4)
    });
    m_data_padding = new char[padding]();
    irr::core::matrix4 identity;
    memcpy(m_data_padding, identity.pointer(), sizeof(irr::core::matrix4));
    m_data_layout = VK_NULL_HANDLE;
    m_descriptor_pool = VK_NULL_HANDLE;
    m_pipeline_layout = VK_NULL_HANDLE;
    m_texture_descriptor = getVKDriver()->getMeshTextureDescriptor();
}   // GEVulkanDrawCall

// ----------------------------------------------------------------------------
GEVulkanDrawCall::~GEVulkanDrawCall()
{
    delete [] m_data_padding;
    delete m_culling_tool;
    delete m_dynamic_data;
    if (m_data_layout != VK_NULL_HANDLE)
    {
        GEVulkanDriver* vk = getVKDriver();
        vkDestroyDescriptorSetLayout(vk->getDevice(), m_data_layout, NULL);
        vkDestroyDescriptorPool(vk->getDevice(), m_descriptor_pool, NULL);
        for (auto& p : m_graphics_pipelines)
            vkDestroyPipeline(vk->getDevice(), p.second.first, NULL);
        vkDestroyPipelineLayout(vk->getDevice(), m_pipeline_layout, NULL);
    }
}   // ~GEVulkanDrawCall

// ----------------------------------------------------------------------------
void GEVulkanDrawCall::addNode(irr::scene::ISceneNode* node)
{
    irr::scene::IMesh* mesh;
    GEVulkanAnimatedMeshSceneNode* anode = NULL;
    if (node->getType() == irr::scene::ESNT_ANIMATED_MESH)
    {
        anode = static_cast<GEVulkanAnimatedMeshSceneNode*>(node);
        mesh = anode->getMesh();
    }
    else if (node->getType() == irr::scene::ESNT_MESH)
    {
        mesh = static_cast<irr::scene::IMeshSceneNode*>(node)->getMesh();
        for (unsigned i = 0; i < mesh->getMeshBufferCount(); i++)
        {
            irr::scene::IMeshBuffer* b = mesh->getMeshBuffer(i);
            if (b->getVertexType() != irr::video::EVT_SKINNED_MESH)
                return;
        }
    }
    else
        return;

    bool added_skinning = false;
    for (unsigned i = 0; i < mesh->getMeshBufferCount(); i++)
    {
        GESPMBuffer* buffer = static_cast<GESPMBuffer*>(
            mesh->getMeshBuffer(i));
        if (m_culling_tool->isCulled(buffer, node))
            continue;
        const std::string& shader = getShader(node, i);
        m_visible_nodes[buffer][shader].emplace_back(node, i);
        if (anode && !added_skinning &&
            !anode->getSkinningMatrices().empty() &&
            m_skinning_nodes.find(anode) == m_skinning_nodes.end())
        {
            added_skinning = true;
            m_skinning_nodes.insert(anode);
        }
    }
}   // addNode

// ----------------------------------------------------------------------------
void GEVulkanDrawCall::generate()
{
    if (!m_visible_nodes.empty() && m_data_layout == VK_NULL_HANDLE)
        createVulkanData();

    std::unordered_map<irr::scene::ISceneNode*, int> skinning_offets;
    int added_joint = 1;
    m_skinning_data_padded_size = sizeof(irr::core::matrix4);
    m_data_uploading.emplace_back((void*)m_data_padding,
        sizeof(irr::core::matrix4));
    for (GEVulkanAnimatedMeshSceneNode* node : m_skinning_nodes)
    {
        int bone_count = node->getSPM()->getJointCount();
        size_t bone_size = sizeof(irr::core::matrix4) * bone_count;
        m_data_uploading.emplace_back(
            (void*)node->getSkinningMatrices().data(), bone_size);
        skinning_offets[node] = added_joint;
        added_joint += bone_count;
        m_skinning_data_padded_size += bone_size;
    }

    using Nodes = std::pair<GESPMBuffer*, std::unordered_map<
        std::string, std::vector<std::pair<irr::scene::ISceneNode*, unsigned
        > > > >;
    std::vector<Nodes> visible_nodes;

    for (auto& p : m_visible_nodes)
        visible_nodes.emplace_back(p.first, std::move(p.second));
    std::unordered_map<GESPMBuffer*, float> nodes_area;
    for (auto& p : visible_nodes)
    {
        if (p.second.empty())
        {
            nodes_area[p.first] = std::numeric_limits<float>::max();
            continue;
        }
        for (auto& q : p.second)
        {
            if (q.second.empty())
            {
                nodes_area[p.first] = std::numeric_limits<float>::max();
                continue;
            }
            irr::core::aabbox3df bb = p.first->getBoundingBox();
            q.second[0].first->getAbsoluteTransformation().transformBoxEx(bb);
            nodes_area[p.first] = bb.getArea() * (float)q.second.size();
            break;
        }
    }
    std::sort(visible_nodes.begin(), visible_nodes.end(),
        [&nodes_area](const Nodes& a, const Nodes& b)
        {
            return nodes_area.at(a.first) < nodes_area.at(b.first) ;
        });

    unsigned accumulated_instance = 0;
    for (auto& p : visible_nodes)
    {
        irr::video::SMaterial& m = p.first->getMaterial();
        std::array<const irr::video::ITexture*, 8> textures =
        {{
            m.TextureLayer[0].Texture,
            m.TextureLayer[1].Texture,
            m.TextureLayer[2].Texture,
            m.TextureLayer[3].Texture,
            m.TextureLayer[4].Texture,
            m.TextureLayer[5].Texture,
            m.TextureLayer[6].Texture,
            m.TextureLayer[7].Texture
        }};
        const irr::video::ITexture** list = &textures[0];
        const int material_id = m_texture_descriptor->getTextureID(list);
        if (!GEVulkanFeatures::supportsBindMeshTexturesAtOnce())
            m_materials[p.first->getVBOOffset()] = material_id;

        const bool skinning = p.first->hasSkinning();
        for (auto& q : p.second)
        {
            unsigned visible_count = q.second.size();
            if (visible_count == 0)
                continue;
            std::string cur_shader = q.first;
            if (skinning)
                cur_shader += "_skinning";
            if (m_graphics_pipelines.find(cur_shader) ==
                m_graphics_pipelines.end())
                continue;
            for (auto& r : q.second)
            {
                irr::scene::ISceneNode* node = r.first;
                int skinning_offset = -1000;
                auto it = skinning_offets.find(node);
                if (it != skinning_offets.end())
                    skinning_offset = it->second;
                m_visible_objects.emplace_back(node, material_id,
                    skinning_offset);
            }
            VkDrawIndexedIndirectCommand cmd;
            cmd.indexCount = p.first->getIndexCount();
            cmd.instanceCount = visible_count;
            cmd.firstIndex = p.first->getIBOOffset();
            cmd.vertexOffset = p.first->getVBOOffset();
            cmd.firstInstance = accumulated_instance;
            accumulated_instance += visible_count;
            const PipelineSettings& settings =
                m_graphics_pipelines[cur_shader].second;
            std::string sorting_key =
                std::string(1, settings.m_drawing_priority) + cur_shader;
            m_cmds.push_back({ cmd, cur_shader, sorting_key });
        }
    }
    if (!GEVulkanFeatures::supportsBindMeshTexturesAtOnce())
    {
        std::stable_sort(m_cmds.begin(), m_cmds.end(),
            [this](const DrawCallData& a, const DrawCallData& b)
            {
                return m_materials[a.m_cmd.vertexOffset] <
                    m_materials[b.m_cmd.vertexOffset];
            });
    }

    std::stable_sort(m_cmds.begin(), m_cmds.end(),
        [](const DrawCallData& a, const DrawCallData& b)
        {
            return a.m_sorting_key < b.m_sorting_key;
        });
}   // generate

// ----------------------------------------------------------------------------
std::string GEVulkanDrawCall::getShader(irr::scene::ISceneNode* node,
                                        int material_id)
{
    irr::video::SMaterial& m = node->getMaterial(material_id);
    switch (m.MaterialType)
    {
    case irr::video::EMT_TRANSPARENT_ADD_COLOR: return "additive";
    case irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF: return "alphatest";
    case irr::video::EMT_ONETEXTURE_BLEND: return "alphablend";
    case irr::video::EMT_SOLID_2_LAYER: return "decal";
    case irr::video::EMT_STK_GRASS: return "grass";
    case irr::video::EMT_STK_GHOST: return "ghost";
    default: return "solid";
    }
}   // getShader

// ----------------------------------------------------------------------------
void GEVulkanDrawCall::prepare(GEVulkanCameraSceneNode* cam)
{
    reset();
    m_culling_tool->init(cam);
}   // prepare

// ----------------------------------------------------------------------------
void GEVulkanDrawCall::createAllPipelines(GEVulkanDriver* vk)
{
    PipelineSettings settings = {};
    settings.m_depth_test = true;
    settings.m_depth_write = true;
    settings.m_backface_culling = true;

    settings.m_vertex_shader = "spm.vert";
    settings.m_skinning_vertex_shader = "spm_skinning.vert";
    settings.m_fragment_shader = "solid.frag";
    settings.m_shader_name = "solid";
    createPipeline(vk, settings);

    settings.m_fragment_shader = "decal.frag";
    settings.m_shader_name = "decal";
    createPipeline(vk, settings);

    settings.m_fragment_shader = "alphatest.frag";
    settings.m_shader_name = "alphatest";
    settings.m_drawing_priority = (char)5;
    createPipeline(vk, settings);

    settings.m_vertex_shader = "grass.vert";
    settings.m_skinning_vertex_shader = "";
    settings.m_shader_name = "grass";
    settings.m_drawing_priority = (char)5;
    settings.m_push_constants_func = [](uint32_t* size, void** data)
    {
        static irr::core::vector3df wind_direction;
        wind_direction = irr::core::vector3df(1.0f, 0.0f, 0.0f) *
            (getMonoTimeMs() / 1000.0f) * 1.5f;
        *size = sizeof(irr::core::vector3df);
        *data = &wind_direction;
    };
    createPipeline(vk, settings);

    settings.m_vertex_shader = "spm.vert";
    settings.m_skinning_vertex_shader = "spm_skinning.vert";
    settings.m_push_constants_func = nullptr;

    settings.m_depth_write = true;
    settings.m_backface_culling = true;
    settings.m_alphablend = true;
    settings.m_drawing_priority = (char)9;
    settings.m_fragment_shader = "ghost.frag";
    settings.m_shader_name = "ghost";
    createPipeline(vk, settings);

    settings.m_depth_write = false;
    settings.m_backface_culling = false;
    settings.m_drawing_priority = (char)10;

    settings.m_fragment_shader = "transparent.frag";
    settings.m_shader_name = "alphablend";
    createPipeline(vk, settings);

    settings.m_alphablend = false;
    settings.m_additive = true;
    settings.m_fragment_shader = "transparent.frag";
    settings.m_shader_name = "additive";
    createPipeline(vk, settings);
}   // createAllPipelines

// ----------------------------------------------------------------------------
void GEVulkanDrawCall::createPipeline(GEVulkanDriver* vk,
                                      const PipelineSettings& settings)
{
    bool creating_animated_pipeline_for_skinning = false;
    std::string shader_name = settings.m_shader_name;

start:
    VkPipelineShaderStageCreateInfo vert_shader_stage_info = {};
    vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_shader_stage_info.module = GEVulkanShaderManager::getShader(
        creating_animated_pipeline_for_skinning ?
        settings.m_skinning_vertex_shader : settings.m_vertex_shader);
    vert_shader_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo frag_shader_stage_info = {};
    frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_shader_stage_info.module = GEVulkanShaderManager::getShader(settings.m_fragment_shader);
    frag_shader_stage_info.pName = "main";

    std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages =
    {{
        vert_shader_stage_info,
        frag_shader_stage_info
    }};

    size_t bone_pitch = sizeof(int16_t) * 8;
    size_t static_pitch = sizeof(irr::video::S3DVertexSkinnedMesh) - bone_pitch;
    std::array<VkVertexInputBindingDescription, 2> binding_descriptions;
    binding_descriptions[0].binding = 0;
    binding_descriptions[0].stride = static_pitch;
    binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    binding_descriptions[1].binding = 1;
    binding_descriptions[1].stride = bone_pitch;
    binding_descriptions[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::array<VkVertexInputAttributeDescription, 8> attribute_descriptions = {};
    attribute_descriptions[0].binding = 0;
    attribute_descriptions[0].location = 0;
    attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions[0].offset = offsetof(irr::video::S3DVertexSkinnedMesh, m_position);
    attribute_descriptions[1].binding = 0;
    attribute_descriptions[1].location = 1;
    attribute_descriptions[1].format = VK_FORMAT_A2B10G10R10_SNORM_PACK32;
    attribute_descriptions[1].offset = offsetof(irr::video::S3DVertexSkinnedMesh, m_normal);
    attribute_descriptions[2].binding = 0;
    attribute_descriptions[2].location = 2;
    attribute_descriptions[2].format = VK_FORMAT_A8B8G8R8_UNORM_PACK32;
    attribute_descriptions[2].offset = offsetof(irr::video::S3DVertexSkinnedMesh, m_color);
    attribute_descriptions[3].binding = 0;
    attribute_descriptions[3].location = 3;
    attribute_descriptions[3].format = VK_FORMAT_R16G16_SFLOAT;
    attribute_descriptions[3].offset = offsetof(irr::video::S3DVertexSkinnedMesh, m_all_uvs);
    attribute_descriptions[4].binding = 0;
    attribute_descriptions[4].location = 4;
    attribute_descriptions[4].format = VK_FORMAT_R16G16_SFLOAT;
    attribute_descriptions[4].offset = offsetof(irr::video::S3DVertexSkinnedMesh, m_all_uvs) + (sizeof(int16_t) * 2);
    attribute_descriptions[5].binding = 0;
    attribute_descriptions[5].location = 5;
    attribute_descriptions[5].format = VK_FORMAT_A2B10G10R10_SNORM_PACK32;
    attribute_descriptions[5].offset = offsetof(irr::video::S3DVertexSkinnedMesh, m_tangent);
    attribute_descriptions[6].binding = 1;
    attribute_descriptions[6].location = 6;
    attribute_descriptions[6].format = VK_FORMAT_R16G16B16A16_SINT;
    attribute_descriptions[6].offset = 0;
    attribute_descriptions[7].binding = 1;
    attribute_descriptions[7].location = 7;
    attribute_descriptions[7].format = VK_FORMAT_R16G16B16A16_SFLOAT;
    attribute_descriptions[7].offset = sizeof(int16_t) * 4;

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 2;
    vertex_input_info.vertexAttributeDescriptionCount = 8;
    vertex_input_info.pVertexBindingDescriptions = binding_descriptions.data();
    vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();

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
    rasterizer.cullMode = settings.m_backface_culling ?
        VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depth_stencil = {};
    depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = settings.m_depth_test;
    depth_stencil.depthWriteEnable = settings.m_depth_write;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState color_blend_attachment = {};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = settings.isTransparent();
    if (settings.m_alphablend)
    {
        color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
        color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    }
    if (settings.m_additive)
    {
        color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
        color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
        color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    }
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
    pipeline_info.layout = m_pipeline_layout;
    pipeline_info.renderPass = vk->getRTTTexture() ?
        vk->getRTTTexture()->getRTTRenderPass() : vk->getRenderPass();
    pipeline_info.subpass = 0;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

    VkPipeline graphics_pipeline;
    VkResult result = vkCreateGraphicsPipelines(vk->getDevice(),
        VK_NULL_HANDLE, 1, &pipeline_info, NULL, &graphics_pipeline);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("vkCreateGraphicsPipelines failed for " +
            shader_name);
    }
    m_graphics_pipelines[shader_name] = std::make_pair(
        graphics_pipeline, settings);

    if (settings.m_skinning_vertex_shader.empty())
        return;
    else if (creating_animated_pipeline_for_skinning)
        return;

    creating_animated_pipeline_for_skinning = true;
    shader_name = settings.m_shader_name + "_skinning";
    goto start;
}   // createPipeline

// ----------------------------------------------------------------------------
void GEVulkanDrawCall::createVulkanData()
{
    GEVulkanDriver* vk = getVKDriver();
    const bool use_base_vertex = GEVulkanFeatures::supportsBaseVertexRendering();

    // m_data_layout
    VkDescriptorSetLayoutBinding camera_layout_binding = {};
    camera_layout_binding.binding = 0;
    camera_layout_binding.descriptorCount = 1;
    camera_layout_binding.descriptorType = use_base_vertex ?
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER :
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    camera_layout_binding.pImmutableSamplers = NULL;
    camera_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding object_data_layout_binding = {};
    object_data_layout_binding.binding = 1;
    object_data_layout_binding.descriptorCount = 1;
    object_data_layout_binding.descriptorType = use_base_vertex ?
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER :
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    object_data_layout_binding.pImmutableSamplers = NULL;
    object_data_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding skinning_layout_binding = {};
    skinning_layout_binding.binding = 2;
    skinning_layout_binding.descriptorCount = 1;
    skinning_layout_binding.descriptorType =  use_base_vertex ?
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER :
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    skinning_layout_binding.pImmutableSamplers = NULL;
    skinning_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    std::array<VkDescriptorSetLayoutBinding, 3> bindings =
    {{
         camera_layout_binding,
         object_data_layout_binding,
         skinning_layout_binding
    }};

    VkDescriptorSetLayoutCreateInfo setinfo = {};
    setinfo.flags = 0;
    setinfo.pNext = NULL;
    setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setinfo.pBindings = bindings.data();
    setinfo.bindingCount = bindings.size();

    VkResult result = vkCreateDescriptorSetLayout(vk->getDevice(), &setinfo,
        NULL, &m_data_layout);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("vkCreateDescriptorSetLayout failed for data "
            "layout");
    }

    // m_descriptor_pool
    std::array<VkDescriptorPoolSize, 2> sizes =
    {{
        {
            use_base_vertex ?
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER :
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            vk->getMaxFrameInFlight()
        },
        {
            use_base_vertex ?
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER :
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
            vk->getMaxFrameInFlight() * 2
        }
    }};

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = 0;
    pool_info.maxSets = vk->getMaxFrameInFlight();
    pool_info.poolSizeCount = sizes.size();
    pool_info.pPoolSizes = sizes.data();

    if (vkCreateDescriptorPool(vk->getDevice(), &pool_info, NULL,
        &m_descriptor_pool) != VK_SUCCESS)
        throw std::runtime_error("createDescriptorPool failed");

    // m_data_descriptor_sets
    unsigned set_size = vk->getMaxFrameInFlight();
    m_data_descriptor_sets.resize(set_size);
    std::vector<VkDescriptorSetLayout> data_layouts(
        m_data_descriptor_sets.size(), m_data_layout);

    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = m_descriptor_pool;
    alloc_info.descriptorSetCount = data_layouts.size();
    alloc_info.pSetLayouts = data_layouts.data();

    if (vkAllocateDescriptorSets(vk->getDevice(), &alloc_info,
        m_data_descriptor_sets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("vkAllocateDescriptorSets failed for data "
            "layout");
    }

    // m_pipeline_layout
    std::array<VkDescriptorSetLayout, 2> all_layouts =
    {{
        *m_texture_descriptor->getDescriptorSetLayout(),
        m_data_layout
    }};

    VkPipelineLayoutCreateInfo pipeline_layout_info = {};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = all_layouts.size();
    pipeline_layout_info.pSetLayouts = all_layouts.data();

    VkPushConstantRange push_constant;
    push_constant.offset = 0;
    const VkPhysicalDeviceLimits& limit =
        vk->getPhysicalDeviceProperties().limits;
    push_constant.size = std::min(limit.maxPushConstantsSize, 128u);
    push_constant.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
    pipeline_layout_info.pPushConstantRanges = &push_constant;
    pipeline_layout_info.pushConstantRangeCount = 1;

    result = vkCreatePipelineLayout(vk->getDevice(), &pipeline_layout_info,
        NULL, &m_pipeline_layout);

    if (result != VK_SUCCESS)
        throw std::runtime_error("vkCreatePipelineLayout failed");

    createAllPipelines(vk);

    size_t size = m_visible_objects.size();
    if (m_visible_objects.empty())
        size = 100;

    const bool use_multidraw = GEVulkanFeatures::supportsMultiDrawIndirect() &&
        GEVulkanFeatures::supportsBindMeshTexturesAtOnce();
    VkBufferUsageFlags flags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if (use_multidraw)
        flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

    m_dynamic_data = new GEVulkanDynamicBuffer(GVDBT_GPU_RAM, flags,
        m_skinning_data_padded_size + (sizeof(ObjectData) * size) +
        sizeof(GEVulkanCameraUBO));
}   // createVulkanData

// ----------------------------------------------------------------------------
void GEVulkanDrawCall::uploadDynamicData(GEVulkanDriver* vk,
                                         GEVulkanCameraSceneNode* cam,
                                         VkCommandBuffer custom_cmd)
{
    if (!m_dynamic_data || m_cmds.empty())
        return;

    VkCommandBuffer cmd =
        custom_cmd ? custom_cmd : vk->getCurrentCommandBuffer();

    const VkPhysicalDeviceLimits& limit =
        vk->getPhysicalDeviceProperties().limits;

    size_t sbo_alignment = limit.minStorageBufferOffsetAlignment;
    size_t sbo_padding = getPadding(m_skinning_data_padded_size, sbo_alignment);
    if (sbo_padding != 0)
    {
        m_skinning_data_padded_size += sbo_padding;
        m_data_uploading.emplace_back((void*)m_data_padding, sbo_padding);
    }

    // https://github.com/google/filament/pull/3814
    // Need both vertex and fragment bit
    VkPipelineStageFlags dst_stage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    size_t ubo_alignment = limit.minUniformBufferOffsetAlignment;
    size_t ubo_padding = 0;
    const bool use_base_vertex = GEVulkanFeatures::supportsBaseVertexRendering();
    if (use_base_vertex)
    {
        const size_t object_data_size =
            sizeof(ObjectData) * m_visible_objects.size();
        m_data_uploading.emplace_back((void*)m_visible_objects.data(),
            object_data_size);
        ubo_padding = getPadding(
            m_skinning_data_padded_size + object_data_size, ubo_alignment);
        if (ubo_padding > 0)
            m_data_uploading.emplace_back((void*)m_data_padding, ubo_padding);
        m_object_data_padded_size = object_data_size + ubo_padding;
    }
    else
    {
        m_object_data_padded_size = 0;
        for (unsigned i = 0; i < m_cmds.size(); i++)
        {
            auto& cmd = m_cmds[i];
            size_t instance_size =
                cmd.m_cmd.instanceCount * sizeof(ObjectData);
            m_data_uploading.emplace_back(
                &m_visible_objects[cmd.m_cmd.firstInstance], instance_size);
            size_t cur_padding = getPadding(
                m_skinning_data_padded_size + m_object_data_padded_size +
                instance_size, sbo_alignment);
            if (cur_padding > 0)
            {
                instance_size += cur_padding;
                m_data_uploading.emplace_back((void*)m_data_padding,
                    cur_padding);
            }
            m_sbo_data_offset.push_back(m_object_data_padded_size);
            m_object_data_padded_size += instance_size;
        }
    }
    if (!use_base_vertex)
    {
        ubo_padding = getPadding(m_skinning_data_padded_size +
            m_object_data_padded_size, ubo_alignment);
        if (ubo_padding > 0)
        {
            m_data_uploading.emplace_back((void*)m_data_padding,
                ubo_padding);
            m_object_data_padded_size += ubo_padding;
        }
        // Make sure dynamic offset won't become invaild
        m_dynamic_data->resizeIfNeeded(m_skinning_data_padded_size +
            m_object_data_padded_size * 2 + sizeof(GEVulkanCameraUBO));
    }
    m_data_uploading.emplace_back(cam->getUBOData(), sizeof(GEVulkanCameraUBO));

    const bool use_multidraw = GEVulkanFeatures::supportsMultiDrawIndirect() &&
        GEVulkanFeatures::supportsBindMeshTexturesAtOnce();
    if (use_multidraw)
    {
        for (auto& cmd : m_cmds)
        {
            m_data_uploading.emplace_back(
                (void*)&cmd.m_cmd, sizeof(VkDrawIndexedIndirectCommand));
        }
        dst_stage |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
    }
    m_dynamic_data->setCurrentData(m_data_uploading, cmd);

    VkBufferMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    if (use_multidraw)
        barrier.dstAccessMask |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.buffer = m_dynamic_data->getCurrentBuffer();
    barrier.size = m_dynamic_data->getRealSize();

    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, dst_stage, 0, 0,
        NULL, 1, &barrier, 0, NULL);
}   // uploadDynamicData

// ----------------------------------------------------------------------------
void GEVulkanDrawCall::render(GEVulkanDriver* vk, GEVulkanCameraSceneNode* cam,
                              VkCommandBuffer custom_cmd)
{
    if (m_data_layout == VK_NULL_HANDLE || m_cmds.empty())
        return;

    VkCommandBuffer cmd =
        custom_cmd ? custom_cmd : vk->getCurrentCommandBuffer();
    const unsigned cur_frame = vk->getCurrentFrame();

    const bool use_base_vertex = GEVulkanFeatures::supportsBaseVertexRendering();
    VkDescriptorBufferInfo ubo_info;
    ubo_info.buffer = m_dynamic_data->getCurrentBuffer();
    ubo_info.offset = m_skinning_data_padded_size + m_object_data_padded_size;
    ubo_info.range = sizeof(GEVulkanCameraUBO);

    std::array<VkWriteDescriptorSet, 3> data_set = {};
    data_set[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    data_set[0].dstSet = m_data_descriptor_sets[cur_frame];
    data_set[0].dstBinding = 0;
    data_set[0].dstArrayElement = 0;
    data_set[0].descriptorType = use_base_vertex ?
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER :
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    data_set[0].descriptorCount = 1;
    data_set[0].pBufferInfo = &ubo_info;

    VkDescriptorBufferInfo sbo_info_objects;
    sbo_info_objects.buffer = m_dynamic_data->getCurrentBuffer();
    sbo_info_objects.offset = m_skinning_data_padded_size;
    sbo_info_objects.range = m_object_data_padded_size;

    data_set[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    data_set[1].dstSet = m_data_descriptor_sets[cur_frame];
    data_set[1].dstBinding = 1;
    data_set[1].dstArrayElement = 0;
    data_set[1].descriptorType = use_base_vertex ?
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER :
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    data_set[1].descriptorCount = 1;
    data_set[1].pBufferInfo = &sbo_info_objects;

    VkDescriptorBufferInfo sbo_info_skinning;
    sbo_info_skinning.buffer = m_dynamic_data->getCurrentBuffer();
    sbo_info_skinning.offset = 0;
    sbo_info_skinning.range = m_skinning_data_padded_size;

    data_set[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    data_set[2].dstSet = m_data_descriptor_sets[cur_frame];
    data_set[2].dstBinding = 2;
    data_set[2].dstArrayElement = 0;
    data_set[2].descriptorType = use_base_vertex ?
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER :
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC; ;
    data_set[2].descriptorCount = 1;
    data_set[2].pBufferInfo = &sbo_info_skinning;

    vkUpdateDescriptorSets(vk->getDevice(), data_set.size(), data_set.data(),
        0, NULL);

    m_texture_descriptor->updateDescriptor();

    if (GEVulkanFeatures::supportsBindMeshTexturesAtOnce())
    {
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pipeline_layout, 0, 1, m_texture_descriptor->getDescriptorSet(),
            0, NULL);
    }

    if (use_base_vertex)
    {
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pipeline_layout, 1, 1, &m_data_descriptor_sets[cur_frame], 0,
            NULL);
    }

    GEVulkanMeshCache* mc = vk->getVulkanMeshCache();
    std::array<VkBuffer, 2> vertex_buffer =
    {{
        mc->getBuffer(),
        mc->getBuffer()
    }};
    std::array<VkDeviceSize, 2> offsets =
    {{
        0,
        mc->getSkinningVBOOffset()
    }};
    vkCmdBindVertexBuffers(cmd, 0, vertex_buffer.size(), vertex_buffer.data(),
        offsets.data());

    vkCmdBindIndexBuffer(cmd, mc->getBuffer(), mc->getIBOOffset(),
        VK_INDEX_TYPE_UINT16);

    VkViewport vp;
    vp.x = cam->getViewPort().UpperLeftCorner.X;
    vp.y = cam->getViewPort().UpperLeftCorner.Y;
    vp.width = cam->getViewPort().getWidth();
    vp.height = cam->getViewPort().getHeight();
    vp.minDepth = 0;
    vp.maxDepth = 1.0f;
    vk->getRotatedViewport(&vp);
    vkCmdSetViewport(cmd, 0, 1, &vp);

    VkRect2D scissor;
    scissor.offset.x = vp.x;
    scissor.offset.y = vp.y;
    scissor.extent.width = vp.width;
    scissor.extent.height = vp.height;
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    const bool use_multidraw = GEVulkanFeatures::supportsMultiDrawIndirect() &&
        GEVulkanFeatures::supportsBindMeshTexturesAtOnce();

    std::string cur_pipeline = m_cmds[0].m_shader;
    if (use_multidraw)
    {
        size_t indirect_offset = m_skinning_data_padded_size +
            m_object_data_padded_size + sizeof(GEVulkanCameraUBO);
        const size_t indirect_size = sizeof(VkDrawIndexedIndirectCommand);
        unsigned draw_count = 0;
        for (unsigned i = 0; i < m_cmds.size(); i++)
        {
            if (m_cmds[i].m_shader != cur_pipeline)
            {
                bindPipeline(cmd, cur_pipeline);
                vkCmdDrawIndexedIndirect(cmd,
                    m_dynamic_data->getCurrentBuffer(), indirect_offset,
                    draw_count, indirect_size);
                indirect_offset += draw_count * indirect_size;
                draw_count = 1;
                cur_pipeline = m_cmds[i].m_shader;
                continue;
            }
            draw_count++;
        }
        bindPipeline(cmd, m_cmds.back().m_shader);
        vkCmdDrawIndexedIndirect(cmd,
            m_dynamic_data->getCurrentBuffer(), indirect_offset,
            draw_count, indirect_size);
    }
    else
    {
        int cur_mid = m_materials[m_cmds[0].m_cmd.vertexOffset];
        bindPipeline(cmd, cur_pipeline);
        if (!GEVulkanFeatures::supportsBindMeshTexturesAtOnce())
        {
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_pipeline_layout, 0, 1,
                &m_texture_descriptor->getDescriptorSet()[cur_mid], 0, NULL);
        }
        for (unsigned i = 0; i < m_cmds.size(); i++)
        {
            const VkDrawIndexedIndirectCommand& cur_cmd = m_cmds[i].m_cmd;
            int mid = m_materials[cur_cmd.vertexOffset];
            if (!GEVulkanFeatures::supportsBindMeshTexturesAtOnce() &&
                cur_mid != mid)
            {
                cur_mid = mid;
                vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    m_pipeline_layout, 0, 1,
                    &m_texture_descriptor->getDescriptorSet()[cur_mid], 0,
                    NULL);
            }
            if (m_cmds[i].m_shader != cur_pipeline)
            {
                cur_pipeline = m_cmds[i].m_shader;
                bindPipeline(cmd, cur_pipeline);
            }
            if (!use_base_vertex)
            {
                std::array<uint32_t, 3> dynamic_offsets =
                {{
                    0u,
                    uint32_t(m_sbo_data_offset[i]),
                    0u
                }};
                vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    m_pipeline_layout, 1, 1, &m_data_descriptor_sets[cur_frame],
                    dynamic_offsets.size(), dynamic_offsets.data());
            }
            vkCmdDrawIndexed(cmd, cur_cmd.indexCount, cur_cmd.instanceCount,
                cur_cmd.firstIndex, cur_cmd.vertexOffset,
                use_base_vertex ? cur_cmd.firstInstance : 0);
        }
    }
}   // render

}
