//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2013 Joerg Henrichs
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
#include "IrrlichtDevice.h"
#include "ISkinnedMesh.h"
namespace irr
{
    namespace scene { class ISceneManager; class IMesh; class IAnimatedMeshSceneNode; class IAnimatedMesh;
        class IMeshSceneNode; class IParticleSystemSceneNode; class ICameraSceneNode; class ILightSceneNode;
        class CLensFlareSceneNode; }
    namespace gui   { class IGUIEnvironment; class IGUIFont; }
}
using namespace irr;

class ShadowImportanceProvider;

#include "graphics/rtts.hpp"
#include "graphics/shaders.hpp"
#include "graphics/wind.hpp"
#include "io/file_manager.hpp"
#include "utils/aligned_array.hpp"
#include "utils/no_copy.hpp"
#include "utils/ptr_vector.hpp"
#include "utils/vec3.hpp"

class AbstractKart;
class Camera;
class PerCameraNode;
class PostProcessing;
class LightNode;
class ShadowImportance;

enum STKRenderingPass
{
    SOLID_NORMAL_AND_DEPTH_PASS,
    SOLID_LIT_PASS,
    TRANSPARENT_PASS,
    GLOW_PASS,
    DISPLACEMENT_PASS,
    SHADOW_PASS,
    PASS_COUNT,
};

enum QueryPerf
{
    Q_SOLID_PASS1,
    Q_SOLID_PASS2,
    Q_LIGHT,
    Q_SHADOWS,
    Q_LAST
};

/**
  * \brief class that creates the irrLicht device and offers higher-level
  *  ways to manage the 3D scene
  * \ingroup graphics
  */
class IrrDriver : public IEventReceiver, public NoCopy
{
private:
    int GLMajorVersion, GLMinorVersion;
    bool hasVSLayer;
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
    /** Post-processing. */
    PostProcessing             *m_post_processing;
    /** Shaders. */
    Shaders              *m_shaders;
    /** Wind. */
    Wind                 *m_wind;
    float                m_exposure;
    float                m_lwhite;
    /** RTTs. */
    RTT                *m_rtts;
    /** Shadow importance. */
    ShadowImportance   *m_shadow_importance;
    std::vector<core::matrix4> sun_ortho_matrix;

    /** Additional details to be shown in case that a texture is not found.
     *  This is used to specify details like: "while loading kart '...'" */
    std::string                 m_texture_error_message;

    /** The main MRT setup. */
    core::array<video::IRenderTarget> m_mrt;

    /** Matrixes used in several places stored here to avoid recomputation. */
    core::matrix4 m_ViewMatrix, m_InvViewMatrix, m_ProjMatrix, m_InvProjMatrix, m_ProjViewMatrix, m_InvProjViewMatrix;

    std::vector<video::ITexture *> SkyboxTextures;
    std::vector<video::ITexture *> SphericalHarmonicsTextures;

    float blueSHCoeff[9];
    float greenSHCoeff[9];
    float redSHCoeff[9];

    /** Keep a trace of the origin file name of a texture. */
    std::map<video::ITexture*, std::string> m_texturesFileName;

    /** Flag to indicate if a resolution change is pending (which will be
     *  acted upon in the next update). None means no change, yes means
     *  change to new resolution and trigger confirmation dialog.
     *  Cancel indicates a change of the resolution (back to the original
     *  one), but no confirmation dialog. */
    enum {RES_CHANGE_NONE, RES_CHANGE_YES,
          RES_CHANGE_CANCEL}                m_resolution_changing;

public:
    GLuint SkyboxCubeMap;
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

    unsigned getGLSLVersion() const
    {
        if (GLMajorVersion > 3 || (GLMajorVersion == 3 && GLMinorVersion == 3))
            return GLMajorVersion * 100 + GLMinorVersion * 10;
        else if (GLMajorVersion == 3)
            return 100 + (GLMinorVersion + 3) * 10;
        else
            return 120;
    }

    bool hasVSLayerExtension() const
    {
        return hasVSLayer;
    }

    float getExposure() const
    {
        return m_exposure;
    }

    void setExposure(float v)
    {
        m_exposure = v;
    }

    float getLwhite() const
    {
      return m_lwhite;
    }

    void setLwhite(float v)
    {
        m_lwhite = v;
    }

private:
    std::vector<VideoMode> m_modes;

    void                  setupViewports();

    /** Whether the mouse cursor is currently shown */
    bool                  m_pointer_shown;

    /** Supports GLSL */
    bool                  m_glsl;

    /** Internal method that applies the resolution in user settings. */
    void                 applyResolutionSettings();
    void                 createListOfVideoModes();

    bool                 m_request_screenshot;

    bool                 m_wireframe;
    bool                 m_mipviz;
    bool                 m_normals;
    bool                 m_ssaoviz;
    bool                 m_shadowviz;
    bool                 m_lightviz;
    bool                 m_distortviz;
    /** Performance stats */
    unsigned             m_last_light_bucket_distance;
    unsigned             object_count[PASS_COUNT];
    u32                  m_renderpass;
    u32                  m_lensflare_query;
    bool                 m_query_issued;
    class STKMeshSceneNode *m_sun_interposer;
    scene::CLensFlareSceneNode *m_lensflare;
    scene::ICameraSceneNode *m_suncam;

    struct GlowData {
        scene::ISceneNode * node;
        float r, g, b;
    };

    std::vector<GlowData> m_glowing;

    std::vector<LightNode *> m_lights;

    std::vector<BloomData> m_forcedbloom;

    std::vector<scene::ISceneNode *> m_displacing;

    std::vector<scene::ISceneNode *> m_background;

    STKRenderingPass m_phase;

#ifdef DEBUG
    /** Used to visualise skeletons. */
    std::vector<irr::scene::IAnimatedMeshSceneNode*> m_debug_meshes;

    void drawDebugMeshes();
    void drawJoint(bool drawline, bool drawname,
                   irr::scene::ISkinnedMesh::SJoint* joint,
                   irr::scene::ISkinnedMesh* mesh, int id);
#endif

    void renderFixed(float dt);
    void renderGLSL(float dt);
    void renderSolidFirstPass();
    void renderSolidSecondPass();
    void renderTransparent();
    void renderParticles();
    void computeSunVisibility();
    void renderScene(scene::ICameraSceneNode * const camnode, std::vector<GlowData>& glows, float dt, bool hasShadows);
    void computeCameraMatrix(scene::ICameraSceneNode * const camnode);
    void renderShadows();
    void renderGlow(std::vector<GlowData>& glows);
    void renderLights(float dt);
    void renderDisplacement();
    void doScreenShot();
public:
         IrrDriver();
        ~IrrDriver();
    void initDevice();
    void reset();
    void generateSkyboxCubemap();
    void renderSkybox(const scene::ICameraSceneNode *camera);
    void setPhase(STKRenderingPass);
    STKRenderingPass getPhase() const;
    const std::vector<core::matrix4> &getShadowViewProj() const
    {
        return sun_ortho_matrix;
    }
    void IncreaseObjectCount();
    core::array<video::IRenderTarget> &getMainSetup();
    void updateConfigIfRelevant();
    void setAllMaterialFlags(scene::IMesh *mesh) const;
    scene::IAnimatedMesh *getAnimatedMesh(const std::string &name);
    scene::IMesh         *getMesh(const std::string &name);
    video::ITexture      *applyMask(video::ITexture* texture,
                                    const std::string& mask_path);
    void displayFPS();
    bool                  OnEvent(const irr::SEvent &event);
    void                  setAmbientLight(const video::SColor &light);
    std::string           generateSmallerTextures(const std::string& dir);
    std::string           getSmallerTexture(const std::string& texture);
    video::ITexture      *getTexture(FileManager::AssetType type,
                                     const std::string &filename,
                                     bool is_premul=false,
                                     bool is_prediv=false,
                                     bool complain_if_not_found=true);
    video::ITexture      *getTexture(const std::string &filename,
                                     bool is_premul=false,
                                     bool is_prediv=false,
                                     bool complain_if_not_found=true);
    void                  clearTexturesFileName();
    std::string           getTextureName(video::ITexture* tex);
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
    scene::IMeshSceneNode*addSphere(float radius,
                 const video::SColor &color=video::SColor(128, 255, 255, 255));
    scene::IMeshSceneNode*addMesh(scene::IMesh *mesh,
                                  scene::ISceneNode *parent=NULL);
    PerCameraNode        *addPerCameraNode(scene::ISceneNode* node,
                                           scene::ICameraSceneNode* cam,
                                           scene::ISceneNode *parent = NULL);
    scene::ISceneNode    *addBillboard(const core::dimension2d< f32 > size,
                                       video::ITexture *texture,
                                       scene::ISceneNode* parent=NULL, bool alphaTesting = false);

    scene::IParticleSystemSceneNode
                         *addParticleNode(bool default_emitter=true);
    scene::ISceneNode    *addSkyDome(video::ITexture *texture, int hori_res,
                                     int vert_res, float texture_percent,
                                     float sphere_percent);
    scene::ISceneNode    *addSkyBox(const std::vector<video::ITexture*> &texture_names,
                                    const std::vector<video::ITexture*> &sphericalHarmonics);
    void suppressSkyBox();
    void                  removeNode(scene::ISceneNode *node);
    void                  removeMeshFromCache(scene::IMesh *mesh);
    void                  removeTexture(video::ITexture *t);
    scene::IAnimatedMeshSceneNode
                         *addAnimatedMesh(scene::IAnimatedMesh *mesh, scene::ISceneNode* parent=NULL);
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
    void                  setLastLightBucketDistance(unsigned d) { m_last_light_bucket_distance = d; }
    bool                  isPointerShown() const { return m_pointer_shown; }
    core::position2di     getMouseLocation();

    void                  printRenderStats();
    bool                  supportsSplatting();
    void                  requestScreenshot();
    void                  setTextureErrorMessage(const std::string &error,
                                                 const std::string &detail="");
    void                  unsetTextureErrorMessage();
    class GPUTimer        &getGPUTimer(unsigned);

    void draw2dTriangle(const core::vector2df &a, const core::vector2df &b,
                        const core::vector2df &c,
                        const video::ITexture *texture = NULL,
                        const video::SColor *ca=NULL,
                        const video::SColor *cb=NULL,
                        const video::SColor *cc=NULL);



    // ------------------------------------------------------------------------
    /** Convenience function that loads a texture with default parameters
     *  but includes an error message.
     *  \param filename File name of the texture to load.
     *  \param error Error message, potentially with a '%' which will be replaced
     *               with detail.
     *  \param detail String to replace a '%' in the error message.
     */
    video::ITexture* getTexture(const std::string &filename,
                                const std::string &error_message,
                                const std::string &detail="")
    {
        setTextureErrorMessage(error_message, detail);
        video::ITexture *tex = getTexture(filename);
        unsetTextureErrorMessage();
        return tex;
    }   // getTexture

    // ------------------------------------------------------------------------
    /** Convenience function that loads a texture with default parameters
     *  but includes an error message.
     *  \param filename File name of the texture to load.
     *  \param error Error message, potentially with a '%' which will be replaced
     *               with detail.
     *  \param detail String to replace a '%' in the error message.
     */
    video::ITexture* getTexture(const std::string &filename,
                                char *error_message,
                                char *detail=NULL)
    {
        if(!detail)
            return getTexture(filename, std::string(error_message),
                              std::string(""));

        return getTexture(filename, std::string(error_message),
                          std::string(detail));
    }   // getTexture

    // ------------------------------------------------------------------------
    /** Returns the currently defined texture error message, which is used
     *  by event_handler.cpp to print additional info about irrlicht
     *  internal errors or warnings. If no error message is currently
     *  defined, the error message is "".
     */
    const std::string &getTextureErrorMessage()
    {
        return m_texture_error_message;
    }   // getTextureErrorMessage

    // ------------------------------------------------------------------------
    /** Returns a list of all video modes supports by the graphics card. */
    const std::vector<VideoMode>& getVideoModes() const { return m_modes; }
    // ------------------------------------------------------------------------
    /** Returns the frame size. */
    const core::dimension2d<u32>& getFrameSize() const
                       { return m_video_driver->getCurrentRenderTargetSize(); }
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
    unsigned int getRealTime() {return m_device->getTimer()->getRealTime(); }
    // ------------------------------------------------------------------------
    /** Returns a pointer to the post processing object. */
    inline PostProcessing* getPostProcessing()  {return m_post_processing;}
    // ------------------------------------------------------------------------
    inline core::vector3df getWind()  {return m_wind->getWind();}
    // -----------------------------------------------------------------------
    inline video::E_MATERIAL_TYPE getShader(const ShaderType num)  {return m_shaders->getShader(num);}
    // -----------------------------------------------------------------------
    inline void updateShaders()  {m_shaders->loadShaders();}
    // ------------------------------------------------------------------------
    inline video::IShaderConstantSetCallBack* getCallback(const ShaderType num)
    {
        return (m_shaders == NULL ? NULL : m_shaders->m_callbacks[num]);
    }
    // ------------------------------------------------------------------------
    inline GLuint getRenderTargetTexture(TypeRTT which)  { return m_rtts->getRenderTarget(which); }
    inline GLuint getFBO(TypeFBO which)  { return m_rtts->getFBO(which); }
    inline GLuint getDepthStencilTexture()  { return m_rtts->getDepthStencilTexture(); }
    // ------------------------------------------------------------------------
    inline bool isGLSL() const { return m_glsl; }
    // ------------------------------------------------------------------------
    /** Called when the driver pretends to support it, but fails at some
     *  operations. */
    void disableGLSL() { m_glsl = false; }
    // ------------------------------------------------------------------------
    void resetDebugModes()
    {
        m_wireframe = false;
        m_mipviz = false;
        m_normals = false;
        m_ssaoviz = false;
        m_shadowviz = false;
        m_lightviz = false;
        m_distortviz = false;
    }
    // ------------------------------------------------------------------------
    void toggleWireframe() { m_wireframe = !m_wireframe; }
    // ------------------------------------------------------------------------
    void toggleMipVisualization() { m_mipviz = !m_mipviz; }
    // ------------------------------------------------------------------------
    void toggleNormals() { m_normals = !m_normals; }
    // ------------------------------------------------------------------------
    bool getNormals() { return m_normals; }
    // ------------------------------------------------------------------------
    void toggleSSAOViz() { m_ssaoviz = !m_ssaoviz; }
    // ------------------------------------------------------------------------
    void toggleLightViz() { m_lightviz = !m_lightviz; }
    // ------------------------------------------------------------------------
    bool getLightViz() { return m_lightviz; }
    // ------------------------------------------------------------------------
    bool getSSAOViz() { return m_ssaoviz; }
    // ------------------------------------------------------------------------
    void toggleShadowViz() { m_shadowviz = !m_shadowviz; }
    // ------------------------------------------------------------------------
    bool getShadowViz() { return m_shadowviz; }
    // ------------------------------------------------------------------------
    void toggleDistortViz() { m_distortviz = !m_distortviz; }
    // ------------------------------------------------------------------------
    bool getDistortViz() { return m_distortviz; }
    // ------------------------------------------------------------------------
    u32 getRenderPass() { return m_renderpass; }
    // ------------------------------------------------------------------------
    void addGlowingNode(scene::ISceneNode *n, float r = 1.0f, float g = 1.0f, float b = 1.0f)
    {
        GlowData dat;
        dat.node = n;
        dat.r = r;
        dat.g = g;
        dat.b = b;

        m_glowing.push_back(dat);
    }
    // ------------------------------------------------------------------------
    void clearGlowingNodes() { m_glowing.clear(); }
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
    const std::vector<BloomData> &getForcedBloom() const { return m_forcedbloom; }
    // ------------------------------------------------------------------------
    void clearDisplacingNodes() { m_displacing.clear(); }
    // ------------------------------------------------------------------------
    const std::vector<scene::ISceneNode *> &getDisplacingNodes() const { return m_displacing; }
    // ------------------------------------------------------------------------
    void addDisplacingNode(scene::ISceneNode * const n) { m_displacing.push_back(n); }
    // ------------------------------------------------------------------------
    void clearBackgroundNodes() { m_background.clear(); }
    // ------------------------------------------------------------------------
    void addBackgroundNode(scene::ISceneNode * const n) { m_background.push_back(n); }
    // ------------------------------------------------------------------------
    void applyObjectPassShader();
    void applyObjectPassShader(scene::ISceneNode * const node, bool rimlit = false);
    // ------------------------------------------------------------------------
    scene::ISceneNode *addLight(const core::vector3df &pos, float energy = 1., float r = 1.0f,
                  float g = 1.0f, float b = 1.0f, bool sun = false, scene::ISceneNode* parent = NULL);
    // ------------------------------------------------------------------------
    void clearLights();
    // ------------------------------------------------------------------------
    class STKMeshSceneNode *getSunInterposer() { return m_sun_interposer; }
    // ------------------------------------------------------------------------
    void setViewMatrix(core::matrix4 matrix) { m_ViewMatrix = matrix; matrix.getInverse(m_InvViewMatrix); }
    const core::matrix4 &getViewMatrix() const { return m_ViewMatrix; }
    const core::matrix4 &getInvViewMatrix() const { return m_InvViewMatrix; }
    void setProjMatrix(core::matrix4 matrix) { m_ProjMatrix = matrix; matrix.getInverse(m_InvProjMatrix); }
    const core::matrix4 &getProjMatrix() const { return m_ProjMatrix; }
    const core::matrix4 &getInvProjMatrix() const { return m_InvProjMatrix; }
    void genProjViewMatrix() { m_ProjViewMatrix = m_ProjMatrix * m_ViewMatrix; m_InvProjViewMatrix = m_ProjViewMatrix; m_InvProjViewMatrix.makeInverse(); }
    const core::matrix4 &getProjViewMatrix() const { return m_ProjViewMatrix; }
    const core::matrix4 &getInvProjViewMatrix() const { return m_InvProjViewMatrix; }
#ifdef DEBUG
    /** Removes debug meshes. */
    void clearDebugMesh() { m_debug_meshes.clear(); }
    // ------------------------------------------------------------------------
    /** Adds a debug mesh to be displaed. */
    void addDebugMesh(scene::IAnimatedMeshSceneNode *node)
    {
        m_debug_meshes.push_back(node);
    }   // addDebugMesh

#endif
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


};   // IrrDriver

extern IrrDriver *irr_driver;

#endif   // HEADER_IRR_DRIVER_HPP
