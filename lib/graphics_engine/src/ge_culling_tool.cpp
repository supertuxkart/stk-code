#include "ge_culling_tool.hpp"

#include "ge_main.hpp"
#include "ge_spm_buffer.hpp"
#include "ge_vulkan_camera_scene_node.hpp"
#include "ge_vulkan_shadow_camera_scene_node.hpp"

#include "occlusion/CullingThreadpool.h"

#include "IAnimatedMeshSceneNode.h"
#include "IMeshSceneNode.h"

namespace GE
{
// ----------------------------------------------------------------------------
GECullingTool::GECullingTool()
{
    m_occlusion = NULL;
    m_occlusion_manager = NULL;

    m_ignore_near_plane = false;
}

// ----------------------------------------------------------------------------
GECullingTool::~GECullingTool()
{
    if (m_occlusion_manager)
    {
        delete m_occlusion_manager;
        MaskedOcclusionCulling::Destroy(m_occlusion);
        m_occlusion_manager = NULL;
        m_occlusion = NULL;
    }
}

// ----------------------------------------------------------------------------
void GECullingTool::init(GEVulkanCameraSceneNode* cam, bool occlusion)
{
    mathPlaneFrustumf(&m_frustum[0].X, cam->getProjectionViewMatrix());
    m_cam_bbox = cam->getViewFrustum()->getBoundingBox();
    m_pvm_matrix = cam->getProjectionViewMatrix();
    m_cam_position = cam->getPosition();

    if (occlusion)
    {
        if (!m_occlusion_manager)
        {
            m_occlusion = MaskedOcclusionCulling::Create();
            m_occlusion_manager = new CullingThreadpool(4, 4, 4);
            m_occlusion_manager->SetBuffer(m_occlusion);
            m_occlusion_manager->SetResolution(256, 160);
        }
        m_occlusion_manager->SetNearClipPlane(0.0f);
        m_occlusion_manager->ClearBuffer();
    }
    else
    {
        m_occlusion = NULL;
        m_occlusion_manager = NULL;
    }
}   // init

// ----------------------------------------------------------------------------
void GECullingTool::init(GEVulkanShadowCameraSceneNode* cam, GEVulkanShadowCameraCascade cascade)
{
    mathPlaneFrustumf(&m_frustum[0].X, cam->getProjectionViewMatrix(cascade));
    m_ignore_near_plane = true;

    m_cam_bbox = cam->getBoundingBox();
    m_pvm_matrix = cam->getProjectionViewMatrix(cascade);
    m_cam_position = cam->getPosition();
    m_occlusion = NULL;
    m_occlusion_manager = NULL;
}   // init

// ----------------------------------------------------------------------------
void GECullingTool::addOccluder(GESPMBuffer* buffer, irr::scene::ISceneNode* node)
{
    if (!m_occlusion_manager)
    {
        return;
    }

    std::vector<float> &vertices = m_occluder_vertices[buffer];
    std::vector<unsigned> &indices = m_occluder_indices[buffer];
    vertices.resize(buffer->getVertexCount() * 4);
    indices.resize(buffer->getIndexCount());

    for (int i = 0; i < buffer->getVertexCount(); i++)
    {
        vertices[4 * i + 0] = static_cast<float*>(buffer->getVertices())[12 * i + 0];
        vertices[4 * i + 1] = static_cast<float*>(buffer->getVertices())[12 * i + 1];
        vertices[4 * i + 3] = static_cast<float*>(buffer->getVertices())[12 * i + 2];
    }
    for (int i = 0; i < buffer->getIndexCount(); i++)
    {
        indices[i] = buffer->getIndices()[i];
    }

    m_all_occluder_nodes.insert(node);
}

// ----------------------------------------------------------------------------
void GECullingTool::clearOccluders()
{
    if (!m_occlusion_manager)
    {
        return;
    }

    m_occluder_vertices.clear();
    m_occluder_indices.clear();
    m_all_occluder_nodes.clear();
}

// ----------------------------------------------------------------------------
void GECullingTool::processOccluders()
{
    if (!m_occlusion_manager)
    {
        return;
    }
    
    const MaskedOcclusionCulling::ClipPlanes clip_planes[6] = 
    {
        MaskedOcclusionCulling::CLIP_PLANE_NEAR,
        MaskedOcclusionCulling::CLIP_PLANE_RIGHT,
        MaskedOcclusionCulling::CLIP_PLANE_NEAR,
        MaskedOcclusionCulling::CLIP_PLANE_BOTTOM,
        MaskedOcclusionCulling::CLIP_PLANE_TOP,
        MaskedOcclusionCulling::CLIP_PLANE_NONE, // Don't clip far plane
    };

    m_occlusion_manager->WakeThreads();
    for (auto& node : m_all_occluder_nodes)
    {
        irr::core::matrix4 matrix = m_pvm_matrix * node->getAbsoluteTransformation();
        m_occlusion_manager->SetMatrix(matrix.pointer());
        irr::scene::IMesh *mesh;

        if (node->getType() == irr::scene::ESNT_ANIMATED_MESH)
        {
            mesh = static_cast<irr::scene::IAnimatedMeshSceneNode*>(node)->getMesh();
        }
        else if (node->getType() == irr::scene::ESNT_MESH)
        {
            mesh = static_cast<irr::scene::IMeshSceneNode*>(node)->getMesh();
        }
        for (int i = 0; i < mesh->getMeshBufferCount(); i++)
        {
            irr::scene::IMeshBuffer *buffer = mesh->getMeshBuffer(i);
            irr::core::aabbox3df bb = buffer->getBoundingBox();

            if (!m_cam_bbox.intersectsWithBox(bb))
                continue;

            irr::core::quaternion edges[8] =
            {
                irr::core::quaternion(bb.MinEdge.X, bb.MinEdge.Y, bb.MinEdge.Z, 1.0f),
                irr::core::quaternion(bb.MaxEdge.X, bb.MinEdge.Y, bb.MinEdge.Z, 1.0f),
                irr::core::quaternion(bb.MinEdge.X, bb.MaxEdge.Y, bb.MinEdge.Z, 1.0f),
                irr::core::quaternion(bb.MaxEdge.X, bb.MaxEdge.Y, bb.MinEdge.Z, 1.0f),
                irr::core::quaternion(bb.MinEdge.X, bb.MinEdge.Y, bb.MaxEdge.Z, 1.0f),
                irr::core::quaternion(bb.MaxEdge.X, bb.MinEdge.Y, bb.MaxEdge.Z, 1.0f),
                irr::core::quaternion(bb.MinEdge.X, bb.MaxEdge.Y, bb.MaxEdge.Z, 1.0f),
                irr::core::quaternion(bb.MaxEdge.X, bb.MaxEdge.Y, bb.MaxEdge.Z, 1.0f)
            };

            bool culled = false;
            MaskedOcclusionCulling::ClipPlanes planes
                 = MaskedOcclusionCulling::CLIP_PLANE_NONE;
            
            for (int i = 0; i < 6; i++)
            {
                bool culled_by_plane = true;
                bool cliped_by_plane = false;

                for (int j = 0; j < 8; j++)
                {
                    if (m_frustum[i].dotProduct(edges[j]) >= 0.0)
                    {
                        culled_by_plane = false;
                    }
                    else
                    {
                        cliped_by_plane = true;
                    }
                }
                if (culled_by_plane)
                {
                    culled = true;
                    break;
                }
                if (cliped_by_plane)
                {
                    planes = (MaskedOcclusionCulling::ClipPlanes)
                             (planes | clip_planes[i]);
                }
            }

            if (!culled)
            {
                m_occlusion_manager->RenderTriangles(
                    m_occluder_vertices[buffer].data(),
                    m_occluder_indices[buffer].data(),
                    m_occluder_indices[buffer].size() / 3,
                    MaskedOcclusionCulling::BACKFACE_CCW,
                    planes);
            }
        }
        
        m_occlusion_manager->Flush();
    }
    m_occlusion_manager->SuspendThreads();
}

// ----------------------------------------------------------------------------
bool GECullingTool::isViewCulled(irr::core::aabbox3df& bb)
{
    if (!m_cam_bbox.intersectsWithBox(bb))
        return true;

    using namespace irr;
    using namespace core;
    quaternion edges[8] =
    {
        quaternion(bb.MinEdge.X, bb.MinEdge.Y, bb.MinEdge.Z, 1.0f),
        quaternion(bb.MaxEdge.X, bb.MinEdge.Y, bb.MinEdge.Z, 1.0f),
        quaternion(bb.MinEdge.X, bb.MaxEdge.Y, bb.MinEdge.Z, 1.0f),
        quaternion(bb.MaxEdge.X, bb.MaxEdge.Y, bb.MinEdge.Z, 1.0f),
        quaternion(bb.MinEdge.X, bb.MinEdge.Y, bb.MaxEdge.Z, 1.0f),
        quaternion(bb.MaxEdge.X, bb.MinEdge.Y, bb.MaxEdge.Z, 1.0f),
        quaternion(bb.MinEdge.X, bb.MaxEdge.Y, bb.MaxEdge.Z, 1.0f),
        quaternion(bb.MaxEdge.X, bb.MaxEdge.Y, bb.MaxEdge.Z, 1.0f)
    };

    for (int i = m_ignore_near_plane ? 1 : 0; i < 6; i++)
    {
        bool culled = true;
        for (int j = 0; j < 8; j++)
        {
            if (m_frustum[i].dotProduct(edges[j]) >= 0.0)
            {
                culled = false;
                break;
            }
        }
        if (culled)
            return true;
    }
    return false;
}   // isCulled

// ----------------------------------------------------------------------------
bool GECullingTool::isOcclusionCulled(irr::core::aabbox3df& bb)
{
    if (!m_occlusion_manager)
    {
        return false;
    }

    if (bb.isPointInside(m_cam_position))
    {
        return false;
    }

    std::vector<float> vertices;
    vertices.resize(24);

    for (int i = 0; i < 8; i++)
    {
        vertices[i * 3] = i & 4 ? bb.MaxEdge.X : bb.MinEdge.X;            
        vertices[i * 3 + 1] = i & 2 ? bb.MaxEdge.Y : bb.MinEdge.Y;
        vertices[i * 3 + 2] = i & 1 ? bb.MaxEdge.Z : bb.MinEdge.Z;
    }

    std::vector<float> trans_vertices;
    trans_vertices.resize(32);
    m_occlusion->TransformVertices(m_pvm_matrix.pointer(), vertices.data(), trans_vertices.data(), 8);

    float x1 = trans_vertices[0] / trans_vertices[3], x2 = trans_vertices[0] / trans_vertices[3];
    float y1 = trans_vertices[1] / trans_vertices[3], y2 = trans_vertices[1] / trans_vertices[3];
    float w1 = trans_vertices[3];

    for (int i = 1; i < 8; i++)
    {
        x1 = std::min(x1, trans_vertices[i * 4] / trans_vertices[i * 4 + 3]);
        x2 = std::max(x2, trans_vertices[i * 4] / trans_vertices[i * 4 + 3]);
        y1 = std::min(y1, trans_vertices[i * 4 + 1] / trans_vertices[i * 4 + 3]);
        y2 = std::max(y2, trans_vertices[i * 4 + 1] / trans_vertices[i * 4 + 3]);
        w1 = std::min(w1, trans_vertices[i * 4 + 3]);
    }
    if (w1 < 0.0f)
    {
        return false;
    }
    MaskedOcclusionCulling::CullingResult res = 
        m_occlusion_manager->TestRect(x1, y1, x2, y2, w1);
    
    return res == MaskedOcclusionCulling::CullingResult::VISIBLE ? false : true;
}

// ----------------------------------------------------------------------------
bool GECullingTool::isCulled(irr::core::aabbox3df& bb)
{
    return isViewCulled(bb) || isOcclusionCulled(bb);
}   // isCulled

// ----------------------------------------------------------------------------
bool GECullingTool::isCulled(GESPMBuffer* buffer, irr::scene::ISceneNode* node)
{
    irr::core::aabbox3df bb = buffer->getBoundingBox();
    node->getAbsoluteTransformation().transformBoxEx(bb);
    return isCulled(bb);
}   // isCulled

}
