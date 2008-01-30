//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#include <plib/ssg.h>
#include "material_manager.hpp"
#include "material.hpp"
#include "camera.hpp"
#include "track.hpp"
#include "world.hpp"
#include "user_config.hpp"

#include "btBulletDynamicsCommon.h"
#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#include "scene.hpp"

Scene *scene = 0;

Scene::Scene() :
m_scenegraph(new ssgRoot)
{
}

//-----------------------------------------------------------------------------
Scene::~Scene ()
{
    delete m_scenegraph;
}

//-----------------------------------------------------------------------------
void Scene::clear ()
{
    if(m_scenegraph != 0)
    {
        m_scenegraph->removeAllKids();
    }

    for (Cameras::iterator i = m_cameras.begin(); i != m_cameras.end(); ++i)
        delete *i;

    m_cameras.clear();
}

//-----------------------------------------------------------------------------

Camera *
Scene::createCamera(int numPlayers, int playerId)
{
  Camera *cam = new Camera(numPlayers, playerId);

  m_cameras.push_back(cam);

  return cam;
}

//-----------------------------------------------------------------------------
void Scene::add(ssgEntity *kid)
{
    m_scenegraph->addKid( kid );
}

//-----------------------------------------------------------------------------
void Scene::remove(ssgEntity *kid)
{
    m_scenegraph->removeKid( kid );
}

//-----------------------------------------------------------------------------
void Scene::draw(float dt)
{
    glEnable ( GL_DEPTH_TEST ) ;

    const Track* TRACK = world->m_track;

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

    glClear      ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

    for (Cameras::iterator i = m_cameras.begin(); i != m_cameras.end(); ++i)
    {
        (*i)->update(dt);
        (*i) -> apply () ;

        if(!user_config->m_bullet_debug)
        {
            ssgCullAndDraw ( m_scenegraph );
        }
        else
        {
            // Use bullets debug drawer
            GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
            GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
            GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
            /*	light_position is NOT default value	*/
            GLfloat light_position0[] = { 1.0, 1.0, 1.0, 0.0 };
            GLfloat light_position1[] = { -1.0, -1.0, -1.0, 0.0 };
            
            glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
            glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
            glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
            glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
            
            glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
            glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
            glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
            glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
            
            glEnable(GL_LIGHTING);
            glEnable(GL_LIGHT0);
            glEnable(GL_LIGHT1);
            
            glShadeModel(GL_SMOOTH);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
            
            glClearColor(0.8,0.8,0.8,0);
            
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            float f=2.0f;
            glFrustum(-f, f, -f, f, 1.0, 1000.0);
            
            btVector3 pos;
            sgCoord *c = world->getKart(world->getNumKarts()-1)->getCoord();
            gluLookAt(c->xyz[0], c->xyz[1]-5.f, c->xyz[2]+4,
                      c->xyz[0], c->xyz[1],     c->xyz[2],
                      0.0f, 0.0f, 1.0f);
            glMatrixMode(GL_MODELVIEW);
            
            for (World::Karts::size_type i = 0 ; i < world->getNumKarts(); ++i)
            {
                world->getKart((int)i)->draw();
            }
            world->getPhysics()->draw();
        }   //  bullet_debug
    }   // for cameras

    if (TRACK->useFog())
    {
        glDisable ( GL_FOG ) ;
    }

    glViewport ( 0, 0, user_config->m_width, user_config->m_height ) ;
}

