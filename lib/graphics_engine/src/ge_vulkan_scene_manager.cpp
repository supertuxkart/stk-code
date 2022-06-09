#include "ge_vulkan_scene_manager.hpp"

#include "ge_vulkan_mesh_cache.hpp"

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

}
