#include "ge_vulkan_sun_scene_node.hpp"

#include "ge_main.hpp"
#include "ge_vulkan_driver.hpp"
#include "ge_vulkan_scene_manager.hpp"

namespace GE
{
// ----------------------------------------------------------------------------
GEVulkanSunSceneNode::GEVulkanSunSceneNode(irr::scene::ISceneNode* parent,
                irr::scene::ISceneManager* mgr, irr::s32 id,
          const irr::core::vector3df& position, irr::video::SColorf color, 
                irr::f32 radius)
                    : CLightSceneNode(parent, mgr, id, position, color, radius)
{
    getLightData().Type = irr::video::ELT_DIRECTIONAL;
}   // GEVulkanCameraSceneNode

// ----------------------------------------------------------------------------
void GEVulkanSunSceneNode::OnRegisterSceneNode()
{
    irr::scene::CLightSceneNode::OnRegisterSceneNode();

    SceneManager->getVideoDriver()->addDynamicLight(getLightData());
}   // OnRegisterSceneNode;

}
