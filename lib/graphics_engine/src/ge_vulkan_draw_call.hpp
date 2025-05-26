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

struct PipelineSettings
{
    std::string m_vertex_shader;
    std::string m_skinning_vertex_shader;
    std::string m_fragment_shader;
    std::string m_depth_only_fragment_shader;
    std::string m_shader_name;
    bool m_alphablend;
    bool m_additive;
    bool m_backface_culling;
    bool m_depth_test;
    bool m_depth_write;
    char m_drawing_priority;
    std::function<void(uint32_t*, void**)> m_push_constants_func;
    VkPipelineLayout m_custom_pl;
    VkCompareOp m_depth_op;
    VertexDescription m_vertex_description;

    bool isTransparent() const { return m_alphablend || m_additive; }
};


struct PipelineData
{
    PipelineSettings m_settings;
    std::shared_ptr<VkPipeline> m_pipeline, m_depth_only_pipeline;
};

struct DrawCallData
{
    VkDrawIndexedIndirectCommand m_cmd;
    std::string m_shader;
    std::string m_sorting_key;
    GESPMBuffer* m_mb;
    int m_material_id;
    bool m_transparent;
    uint32_t m_dynamic_offset;
};

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

    std::unordered_map<std::string, std::vector<char> > m_push_constants;

    VkPipelineLayout m_pipeline_layout, m_skybox_layout;

    std::unordered_map<std::string, PipelineData> m_graphics_pipelines;

    std::unordered_map<GEVulkanDynamicSPMBuffer*, int> m_dyspmb_materials;

    GEVulkanSkyBoxRenderer* m_skybox_renderer;

    GEVulkanTextureDescriptor* m_texture_descriptor;

    std::unordered_set<GEVulkanAnimatedMeshSceneNode*> m_skinning_nodes;

    std::unordered_map<std::string, std::pair<uint32_t, std::vector<int> > >
        m_materials_data;

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
                      bool depth_only, VkPipeline* prev_dp) const
    {
        auto& ret = m_graphics_pipelines.at(name);
        VkPipeline p = *ret.m_pipeline.get();
        if (depth_only)
        {
            if (!ret.m_depth_only_pipeline)
                return false;
            p = *ret.m_depth_only_pipeline.get();
            if (*prev_dp == p)
                return true;
            *prev_dp = p;
        }
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, p);
        auto it = m_push_constants.find(ret.m_settings.m_shader_name);
        if (it != m_push_constants.end())
        {
            vkCmdPushConstants(cmd, m_pipeline_layout,
                VK_SHADER_STAGE_ALL_GRAPHICS, 0, it->second.size(),
                it->second.data());
        }
        return true;
    }
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
        static PipelineSettings default_settings = {};
        const PipelineSettings* settings = &default_settings;
        auto it = m_graphics_pipelines.find(shader);
        if (it != m_graphics_pipelines.end())
            settings = &it->second.m_settings;
        return std::string(1, settings->isTransparent() ? (char)1 : (char)0) +
            std::string(1, settings->m_drawing_priority) + shader;
    }
    // ------------------------------------------------------------------------
    std::string getShaderFromKey(const std::string& key) const
                                                      { return key.substr(2); }
    // ------------------------------------------------------------------------
    void drawSkyBox(VkCommandBuffer cmd, int current_buffer_idx,
                    std::vector<uint32_t>& dynamic_offsets);
    // ------------------------------------------------------------------------
    void updatePushConstants()
    {
        for (auto& p : m_push_constants)
        {
            auto& f =
                m_graphics_pipelines.at(p.first).m_settings
                .m_push_constants_func;
            uint32_t size;
            void* data;
            f(&size, &data);
            p.second.resize(size);
            memcpy(p.second.data(), data, size);
        }
    }
    // ------------------------------------------------------------------------
    void bindSingleMaterial(VkCommandBuffer cmd,
                            const std::string& cur_pipeline,
                            int material_id, bool depth_only);
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
    bool doDepthOnlyRenderingFirst();
    // ------------------------------------------------------------------------
    VertexDescription getDefaultVertexDescription() const;
    // ------------------------------------------------------------------------
    size_t getLightDataOffset() const;
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
};   // GEVulkanDrawCall

}

#endif
