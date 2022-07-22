#include "ge_vulkan_mesh_scene_node.hpp"

#include "ge_spm.hpp"

#include "ISceneManager.h"

namespace GE
{
GEVulkanMeshSceneNode::GEVulkanMeshSceneNode(irr::scene::IMesh* mesh,
    irr::scene::ISceneNode* parent, irr::scene::ISceneManager* mgr, irr::s32 id,
    const irr::core::vector3df& position,
    const irr::core::vector3df& rotation,
    const irr::core::vector3df& scale)
    : irr::scene::CMeshSceneNode(mesh, parent, mgr, id, position, rotation,
                                 scale)
{
}   // GEVulkanMeshSceneNode

// ----------------------------------------------------------------------------
GESPM* GEVulkanMeshSceneNode::getSPM() const
{
    return static_cast<GESPM*>(Mesh);
}   // getSPM

// ----------------------------------------------------------------------------
void GEVulkanMeshSceneNode::OnRegisterSceneNode()
{
    if (!IsVisible)
        return;
    SceneManager->registerNodeForRendering(this, scene::ESNRP_SOLID);
    ISceneNode::OnRegisterSceneNode();
}   // OnRegisterSceneNode

}
