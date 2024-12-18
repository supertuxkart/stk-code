#ifndef HEADER_GE_VULKAN_SCENE_MANAGER_HPP
#define HEADER_GE_VULKAN_SCENE_MANAGER_HPP

#include "../source/Irrlicht/CSceneManager.h"
#include "IMesh.h"
#include "SMaterial.h"
#include <memory>
#include <map>
#include <vector>

namespace GE
{
class GEVulkanCameraSceneNode;
class GEVulkanDrawCall;
class GEVulkanShadowCameraSceneNode;
class GEVulkanSunSceneNode;

class GEVulkanSceneManager : public irr::scene::CSceneManager
{
private:
    GEVulkanShadowCameraSceneNode *m_active_shadow_camera;

    std::map<GEVulkanCameraSceneNode*, std::unique_ptr<GEVulkanDrawCall> >
        m_draw_calls;

    std::map<GEVulkanShadowCameraSceneNode*, std::vector<std::unique_ptr<GEVulkanDrawCall> > >
        m_shadow_draw_calls;

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
    virtual irr::scene::ILightSceneNode* addSunSceneNode(irr::scene::ISceneNode* parent = 0,
        const irr::core::vector3df& position = irr::core::vector3df(0, 0, 0),
        irr::video::SColorf color = irr::video::SColorf(1.0f, 1.0f, 1.0f),
        irr::f32 range=100.0f, irr::s32 id=-1);
    // ------------------------------------------------------------------------
    virtual GEVulkanShadowCameraSceneNode* addShadowCameraSceneNode(
        irr::scene::ICameraSceneNode* parent,
        const irr::core::vector3df& position = irr::core::vector3df(1000, 1000, 1000),
        irr::s32 id = -1, bool make_active = true);
    // ------------------------------------------------------------------------
    //! Returns the current active camera.
    //! \return The active camera is returned. Note that this can be NULL, if there
    //! was no camera created yet.
    virtual GEVulkanShadowCameraSceneNode* getActiveShadowCamera() const
                                             { return m_active_shadow_camera; }
    // ------------------------------------------------------------------------
    //! Sets the active camera. The previous active camera will be deactivated.
    //! \param camera: The new camera which should be active.
    virtual void setActiveShadowCamera(GEVulkanShadowCameraSceneNode* camera)
                                           { m_active_shadow_camera = camera; }
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
    void addShadowDrawCall(GEVulkanShadowCameraSceneNode* cam);
    // ------------------------------------------------------------------------
    void removeShadowDrawCall(GEVulkanShadowCameraSceneNode* cam);
    // ------------------------------------------------------------------------
    std::map<GEVulkanCameraSceneNode*, std::unique_ptr<GEVulkanDrawCall> >&
            getDrawCalls() { return m_draw_calls; }
    // ------------------------------------------------------------------------
    std::map<GEVulkanShadowCameraSceneNode*, std::vector<std::unique_ptr<GEVulkanDrawCall> > >&
            getShadowDrawCalls() { return m_shadow_draw_calls; }        
};   // GEVulkanSceneManager

}

#endif
