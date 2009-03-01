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

#ifdef HAVE_IRRLICHT

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
    scene::ISceneManager       *m_scene_manager;

public:
                          IrrDriver();
                         ~IrrDriver();
    IrrlichtDevice       *getDevice()       const { return m_device;        }
    scene::ISceneManager *getSceneManager() const { return m_scene_manager; }
    scene::IAnimatedMesh *getAnimatedMesh(const std::string &name);
    scene::IMesh         *getMesh(const std::string &name);
    bool                  OnEvent(const irr::SEvent &event);
    scene::ISceneNode    *addOctTree(scene::IMesh *mesh);
    scene::ISceneNode    *addMesh(scene::IMesh *mesh);
    scene::ISceneNode    *addAnimatedMesh(scene::IAnimatedMesh *mesh);
    scene::ICameraSceneNode 
                         *addCamera();
    void                  update(float dt);
};   // IrrDriver

extern IrrDriver *irr_driver;

#endif   // HEADER_IRR_DRIVER_HPP

#endif   // HAVE_IRRLICHT

