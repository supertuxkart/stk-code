#ifndef HEADER_GE_VULKAN_CAMERA_SCENE_NODE_HPP
#define HEADER_GE_VULKAN_CAMERA_SCENE_NODE_HPP

#include "../source/Irrlicht/CCameraSceneNode.h"

#include <array>

namespace GE
{
struct GEVulkanCameraUBO
{
std::array<float, 16> m_view_matrix;
std::array<float, 16> m_projection_matrix;
std::array<float, 16> m_inverse_view_matrix;
std::array<float, 16> m_inverse_projection_matrix;
std::array<float, 16> m_projection_view_matrix;
};

class GEVulkanDynamicBuffer;

class GEVulkanCameraSceneNode : public irr::scene::CCameraSceneNode
{
private:
    GEVulkanDynamicBuffer* m_buffer;

    GEVulkanCameraUBO m_ubo_data;
public:
    // ------------------------------------------------------------------------
    GEVulkanCameraSceneNode(irr::scene::ISceneNode* parent,
                            irr::scene::ISceneManager* mgr, irr::s32 id,
          const irr::core::vector3df& position = irr::core::vector3df(0, 0, 0),
         const irr::core::vector3df& lookat = irr::core::vector3df(0, 0, 100));
    // ------------------------------------------------------------------------
    ~GEVulkanCameraSceneNode();
    // ------------------------------------------------------------------------
    virtual void render();
    // ------------------------------------------------------------------------
    GEVulkanDynamicBuffer* getBuffer() const               { return m_buffer; }
};   // GEVulkanCameraSceneNode

}

#endif
