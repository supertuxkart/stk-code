//  $Id: irr_driver.hpp 694 2006-08-29 07:42:36Z hiker $
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

#include <string>
#include <vector>

#include "utils/ptr_vector.hpp"
#include "utils/vec3.hpp"

#include "irrlicht.h"
using namespace irr;

struct VideoMode
{
    int width, height;
};

class IrrDriver : public IEventReceiver
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
    irr::gui::IGUIFont         *m_race_font;

    /** A pointer to texture on which a scene is rendered. Only used
     *  in between beginRenderToTexture() and endRenderToTexture calls. */
    video::ITexture            *m_render_target_texture;
    void setAllMaterialFlags(scene::IAnimatedMesh *mesh) const;
    std::vector<VideoMode> m_modes;

    void renderBulletDebugView();
    void displayFPS();
public:
                          IrrDriver();
                         ~IrrDriver();
    void                 initDevice();
    
    /** Returns a list of all video modes supports by the graphics card. */
    const std::vector<VideoMode>& getVideoModes() const { return m_modes; }
    /** Returns the frame size. */
    const core::dimension2d<s32> getFrameSize() const 
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
    irr::gui::IGUIFont   *getRaceFont() const { return m_race_font; }
    bool                  OnEvent(const irr::SEvent &event);
    void                  setAmbientLight(const video::SColor &light);
    video::ITexture      *getTexture(const std::string &filename);
    scene::IMesh         *createQuadMesh(const video::SMaterial *material=NULL, 
                                         bool create_one_quad=false);
    scene::ISceneNode    *addWaterNode(scene::IMesh *mesh, float wave_height,
                                       float wave_speed, float wave_length);
    scene::ISceneNode    *addOctTree(scene::IMesh *mesh);
    scene::ISceneNode    *addMesh(scene::IMesh *mesh);
    scene::IParticleSystemSceneNode
                         *addParticleNode(bool default_emitter=true);
    scene::ISceneNode    *addSkyDome(const std::string &texture, int hori_res,
                                     int vert_res, float texture_percent, 
                                     float sphere_percent);
    scene::ISceneNode    *addSkyBox(const std::vector<std::string> &texture_names);
    void                  removeNode(scene::ISceneNode *node);
    void                  removeMesh(scene::IMesh *mesh);
    scene::IAnimatedMeshSceneNode
                         *addAnimatedMesh(scene::IAnimatedMesh *mesh);
    scene::ICameraSceneNode 
                         *addCamera();
    void                  update(float dt);
    
    void                  changeResolution();
    void showPointer();
    void hidePointer();
    
    void renderToTexture(ptr_vector<scene::IMesh, REF>& mesh, 
                         std::vector<Vec3>& mesh_location, 
                         video::ITexture* target, float angle);
    void beginRenderToTexture(const core::dimension2di &dimension, 
                              const std::string &name);
    video::ITexture *endRenderToTexture();
};   // IrrDriver

extern IrrDriver *irr_driver;

#endif   // HEADER_IRR_DRIVER_HPP

