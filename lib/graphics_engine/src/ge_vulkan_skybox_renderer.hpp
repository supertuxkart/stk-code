#ifndef HEADER_GE_VULKAN_SKYBOX_RENDERER_HPP
#define HEADER_GE_VULKAN_SKYBOX_RENDERER_HPP

#include "vulkan_wrapper.h"

namespace irr
{
    namespace scene { class ISceneNode; }
}

namespace GE
{
class GEVulkanCameraSceneNode;
class GEVulkanDriver;
namespace GEVulkanSkyBoxRenderer
{
// ----------------------------------------------------------------------------
void init();
// ----------------------------------------------------------------------------
void destroy();
// ----------------------------------------------------------------------------
void render(VkCommandBuffer, GEVulkanCameraSceneNode*);
// ----------------------------------------------------------------------------
void addSkyBox(GEVulkanCameraSceneNode*, irr::scene::ISceneNode*);
};   // GEVulkanSkyBoxRenderer

}

#endif
