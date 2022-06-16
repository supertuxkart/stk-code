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
};   // GEVulkanSceneManager

}

#endif
