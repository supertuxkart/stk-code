//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Joerg Henrichs
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

#include <string>
#include <vector>

#include <IVideoDriver.h>
#include <vector2d.h>
#include <dimension2d.h>
#include <SColor.h>
#include <IrrlichtDevice.h>
namespace irr
{
    namespace scene { class ISceneManager; class IMesh; class IAnimatedMeshSceneNode; class IAnimatedMesh;
        class IMeshSceneNode; class IParticleSystemSceneNode; class ICameraSceneNode; class ILightSceneNode; }
    namespace gui   { class IGUIEnvironment; class IGUIFont; }
}
using namespace irr;

#include "post_processing.hpp"
#include "utils/aligned_array.hpp"
#include "utils/no_copy.hpp"
#include "utils/ptr_vector.hpp"
#include "utils/vec3.hpp"

class AbstractKart;
class Camera;
class PerCameraNode;

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
    /** Post-processing */
    PostProcessing              m_post_processing;
    
    /** Flag to indicate if a resolution change is pending (which will be
     *  acted upon in the next update). None means no change, yes means
     *  change to new resolution and trigger confirmation dialog. 
     *  Cancel indicates a change of the resolution (back to the original
     *  one), but no confirmation dialog. */
    enum {RES_CHANGE_NONE, RES_CHANGE_YES, 
          RES_CHANGE_CANCEL}                m_resolution_changing;
    
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
private:
    std::vector<VideoMode> m_modes;

    void                  setupViewports();
    video::E_DRIVER_TYPE  getEngineDriverType(int index);
    
    /** Whether the mouse cursor is currently shown */
    bool                  m_pointer_shown;
    
    /** Internal method that applies the resolution in user settings. */
    void                 applyResolutionSettings();
    void                 createListOfVideoModes();

    bool                 m_request_screenshot;
    
public:
                          IrrDriver();
                         ~IrrDriver();
    void                  initDevice();
    
    void                  updateConfigIfRelevant();
    
    void setAllMaterialFlags(scene::IMesh *mesh) const;

    /** Returns a list of all video modes supports by the graphics card. */
    const std::vector<VideoMode>& getVideoModes() const { return m_modes; }
    /** Returns the frame size. */
    const core::dimension2d<u32>& getFrameSize() const 
                       { return m_video_driver->getCurrentRenderTargetSize(); }
    /** Returns the irrlicht device. */
    IrrlichtDevice       *getDevice()       const { return m_device;        }
    /** Returns the irrlicht video driver. */
    video::IVideoDriver  *getVideoDriver()  const { return m_video_driver;  }
    /** Returns the irrlicht scene manager. */
    scene::ISceneManager *getSceneManager() const { return m_scene_manager; }
    scene::IAnimatedMesh *getAnimatedMesh(const std::string &name);
    scene::IMesh         *getMesh(const std::string &name);
    /** Returns the gui environment, used to add widgets to a screen. */
    gui::IGUIEnvironment *getGUI() const { return m_gui_env; }
    //irr::gui::IGUIFont   *getRaceFont() const { return m_race_font; }
    
    video::ITexture      *applyMask(video::ITexture* texture, 
                                    const std::string& mask_path);
    
    void                  displayFPS();
    /** this is not really used to process events, it's only used to shut down irrLicht's
      * chatty logging until the event handler is ready to take the task
      */
    bool                  OnEvent(const irr::SEvent &event);
    
    void                  setAmbientLight(const video::SColor &light);
    video::ITexture      *getTexture(const std::string &filename,
                                     bool is_premul=false,
                                     bool is_prediv=false,
                                     bool complain_if_not_found=true);
    void                  grabAllTextures(const scene::IMesh *mesh);
    void                  dropAllTextures(const scene::IMesh *mesh);
    scene::IMesh         *createQuadMesh(const video::SMaterial *material=NULL, 
                                         bool create_one_quad=false);
    scene::IMesh         *createTexturedQuadMesh(const video::SMaterial *material, 
                                                 const double w, const double h);
    scene::ISceneNode    *addWaterNode(scene::IMesh *mesh, float wave_height,
                                       float wave_speed, float wave_length);
    scene::IMeshSceneNode*addOctTree(scene::IMesh *mesh);
    scene::IMeshSceneNode*addMesh(scene::IMesh *mesh,
                                  scene::ISceneNode *parent=NULL);
    PerCameraNode        *addPerCameraMesh(scene::IMesh* mesh, 
                                           scene::ICameraSceneNode* node,
                                           scene::ISceneNode *parent = NULL);
    scene::ISceneNode    *addBillboard(const core::dimension2d< f32 > size, 
                                       video::ITexture *texture, 
                                       scene::ISceneNode* parent=NULL);

    scene::IParticleSystemSceneNode
                         *addParticleNode(bool default_emitter=true);
    scene::ISceneNode    *addSkyDome(video::ITexture *texture, int hori_res,
                                     int vert_res, float texture_percent, 
                                     float sphere_percent);
    scene::ISceneNode    *addSkyBox(const std::vector<video::ITexture*> &texture_names);
    void                  removeNode(scene::ISceneNode *node);
    void                  removeMeshFromCache(scene::IMesh *mesh);
    void                  removeTexture(video::ITexture *t);
    scene::IAnimatedMeshSceneNode
                         *addAnimatedMesh(scene::IAnimatedMesh *mesh);
    scene::ICameraSceneNode 
                         *addCameraSceneNode();
    Camera               *addCamera(unsigned int index, AbstractKart *kart);
    void                  removeCameraSceneNode(scene::ICameraSceneNode *camera);
    void                  removeCamera(Camera *camera);
    void                  update(float dt);
    
    /** Call to change resolution */
    void                  changeResolution(const int w, const int h, const bool fullscreen);
  /** Call this to roll back to the previous resolution if a resolution switch attempt goes bad */
    void                  cancelResChange();

    bool                  moveWindow(const int x, const int y);
    
    void                  showPointer();
    void                  hidePointer();
    bool                  isPointerShown() const { return m_pointer_shown; }
    core::position2di     getMouseLocation();
    
    void                  printRenderStats();
    /** Returns the current real time, which might not be 0 at start of the
     *  application. Value in msec.
     */
    unsigned int getRealTime() {return m_device->getTimer()->getRealTime(); }
    

    void draw2dTriangle(const core::vector2df &a, const core::vector2df &b,
                        const core::vector2df &c, 
                        const video::ITexture *texture = NULL,
                        const video::SColor *ca=NULL,  
                        const video::SColor *cb=NULL,
                        const video::SColor *cc=NULL);
    
    inline PostProcessing* getPostProcessing()  {return &m_post_processing;}
    
    bool supportsSplatting();
    
    void requestScreenshot();
    
    // --------------------- RTT --------------------
    /**
      * Class that provides RTT (currently, only when no other 3D rendering 
      * in the main scene is required)
      * Provides an optional 'setupRTTScene' method to make it quick and easy
      * to prepare rendering of 3D objects but you can also manually set the 
      * scene/camera. If you use the factory 'setupRTTScene', cleanup can be
      * done through 'tearDownRTTScene' (destructor will also do this). If 
      * you set it up manually, you need to clean it up manually.
      */
    class RTTProvider
    {
        /** A pointer to texture on which a scene is rendered. Only used
         *  in between beginRenderToTexture() and endRenderToTexture calls. */
        video::ITexture            *m_render_target_texture;
        
        bool                        m_persistent_texture;
        
        /** Main node of the RTT scene */
        scene::ISceneNode          *m_rtt_main_node;
        
        scene::ICameraSceneNode    *m_camera;
        
        scene::ILightSceneNode     *m_light;
        
        /** Irrlicht video driver. */
        video::IVideoDriver        *m_video_driver;
        
    public:
        RTTProvider(const core::dimension2du &dimension, 
                    const std::string &name, bool persistent_texture);
        
        ~RTTProvider();
        
        /**
          * \brief Quick utility method to setup a scene from a plain list 
          *  of models
          *
          * Sets up a given vector of meshes for render-to-texture. Ideal to
          * embed a 3D object inside the GUI. If there are multiple meshes,
          * the first mesh is considered to be the root, and all following 
          * meshes will have their locations relative to the location of the 
          * first mesh.
          *
          * \param mesh             The list of meshes to add to the scene
          * \param mesh_location    Location of each fo these meshes
          * \param model_frames     For animated meshes, which frame to use 
          *                         (value can be -1 to set none)
          *                         When frame is not -1, the corresponding 
          *                         IMesh must be an IAnimatedMesh.
          * \pre           The 3 vectors have the same size.
          */
        void setupRTTScene(PtrVector<scene::IMesh, REF>& mesh, 
                           AlignedArray<Vec3>& mesh_location,
                           AlignedArray<Vec3>& mesh_scale,
                           const std::vector<int>& model_frames);
        
        /** Optional 'angle' parameter will rotate the object added 
         *  *through setupRTTScene* */
        video::ITexture* renderToTexture(float angle=-1,
                                         bool is_2d_render=false);
        
        void tearDownRTTScene();
        
    };
    
#ifdef DEBUG
    std::vector<irr::scene::IAnimatedMeshSceneNode*> debug_meshes;
#endif
    

};   // IrrDriver

extern IrrDriver *irr_driver;

#endif   // HEADER_IRR_DRIVER_HPP
