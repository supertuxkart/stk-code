#ifndef HEADER_GE_VULKAN_ANIMATED_MESH_SCENE_NODE_HPP
#define HEADER_GE_VULKAN_ANIMATED_MESH_SCENE_NODE_HPP

#include "../source/Irrlicht/CAnimatedMeshSceneNode.h"

#include <cassert>
#include <string>
#include <vector>
#include <unordered_map>

namespace GE
{
class GESPM;

class GEVulkanAnimatedMeshSceneNode : public irr::scene::CAnimatedMeshSceneNode
{
private:
    std::unordered_map<std::string, irr::scene::IBoneSceneNode*> m_joint_nodes;

    float m_saved_transition_frame;

    std::vector<irr::core::matrix4> m_skinning_matrices;

    // ------------------------------------------------------------------------
    void cleanJoints()
    {
        for (auto& p : m_joint_nodes)
            removeChild(p.second);
        m_joint_nodes.clear();
        m_skinning_matrices.clear();
    }
public:
    // ------------------------------------------------------------------------
    GEVulkanAnimatedMeshSceneNode(irr::scene::IAnimatedMesh* mesh,
        irr::scene::ISceneNode* parent, irr::scene::ISceneManager* mgr, irr::s32 id,
        const irr::core::vector3df& position = irr::core::vector3df(0, 0, 0),
        const irr::core::vector3df& rotation = irr::core::vector3df(0, 0, 0),
        const irr::core::vector3df& scale = irr::core::vector3df(1.0f, 1.0f, 1.0f));
    // ------------------------------------------------------------------------
    ~GEVulkanAnimatedMeshSceneNode()                         { cleanJoints(); }
    // ------------------------------------------------------------------------
    virtual void setMesh(irr::scene::IAnimatedMesh* mesh);
    // ------------------------------------------------------------------------
    virtual void OnAnimate(irr::u32 time_ms);
    // ------------------------------------------------------------------------
    virtual irr::scene::IBoneSceneNode* getJointNode(const irr::c8* joint_name);
    // ------------------------------------------------------------------------
    virtual irr::scene::IBoneSceneNode* getJointNode(irr::u32 joint_id);
    // ------------------------------------------------------------------------
    virtual irr::u32 getJointCount() const     { return m_joint_nodes.size(); }
    // ------------------------------------------------------------------------
    virtual void setTransitionTime(irr::f32 Time);
    // ------------------------------------------------------------------------
    GESPM* getSPM() const;
    // ------------------------------------------------------------------------
    virtual void OnRegisterSceneNode();
    // ------------------------------------------------------------------------
    const std::vector<irr::core::matrix4>& getSkinningMatrices() const
                                                { return m_skinning_matrices; }
};   // GEVulkanAnimatedMeshSceneNode

}

#endif
