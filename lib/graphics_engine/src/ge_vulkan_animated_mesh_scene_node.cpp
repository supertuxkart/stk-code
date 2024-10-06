#include "ge_vulkan_animated_mesh_scene_node.hpp"

#include "ge_animation.hpp"
#include "ge_spm.hpp"

#include "ISceneManager.h"
#include "../../../lib/irrlicht/source/Irrlicht/CBoneSceneNode.h"
#include <limits>

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
    m_saved_transition_frame = -1.0f;
}   // GEVulkanAnimatedMeshSceneNode

// ----------------------------------------------------------------------------
GESPM* GEVulkanAnimatedMeshSceneNode::getSPM() const
{
    return static_cast<GESPM*>(Mesh);
}   // getSPM

// ----------------------------------------------------------------------------
void GEVulkanAnimatedMeshSceneNode::OnRegisterSceneNode()
{
    if (!IsVisible)
        return;
    SceneManager->registerNodeForRendering(this, scene::ESNRP_SOLID);
    ISceneNode::OnRegisterSceneNode();
}   // OnRegisterSceneNode

// ----------------------------------------------------------------------------
void GEVulkanAnimatedMeshSceneNode::setMesh(irr::scene::IAnimatedMesh* mesh)
{
    CAnimatedMeshSceneNode::setMesh(mesh);
    cleanJoints();
    GESPM* spm = getSPM();
    if (!spm || spm->isStatic())
        return;

    unsigned bone_idx = 0;
    m_skinning_matrices.resize(spm->getJointCount());
    for (Armature& arm : spm->getArmatures())
    {
        for (const std::string& bone_name : arm.m_joint_names)
        {
            m_joint_nodes[bone_name] = new CBoneSceneNode(this,
                SceneManager, 0, bone_idx++, bone_name.c_str());
            m_joint_nodes.at(bone_name)->drop();
            m_joint_nodes.at(bone_name)->setSkinningSpace(EBSS_GLOBAL);
        }
    }
}   // setMesh

// ----------------------------------------------------------------------------
void GEVulkanAnimatedMeshSceneNode::OnAnimate(irr::u32 time_ms)
{
    GESPM* spm = getSPM();
    if (!spm || spm->isStatic())
    {
        IAnimatedMeshSceneNode::OnAnimate(time_ms);
        return;
    }

    // first frame
    if (LastTimeMs == 0)
        LastTimeMs = time_ms;

    // set CurrentFrameNr
    buildFrameNr(time_ms - LastTimeMs);
    LastTimeMs = time_ms;

    spm->getSkinningMatrices(getFrameNr(), m_skinning_matrices,
        m_saved_transition_frame, TransitingBlend);
    recursiveUpdateAbsolutePosition();

    for (Armature& arm : spm->getArmatures())
    {
        for (unsigned i = 0; i < arm.m_joint_names.size(); i++)
        {
            m_joint_nodes.at(arm.m_joint_names[i])->setAbsoluteTransformation
                (AbsoluteTransformation * arm.m_world_matrices[i].first);
        }
    }

    IAnimatedMeshSceneNode::OnAnimate(time_ms);
}   // OnAnimate

// ----------------------------------------------------------------------------
irr::scene::IBoneSceneNode* GEVulkanAnimatedMeshSceneNode::getJointNode(const irr::c8* joint_name)
{
    auto ret = m_joint_nodes.find(joint_name);
    if (ret != m_joint_nodes.end())
        return ret->second;
    return NULL;
}   // getJointNode

// ----------------------------------------------------------------------------
irr::scene::IBoneSceneNode* GEVulkanAnimatedMeshSceneNode::getJointNode(irr::u32 joint_id)
{
    irr::u32 idx = 0;
    for (auto& p : m_joint_nodes)
    {
        if (joint_id == idx)
            return p.second;
        idx++;
    }
    return NULL;
}   // getJointNode

// ----------------------------------------------------------------------------
void GEVulkanAnimatedMeshSceneNode::setTransitionTime(irr::f32 Time)
{
    if (Time == 0.0f)
    {
        TransitingBlend = TransitionTime = Transiting = 0;
        m_saved_transition_frame = -1.0;
    }
    else
    {
        const u32 ttime = (u32)core::floor32(Time * 1000.0f);
        TransitionTime = ttime;
        Transiting = core::reciprocal((f32)TransitionTime);
        TransitingBlend = 0.0f;
        m_saved_transition_frame = getFrameNr();
    }
}   // setTransitionTime

}
