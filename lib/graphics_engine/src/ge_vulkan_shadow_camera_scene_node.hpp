#ifndef HEADER_GE_VULKAN_SHADOW_CAMERA_SCENE_NODE_HPP
#define HEADER_GE_VULKAN_SHADOW_CAMERA_SCENE_NODE_HPP

#include "ge_vulkan_camera_scene_node.hpp"

namespace irr
{
    namespace scene
    {
        const int ESNT_SHADOW_CAMERA = MAKE_IRR_ID('s','h','d','c');
    }
}

namespace GE
{

enum GEVulkanShadowCameraCascade : unsigned
{
    GVSCC_NEAR,
    GVSCC_MIDDLE,
    GVSCC_FAR,
    GVSCC_COUNT
};

struct GEVulkanShadowUBO
{
    irr::core::matrix4 m_light_projection_view_matrix[GVSCC_COUNT];

    float m_warp_strength[GVSCC_COUNT];

    float _padding;
};

class GEVulkanShadowCameraSceneNode : public irr::scene::ISceneNode
{
private:
    irr::scene::ICameraSceneNode* m_camera;

    irr::core::matrix4 m_view_matrix;

    irr::core::matrix4 m_projection_matrices[GVSCC_COUNT];

    irr::core::aabbox3df m_bounding_box;

    GEVulkanCameraUBO m_camera_ubo_data[GVSCC_COUNT];

    GEVulkanShadowUBO m_shadow_ubo_data;

public:
    // ------------------------------------------------------------------------
    static const float getSplitNear(GEVulkanShadowCameraCascade cascade)
    {
        const float cascade_near[GVSCC_COUNT] = { 1.0f, 10.0f, 35.0f };
        return cascade_near[cascade];
    }
    // ------------------------------------------------------------------------
    static const float getSplitFar(GEVulkanShadowCameraCascade cascade)
    {
        const float cascade_far[GVSCC_COUNT] = { 11.0f, 40.0f, 150.0f };
        return cascade_far[cascade];
    }
    // ------------------------------------------------------------------------
    static const irr::s32 getShadowMapSize()
    {
        return 1024;
    }
    // ------------------------------------------------------------------------
    /**
     * "Sun" is a point in the 3D space. The light direction is the vector from
     * this point to origin. The potential shadow receiver is clipped by a plane
     * going through the point, which normal is the light direction. It will be
     * stored as position of ISceneNode.
     */
    GEVulkanShadowCameraSceneNode(irr::scene::ICameraSceneNode* parent,
                irr::scene::ISceneManager* mgr, irr::s32 id,
          const irr::core::vector3df& sun = irr::core::vector3df(1000, 1000, 1000));
    // ------------------------------------------------------------------------
    ~GEVulkanShadowCameraSceneNode();
    // ------------------------------------------------------------------------
    virtual void render();
    // ------------------------------------------------------------------------
    virtual const irr::core::aabbox3df& getBoundingBox() const
                                                     { return m_bounding_box; }
    // ------------------------------------------------------------------------
    irr::scene::ICameraSceneNode* getCamera() const { return m_camera; }
    // ------------------------------------------------------------------------
    const irr::core::rect<irr::s32> getViewPort() const 
    {
        return irr::core::rect<irr::s32>(0, 0, getShadowMapSize(), getShadowMapSize()); 
    }
    // ------------------------------------------------------------------------
    const irr::core::matrix4& getViewMatrix() const { return m_view_matrix; }
    // ------------------------------------------------------------------------
    const irr::core::matrix4& getProjectionMatrix(GEVulkanShadowCameraCascade cascade) const
    {
        return m_projection_matrices[cascade];
    }
    // ------------------------------------------------------------------------
    irr::core::matrix4 getProjectionViewMatrix(GEVulkanShadowCameraCascade cascade) const
    {
        return m_projection_matrices[cascade] * m_view_matrix;
    }
    // ------------------------------------------------------------------------
    const GEVulkanCameraUBO* const getCameraUBOData(GEVulkanShadowCameraCascade cascade) const   
    { 
        return &m_camera_ubo_data[cascade]; 
    }
    // ------------------------------------------------------------------------
    const GEVulkanShadowUBO* const getShadowUBOData() const   
    { 
        return &m_shadow_ubo_data; 
    }
    // ------------------------------------------------------------------------
    virtual irr::scene::ESCENE_NODE_TYPE getType() const 
    {
        return irr::scene::ESCENE_NODE_TYPE(irr::scene::ESNT_SHADOW_CAMERA); 
    }
};   // GEVulkanCameraSceneNode

}

#endif
