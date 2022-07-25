#ifndef HEADER_GE_VULKAN_MESH_SCENE_NODE_HPP
#define HEADER_GE_VULKAN_MESH_SCENE_NODE_HPP

#include "../source/Irrlicht/CMeshSceneNode.h"

namespace GE
{
class GESPM;

class GEVulkanMeshSceneNode : public irr::scene::CMeshSceneNode
{
private:
    bool m_remove_from_mesh_cache;
public:
    // ------------------------------------------------------------------------
    GEVulkanMeshSceneNode(irr::scene::IMesh* mesh,
        irr::scene::ISceneNode* parent, irr::scene::ISceneManager* mgr, irr::s32 id,
        const irr::core::vector3df& position = irr::core::vector3df(0, 0, 0),
        const irr::core::vector3df& rotation = irr::core::vector3df(0, 0, 0),
        const irr::core::vector3df& scale = irr::core::vector3df(1.0f, 1.0f, 1.0f));
    // ------------------------------------------------------------------------
    ~GEVulkanMeshSceneNode();
    // ------------------------------------------------------------------------
    void setRemoveFromMeshCache(bool val)   { m_remove_from_mesh_cache = val; }
    // ------------------------------------------------------------------------
    GESPM* getSPM() const;
    // ------------------------------------------------------------------------
    virtual void OnRegisterSceneNode();
};   // GEVulkanMeshSceneNode

}

#endif
