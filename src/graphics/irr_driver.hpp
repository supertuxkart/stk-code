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

#include "irrlicht.h"
using namespace irr;

class IrrDriver : public IEventReceiver
{
private:
    /** The irrlicht device. */
    IrrlichtDevice             *m_device;
    /** Irrlicht scene manager. */
    scene::ISceneManager       *m_scene_manager;
    /** Irrlicht gui environment. */
    gui::IGUIEnvironment       *m_gui_env;
    /** Irrlicht race font. */
    irr::gui::IGUIFont         *m_race_font;

public:
                          IrrDriver();
                         ~IrrDriver();
    IrrlichtDevice       *getDevice()       const { return m_device;        }
    scene::ISceneManager *getSceneManager() const { return m_scene_manager; }
    scene::IAnimatedMesh *getAnimatedMesh(const std::string &name);
    scene::IMesh         *getMesh(const std::string &name);
    /** Returns the gui environment, used to add widgets to a screen. */
    gui::IGUIEnvironment *getGUI() const { return m_gui_env; }
    irr::gui::IGUIFont   *getRaceFont() const { return m_race_font; }
    bool                  OnEvent(const irr::SEvent &event);
    void                  setAmbientLight(const video::SColor &light);
    video::ITexture      *getTexture(const std::string &filename);
    scene::ISceneNode    *addOctTree(scene::IMesh *mesh);
    scene::ISceneNode    *addMesh(scene::IMesh *mesh);
    void                  removeNode(scene::ISceneNode *node);
    void                  removeMesh(scene::IMesh *mesh);
    scene::ISceneNode    *addAnimatedMesh(scene::IAnimatedMesh *mesh);
    scene::ICameraSceneNode 
                         *addCamera();
    void                  update(float dt);
};   // IrrDriver

extern IrrDriver *irr_driver;

#endif   // HEADER_IRR_DRIVER_HPP

