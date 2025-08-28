#ifndef HEADER_GE_VULKAN_DRAW_CALL_HPP
#define HEADER_GE_VULKAN_DRAW_CALL_HPP

#include <array>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "vulkan_wrapper.h"

#include "matrix4.h"
#include "vector3d.h"
#include "ESceneNodeTypes.h"
#include "SColor.h"
#include "SMaterial.h"

#include "LinearMath/btQuaternion.h"

namespace irr
{
    namespace scene
    {
        class ISceneNode; class IBillboardSceneNode; struct SParticle;
        class IMesh; class ILightSceneNode;
    }
}

namespace GE
{
class GECullingTool;
class GESPMBuffer;
class GEVulkanAnimatedMeshSceneNode;
class GEVulkanCameraSceneNode;
class GEVulkanDriver;
class GEVulkanDynamicBuffer;
class GEVulkanDynamicSPMBuffer;
class GEVulkanLightHandler;
class GEVulkanSkyBoxRenderer;
class GEVulkanTextureDescriptor;

typedef std::pair<std::vector<VkVertexInputBindingDescription>,
    std::vector<VkVertexInputAttributeDescription> > VertexDescription;

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
    void init(irr::scene::ISceneNode* node, int material_id,
              int skinning_offset, int irrlicht_material_id);
    // ------------------------------------------------------------------------
    void init(irr::scene::IBillboardSceneNode* node, int material_id,
              const btQuaternion& rotation);
    // ------------------------------------------------------------------------
    void init(const irr::scene::SParticle& particle, int material_id,
              const btQuaternion& rotation,
              const irr::core::vector3df& view_position, bool flips,
              bool sky_particle, bool backface_culling);
};

enum GEVulkanPipelineType : unsigned
{
    GVPT_DEPTH = 1,
    GVPT_SOLID,
    GVPT_DEFERRED_LIGHTING,
    GVPT_DEFERRED_CONVERT_COLOR,
    GVPT_GHOST_DEPTH,
    GVPT_TRANSPARENT,
    GVPT_SKYBOX,
    GVPT_DISPLACE_MASK,
    GVPT_DISPLACE_COLOR,
};

struct GEMaterial;

struct PipelineSettings
{
    std::string m_shader_name;
    std::shared_ptr<const GEMaterial> m_material;
    char m_drawing_priority;
    VkPipelineLayout m_custom_pl;
    VkCompareOp m_depth_op;
    VkPrimitiveTopology m_topology;
    VertexDescription m_vertex_description;
    GEVulkanPipelineType m_pipeline_type;

    PipelineSettings();
    void loadMaterial(const GEMaterial& m);
};

struct PipelineData
{
    PipelineSettings m_settings;
    std::map<GEVulkanPipelineType, std::shared_ptr<VkPipeline> > m_pipelines;
};

struct DrawCallData
{
    VkDrawIndexedIndirectCommand m_cmd;
    std::string m_shader;
    std::string m_sorting_key;
    GESPMBuffer* m_mb;
    int m_material_id;
    uint32_t m_dynamic_offset;
};

class GEVulkanHiZDepth;
class GEVulkanDrawCall
{
private:
    typedef std::array<const irr::video::ITexture*,
        _IRR_MATERIAL_MAX_TEXTURES_> TexturesList;

    const int BILLBOARD_NODE = -1;

    const int PARTICLE_NODE = -2;

    std::map<TexturesList, GESPMBuffer*> m_billboard_buffers;

    irr::core::vector3df m_view_position;

    btQuaternion m_billboard_rotation;

    std::map<std::pair<GESPMBuffer*, TexturesList>, std::unordered_map<std::string,
        std::vector<std::pair<irr::scene::ISceneNode*, int> > > >
        m_visible_nodes;

    std::map<std::pair<GESPMBuffer*, TexturesList>, irr::scene::IMesh*> m_mb_map;

    std::map<std::string, std::vector<
        std::pair<GEVulkanDynamicSPMBuffer*, irr::scene::ISceneNode*> > >
        m_dynamic_spm_buffers;

    GECullingTool* m_culling_tool;

    GEVulkanLightHandler* m_light_handler;

    std::vector<DrawCallData> m_cmds;

    std::vector<ObjectData> m_visible_objects;

    GEVulkanDynamicBuffer* m_dynamic_data;

    GEVulkanDynamicBuffer* m_sbo_data;

    const VkPhysicalDeviceLimits& m_limits;

    size_t m_object_data_padded_size;

    size_t m_skinning_data_padded_size;

    size_t m_materials_padded_size;

    size_t m_dynamic_spm_padded_size;

    bool m_update_data_descriptor_sets;

    VkDescriptorSetLayout m_data_layout;

    VkDescriptorPool m_descriptor_pool;

    std::vector<VkDescriptorSet> m_data_descriptor_sets;

    VkPipelineLayout m_pipeline_layout, m_skybox_layout;

    std::vector<VkPipelineLayout> m_deferred_layouts;

    std::unordered_map<std::string, PipelineData> m_graphics_pipelines;

    std::unordered_map<GEVulkanDynamicSPMBuffer*, std::pair<int, size_t> > m_dyspmb_materials;

    GEVulkanSkyBoxRenderer* m_skybox_renderer;

    GEVulkanTextureDescriptor* m_texture_descriptor;

    std::unordered_set<GEVulkanAnimatedMeshSceneNode*> m_skinning_nodes;

    std::unordered_map<std::string, std::pair<uint32_t, std::vector<int> > >
        m_materials_data;

    GEVulkanHiZDepth* m_hiz_depth;

    // ------------------------------------------------------------------------
    void createAllPipelines(GEVulkanDriver* vk);
    // ------------------------------------------------------------------------
    void createPipeline(GEVulkanDriver* vk, const PipelineSettings& settings,
      std::unordered_map<std::string, std::shared_ptr<VkPipeline> >& dp_cache);
    // ------------------------------------------------------------------------
    void createVulkanData();
    // ------------------------------------------------------------------------
    std::string getShader(const irr::video::SMaterial& m);
    // ------------------------------------------------------------------------
    std::string getShader(irr::scene::ISceneNode* node, int material_id);
    // ------------------------------------------------------------------------
    bool bindPipeline(VkCommandBuffer cmd, const std::string& name,
                      VkPipeline* prev_pipeline,
                      GEVulkanPipelineType pt) const;
    // ------------------------------------------------------------------------
    TexturesList getTexturesList(const irr::video::SMaterial& m)
    {
        TexturesList textures;
        for (unsigned i = 0; i < textures.size(); i++)
            textures[i] = m.TextureLayer[i].Texture;
        return textures;
    }
    // ------------------------------------------------------------------------
    size_t getInitialSBOSize() const;
    // ------------------------------------------------------------------------
    void updateDataDescriptorSets(GEVulkanDriver* vk);
    // ------------------------------------------------------------------------
    void bindBaseVertex(GEVulkanDriver* vk, VkCommandBuffer cmd);
    // ------------------------------------------------------------------------
    std::string getDynamicBufferKey(const std::string& shader) const
    {
        char drawing_priority = (char)1;
        auto it = m_graphics_pipelines.find(shader);
        if (it != m_graphics_pipelines.end())
            drawing_priority = it->second.m_settings.m_drawing_priority;
        return std::string(1, drawing_priority) + shader;
    }
    // ------------------------------------------------------------------------
    std::string getShaderFromKey(const std::string& key) const
                                                      { return key.substr(1); }
    // ------------------------------------------------------------------------
    void bindSingleMaterial(VkCommandBuffer cmd,
                            const std::string& cur_pipeline,
                            int material_id, GEVulkanPipelineType pt);
    // ------------------------------------------------------------------------
    void bindDataDescriptor(VkCommandBuffer cmd, int current_buffer_idx,
                            std::vector<uint32_t>& dynamic_offsets)
    {
        vkCmdBindDescriptorSets(cmd,
            VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 1, 1,
            &m_data_descriptor_sets[current_buffer_idx],
            dynamic_offsets.size(), dynamic_offsets.data());
    }
    // ------------------------------------------------------------------------
    VertexDescription getDefaultVertexDescription() const;
    // ------------------------------------------------------------------------
    size_t getLightDataOffset() const;
    // ------------------------------------------------------------------------
    std::vector<uint32_t> getDefaultDynamicOffsets() const;
    // ------------------------------------------------------------------------
    VkRenderPass getRenderPassForPipelineCreation(GEVulkanDriver* vk,
                                                  GEVulkanPipelineType type);
    // ------------------------------------------------------------------------
    uint32_t getSubpassForPipelineCreation(GEVulkanDriver* vk,
                                           GEVulkanPipelineType type);

public:
    // ------------------------------------------------------------------------
    GEVulkanDrawCall();
    // ------------------------------------------------------------------------
    ~GEVulkanDrawCall();
    // ------------------------------------------------------------------------
    void addNode(irr::scene::ISceneNode* node);
    // ------------------------------------------------------------------------
    void addBillboardNode(irr::scene::ISceneNode* node,
                          irr::scene::ESCENE_NODE_TYPE node_type);
    // ------------------------------------------------------------------------
    void prepare(GEVulkanCameraSceneNode* cam);
    // ------------------------------------------------------------------------
    void generate(GEVulkanDriver* vk);
    // ------------------------------------------------------------------------
    void uploadDynamicData(GEVulkanDriver* vk, GEVulkanCameraSceneNode* cam,
                           VkCommandBuffer custom_cmd = VK_NULL_HANDLE);
    // ------------------------------------------------------------------------
    bool doDepthOnlyRenderingFirst();
    // ------------------------------------------------------------------------
    void bindAllMaterials(VkCommandBuffer cmd);
    // ------------------------------------------------------------------------
    void prepareRendering(GEVulkanDriver* vk);
    // ------------------------------------------------------------------------
    void prepareViewport(GEVulkanDriver* vk, GEVulkanCameraSceneNode* cam,
                         VkCommandBuffer cmd);
    // ------------------------------------------------------------------------
    void renderPipeline(GEVulkanDriver* vk, VkCommandBuffer cmd,
                        GEVulkanPipelineType pt, bool& rebind_base_vertex);
    // ------------------------------------------------------------------------
    bool renderSkyBox(GEVulkanDriver* vk, VkCommandBuffer cmd);
    // ------------------------------------------------------------------------
    void renderDeferredLighting(GEVulkanDriver* vk, VkCommandBuffer cmd);
    // ------------------------------------------------------------------------
    void renderDeferredConvertColor(GEVulkanDriver* vk, VkCommandBuffer cmd);
    // ------------------------------------------------------------------------
    void renderDisplaceColor(GEVulkanDriver* vk, VkCommandBuffer cmd,
                             VkBool32 has_displace);
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
        m_mb_map.clear();
        m_cmds.clear();
        m_visible_objects.clear();
        m_dyspmb_materials.clear();
        m_skinning_nodes.clear();
        m_materials_data.clear();
        m_dynamic_spm_buffers.clear();
        m_skybox_renderer = NULL;
    }
    // ------------------------------------------------------------------------
    void addSkyBox(irr::scene::ISceneNode* node);
    // ------------------------------------------------------------------------
    void addLightNode(irr::scene::ILightSceneNode* node);
    // ------------------------------------------------------------------------
    bool hasShaderForRendering(const std::string& shader)
    {
        const std::string& dbk = getDynamicBufferKey(shader);
        if (m_dynamic_spm_buffers.find(dbk) != m_dynamic_spm_buffers.end())
            return true;
        return m_materials_data.find(shader) != m_materials_data.end();
    }
    // ------------------------------------------------------------------------
    GEVulkanHiZDepth* getHiZDepth() const               { return m_hiz_depth; }
};   // GEVulkanDrawCall

}

#endif
