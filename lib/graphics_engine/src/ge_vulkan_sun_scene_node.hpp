#ifndef HEADER_GE_VULKAN_SUN_SCENE_NODE_HPP
#define HEADER_GE_VULKAN_SUN_SCENE_NODE_HPP

#include "../source/Irrlicht/CLightSceneNode.h"

#include "ge_vulkan_camera_scene_node.hpp"

namespace irr
{
    namespace scene
    {
        const int ESNT_SUN = MAKE_IRR_ID('s','u','n','n');
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
    
    irr::core::matrix4 m_light_view_matrix;
};

class GEVulkanSunSceneNode : public irr::scene::CLightSceneNode
{
private:
    irr::core::matrix4 m_shadow_view_matrix;

    irr::core::matrix4 m_shadow_projection_matrices[GVSCC_COUNT];

    GEVulkanCameraUBO m_shadow_camera_ubo_data[GVSCC_COUNT];

    GEVulkanShadowUBO m_shadow_ubo_data;

public:
    // ------------------------------------------------------------------------
    static const float getSplitNear(GEVulkanShadowCameraCascade cascade)
    {
        const float cascade_near[GVSCC_COUNT] = { 1.0f, 9.0f, 40.0f };
        return cascade_near[cascade];
    }
    // ------------------------------------------------------------------------
    static const float getSplitFar(GEVulkanShadowCameraCascade cascade)
    {
        const float cascade_far[GVSCC_COUNT] = { 10.0f, 45.0f, 150.0f };
        return cascade_far[cascade];
    }
    // ------------------------------------------------------------------------
    static const irr::s32 getShadowMapSize()
    {
        return 1024;
    }
    // ------------------------------------------------------------------------
    // Radius for the radial representation of the sun view angle (default 0.54)
    GEVulkanSunSceneNode(irr::scene::ISceneNode* parent,
                irr::scene::ISceneManager* mgr, irr::s32 id,
          const irr::core::vector3df& position = irr::core::vector3df(0, 0, 0),
                irr::video::SColorf color = irr::video::SColorf(0, 0, 0, 1),
                irr::f32 radius = 0.26f);
    // ------------------------------------------------------------------------
    ~GEVulkanSunSceneNode() {}
    // ------------------------------------------------------------------------
    virtual void render();
    // ------------------------------------------------------------------------
    virtual void setLightType(irr::video::E_LIGHT_TYPE type) {}
    // ------------------------------------------------------------------------
    virtual irr::video::E_LIGHT_TYPE getLightType() 
                                        { return irr::video::ELT_DIRECTIONAL; }
    // ------------------------------------------------------------------------
    const irr::core::matrix4& getShadowViewMatrix() const 
                                                { return m_shadow_view_matrix; }
    // ------------------------------------------------------------------------
    const irr::core::matrix4& getShadowProjectionMatrix(GEVulkanShadowCameraCascade cascade) const
    {
        return m_shadow_projection_matrices[cascade];
    }
    // ------------------------------------------------------------------------
    irr::core::matrix4 getShadowProjectionViewMatrix(GEVulkanShadowCameraCascade cascade) const
    {
        return m_shadow_projection_matrices[cascade] * m_shadow_view_matrix;
    }
    // ------------------------------------------------------------------------
    const GEVulkanCameraUBO* const getShadowCameraUBOData
                                    (GEVulkanShadowCameraCascade cascade) const   
    { 
        return &m_shadow_camera_ubo_data[cascade]; 
    }
    // ------------------------------------------------------------------------
    const GEVulkanShadowUBO* const getShadowUBOData() const   
    { 
        return &m_shadow_ubo_data; 
    }
    // ------------------------------------------------------------------------
    virtual irr::scene::ESCENE_NODE_TYPE getType() const 
    {
        return irr::scene::ESCENE_NODE_TYPE(irr::scene::ESNT_SUN); 
    }
};   // GEVulkanCameraSceneNode

}

#endif
