#include "ge_vulkan_draw_call.hpp"

#include "ge_culling_tool.hpp"
#include "ge_main.hpp"
#include "ge_render_info.hpp"
#include "ge_spm.hpp"
#include "ge_vulkan_animated_mesh_scene_node.hpp"
#include "ge_vulkan_billboard_buffer.hpp"
#include "ge_vulkan_camera_scene_node.hpp"
#include "ge_vulkan_driver.hpp"
#include "ge_vulkan_dynamic_buffer.hpp"
#include "ge_vulkan_dynamic_spm_buffer.hpp"
#include "ge_vulkan_fbo_texture.hpp"
#include "ge_vulkan_features.hpp"
#include "ge_vulkan_mesh_cache.hpp"
#include "ge_vulkan_mesh_scene_node.hpp"
#include "ge_vulkan_shader_manager.hpp"
#include "ge_vulkan_skybox_renderer.hpp"
#include "ge_vulkan_texture_descriptor.hpp"

#include "mini_glm.hpp"
#include "IBillboardSceneNode.h"
#include "IParticleSystemSceneNode.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

#include "../source/Irrlicht/os.h"
#include "quaternion.h"

namespace GE
{
// ============================================================================
void ObjectData::init(irr::scene::ISceneNode* node, int material_id,
                      int skinning_offset, int irrlicht_material_id)
{
    using namespace MiniGLM;
    const irr::core::matrix4& model_mat = node->getAbsoluteTransformation();
    irr::core::quaternion rotation(0.0f, 0.0f, 0.0f, 1.0f);
    irr::core::vector3df scale = model_mat.getScale();
    if (scale.X != 0.0f && scale.Y != 0.0f && scale.Z != 0.0f)
    {
        irr::core::matrix4 local_mat = model_mat;
        local_mat[0] = local_mat[0] / scale.X / local_mat[15];
        local_mat[1] = local_mat[1] / scale.X / local_mat[15];
        local_mat[2] = local_mat[2] / scale.X / local_mat[15];
        local_mat[4] = local_mat[4] / scale.Y / local_mat[15];
        local_mat[5] = local_mat[5] / scale.Y / local_mat[15];
        local_mat[6] = local_mat[6] / scale.Y / local_mat[15];
        local_mat[8] = local_mat[8] / scale.Z / local_mat[15];
        local_mat[9] = local_mat[9] / scale.Z / local_mat[15];
        local_mat[10] = local_mat[10] / scale.Z / local_mat[15];
        rotation = getQuaternion(local_mat);
        // Conjugated quaternion in glsl
        rotation.W = -rotation.W;
    }
    memcpy(&m_translation_x, &node->getAbsoluteTransformation()[12],
        sizeof(float) * 3);
    memcpy(m_rotation, &rotation, sizeof(irr::core::quaternion));
    memcpy(&m_scale_x, &scale, sizeof(irr::core::vector3df));
    m_skinning_offset = skinning_offset;
    m_material_id = material_id;
    const irr::core::matrix4& texture_matrix =
        node->getMaterial(irrlicht_material_id).getTextureMatrix(0);
    m_texture_trans[0] = texture_matrix[8];
    m_texture_trans[1] = texture_matrix[9];
    auto& ri = node->getMaterial(irrlicht_material_id).getRenderInfo();
    if (ri && ri->getHue() > 0.0f)
        m_hue_change = ri->getHue();
    else
        m_hue_change = 0.0f;
    if (ri)
        m_custom_vertex_color = ri->getVertexColor();
    else
        m_custom_vertex_color = irr::video::SColor((uint32_t)-1);
}   // init

// ============================================================================
void ObjectData::init(irr::scene::IBillboardSceneNode* node, int material_id,
                      const btQuaternion& rotation)
{
    memcpy(&m_translation_x, &node->getAbsoluteTransformation()[12],
        sizeof(float) * 3);
    memcpy(m_rotation, &rotation[0], sizeof(btQuaternion));
    irr::core::vector2df billboard_size = node->getSize();
    m_scale_x = billboard_size.X / 2.0f;
    m_scale_y = billboard_size.Y / 2.0f;
    m_scale_z = 0.0f;
    m_skinning_offset = 0;
    m_material_id = material_id;
    m_texture_trans[0] = 0.0f;
    m_texture_trans[1] = 0.0f;
    m_hue_change = 0.0f;
    // Only support average of them at the moment
    irr::video::SColor top, bottom, output;
    node->getColor(top, bottom);
    output.setAlpha((top.getAlpha() + bottom.getAlpha()) / 2);
    output.setRed((top.getRed() + bottom.getRed()) / 2);
    output.setGreen((top.getGreen() + bottom.getGreen()) / 2);
    output.setBlue((top.getBlue() + bottom.getBlue()) / 2);
    m_custom_vertex_color = output;
}   // init

// ============================================================================
std::vector<float> g_flips_data;
// ============================================================================
void ObjectData::init(const irr::scene::SParticle& particle, int material_id,
                      const btQuaternion& rotation,
                      const irr::core::vector3df& view_position, bool flips,
                      bool sky_particle, bool backface_culling)
{
    memcpy(&m_translation_x, &particle.pos, sizeof(float) * 3);
    float scale_x = particle.size.Width / 2.0f;
    if (flips)
    {
        // Following stk_particle.cpp
        const unsigned particle_index = particle.startTime;
        const float lifetime = particle.startSize.Width;
        const float pi = 3.14159265358979323846f;
        while (particle_index + 1 > g_flips_data.size())
        {
            // Maximum 3 rotation around axis (0, 1, 0) during lifetime
            g_flips_data.push_back(pi * 2.0f * 3.0f * os::Randomizer::frand() *
                (g_flips_data.size() % 2 == 0 ? 1.0f : -1.0f));
        }
        float angle = fmodf(lifetime * g_flips_data[particle_index],
            pi * 2.0f);
        btQuaternion rotated(btVector3(0.0f, 1.0f, 0.0f), angle);
        rotated = btQuaternion(rotation[0], rotation[1], rotation[2],
            -rotation[3]) * rotated;
        rotated.normalize();
        // Conjugated quaternion in glsl
        rotated[3] = -rotated[3];
        memcpy(m_rotation, &rotated[0], sizeof(btQuaternion));
        if (backface_culling)
        {
            irr::core::quaternion q(rotated[0], rotated[1], rotated[2],
                -rotated[3]);
            irr::core::matrix4 m;
            q.getMatrix(m, particle.pos);
            irr::core::vector3df tri[3] =
            {
                irr::core::vector3df( 1.0f, -1.0f, 0.0f),
                irr::core::vector3df( 1.0f,  1.0f, 0.0f),
                irr::core::vector3df(-1.0f,  1.0f, 0.0f)
            };
            m.transformVect(tri[0]);
            m.transformVect(tri[1]);
            m.transformVect(tri[2]);
            irr::core::vector3df normal = (tri[1] - tri[0])
                .crossProduct(tri[2] - tri[0]);
            float dot_product = (tri[0] - view_position).dotProduct(normal);
            if (dot_product < 0.0f)
                scale_x = -scale_x;
        }
    }
    else if (sky_particle)
    {
        irr::core::vector3df diff = particle.pos - view_position;
        float angle = atan2f(diff.X, diff.Z);
        btQuaternion rotated(btVector3(0.0f, 1.0f, 0.0f), angle);
        rotated.normalize();
        // Conjugated quaternion in glsl
        rotated[3] = -rotated[3];
        memcpy(m_rotation, &rotated[0], sizeof(btQuaternion));
    }
    else
        memcpy(m_rotation, &rotation[0], sizeof(btQuaternion));
    m_scale_x = scale_x;
    m_scale_y = particle.size.Height / 2.0f;
    m_scale_z = 0.0f;
    m_skinning_offset = 0;
    m_material_id = material_id;
    m_texture_trans[0] = 0.0f;
    m_texture_trans[1] = 0.0f;
    m_hue_change = 0.0f;
    m_custom_vertex_color = particle.color;
}   // init

// ----------------------------------------------------------------------------
GEVulkanDrawCall::GEVulkanDrawCall()
                : m_limits(getVKDriver()->getPhysicalDeviceProperties().limits)
{
    m_culling_tool = new GECullingTool;
    m_dynamic_data = NULL;
    m_sbo_data = NULL;
    m_object_data_padded_size = 0;
    m_skinning_data_padded_size = 0;
    m_materials_padded_size = 0;
    m_dynamic_spm_padded_size = 0;
    m_update_data_descriptor_sets = true;
    m_data_layout = VK_NULL_HANDLE;
    m_descriptor_pool = VK_NULL_HANDLE;
    m_pipeline_layout = VK_NULL_HANDLE;
    m_texture_descriptor = getVKDriver()->getMeshTextureDescriptor();
}   // GEVulkanDrawCall

// ----------------------------------------------------------------------------
GEVulkanDrawCall::~GEVulkanDrawCall()
{
    delete m_culling_tool;
    delete m_dynamic_data;
    delete m_sbo_data;
    for (auto& p : m_billboard_buffers)
       p.second->drop();
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
        if (buffer->getHardwareMappingHint_Vertex() == irr::scene::EHM_STREAM ||
            buffer->getHardwareMappingHint_Index() == irr::scene::EHM_STREAM)
        {
            GEVulkanDynamicSPMBuffer* dbuffer = static_cast<
                GEVulkanDynamicSPMBuffer*>(buffer);
            m_dynamic_spm_buffers[shader].emplace_back(dbuffer, node);
            continue;
        }
        m_visible_nodes[buffer][shader].emplace_back(node, i);
        m_mb_map[buffer] = mesh;
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
void GEVulkanDrawCall::addBillboardNode(irr::scene::ISceneNode* node,
                                        irr::scene::ESCENE_NODE_TYPE node_type)
{
    irr::core::aabbox3df bb = node->getTransformedBoundingBox();
    if (m_culling_tool->isCulled(bb))
        return;
    irr::video::SMaterial m = node->getMaterial(0);
    TexturesList textures = {};
    if (!GEVulkanFeatures::supportsDifferentTexturePerDraw() ||
        !GEVulkanFeatures::supportsBindMeshTexturesAtOnce())
        textures = getTexturesList(m);
    if (m_billboard_buffers.find(textures) == m_billboard_buffers.end())
        m_billboard_buffers[textures] = new GEVulkanBillboardBuffer(m);
    GESPMBuffer* buffer = m_billboard_buffers.at(textures);
    const std::string& shader = getShader(node, 0);
    m_visible_nodes[buffer][shader].emplace_back(node,
        node_type == irr::scene::ESNT_BILLBOARD ? BILLBOARD_NODE :
        PARTICLE_NODE);
}   // addBillboardNode

// ----------------------------------------------------------------------------
void GEVulkanDrawCall::generate(GEVulkanDriver* vk)
{
    if (!m_visible_nodes.empty() && m_data_layout == VK_NULL_HANDLE)
        createVulkanData();

    using Nodes = std::pair<GESPMBuffer*, std::unordered_map<
        std::string, std::vector<std::pair<irr::scene::ISceneNode*, int
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
    std::stable_sort(visible_nodes.begin(), visible_nodes.end(),
        [&nodes_area](const Nodes& a, const Nodes& b)
        {
            return nodes_area.at(a.first) < nodes_area.at(b.first) ;
        });

    size_t min_size = 0;
start:
    m_cmds.clear();
    m_materials.clear();
    m_materials_data.clear();

    m_update_data_descriptor_sets = m_sbo_data->resizeIfNeeded(min_size) ||
        m_update_data_descriptor_sets;
    int current_buffer_idx = vk->getCurrentBufferIdx();
    uint8_t* mapped_addr = (uint8_t*)m_sbo_data->getMappedAddr()
        [current_buffer_idx];

    std::unordered_map<irr::scene::ISceneNode*, int> skinning_offets;
    int added_joint = 1;
    size_t skinning_data_padded_size = sizeof(irr::core::matrix4);

    static irr::core::matrix4 identity = irr::core::matrix4();
    memcpy(mapped_addr, identity.pointer(), sizeof(irr::core::matrix4));
    size_t written_size = sizeof(irr::core::matrix4);
    mapped_addr += sizeof(irr::core::matrix4);

    for (GEVulkanAnimatedMeshSceneNode* node : m_skinning_nodes)
    {
        int bone_count = node->getSPM()->getJointCount();
        size_t bone_size = sizeof(irr::core::matrix4) * bone_count;
        if (written_size + bone_size > m_sbo_data->getSize())
        {
            min_size = (written_size + bone_size) * 2;
            goto start;
        }
        memcpy(mapped_addr, node->getSkinningMatrices().data(), bone_size);
        written_size += bone_size;
        mapped_addr += bone_size;
        skinning_offets[node] = added_joint;
        added_joint += bone_count;
        skinning_data_padded_size += bone_size;
    }

    size_t sbo_alignment = m_limits.minStorageBufferOffsetAlignment;
    if (skinning_data_padded_size > m_skinning_data_padded_size)
    {
        m_update_data_descriptor_sets = true;
        size_t skinning_padding = getPadding(skinning_data_padded_size,
            sbo_alignment);
        if (skinning_padding > 0)
        {
            if (written_size + skinning_padding > m_sbo_data->getSize())
            {
                min_size = (written_size + skinning_padding) * 2;
                goto start;
            }
            skinning_data_padded_size += skinning_padding;
            written_size += skinning_padding;
            mapped_addr += skinning_padding;
        }
        m_skinning_data_padded_size = skinning_data_padded_size;
    }
    else
    {
        size_t extra = m_skinning_data_padded_size - skinning_data_padded_size;
        if (written_size + extra > m_sbo_data->getSize())
        {
            min_size = (written_size + extra) * 2;
            goto start;
        }
        skinning_data_padded_size = m_skinning_data_padded_size;
        written_size += extra;
        mapped_addr += extra;
    }

    for (auto& p : m_dynamic_spm_buffers)
    {
        for (auto& q : p.second)
        {
            const size_t dynamic_spm_size = sizeof(ObjectData) +
                getPadding(sizeof(ObjectData), sbo_alignment);
            if (written_size + dynamic_spm_size > m_sbo_data->getSize())
            {
                min_size = (written_size + dynamic_spm_size) * 2;
                goto start;
            }
            irr::scene::ISceneNode* node = q.second;
            const irr::video::SMaterial& m = node->getMaterial(0);
            TexturesList textures = getTexturesList(m);
            const irr::video::ITexture** list = &textures[0];
            int material_id = m_texture_descriptor->getTextureID(list,
                getShader(m));
            ObjectData* data = (ObjectData*)mapped_addr;
            data->init(node, material_id, -1, 0);
            m_materials[q.first] = material_id;
            written_size += dynamic_spm_size;
            mapped_addr += dynamic_spm_size;
        }
    }
    m_dynamic_spm_padded_size = written_size - skinning_data_padded_size;

    const bool use_base_vertex = GEVulkanFeatures::supportsBaseVertexRendering();
    const bool bind_mesh_textures =
        GEVulkanFeatures::supportsBindMeshTexturesAtOnce();
    unsigned accumulated_instance = 0;

    struct InstanceKey
    {
        std::vector<irr::scene::ISceneNode*> m_nodes;
        unsigned m_instance_count;
        unsigned m_first_instance;
        bool m_hue_change;
    };
    std::unordered_map<irr::scene::IMesh*, std::vector<InstanceKey> >
        instance_keys;
    size_t instance_offset = 0;
    const uint8_t* instance_start_ptr = mapped_addr;
    std::unordered_map<uint32_t, uint32_t> offset_map;
    for (auto& p : visible_nodes)
    {
        const irr::video::SMaterial& m = p.first->getMaterial();
        TexturesList textures = getTexturesList(m);
        const irr::video::ITexture** list = &textures[0];
        int material_id = m_texture_descriptor->getTextureID(list,
            getShader(m));
        m_materials[p.first] = material_id;

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
            InstanceKey key;
            key.m_nodes.reserve(q.second.size());
            key.m_instance_count = visible_count;
            key.m_first_instance = accumulated_instance;
            key.m_hue_change = false;
            bool skip_instance_key = false;
            for (auto& r : q.second)
            {
                if (r.second == BILLBOARD_NODE || r.second == PARTICLE_NODE)
                {
                    skip_instance_key = true;
                    break;
                }
                irr::scene::ISceneNode* node = r.first;
                const irr::core::matrix4& texture_matrix =
                    node->getMaterial(r.second).getTextureMatrix(0);
                if (texture_matrix[8] != 0.0f || texture_matrix[9] != 0.0f)
                {
                    skip_instance_key = true;
                    break;
                }
                auto& ri = node->getMaterial(r.second).getRenderInfo();
                if (ri && ri->getHue() > 0.0f)
                {
                    key.m_hue_change = true;
                    break;
                }
                key.m_nodes.push_back(node);
            }
            irr::scene::IMesh* m = m_mb_map[p.first];
            auto& cur_key = instance_keys[m];
            auto it = cur_key.end();
            if (!skip_instance_key)
            {
                it = std::find_if(cur_key.begin(), cur_key.end(),
                    [key](const InstanceKey& k)
                    {
                        return k.m_nodes == key.m_nodes &&
                            k.m_instance_count == key.m_instance_count &&
                            k.m_hue_change == key.m_hue_change;
                    });
            }
            const PipelineSettings& settings =
                m_graphics_pipelines[cur_shader].second;
            for (auto& r : q.second)
            {
                irr::scene::ISceneNode* node = r.first;
                if (r.second == BILLBOARD_NODE || r.second == PARTICLE_NODE)
                {
                    if (GEVulkanFeatures::supportsDifferentTexturePerDraw())
                    {
                        const irr::video::SMaterial& m = node->getMaterial(0);
                        TexturesList textures = getTexturesList(m);
                        const irr::video::ITexture** list = &textures[0];
                        material_id = m_texture_descriptor->getTextureID(list,
                            getShader(m));
                    }
                    if (r.second == BILLBOARD_NODE)
                    {
                        if (written_size + sizeof(ObjectData) >
                            m_sbo_data->getSize())
                        {
                            min_size = (written_size + sizeof(ObjectData)) * 2;
                            goto start;
                        }
                        ObjectData* obj = (ObjectData*)mapped_addr;
                        obj->init(
                            static_cast<irr::scene::IBillboardSceneNode*>(
                            node), material_id, m_billboard_rotation);
                        written_size += sizeof(ObjectData);
                        mapped_addr += sizeof(ObjectData);
                    }
                    else
                    {
                        irr::scene::IParticleSystemSceneNode* pn =
                            static_cast<irr::scene::IParticleSystemSceneNode*>(
                            node);
                        const core::array<SParticle>& particles =
                            pn->getParticles();
                        unsigned ps = particles.size();
                        if (ps == 0)
                        {
                            visible_count--;
                            continue;
                        }
                        visible_count += ps - 1;
                        if (written_size + sizeof(ObjectData) * ps >
                            m_sbo_data->getSize())
                        {
                            min_size =
                                (written_size + sizeof(ObjectData) * ps) * 2;
                            goto start;
                        }
                        ObjectData* obj = (ObjectData*)mapped_addr;
                        bool flips = pn->getFlips();
                        bool sky_particle = pn->isSkyParticle();
                        for (unsigned i = 0; i < ps; i++)
                        {
                            obj[i].init(particles[i], material_id,
                                m_billboard_rotation, m_view_position, flips,
                                sky_particle, settings.m_backface_culling);
                            written_size += sizeof(ObjectData);
                            mapped_addr += sizeof(ObjectData);
                        }
                    }
                }
                else if (skip_instance_key || it == cur_key.end())
                {
                    int skinning_offset = -1000;
                    auto it = skinning_offets.find(node);
                    if (it != skinning_offets.end())
                        skinning_offset = it->second;
                    if (written_size + sizeof(ObjectData) >
                        m_sbo_data->getSize())
                    {
                        min_size = (written_size + sizeof(ObjectData)) * 2;
                        goto start;
                    }
                    ObjectData* obj = (ObjectData*)mapped_addr;
                    obj->init(node, bind_mesh_textures ? -1 : material_id,
                        skinning_offset, r.second);
                    written_size += sizeof(ObjectData);
                    mapped_addr += sizeof(ObjectData);
                }
            }
            VkDrawIndexedIndirectCommand cmd;
            cmd.indexCount = p.first->getIndexCount();
            cmd.instanceCount = visible_count;
            cmd.firstIndex = use_base_vertex ? p.first->getIBOOffset() : 0;
            cmd.vertexOffset = use_base_vertex ? p.first->getVBOOffset() : 0;
            if (skip_instance_key || it == cur_key.end())
            {
                cmd.firstInstance = accumulated_instance;
                if (!use_base_vertex)
                {
                    offset_map[accumulated_instance] = instance_offset;
                    size_t instance_padding = getPadding(written_size,
                        sbo_alignment);
                    if (instance_padding > 0)
                    {
                        if (written_size + instance_padding >
                            m_sbo_data->getSize())
                        {
                            min_size = (written_size + instance_padding) * 2;
                            goto start;
                        }
                        written_size += instance_padding;
                        mapped_addr += instance_padding;
                    }
                    instance_offset = mapped_addr - instance_start_ptr;
                }
                accumulated_instance += visible_count;
            }
            else
                cmd.firstInstance = it->m_first_instance;
            std::string sorting_key =
                std::string(1, settings.m_drawing_priority) + cur_shader;
            m_cmds.push_back({ cmd, cur_shader, sorting_key, p.first,
                settings.isTransparent(), offset_map[cmd.firstInstance] });
            if (!skip_instance_key && it == cur_key.end())
                 cur_key.push_back(key);
        }
    }
    if (!bind_mesh_textures)
    {
        std::stable_sort(m_cmds.begin(), m_cmds.end(),
            [this](const DrawCallData& a, const DrawCallData& b)
            {
                return m_materials[a.m_mb] < m_materials[b.m_mb];
            });
    }

    std::stable_sort(m_cmds.begin(), m_cmds.end(),
        [](const DrawCallData& a, const DrawCallData& b)
        {
            return a.m_sorting_key < b.m_sorting_key;
        });

    std::stable_partition(m_cmds.begin(), m_cmds.end(),
        [](const DrawCallData& a)
        {
            return !a.m_transparent;
        });

    size_t object_data_padded_size = written_size - skinning_data_padded_size;
    if (object_data_padded_size > m_object_data_padded_size)
    {
        m_update_data_descriptor_sets = true;
        size_t object_padding = getPadding(written_size, sbo_alignment);
        if (object_padding > 0)
        {
            if (written_size + object_padding > m_sbo_data->getSize())
            {
                min_size = (written_size + object_padding) * 2;
                goto start;
            }
            object_data_padded_size += object_padding;
            written_size += object_padding;
            mapped_addr += object_padding;
        }
        m_object_data_padded_size = object_data_padded_size;
    }
    else
    {
        size_t extra = m_object_data_padded_size - object_data_padded_size;
        if (written_size + extra > m_sbo_data->getSize())
        {
            min_size = (written_size + extra) * 2;
            goto start;
        }
        object_data_padded_size = m_object_data_padded_size;
        written_size += extra;
        mapped_addr += extra;
    }

    size_t materials_padded_size = 0;
    if (bind_mesh_textures && !m_cmds.empty())
    {
        std::string cur_shader = m_cmds[0].m_shader;
        for (unsigned i = 0; i < m_cmds.size(); i++)
        {
            auto& cmd = m_cmds[i];
            auto& material = m_materials_data[cur_shader];
            if (cmd.m_shader != cur_shader)
            {
                size_t material_size = material.second.size() * sizeof(int);
                if (written_size + material_size > m_sbo_data->getSize())
                {
                    min_size = (written_size + material_size) * 2;
                    goto start;
                }
                memcpy(mapped_addr, material.second.data(), material_size);
                written_size += material_size;
                mapped_addr += material_size;
                size_t cur_padding = getPadding(written_size, sbo_alignment);
                if (cur_padding > 0)
                {
                    if (written_size + cur_padding > m_sbo_data->getSize())
                    {
                        min_size = (written_size + cur_padding) * 2;
                        goto start;
                    }
                    written_size += cur_padding;
                    mapped_addr += cur_padding;
                    material_size += cur_padding;
                }
                material.first = materials_padded_size;
                materials_padded_size += material_size;
                cur_shader = cmd.m_shader;
            }
            m_materials_data[cmd.m_shader].second
                .push_back(m_materials[cmd.m_mb]);
        }

        auto& material = m_materials_data[m_cmds.back().m_shader];
        size_t material_size = material.second.size() * sizeof(int);
        if (written_size + material_size > m_sbo_data->getSize())
        {
            min_size = (written_size + material_size) * 2;
            goto start;
        }
        memcpy(mapped_addr, material.second.data(), material_size);
        written_size += material_size;
        mapped_addr += material_size;
        size_t cur_padding = getPadding(written_size, sbo_alignment);
        if (cur_padding > 0)
        {
            if (written_size + cur_padding > m_sbo_data->getSize())
            {
                min_size = (written_size + cur_padding) * 2;
                goto start;
            }
            written_size += cur_padding;
            mapped_addr += cur_padding;
            material_size += cur_padding;
        }
        material.first = materials_padded_size;
        materials_padded_size += material_size;
        if (materials_padded_size > m_materials_padded_size)
        {
            m_update_data_descriptor_sets = true;
            m_materials_padded_size = materials_padded_size;
        }

        // Make sure dynamic offsets won't become invalid
        if (skinning_data_padded_size + (object_data_padded_size * 2) +
            (materials_padded_size * 2) > m_sbo_data->getSize())
        {
            min_size = skinning_data_padded_size +
                (object_data_padded_size * 2) + (materials_padded_size * 2);
            goto start;
        }
    }
    else
    {
        // Make sure dynamic offset of objects won't become invalid
        if (skinning_data_padded_size + (object_data_padded_size * 2) >
            m_sbo_data->getSize())
        {
            min_size = skinning_data_padded_size +
                (object_data_padded_size * 2);
            goto start;
        }
    }
}   // generate

// ----------------------------------------------------------------------------
std::string GEVulkanDrawCall::getShader(irr::scene::ISceneNode* node,
                                        int material_id)
{
    irr::video::SMaterial& m = node->getMaterial(material_id);
    return getShader(m);
}   // getShader

// ----------------------------------------------------------------------------
std::string GEVulkanDrawCall::getShader(const irr::video::SMaterial& m)
{
    std::string shader;
    switch (m.MaterialType)
    {
    case irr::video::EMT_TRANSPARENT_ADD_COLOR:
        shader = "additive";
        break;
    case irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF:
        shader = "alphatest";
        break;
    case irr::video::EMT_ONETEXTURE_BLEND:
        shader = "alphablend";
        break;
    case irr::video::EMT_SOLID_2_LAYER:
        shader = "decal";
        break;
    case irr::video::EMT_STK_GRASS:
        shader = "grass";
        break;
    default:
        shader = "solid";
        break;
    }
    auto& ri = m.getRenderInfo();
    // Use real transparent shader first
    if (m.MaterialType != irr::video::EMT_TRANSPARENT_ADD_COLOR &&
        m.MaterialType != irr::video::EMT_ONETEXTURE_BLEND &&
        ri && ri->isTransparent())
        return "ghost";
    return shader;
}   // getShader

// ----------------------------------------------------------------------------
void GEVulkanDrawCall::prepare(GEVulkanCameraSceneNode* cam)
{
    reset();
    m_culling_tool->init(cam);
    m_view_position = cam->getPosition();
    m_billboard_rotation = MiniGLM::getBulletQuaternion(cam->getViewMatrix());
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
    settings.m_drawing_priority = (char)11;
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

    // m_data_layout
    VkDescriptorSetLayoutBinding camera_layout_binding = {};
    camera_layout_binding.binding = 0;
    camera_layout_binding.descriptorCount = 1;
    camera_layout_binding.descriptorType =
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    camera_layout_binding.pImmutableSamplers = NULL;
    camera_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding object_data_layout_binding = {};
    object_data_layout_binding.binding = 1;
    object_data_layout_binding.descriptorCount = 1;
    object_data_layout_binding.descriptorType =
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    object_data_layout_binding.pImmutableSamplers = NULL;
    object_data_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding skinning_layout_binding = {};
    skinning_layout_binding.binding = 2;
    skinning_layout_binding.descriptorCount = 1;
    skinning_layout_binding.descriptorType =
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    skinning_layout_binding.pImmutableSamplers = NULL;
    skinning_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    std::vector<VkDescriptorSetLayoutBinding> bindings =
    {
         camera_layout_binding,
         object_data_layout_binding,
         skinning_layout_binding
    };
    if (GEVulkanFeatures::supportsBindMeshTexturesAtOnce())
    {
        VkDescriptorSetLayoutBinding material_binding = {};
        material_binding.binding = 3;
        material_binding.descriptorCount = 1;
        material_binding.descriptorType =
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        material_binding.pImmutableSamplers = NULL;
        material_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        bindings.push_back(material_binding);
    }

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
    std::vector<VkDescriptorPoolSize> sizes =
    {
        {
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            vk->getMaxFrameInFlight() + 1
        },
        {
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
            (vk->getMaxFrameInFlight() + 1) * 2
        }
    };
    if (GEVulkanFeatures::supportsBindMeshTexturesAtOnce())
        sizes.push_back(sizes.back());

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = 0;
    pool_info.maxSets = vk->getMaxFrameInFlight() + 1;
    pool_info.poolSizeCount = sizes.size();
    pool_info.pPoolSizes = sizes.data();

    if (vkCreateDescriptorPool(vk->getDevice(), &pool_info, NULL,
        &m_descriptor_pool) != VK_SUCCESS)
        throw std::runtime_error("createDescriptorPool failed");

    // m_data_descriptor_sets
    unsigned set_size = vk->getMaxFrameInFlight() + 1;
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

    size_t extra_size = 0;
    const bool use_multidraw =
        GEVulkanFeatures::supportsBindMeshTexturesAtOnce();
    VkBufferUsageFlags flags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if (use_multidraw)
    {
        flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
        extra_size = 200 * sizeof(VkDrawIndexedIndirectCommand);
    }
    // Use VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
    // or a staging buffer when buffer is small
    m_dynamic_data = new GEVulkanDynamicBuffer(flags,
        extra_size + sizeof(GEVulkanCameraUBO),
        GEVulkanDriver::getMaxFrameInFlight() + 1,
        GEVulkanDynamicBuffer::supportsHostTransfer() ? 0 :
        GEVulkanDriver::getMaxFrameInFlight() + 1);

    flags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    // Using VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
    // will be a lot slower when there are many objects (like particles)
    m_sbo_data = new GEVulkanDynamicBuffer(flags, getInitialSBOSize(),
        GEVulkanDriver::getMaxFrameInFlight() + 1, 0,
        false/*enable_host_transfer*/);
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

    // https://github.com/google/filament/pull/3814
    // Need both vertex and fragment bit
    VkPipelineStageFlags dst_stage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    std::vector<std::pair<void*, size_t> > data_uploading;
    data_uploading.emplace_back((void*)cam->getUBOData(),
        sizeof(GEVulkanCameraUBO));

    const bool use_multidraw =
        GEVulkanFeatures::supportsBindMeshTexturesAtOnce();
    if (use_multidraw)
    {
        for (auto& cmd : m_cmds)
        {
            data_uploading.emplace_back(
                (void*)&cmd.m_cmd, sizeof(VkDrawIndexedIndirectCommand));
        }
        dst_stage |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
    }
    int current_buffer_idx = vk->getCurrentBufferIdx();
    m_update_data_descriptor_sets =
        m_dynamic_data->setCurrentData(data_uploading, cmd,
        current_buffer_idx) || m_update_data_descriptor_sets;

    const size_t whole_size = m_skinning_data_padded_size +
        m_object_data_padded_size + m_materials_padded_size;
    vmaFlushAllocation(vk->getVmaAllocator(),
        m_sbo_data->getHostMemory()[current_buffer_idx], 0,
        whole_size);

    if (!GEVulkanDynamicBuffer::supportsHostTransfer())
    {
        VkMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        if (use_multidraw)
            barrier.dstAccessMask |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;

        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, dst_stage, 0,
            1, &barrier, 0, NULL, 0, NULL);
    }
}   // uploadDynamicData

// ----------------------------------------------------------------------------
void GEVulkanDrawCall::bindBaseVertex(GEVulkanDriver* vk, VkCommandBuffer cmd)
{
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
    vkCmdBindVertexBuffers(cmd, 0, vertex_buffer.size(),
        vertex_buffer.data(), offsets.data());

    vkCmdBindIndexBuffer(cmd, mc->getBuffer(), mc->getIBOOffset(),
        VK_INDEX_TYPE_UINT16);
}   // bindBaseVertex

// ----------------------------------------------------------------------------
void GEVulkanDrawCall::render(GEVulkanDriver* vk, GEVulkanCameraSceneNode* cam,
                              VkCommandBuffer custom_cmd)
{
    if (m_data_layout == VK_NULL_HANDLE || m_cmds.empty())
        return;

    VkCommandBuffer cmd =
        custom_cmd ? custom_cmd : vk->getCurrentCommandBuffer();
    int current_buffer_idx = vk->getCurrentBufferIdx();

    const bool use_base_vertex = GEVulkanFeatures::supportsBaseVertexRendering();
    const bool bind_mesh_textures = GEVulkanFeatures::supportsBindMeshTexturesAtOnce();

    updateDataDescriptorSets(vk);
    m_texture_descriptor->updateDescriptor();

    VkViewport vp;
    float scale = getGEConfig()->m_render_scale;
    if (vk->getSeparateRTTTexture())
        scale = 1.0f;
    vp.x = cam->getViewPort().UpperLeftCorner.X * scale;
    vp.y = cam->getViewPort().UpperLeftCorner.Y * scale;
    vp.width = cam->getViewPort().getWidth() * scale;
    vp.height = cam->getViewPort().getHeight() * scale;
    vp.minDepth = 0;
    vp.maxDepth = 1.0f;
    vk->getRotatedViewport(&vp, true/*handle_rtt*/);
    vkCmdSetViewport(cmd, 0, 1, &vp);

    VkRect2D scissor;
    scissor.offset.x = vp.x;
    scissor.offset.y = vp.y;
    scissor.extent.width = vp.width;
    scissor.extent.height = vp.height;
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    std::string cur_pipeline = m_cmds[0].m_shader;
    bool drawn_skybox = false;
    size_t sbo_alignment = m_limits.minStorageBufferOffsetAlignment;
    const size_t dynamic_spm_size = sizeof(ObjectData) + getPadding(
        sizeof(ObjectData), sbo_alignment);
    size_t dynamic_spm_offset = 0;
    if (bind_mesh_textures)
    {
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pipeline_layout, 0, 1, m_texture_descriptor->getDescriptorSet(),
            0, NULL);

        std::array<uint32_t, 4> dynamic_offsets =
        {{
            0u,
            0u,
            0u,
            0u,
        }};
        size_t indirect_offset = sizeof(GEVulkanCameraUBO);
        const size_t indirect_size = sizeof(VkDrawIndexedIndirectCommand);
        unsigned draw_count = 0;
        VkBuffer indirect_buffer =
            GEVulkanDynamicBuffer::supportsHostTransfer() ?
            m_dynamic_data->getHostBuffer()[current_buffer_idx] :
            m_dynamic_data->getLocalBuffer()[current_buffer_idx];
        bool rebind_base_vertex = true;
        for (unsigned i = 0; i < m_cmds.size(); i++)
        {
            if (m_cmds[i].m_shader != cur_pipeline)
            {
                bindPipeline(cmd, cur_pipeline);
                auto it = m_dynamic_spm_buffers.find(cur_pipeline);
                if (it != m_dynamic_spm_buffers.end() && !it->second.empty())
                {
                    rebind_base_vertex = true;
                    for (auto& buf : it->second)
                    {
                        dynamic_offsets[1] = dynamic_spm_offset;
                        vkCmdBindDescriptorSets(cmd,
                            VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout,
                            1, 1, &m_data_descriptor_sets[current_buffer_idx],
                            dynamic_offsets.size(), dynamic_offsets.data());
                        buf.first->drawDynamicVertexIndexBuffer(cmd,
                            current_buffer_idx);
                        dynamic_spm_offset += dynamic_spm_size;
                    }
                }
                if (rebind_base_vertex)
                {
                    bindBaseVertex(vk, cmd);
                    rebind_base_vertex = false;
                }
                dynamic_offsets[1] = m_dynamic_spm_padded_size;
                dynamic_offsets[3] = m_materials_data[cur_pipeline].first;
                vkCmdBindDescriptorSets(cmd,
                    VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 1, 1,
                    &m_data_descriptor_sets[current_buffer_idx],
                    dynamic_offsets.size(), dynamic_offsets.data());
                vkCmdDrawIndexedIndirect(cmd, indirect_buffer, indirect_offset,
                    draw_count, indirect_size);
                indirect_offset += draw_count * indirect_size;
                draw_count = 1;
                cur_pipeline = m_cmds[i].m_shader;

                if (m_cmds[i].m_transparent && !drawn_skybox)
                {
                    drawn_skybox = true;
                    GEVulkanSkyBoxRenderer::render(cmd, cam);

                    vkCmdBindDescriptorSets(cmd,
                        VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 0,
                        1, m_texture_descriptor->getDescriptorSet(), 0, NULL);
                }
                continue;
            }
            draw_count++;
        }
        bindPipeline(cmd, m_cmds.back().m_shader);
        auto it = m_dynamic_spm_buffers.find(m_cmds.back().m_shader);
        if (it != m_dynamic_spm_buffers.end() && !it->second.empty())
        {
            rebind_base_vertex = true;
            for (auto& buf : it->second)
            {
                dynamic_offsets[1] = dynamic_spm_offset;
                vkCmdBindDescriptorSets(cmd,
                    VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 1, 1,
                    &m_data_descriptor_sets[current_buffer_idx],
                    dynamic_offsets.size(), dynamic_offsets.data());
                buf.first->drawDynamicVertexIndexBuffer(cmd,
                    current_buffer_idx);
                dynamic_spm_offset += dynamic_spm_size;
            }
        }
        if (rebind_base_vertex)
            bindBaseVertex(vk, cmd);
        dynamic_offsets[1] = m_dynamic_spm_padded_size;
        dynamic_offsets[3] = m_materials_data[m_cmds.back().m_shader].first;
        vkCmdBindDescriptorSets(cmd,
            VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 1, 1,
            &m_data_descriptor_sets[current_buffer_idx],
            dynamic_offsets.size(), dynamic_offsets.data());
        vkCmdDrawIndexedIndirect(cmd, indirect_buffer, indirect_offset,
            draw_count, indirect_size);
    }
    else
    {
        std::array<uint32_t, 3> dynamic_offsets =
        {{
            0u,
            0u,
            0u,
        }};
        bool rebind_base_vertex = true;
        int cur_mid = -1;
        bindPipeline(cmd, cur_pipeline);
        auto it = m_dynamic_spm_buffers.find(cur_pipeline);
        if (it != m_dynamic_spm_buffers.end() && !it->second.empty())
        {
            rebind_base_vertex = true;
            for (auto& buf : it->second)
            {
                dynamic_offsets[1] = dynamic_spm_offset;
                vkCmdBindDescriptorSets(cmd,
                    VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout,
                    1, 1, &m_data_descriptor_sets[current_buffer_idx],
                    dynamic_offsets.size(), dynamic_offsets.data());
                int dy_mat = m_materials[buf.first];
                if (dy_mat != cur_mid)
                {
                    cur_mid = dy_mat;
                    vkCmdBindDescriptorSets(cmd,
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        m_pipeline_layout, 0, 1,
                        &m_texture_descriptor->getDescriptorSet()[cur_mid], 0,
                        NULL);
                }
                buf.first->drawDynamicVertexIndexBuffer(cmd,
                    current_buffer_idx);
                dynamic_spm_offset += dynamic_spm_size;
            }
        }
        if (cur_mid != m_materials[m_cmds[0].m_mb])
        {
            cur_mid = m_materials[m_cmds[0].m_mb];
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_pipeline_layout, 0, 1,
                &m_texture_descriptor->getDescriptorSet()[cur_mid], 0, NULL);
        }
        for (unsigned i = 0; i < m_cmds.size(); i++)
        {
            const VkDrawIndexedIndirectCommand& cur_cmd = m_cmds[i].m_cmd;
            if (m_cmds[i].m_transparent && !drawn_skybox)
            {
                drawn_skybox = true;
                GEVulkanSkyBoxRenderer::render(cmd, cam);

                vkCmdBindDescriptorSets(cmd,
                    VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 0, 1,
                    &m_texture_descriptor->getDescriptorSet()[cur_mid], 0,
                    NULL);
                if (use_base_vertex)
                    rebind_base_vertex = true;
            }
            if (m_cmds[i].m_shader != cur_pipeline)
            {
                cur_pipeline = m_cmds[i].m_shader;
                bindPipeline(cmd, cur_pipeline);
                auto it = m_dynamic_spm_buffers.find(cur_pipeline);
                if (it != m_dynamic_spm_buffers.end() && !it->second.empty())
                {
                    rebind_base_vertex = true;
                    for (auto& buf : it->second)
                    {
                        dynamic_offsets[1] = dynamic_spm_offset;
                        vkCmdBindDescriptorSets(cmd,
                            VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout,
                            1, 1, &m_data_descriptor_sets[current_buffer_idx],
                            dynamic_offsets.size(), dynamic_offsets.data());
                        int dy_mat = m_materials[buf.first];
                        if (dy_mat != cur_mid)
                        {
                            cur_mid = dy_mat;
                            vkCmdBindDescriptorSets(cmd,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_pipeline_layout, 0, 1,
                                &m_texture_descriptor->getDescriptorSet()
                                [cur_mid], 0, NULL);
                        }
                        buf.first->drawDynamicVertexIndexBuffer(cmd,
                            current_buffer_idx);
                        dynamic_spm_offset += dynamic_spm_size;
                    }
                }
            }
            int mid = m_materials[m_cmds[i].m_mb];
            if (cur_mid != mid)
            {
                cur_mid = mid;
                vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    m_pipeline_layout, 0, 1,
                    &m_texture_descriptor->getDescriptorSet()[cur_mid], 0,
                    NULL);
            }
            if (use_base_vertex && rebind_base_vertex)
            {
                bindBaseVertex(vk, cmd);
                rebind_base_vertex = false;
                dynamic_offsets[1] = m_dynamic_spm_padded_size;
                vkCmdBindDescriptorSets(cmd,
                    VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 1, 1,
                    &m_data_descriptor_sets[current_buffer_idx],
                    dynamic_offsets.size(), dynamic_offsets.data());
            }
            if (!use_base_vertex)
            {
                dynamic_offsets[1] = m_dynamic_spm_padded_size +
                    m_cmds[i].m_dynamic_offset;
                vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    m_pipeline_layout, 1, 1,
                    &m_data_descriptor_sets[current_buffer_idx],
                    dynamic_offsets.size(), dynamic_offsets.data());
                m_cmds[i].m_mb->bindVertexIndexBuffer(cmd);
            }
            vkCmdDrawIndexed(cmd, cur_cmd.indexCount, cur_cmd.instanceCount,
                cur_cmd.firstIndex, cur_cmd.vertexOffset,
                use_base_vertex ? cur_cmd.firstInstance : 0);
        }
    }
    if (!drawn_skybox)
        GEVulkanSkyBoxRenderer::render(cmd, cam);
}   // render

// ----------------------------------------------------------------------------
size_t GEVulkanDrawCall::getInitialSBOSize() const
{
    // Assume 50 bones per node
    size_t ret = m_skinning_nodes.size() * 50 * sizeof(irr::core::matrix4);
    const bool use_base_vertex =
        GEVulkanFeatures::supportsBaseVertexRendering();
    for (auto& p : m_visible_nodes)
    {
        for (auto& q : p.second)
        {
            unsigned visible_count = q.second.size();
            if (visible_count == 0)
                continue;
            for (auto& r : q.second)
            {
                if (r.second == PARTICLE_NODE)
                {
                    irr::scene::IParticleSystemSceneNode* pn =
                        static_cast<irr::scene::IParticleSystemSceneNode*>(
                        r.first);
                    const core::array<SParticle>& particles =
                        pn->getParticles();
                    unsigned ps = particles.size();
                    if (ps == 0)
                    {
                        visible_count--;
                        continue;
                    }
                    visible_count += ps - 1;
                }
            }
            if (!use_base_vertex)
                visible_count *= 2;
            ret += visible_count * sizeof(ObjectData);
        }
    }
    return ret * 2;
}   // getInitialSBOSize

// ----------------------------------------------------------------------------
void GEVulkanDrawCall::updateDataDescriptorSets(GEVulkanDriver* vk)
{
    if (!m_update_data_descriptor_sets || m_skinning_data_padded_size == 0 ||
        m_object_data_padded_size == 0)
        return;

    m_update_data_descriptor_sets = false;
    vk->waitIdle();

    const bool use_base_vertex =
        GEVulkanFeatures::supportsBaseVertexRendering();
    const bool bind_mesh_textures =
        GEVulkanFeatures::supportsBindMeshTexturesAtOnce();
    for (unsigned i = 0; i < m_data_descriptor_sets.size(); i++)
    {
        VkDescriptorBufferInfo ubo_info;
        ubo_info.buffer = GEVulkanDynamicBuffer::supportsHostTransfer() ?
            m_dynamic_data->getHostBuffer()[i] :
            m_dynamic_data->getLocalBuffer()[i];
        ubo_info.offset = 0;
        ubo_info.range = sizeof(GEVulkanCameraUBO);

        std::vector<VkWriteDescriptorSet> data_set;
        data_set.resize(3, {});
        data_set[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        data_set[0].dstSet = m_data_descriptor_sets[i];
        data_set[0].dstBinding = 0;
        data_set[0].dstArrayElement = 0;
        data_set[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        data_set[0].descriptorCount = 1;
        data_set[0].pBufferInfo = &ubo_info;

        VkDescriptorBufferInfo sbo_info_objects;
        sbo_info_objects.buffer = m_sbo_data->getHostBuffer()[i];
        sbo_info_objects.offset = m_skinning_data_padded_size;
        sbo_info_objects.range = m_object_data_padded_size;

        data_set[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        data_set[1].dstSet = m_data_descriptor_sets[i];
        data_set[1].dstBinding = 1;
        data_set[1].dstArrayElement = 0;
        data_set[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        data_set[1].descriptorCount = 1;
        data_set[1].pBufferInfo = &sbo_info_objects;

        VkDescriptorBufferInfo sbo_info_skinning;
        sbo_info_skinning.buffer =
            m_sbo_data->getHostBuffer()[i];
        sbo_info_skinning.offset = 0;
        sbo_info_skinning.range = m_skinning_data_padded_size;

        data_set[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        data_set[2].dstSet = m_data_descriptor_sets[i];
        data_set[2].dstBinding = 2;
        data_set[2].dstArrayElement = 0;
        data_set[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        data_set[2].descriptorCount = 1;
        data_set[2].pBufferInfo = &sbo_info_skinning;

        VkDescriptorBufferInfo sbo_info_material;
        sbo_info_material.buffer =
            m_sbo_data->getHostBuffer()[i];
        sbo_info_material.offset = m_skinning_data_padded_size +
            m_object_data_padded_size;
        sbo_info_material.range = m_materials_padded_size;
        if (bind_mesh_textures)
        {
            data_set.resize(4, {});
            data_set[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            data_set[3].dstSet = m_data_descriptor_sets[i];
            data_set[3].dstBinding = 3;
            data_set[3].dstArrayElement = 0;
            data_set[3].descriptorType =
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
            data_set[3].descriptorCount = 1;
            data_set[3].pBufferInfo = &sbo_info_material;
        }

        vkUpdateDescriptorSets(vk->getDevice(), data_set.size(),
            data_set.data(), 0, NULL);
    }
}   // updateDataDescriptor
}
