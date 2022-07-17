#ifndef HEADER_GE_VULKAN_SCENE_MANAGER_HPP
#define HEADER_GE_VULKAN_SCENE_MANAGER_HPP

#include "../source/Irrlicht/CSceneManager.h"

namespace GE
{

class GEVulkanSceneManager : public irr::scene::CSceneManager
{
private:
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
};   // GEVulkanSceneManager

}

#endif
