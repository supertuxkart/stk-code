//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef HEADER_IRR_DRIVER_HPP
#define HEADER_IRR_DRIVER_HPP

/**
 * \defgroup graphics
 * This module contains the core graphics engine, that is mostly a thin layer
 * on top of irrlicht providing some additional features we need for STK
 * (like particles, more scene node types, mesh manipulation tools, material
 * management, etc...)
 */

#include "graphics/abstract_renderer.hpp"
#include "graphics/gl_headers.hpp"
#include "graphics/wind.hpp"
#include "io/file_manager.hpp"
#include "utils/aligned_array.hpp"
#include "utils/no_copy.hpp"
#include "utils/ptr_vector.hpp"
#include "utils/vec3.hpp"
#include <memory>
#include <string>
#include <vector>

#include <irrArray.h>
#include <dimension2d.h>
#include <position2d.h>
#include <matrix4.h>
#include <vector2d.h>
#include <IEventReceiver.h>
#include <SColor.h>

namespace SP
{
    class SPDynamicDrawCall;
}


namespace irr
{
    namespace scene { class ISceneManager; class IMesh; class IAnimatedMeshSceneNode; class IAnimatedMesh;
        class IMeshSceneNode; class IParticleSystemSceneNode; class ICameraSceneNode; class ILightSceneNode;
        class CLensFlareSceneNode; }
    namespace gui   { class IGUIEnvironment; class IGUIFont; }
    namespace video { struct IRenderTarget; class ITexture; class IVideoDriver;
                      class SMaterial; }
    class IrrlichtDevice;
}
using namespace irr;

enum TypeRTT : unsigned int;
class AbstractKart;
class AbstractRenderer;
class Camera;
class FrameBuffer;
class LightNode;
class PerCameraNode;
namespace GE { class GERenderInfo; }
class RenderTarget;

struct SHCoefficients;

/**
  * \brief class that creates the irrLicht device and offers higher-level
  *  ways to manage the 3D scene
  * \ingroup graphics
  */
class IrrDriver : public IEventReceiver, public NoCopy
{    
private:
    /** The irrlicht device. */
    IrrlichtDevice             *m_device;
    /** Irrlicht scene manager. */
    scene::ISceneManager       *m_scene_manager;
    /** Irrlicht gui environment. */
    gui::IGUIEnvironment       *m_gui_env;
    /** Irrlicht video driver. */
    video::IVideoDriver        *m_video_driver;
    /** Irrlicht race font. */
    gui::IGUIFont              *m_race_font;
    /** Renderer. */
    AbstractRenderer           *m_renderer;
    
    /** Wind. */
    Wind                 *m_wind;

    core::dimension2du m_actual_screen_size;

    /** The main MRT setup. */
    core::array<video::IRenderTarget> m_mrt;

    /** Matrixes used in several places stored here to avoid recomputation. */
    core::matrix4 m_ViewMatrix, m_InvViewMatrix, m_ProjMatrix, m_InvProjMatrix, m_ProjViewMatrix, m_InvProjViewMatrix;


private:
    /** Flag to indicate if a resolution change is pending (which will be
     *  acted upon in the next update). None means no change, yes means
     *  change to new resolution and trigger confirmation dialog.
     *  Yes_warn means that the new resolution is unsupported and that
     *  the confirmation dialog needs an additional warning message.
     *  Same indicates a change of the resolution (back to the original
     *  one), but no confirmation dialog. */
    enum {RES_CHANGE_NONE, RES_CHANGE_YES, RES_CHANGE_CANCEL,
          RES_CHANGE_SAME, RES_CHANGE_YES_WARN} m_resolution_changing;


public:
    /** A simple class to store video resolutions. */
    class VideoMode
    {
    private:
        int m_width;
        int m_height;
    public:
        VideoMode(int w, int h) {m_width=w; m_height=h; }
        int getWidth() const  {return m_width;  }
        int getHeight() const {return m_height; }
    };   // VideoMode

    struct BloomData {
        scene::ISceneNode * node;
        float power;
    };

    video::SColorf getAmbientLight() const;



private:
    int                   m_screen_orientation;
    std::vector<VideoMode> m_modes;

    void                  setupViewports();

    /** Whether the mouse cursor is currently shown */
    bool                  m_pointer_shown;

    /** Store if the scene is complex (based on polycount, etc) */
    int                  m_scene_complexity;

    /** Internal method that applies the resolution in user settings. */
    void                 applyResolutionSettings(bool recreate_device);
    void                 createListOfVideoModes();

    bool                 m_request_screenshot;

    bool                 m_ssaoviz;
    bool                 m_shadowviz;
    bool                 m_lightviz;
    bool                 m_boundingboxesviz;
    bool                 m_recording;
    bool                 m_render_nw_debug;

    /** Background colour to reset a buffer. Can be changed by each track. */
    irr::video::SColor m_clear_color;


    unsigned             m_last_light_bucket_distance;
    unsigned             m_skinning_joint;

    SP::SPDynamicDrawCall* m_sun_interposer;
    core::vector3df m_sun_direction;
    video::SColorf m_suncolor;


    std::vector<LightNode *> m_lights;

    std::vector<BloomData> m_forcedbloom;

    std::vector<scene::ISceneNode *> m_background;

    float m_ssao_radius;
    float m_ssao_k;
    float m_ssao_sigma;
    irr::ELOG_LEVEL m_logger_level;
#ifdef DEBUG
    /** Used to visualise skeletons. */
    std::vector<irr::scene::IAnimatedMeshSceneNode*> m_debug_meshes;
#endif
    // ------------------------------------------------------------------------
    void resizeWindow();
public:
    void doScreenShot();    
public:
         IrrDriver();
        ~IrrDriver();
    void initDevice();
    void reset();
    void setMaxTextureSize();
    void unsetMaxTextureSize();
    void getOpenGLData(std::string *vendor, std::string *renderer,
                       std::string *version);

    void increaseObjectCount();
    core::array<video::IRenderTarget> &getMainSetup();
    void updateConfigIfRelevant();
    core::recti getSplitscreenWindow(int WindowNum);
    void setAllMaterialFlags(scene::IMesh *mesh) const;
    scene::IAnimatedMesh *getAnimatedMesh(const std::string &name);
    scene::IMesh         *getMesh(const std::string &name);
    void displayFPS();
    void displayStoryModeTimer();
    bool                  OnEvent(const irr::SEvent &event);
    void                  setAmbientLight(const video::SColorf &light,
                                          bool force_SH_computation = true);
    video::ITexture      *getTexture(FileManager::AssetType type,
                                     const std::string &filename);
    video::ITexture      *getTexture(const std::string &filename);
    void                  grabAllTextures(const scene::IMesh *mesh);
    void                  dropAllTextures(const scene::IMesh *mesh);
    scene::IMesh         *createQuadMesh(const video::SMaterial *material=NULL,
                                         bool create_one_quad=false);
    scene::IMesh         *createTexturedQuadMesh(const video::SMaterial *material,
                                                 const double w, const double h);
    scene::ISceneNode    *addWaterNode(scene::IMesh *mesh, scene::IMesh **welded,
                                       float wave_height,
                                       float wave_speed, float wave_length);
    scene::IMeshSceneNode*addOctTree(scene::IMesh *mesh);
    scene::ISceneNode* addSphere(float radius,
                 const video::SColor &color=video::SColor(128, 255, 255, 255));
    scene::ISceneNode* addMesh(scene::IMesh *mesh,
                               const std::string& debug_name,
                               scene::ISceneNode *parent = NULL,
                               std::shared_ptr<GE::GERenderInfo> render_info = nullptr);
    PerCameraNode        *addPerCameraNode(scene::ISceneNode* node,
                                           scene::ICameraSceneNode* cam,
                                           scene::ISceneNode *parent = NULL);
    scene::ISceneNode    *addBillboard(const core::dimension2d< f32 > size,
                                       const std::string& tex_name,
                                       scene::ISceneNode* parent=NULL);
    scene::IParticleSystemSceneNode
                         *addParticleNode(bool default_emitter=true);
    scene::ISceneNode    *addSkyBox(const std::vector<video::ITexture*> &texture_names,
                                    const std::vector<video::ITexture*> &spherical_harmonics_textures);
    void suppressSkyBox();
    void                  removeNode(scene::ISceneNode *node);
    void                  removeMeshFromCache(scene::IMesh *mesh);
    void                  removeTexture(video::ITexture *t);
    scene::IAnimatedMeshSceneNode
        *addAnimatedMesh(scene::IAnimatedMesh *mesh,
                         const std::string& debug_name,
                         scene::ISceneNode* parent = NULL,
                         std::shared_ptr<GE::GERenderInfo> render_info = nullptr);
    scene::ICameraSceneNode
                         *addCameraSceneNode();
    Camera               *addCamera(unsigned int index, AbstractKart *kart);
    void                  removeCameraSceneNode(scene::ICameraSceneNode *camera);
    void                  removeCamera(Camera *camera);
    void                  update(float dt, bool loading=false);
    /** Call to change resolution */
    void                  changeResolution(const int w, const int h, const bool fullscreen);
  /** Call this to roll back to the previous resolution if a resolution switch attempt goes bad */
    void                  cancelResChange();

    bool                  moveWindow(int x, int y);

    void                  showPointer();
    void                  hidePointer();
    void                  setLastLightBucketDistance(unsigned d) { m_last_light_bucket_distance = d; }
    void                  setSkinningJoint(unsigned d) { m_skinning_joint = d; }
    bool                  isPointerShown() const { return m_pointer_shown; }
    core::position2di     getMouseLocation();

    void                  printRenderStats();
    void                  requestScreenshot();
    class GPUTimer        &getGPUTimer(unsigned);
    const char*           getGPUQueryPhaseName(unsigned);

#ifndef SERVER_ONLY
    std::unique_ptr<RenderTarget> createRenderTarget(const irr::core::dimension2du &dimension,
                                                     const std::string &name);
#endif
    // ------------------------------------------------------------------------
    /** Returns the color to clear the back buffer. */
    const irr::video::SColor& getClearColor() const { return m_clear_color; }
    // ------------------------------------------------------------------------
    /** Sets the color to use when clearing the back buffer. */
    void setClearbackBufferColor(irr::video::SColor color)
    {
        m_clear_color = color;
    }   // setClearbackBufferColor


    /** Returns a list of all video modes supports by the graphics card. */
    const std::vector<VideoMode>& getVideoModes() const { return m_modes; }
    // ------------------------------------------------------------------------
    /** Returns the frame size. */
    const core::dimension2d<u32>& getFrameSize() const;
    // ------------------------------------------------------------------------
    /** Returns the irrlicht device. */
    IrrlichtDevice       *getDevice()       const { return m_device;        }
    // ------------------------------------------------------------------------
    /** Returns the irrlicht video driver. */
    video::IVideoDriver  *getVideoDriver()  const { return m_video_driver;  }
    // ------------------------------------------------------------------------
    /** Returns the irrlicht scene manager. */
    scene::ISceneManager *getSceneManager() const { return m_scene_manager; }
    // ------------------------------------------------------------------------
    /** Returns the gui environment, used to add widgets to a screen. */
    gui::IGUIEnvironment *getGUI() const { return m_gui_env; }
    // ------------------------------------------------------------------------
    /** Returns the current real time, which might not be 0 at start of the
     *  application. Value in msec. */
    unsigned int getRealTime();
    // ------------------------------------------------------------------------
    /** Use motion blur for a short time */
    void giveBoost(unsigned int cam_index) { m_renderer->giveBoost(cam_index);}
    // ------------------------------------------------------------------------
    inline core::vector3df getWind()  {return m_wind->getWind();}

    // -----------------------------------------------------------------------
    /** Returns a pointer to the spherical harmonics coefficients. */
    inline const SHCoefficients* getSHCoefficients()  {return m_renderer->getSHCoefficients();}
    // -----------------------------------------------------------------------
    const core::vector3df& getSunDirection() const { return m_sun_direction; };
    // -----------------------------------------------------------------------
    void setSunDirection(const core::vector3df &SunPos)
    {
        m_sun_direction = SunPos;
    }
    // -----------------------------------------------------------------------
    video::SColorf getSunColor() const { return m_suncolor; }
    // -----------------------------------------------------------------------
    void setSunColor(const video::SColorf &col)
    {
        m_suncolor = col;
    }
    // ------------------------------------------------------------------------
    GLuint getRenderTargetTexture(TypeRTT which);
    GLuint getDepthStencilTexture();
    // ------------------------------------------------------------------------
    void resetDebugModes();
    // ------------------------------------------------------------------------
    void toggleSSAOViz()          { m_ssaoviz = !m_ssaoviz;         }
    // ------------------------------------------------------------------------
    bool getSSAOViz()             { return m_ssaoviz;               }
    // ------------------------------------------------------------------------
    void toggleShadowViz()        { m_shadowviz = !m_shadowviz;     }
    // ------------------------------------------------------------------------
    bool getShadowViz()           { return m_shadowviz;             }
    // ------------------------------------------------------------------------
    void toggleBoundingBoxesViz() { m_boundingboxesviz = !m_boundingboxesviz; }
    // ------------------------------------------------------------------------
    void toggleRenderNetworkDebug() { m_render_nw_debug = !m_render_nw_debug; }
    // ------------------------------------------------------------------------
    bool getRenderNetworkDebug() const            { return m_render_nw_debug; }
    // ------------------------------------------------------------------------
    void renderNetworkDebug();
    // ------------------------------------------------------------------------
    bool getBoundingBoxesViz()    { return m_boundingboxesviz;      }
    // ------------------------------------------------------------------------
    int getSceneComplexity() { return m_scene_complexity;           }
    void resetSceneComplexity() { m_scene_complexity = 0;           }
    void addSceneComplexity(int complexity)
    {
        if (complexity > 1) m_scene_complexity += (complexity - 1);
    }
    // ------------------------------------------------------------------------
    bool isRecording() const { return m_recording; }
    // ------------------------------------------------------------------------
    void setRecording(bool val);
    // ------------------------------------------------------------------------
    std::vector<LightNode *> getLights() { return m_lights; }
    // ------------------------------------------------------------------------
    void addGlowingNode(scene::ISceneNode *n, float r = 1.0f, float g = 1.0f,
                        float b = 1.0f)
    {
        m_renderer->addGlowingNode(n, r, g, b);
    }
    // ------------------------------------------------------------------------
    void clearGlowingNodes() { m_renderer->clearGlowingNodes(); }
    // ------------------------------------------------------------------------
    void addForcedBloomNode(scene::ISceneNode *n, float power = 1)
    {
        BloomData dat;
        dat.node = n;
        dat.power = power;

        m_forcedbloom.push_back(dat);
    }
    // ------------------------------------------------------------------------
    void clearForcedBloom() { m_forcedbloom.clear(); }
    // ------------------------------------------------------------------------
    const std::vector<BloomData> &getForcedBloom() const
    { 
        return m_forcedbloom;
    }
    // ------------------------------------------------------------------------
    void clearBackgroundNodes() { m_background.clear(); }
    // ------------------------------------------------------------------------
    void addBackgroundNode(scene::ISceneNode * const n) 
    {
        m_background.push_back(n);
    }
    // ------------------------------------------------------------------------
    scene::ISceneNode *addLight(const core::vector3df &pos, float energy,
                                float radius, float r, float g, float b,
                                bool sun_ = false, 
                                scene::ISceneNode* parent = NULL);
    // ------------------------------------------------------------------------
    void clearLights();
    // ------------------------------------------------------------------------
    SP::SPDynamicDrawCall* getSunInterposer() { return m_sun_interposer; }
    // ------------------------------------------------------------------------
    
    void cleanSunInterposer();
    void createSunInterposer();
    // ------------------------------------------------------------------------
    void setViewMatrix(core::matrix4 matrix)
    {
        m_ViewMatrix = matrix; matrix.getInverse(m_InvViewMatrix);
    }
    // ------------------------------------------------------------------------
    const core::matrix4 &getViewMatrix() const { return m_ViewMatrix; }
    // ------------------------------------------------------------------------
    const core::matrix4 &getInvViewMatrix() const { return m_InvViewMatrix; }
    // ------------------------------------------------------------------------
    void setProjMatrix(core::matrix4 matrix)
    {
        m_ProjMatrix = matrix; matrix.getInverse(m_InvProjMatrix);
    }
    // ------------------------------------------------------------------------
    const core::matrix4 &getProjMatrix() const { return m_ProjMatrix; }
    // ------------------------------------------------------------------------
    const core::matrix4 &getInvProjMatrix() const { return m_InvProjMatrix; }
    // ------------------------------------------------------------------------
    void genProjViewMatrix() 
    {
        m_ProjViewMatrix = m_ProjMatrix * m_ViewMatrix; 
        m_InvProjViewMatrix = m_ProjViewMatrix; 
        m_InvProjViewMatrix.makeInverse(); 
    }
    // ------------------------------------------------------------------------
    const core::matrix4 &getProjViewMatrix() const { return m_ProjViewMatrix; }
    // ------------------------------------------------------------------------
    const core::matrix4 &getInvProjViewMatrix() const 
    {
        return m_InvProjViewMatrix;
    }
    // ------------------------------------------------------------------------
    const core::vector2df &getCurrentScreenSize() const
    {
        return m_renderer->getCurrentScreenSize();
    }
    // ------------------------------------------------------------------------
    const core::dimension2du getActualScreenSize() const
    { 
        return m_actual_screen_size;
    }
    // ------------------------------------------------------------------------
    float getSSAORadius() const
    {
        return m_ssao_radius;
    }

    // ------------------------------------------------------------------------
    void setSSAORadius(float v)
    {
        m_ssao_radius = v;
    }

    // ------------------------------------------------------------------------
    float getSSAOK() const
    {
        return m_ssao_k;
    }

    // ------------------------------------------------------------------------
    void setSSAOK(float v)
    {
        m_ssao_k = v;
    }

    // ------------------------------------------------------------------------
    float getSSAOSigma() const
    {
        return m_ssao_sigma;
    }

    // ------------------------------------------------------------------------
    void setSSAOSigma(float v)
    {
        m_ssao_sigma = v;
    }
#ifdef DEBUG
    std::vector<scene::IAnimatedMeshSceneNode*> getDebugMeshes()
    {
        return m_debug_meshes;
    }
    /** Removes debug meshes. */
    void clearDebugMesh() { m_debug_meshes.clear(); }
    // ------------------------------------------------------------------------
    /** Adds a debug mesh to be displaed. */
    void addDebugMesh(scene::IAnimatedMeshSceneNode *node)
    {
        m_debug_meshes.push_back(node);
    }   // addDebugMesh

#endif
    void onLoadWorld();
    void onUnloadWorld();

    void updateSplitAndLightcoordRangeFromComputeShaders(size_t width,
                                                         size_t height);

    void uploadLightingData();
    void sameRestart()             { m_resolution_changing = RES_CHANGE_SAME; }
    // ------------------------------------------------------------------------
    u32 getDefaultFramebuffer() const;
    // ------------------------------------------------------------------------
    void handleWindowResize();
};   // IrrDriver

extern IrrDriver *irr_driver;

#endif   // HEADER_IRR_DRIVER_HPP
