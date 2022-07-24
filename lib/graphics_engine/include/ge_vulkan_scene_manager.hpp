#ifndef HEADER_GE_VULKAN_SCENE_MANAGER_HPP
#define HEADER_GE_VULKAN_SCENE_MANAGER_HPP

#include "../source/Irrlicht/CSceneManager.h"
#include <memory>
#include <map>

namespace GE
{
class GEVulkanCameraSceneNode;
class GEVulkanDrawCall;

class GEVulkanSceneManager : public irr::scene::CSceneManager
{
private:
    std::map<GEVulkanCameraSceneNode*, std::unique_ptr<GEVulkanDrawCall> > m_draw_calls;

    // ------------------------------------------------------------------------
    void drawAllInternal();
public:
    // ------------------------------------------------------------------------
    GEVulkanSceneManager(irr::video::IVideoDriver* driver,
                         irr::io::IFileSystem* fs,
                         irr::gui::ICursorControl* cursor_control,
                         irr::gui::IGUIEnvironment* gui_environment);
    // ------------------------------------------------------------------------
    ~GEVulkanSceneManager();
    // ------------------------------------------------------------------------
    virtual irr::scene::ICameraSceneNode* addCameraSceneNode(
        irr::scene::ISceneNode* parent = 0,
        const irr::core::vector3df& position = irr::core::vector3df(0, 0, 0),
        const irr::core::vector3df& lookat = irr::core::vector3df(0, 0, 100),
        irr::s32 id = -1, bool make_active = true);
    // ------------------------------------------------------------------------
    virtual irr::scene::IAnimatedMeshSceneNode* addAnimatedMeshSceneNode(
        irr::scene::IAnimatedMesh* mesh, irr::scene::ISceneNode* parent = NULL,
        irr::s32 id = -1,
        const irr::core::vector3df& position = irr::core::vector3df(0, 0, 0),
        const irr::core::vector3df& rotation = irr::core::vector3df(0, 0, 0),
        const irr::core::vector3df& scale = irr::core::vector3df(1.0f, 1.0f, 1.0f),
        bool alsoAddIfMeshPointerZero = false);
    // ------------------------------------------------------------------------
    virtual irr::scene::IMeshSceneNode* addMeshSceneNode(irr::scene::IMesh* mesh,
        irr::scene::ISceneNode* parent = NULL, irr::s32 id = -1,
        const irr::core::vector3df& position = irr::core::vector3df(0, 0, 0),
        const irr::core::vector3df& rotation = irr::core::vector3df(0, 0, 0),
        const irr::core::vector3df& scale = irr::core::vector3df(1.0f, 1.0f, 1.0f),
        bool alsoAddIfMeshPointerZero = false);
    // ------------------------------------------------------------------------
    virtual void clear();
    // ------------------------------------------------------------------------
    virtual void drawAll(irr::u32 flags = 0xFFFFFFFF);
    // ------------------------------------------------------------------------
    virtual irr::u32 registerNodeForRendering(irr::scene::ISceneNode* node,
        irr::scene::E_SCENE_NODE_RENDER_PASS pass = irr::scene::ESNRP_AUTOMATIC);
    // ------------------------------------------------------------------------
    void addDrawCall(GEVulkanCameraSceneNode* cam);
    // ------------------------------------------------------------------------
    void removeDrawCall(GEVulkanCameraSceneNode* cam);
    // ------------------------------------------------------------------------
    std::map<GEVulkanCameraSceneNode*, std::unique_ptr<GEVulkanDrawCall> >&
                                        getDrawCalls() { return m_draw_calls; }
};   // GEVulkanSceneManager

}

#endif
