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
                         irr::scene::IMeshCache* cache,
                         irr::gui::IGUIEnvironment* gui_environment);
    // ------------------------------------------------------------------------
    ~GEVulkanSceneManager();
    // ------------------------------------------------------------------------
};   // GEVulkanSceneManager

}

#endif
