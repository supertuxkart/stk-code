//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 SuperTuxKart-Team
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

#include "graphics/scene.hpp"

#include "btBulletDynamicsCommon.h"

#include "config/user_config.hpp"
#include "graphics/camera.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "modes/world.hpp"
#include "tracks/track.hpp"

Scene *stk_scene = 0;

Scene::Scene()
{
}

//-----------------------------------------------------------------------------
Scene::~Scene ()
{
}

//-----------------------------------------------------------------------------
void Scene::clear ()
{
    for (Cameras::iterator i = m_cameras.begin(); i != m_cameras.end(); ++i)
        delete *i;

    m_cameras.clear();
}

//-----------------------------------------------------------------------------

Camera *Scene::createCamera(int playerId, const Kart* kart)
{
  Camera *cam = new Camera(playerId, kart);

  m_cameras.push_back(cam);

  return cam;
}

//-----------------------------------------------------------------------------
void Scene::reset()
{
    /** Note: the cameras are reset in player_kart. This is necessary since
     *  the camera needs the correct starting position and rotation of the
     *  kart (to avoid that the camera jumps in the first frame).    */
}   // reset

//-----------------------------------------------------------------------------
void Scene::draw(float dt)
{
    const Track* TRACK = RaceManager::getTrack();
    for (Cameras::iterator i = m_cameras.begin(); i != m_cameras.end(); ++i)
    {
        (*i)->update(dt);
        (*i)->apply ();
    }   // for cameras

}
