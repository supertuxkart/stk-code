#include "ge_vulkan_scene_manager.hpp"

namespace GE
{
// ----------------------------------------------------------------------------
GEVulkanSceneManager::GEVulkanSceneManager(irr::video::IVideoDriver* driver,
                                           irr::io::IFileSystem* fs,
                                           irr::gui::ICursorControl* cursor_control,
                                           irr::scene::IMeshCache* cache,
                                           irr::gui::IGUIEnvironment* gui_environment)
                    : CSceneManager(driver, fs, cursor_control, cache,
                                    gui_environment)
{
}   // GEVulkanSceneManager

// ----------------------------------------------------------------------------
GEVulkanSceneManager::~GEVulkanSceneManager()
{
}   // ~GEVulkanSceneManager

}
