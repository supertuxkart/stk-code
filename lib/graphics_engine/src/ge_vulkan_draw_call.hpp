#ifndef HEADER_GE_VULKAN_DRAW_CALL_HPP
#define HEADER_GE_VULKAN_DRAW_CALL_HPP

#include <unordered_map>
#include <vector>

#include "vulkan_wrapper.h"

#include "matrix4.h"

namespace irr
{
    namespace scene { class ISceneNode; }
}

namespace GE
{
class GECullingTool;
class GESPMBuffer;
class GEVulkanCameraSceneNode;
class GEVulkanDriver;
class GEVulkanDynamicBuffer;
class GEVulkanTextureDescriptor;

struct ObjectData
{
    float m_mat_1[4];
    float m_mat_2[4];
    float m_mat_3[4];
    float m_mat_4[4];
    int m_skinning_offset;
    int m_material_id;
    float m_texture_trans[2];
    // ------------------------------------------------------------------------
    ObjectData(irr::scene::ISceneNode* node, int material_id);
};

class GEVulkanDrawCall
{
private:
    std::unordered_map<GESPMBuffer*, std::vector<irr::scene::ISceneNode*> > m_visible_nodes;

    GECullingTool* m_culling_tool;

    std::vector<VkDrawIndexedIndirectCommand> m_cmds;

    std::vector<ObjectData> m_visible_objects;

    GEVulkanDynamicBuffer* m_dynamic_data;

    size_t m_object_data_padded_size;

    char* m_object_data_padding;

    VkDescriptorSetLayout m_data_layout;

    VkDescriptorPool m_descriptor_pool;

    std::vector<VkDescriptorSet> m_data_descriptor_sets;

    VkPipelineLayout m_pipeline_layout;

    VkPipeline m_graphics_pipeline;

    std::vector<int> m_materials;

    GEVulkanTextureDescriptor* m_texture_descriptor;
    // ------------------------------------------------------------------------
    void createVulkanData();
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
    void uploadDynamicData(GEVulkanDriver* vk, GEVulkanCameraSceneNode* cam);
    // ------------------------------------------------------------------------
    void render(GEVulkanDriver* vk, GEVulkanCameraSceneNode* cam);
    // ------------------------------------------------------------------------
    unsigned getPolyCount() const
    {
        unsigned result = 0;
        for (auto& cmd : m_cmds)
            result += (cmd.indexCount / 3) * cmd.instanceCount;
        return result;
    }
};   // GEVulkanDrawCall

}

#endif
