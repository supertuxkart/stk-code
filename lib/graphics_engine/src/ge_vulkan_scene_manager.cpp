#include "ge_vulkan_scene_manager.hpp"

#include "../source/Irrlicht/os.h"

#include "ge_spm.hpp"
#include "ge_vulkan_animated_mesh_scene_node.hpp"
#include "ge_vulkan_camera_scene_node.hpp"
#include "ge_vulkan_draw_call.hpp"
#include "ge_vulkan_driver.hpp"
#include "ge_vulkan_mesh_cache.hpp"
#include "ge_vulkan_mesh_scene_node.hpp"
#include "ge_vulkan_texture_descriptor.hpp"

namespace GE
{
// ----------------------------------------------------------------------------
GEVulkanSceneManager::GEVulkanSceneManager(irr::video::IVideoDriver* driver,
                                           irr::io::IFileSystem* fs,
                                           irr::gui::ICursorControl* cursor_control,
                                           irr::gui::IGUIEnvironment* gui_environment)
                    : CSceneManager(driver, fs, cursor_control,
                                    new GEVulkanMeshCache(), gui_environment)
{
    // CSceneManager grabbed it
    getMeshCache()->drop();
}   // GEVulkanSceneManager

// ----------------------------------------------------------------------------
GEVulkanSceneManager::~GEVulkanSceneManager()
{
}   // ~GEVulkanSceneManager

// ----------------------------------------------------------------------------
void GEVulkanSceneManager::clear()
{
    irr::scene::CSceneManager::clear();
    static_cast<GEVulkanDriver*>(getVideoDriver())
        ->getMeshTextureDescriptor()->clear();
}   // clear

// ----------------------------------------------------------------------------
irr::scene::ICameraSceneNode* GEVulkanSceneManager::addCameraSceneNode(
                                                irr::scene::ISceneNode* parent,
                                          const irr::core::vector3df& position,
                                            const irr::core::vector3df& lookat,
                                                 irr::s32 id, bool make_active)
{
    if (!parent)
        parent = this;

    irr::scene::ICameraSceneNode* node = new GEVulkanCameraSceneNode(parent,
        this, id, position, lookat);

    if (make_active)
        setActiveCamera(node);
    node->drop();

    return node;
}   // addCameraSceneNode

// ----------------------------------------------------------------------------
irr::scene::IAnimatedMeshSceneNode* GEVulkanSceneManager::addAnimatedMeshSceneNode(
    irr::scene::IAnimatedMesh* mesh, irr::scene::ISceneNode* parent,
    irr::s32 id,
    const irr::core::vector3df& position,
    const irr::core::vector3df& rotation,
    const irr::core::vector3df& scale,
    bool alsoAddIfMeshPointerZero)
{
    if (!alsoAddIfMeshPointerZero && (!mesh || !dynamic_cast<GESPM*>(mesh)))
        return NULL;

    if (!parent)
        parent = this;

    irr::scene::IAnimatedMeshSceneNode* node =
        new GEVulkanAnimatedMeshSceneNode(mesh, parent, this, id, position,
        rotation, scale);
    node->drop();
    node->setMesh(mesh);
    return node;
}   // addAnimatedMeshSceneNode

// ----------------------------------------------------------------------------
irr::scene::IMeshSceneNode* GEVulkanSceneManager::addMeshSceneNode(
    irr::scene::IMesh* mesh,
    irr::scene::ISceneNode* parent, irr::s32 id,
    const irr::core::vector3df& position,
    const irr::core::vector3df& rotation,
    const irr::core::vector3df& scale,
    bool alsoAddIfMeshPointerZero)
{
    if (!alsoAddIfMeshPointerZero && !mesh)
        return NULL;

    if (mesh)
    {
        for (unsigned i = 0; i < mesh->getMeshBufferCount(); i++)
        {
            irr::scene::IMeshBuffer* b = mesh->getMeshBuffer(i);
            if (b->getVertexType() != irr::video::EVT_SKINNED_MESH)
            {
                return irr::scene::CSceneManager::addMeshSceneNode(
                    mesh, parent, id, position, rotation, scale,
                    alsoAddIfMeshPointerZero);
            }
        }
    }

    if (!parent)
        parent = this;

    irr::scene::IMeshSceneNode* node =
        new GEVulkanMeshSceneNode(mesh, parent, this, id, position, rotation,
        scale);
    node->drop();
    return node;
}   // addMeshSceneNode

// ----------------------------------------------------------------------------
void GEVulkanSceneManager::drawAll(irr::u32 flags)
{
    static_cast<GEVulkanMeshCache*>(getMeshCache())->updateCache();
    GEVulkanCameraSceneNode* cam = NULL;
    if (getActiveCamera())
    {
        cam = static_cast<
            GEVulkanCameraSceneNode*>(getActiveCamera());
    }
    OnAnimate(os::Timer::getTime());
    if (cam)
    {
        cam->render();
        auto it = m_draw_calls.find(cam);
        if (it == m_draw_calls.end())
            return;

        it->second->prepare(cam);
        OnRegisterSceneNode();
        it->second->generate();
    }
}   // drawAll

// ----------------------------------------------------------------------------
irr::u32 GEVulkanSceneManager::registerNodeForRendering(
    irr::scene::ISceneNode* node,
    irr::scene::E_SCENE_NODE_RENDER_PASS pass)
{
    if (!getActiveCamera())
        return 0;

    GEVulkanCameraSceneNode* cam = static_cast<
        GEVulkanCameraSceneNode*>(getActiveCamera());

    if ((node->getType() == irr::scene::ESNT_ANIMATED_MESH &&
        pass != irr::scene::ESNRP_SOLID) ||
        (node->getType() == irr::scene::ESNT_MESH &&
        pass != irr::scene::ESNRP_SOLID))
        return 0;

    m_draw_calls.at(cam)->addNode(node);
    return 1;
}   // registerNodeForRendering

// ----------------------------------------------------------------------------
void GEVulkanSceneManager::addDrawCall(GEVulkanCameraSceneNode* cam)
{
    m_draw_calls[cam] = std::unique_ptr<GEVulkanDrawCall>(new GEVulkanDrawCall);
}   // addDrawCall

// ----------------------------------------------------------------------------
void GEVulkanSceneManager::removeDrawCall(GEVulkanCameraSceneNode* cam)
{
    m_draw_calls.erase(cam);
}   // removeDrawCall

}
