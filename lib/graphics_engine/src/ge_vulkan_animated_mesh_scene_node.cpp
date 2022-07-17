#include "ge_vulkan_animated_mesh_scene_node.hpp"

#include "ge_spm.hpp"

namespace GE
{
GEVulkanAnimatedMeshSceneNode::GEVulkanAnimatedMeshSceneNode(irr::scene::IAnimatedMesh* mesh,
    irr::scene::ISceneNode* parent, irr::scene::ISceneManager* mgr, irr::s32 id,
    const irr::core::vector3df& position,
    const irr::core::vector3df& rotation,
    const irr::core::vector3df& scale)
    : irr::scene::CAnimatedMeshSceneNode(mesh, parent, mgr, id, position,
                                         rotation, scale)
{
}   // GEVulkanAnimatedMeshSceneNode

// ----------------------------------------------------------------------------
GESPM* GEVulkanAnimatedMeshSceneNode::getSPM() const
{
    return static_cast<GESPM*>(Mesh);
}   // getSPM

}
