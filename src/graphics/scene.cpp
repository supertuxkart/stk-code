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

#include "material_manager.hpp"
#include "material.hpp"
#include "camera.hpp"
#include "modes/world.hpp"
#include "tracks/track.hpp"
#include "user_config.hpp"

#include "btBulletDynamicsCommon.h"

#include "scene.hpp"

Scene *stk_scene = 0;

Scene::Scene() //: m_scenegraph(new ssgRoot)
{
}

//-----------------------------------------------------------------------------
Scene::~Scene ()
{
    // delete m_scenegraph;
}

//-----------------------------------------------------------------------------
void Scene::clear ()
{
//    if(m_scenegraph != 0)
//    {
//        m_scenegraph->removeAllKids();
//    }

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
/*
void Scene::add(ssgEntity *kid)
{
    m_scenegraph->addKid( kid );
}

//-----------------------------------------------------------------------------
void Scene::remove(ssgEntity *kid)
{
    m_scenegraph->removeKid( kid );
}*/

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
    glEnable ( GL_DEPTH_TEST ) ;

    const Track* TRACK = RaceManager::getTrack();
#ifndef HAVE_IRRLICHT
    ssgGetLight ( 0 ) -> setPosition ( TRACK->getSunPos() ) ;
    ssgGetLight ( 0 ) -> setColour ( GL_AMBIENT , TRACK->getAmbientCol()  ) ;
    ssgGetLight ( 0 ) -> setColour ( GL_DIFFUSE , TRACK->getDiffuseCol() ) ;
    ssgGetLight ( 0 ) -> setColour ( GL_SPECULAR, TRACK->getSpecularCol() ) ;
    if (TRACK->useFog())
    {
        glEnable ( GL_FOG ) ;

        glFogf ( GL_FOG_DENSITY, TRACK->getFogDensity() ) ;
        glFogfv( GL_FOG_COLOR  , TRACK->getFogColor() ) ;
        glFogf ( GL_FOG_START  , TRACK->getFogStart() ) ;
        glFogf ( GL_FOG_END    , TRACK->getFogEnd() ) ;
        glFogi ( GL_FOG_MODE   , GL_EXP2   ) ;
        glHint ( GL_FOG_HINT   , GL_NICEST ) ;

        /* Clear the screen */
        glClearColor (TRACK->getFogColor()[0],
                      TRACK->getFogColor()[1],
                      TRACK->getFogColor()[2],
                      TRACK->getFogColor()[3]);
    }
    else
    {
        /* Clear the screen */
        glClearColor (TRACK->getSkyColor()[0],
                      TRACK->getSkyColor()[1],
                      TRACK->getSkyColor()[2],
                      TRACK->getSkyColor()[3]);
    }
#endif

    glClear      ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

    for (Cameras::iterator i = m_cameras.begin(); i != m_cameras.end(); ++i)
    {
        (*i)->update(dt);
        (*i)->apply ();
    }   // for cameras

    if (TRACK->useFog())
    {
        glDisable ( GL_FOG ) ;
    }

    glViewport ( 0, 0, user_config->m_width, user_config->m_height ) ;
}
