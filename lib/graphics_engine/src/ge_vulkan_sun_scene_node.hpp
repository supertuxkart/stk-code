#ifndef HEADER_GE_VULKAN_SUN_SCENE_NODE_HPP
#define HEADER_GE_VULKAN_SUN_SCENE_NODE_HPP

#include "../source/Irrlicht/CLightSceneNode.h"

namespace irr
{
    namespace scene
    {
        const int ESNT_SUN = MAKE_IRR_ID('s','u','n','n');
    }
}

namespace GE
{

class GEVulkanCameraSceneNode;

class GEVulkanSunSceneNode : public irr::scene::CLightSceneNode
{
public:
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
    	//! pre render event
	virtual void OnRegisterSceneNode();
    
    virtual irr::scene::ESCENE_NODE_TYPE getType() const 
    {
        return irr::scene::ESCENE_NODE_TYPE(irr::scene::ESNT_SUN); 
    }
};   // GEVulkanCameraSceneNode

}

#endif
