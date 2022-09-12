#ifndef HEADER_GE_VULKAN_DRAW_CALL_HPP
#define HEADER_GE_VULKAN_DRAW_CALL_HPP

#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "vulkan_wrapper.h"

#include "matrix4.h"
#include "SColor.h"

namespace irr
{
    namespace scene { class ISceneNode; }
}

namespace GE
{
class GECullingTool;
class GESPMBuffer;
class GEVulkanAnimatedMeshSceneNode;
class GEVulkanCameraSceneNode;
class GEVulkanDriver;
class GEVulkanDynamicBuffer;
class GEVulkanTextureDescriptor;

struct ObjectData
{
    float m_translation_x;
    float m_translation_y;
    float m_translation_z;
    float m_hue_change;
    float m_rotation[4];
    float m_scale_x;
    float m_scale_y;
    float m_scale_z;
    irr::video::SColor m_custom_vertex_color;
    int m_skinning_offset;
    int m_material_id;
    float m_texture_trans[2];
    // ------------------------------------------------------------------------
    ObjectData(irr::scene::ISceneNode* node, int material_id,
               int skinning_offset, int irrlicht_material_id);
};

struct PipelineSettings
{
    std::string m_vertex_shader;
    std::string m_skinning_vertex_shader;
    std::string m_fragment_shader;
    std::string m_shader_name;
    bool m_alphablend;
    bool m_additive;
    bool m_backface_culling;
    bool m_depth_test;
    bool m_depth_write;
    char m_drawing_priority;
    std::function<void(uint32_t*, void**)> m_push_constants_func;

    bool isTransparent() const { return m_alphablend || m_additive; }
};

struct DrawCallData
{
    VkDrawIndexedIndirectCommand m_cmd;
    std::string m_shader;
    std::string m_sorting_key;
    GESPMBuffer* m_mb;
    bool m_transparent;
};

class GEVulkanDrawCall
{
private:
    std::unordered_map<GESPMBuffer*, std::unordered_map<std::string,
        std::vector<std::pair<irr::scene::ISceneNode*, unsigned> > > >
        m_visible_nodes;

    GECullingTool* m_culling_tool;

    std::vector<DrawCallData> m_cmds;

    std::vector<ObjectData> m_visible_objects;

    GEVulkanDynamicBuffer* m_dynamic_data;

    size_t m_object_data_padded_size;

    size_t m_skinning_data_padded_size;

    char* m_data_padding;

    VkDescriptorSetLayout m_data_layout;

    VkDescriptorPool m_descriptor_pool;

    std::vector<VkDescriptorSet> m_data_descriptor_sets;

    VkPipelineLayout m_pipeline_layout;

    std::unordered_map<std::string, std::pair<VkPipeline, PipelineSettings> >
        m_graphics_pipelines;

    std::unordered_map<GESPMBuffer*, int> m_materials;

    GEVulkanTextureDescriptor* m_texture_descriptor;

    std::unordered_set<GEVulkanAnimatedMeshSceneNode*> m_skinning_nodes;

    std::vector<std::pair<void*, size_t> > m_data_uploading;

    std::vector<size_t> m_sbo_data_offset;

    // ------------------------------------------------------------------------
    void createAllPipelines(GEVulkanDriver* vk);
    // ------------------------------------------------------------------------
    void createPipeline(GEVulkanDriver* vk, const PipelineSettings& settings);
    // ------------------------------------------------------------------------
    void createVulkanData();
    // ------------------------------------------------------------------------
    std::string getShader(irr::scene::ISceneNode* node, int material_id);
    // ------------------------------------------------------------------------
    void bindPipeline(VkCommandBuffer cmd, const std::string& name) const
    {
        auto& ret = m_graphics_pipelines.at(name);
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ret.first);
        if (ret.second.m_push_constants_func)
        {
            uint32_t size;
            void* data;
            ret.second.m_push_constants_func(&size, &data);
            vkCmdPushConstants(cmd, m_pipeline_layout,
                VK_SHADER_STAGE_ALL_GRAPHICS, 0, size, data);
        }
    }
public:
    // ------------------------------------------------------------------------
    GEVulkanDrawCall();
    // ------------------------------------------------------------------------
    ~GEVulkanDrawCall();
    // ------------------------------------------------------------------------
    void addNode(irr::scene::ISceneNode* node);
    // ------------------------------------------------------------------------
    void prepare(GEVulkanCameraSceneNode* cam);
    // ------------------------------------------------------------------------
    void generate();
    // ------------------------------------------------------------------------
    void uploadDynamicData(GEVulkanDriver* vk, GEVulkanCameraSceneNode* cam,
                           VkCommandBuffer custom_cmd = VK_NULL_HANDLE);
    // ------------------------------------------------------------------------
    void render(GEVulkanDriver* vk, GEVulkanCameraSceneNode* cam,
                VkCommandBuffer custom_cmd = VK_NULL_HANDLE);
    // ------------------------------------------------------------------------
    unsigned getPolyCount() const
    {
        unsigned result = 0;
        for (auto& cmd : m_cmds)
            result += (cmd.m_cmd.indexCount / 3) * cmd.m_cmd.instanceCount;
        return result;
    }
    // ------------------------------------------------------------------------
    void reset()
    {
        m_visible_nodes.clear();
        m_cmds.clear();
        m_visible_objects.clear();
        m_materials.clear();
        m_skinning_nodes.clear();
        m_data_uploading.clear();
        m_sbo_data_offset.clear();
    }
};   // GEVulkanDrawCall

}

#endif
