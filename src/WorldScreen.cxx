//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
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

#include <iostream>
#include <plib/ul.h>
#include "sdldrv.h"
#include "World.h"
#include "tuxkart.h"
#include "WidgetSet.h"
#include "RaceSetup.h"
#include "WorldScreen.h"
#include "WorldLoader.h"
#include "sound.h"
#include "Camera.h"
#include "TrackManager.h"

WorldScreen* WorldScreen::current_ = 0;

WorldScreen::WorldScreen(const RaceSetup& raceSetup)
  : overtime(0)
{
  delete world;
  world = new World(raceSetup);

  current_ = this;
  fclock = new ulClock;
  fclock->reset();
  
  for(int i = 0; i < raceSetup.getNumPlayers(); ++i)
    cameras.push_back(new Camera(raceSetup.getNumPlayers(), i));
}

WorldScreen::~WorldScreen()
{
  for (Cameras::iterator i = cameras.begin(); i != cameras.end(); ++i)
    delete *i;

  delete fclock;

  if(current() == this){
    delete world;
    world = 0;
  }
}

void
WorldScreen::update()
{
  fclock->update    () ;

  float inc = 0.02f;
  float dt = fclock->getDeltaTime () + overtime;
  overtime = 0;

  while ( dt > inc )
    {
      if ( ! widgetSet -> get_paused () )
        world->update(inc);

      dt -= inc ;
    }

  if ( dt > 0.0f )
    overtime = dt;


  for (Cameras::iterator i = cameras.begin(); i != cameras.end(); ++i)
    (*i)->update();

  draw();

  pollEvents();
  kartInput (world->raceSetup) ;
  updateGUI();
  sound    -> update () ;

  updateWorld        () ;

  swapBuffers() ;
}

void 
WorldScreen::draw()
{
  const Track* track = world->track;

  glEnable ( GL_DEPTH_TEST ) ;

  if (track->use_fog)
    {
      glEnable ( GL_FOG ) ;
      
      glFogf ( GL_FOG_DENSITY, track->fog_density ) ;
      glFogfv( GL_FOG_COLOR  , track->fog_color ) ;
      glFogf ( GL_FOG_START  , track->fog_start ) ;
      glFogf ( GL_FOG_END    , track->fog_end ) ;
      glFogi ( GL_FOG_MODE   , GL_EXP2   ) ;
      glHint ( GL_FOG_HINT   , GL_NICEST ) ;

      /* Clear the screen */
      glClearColor (track->fog_color[0], 
                    track->fog_color[1], 
                    track->fog_color[2], 
                    track->fog_color[3]);
    }
  else
    {
      /* Clear the screen */
      glClearColor (track->sky_color[0], 
                    track->sky_color[1], 
                    track->sky_color[2], 
                    track->sky_color[3]);
    }

  glClear      ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

  for ( Cameras::iterator i = cameras.begin(); i != cameras.end(); ++i)
    {
      (*i) -> apply () ;
      world->draw() ;
    }

  if (track->use_fog)
    {
      glDisable ( GL_FOG ) ;
    }

  glViewport ( 0, 0, getScreenWidth(), getScreenHeight() ) ;
}

Camera*
WorldScreen::getCamera(int i) const
{
  if (i >= 0 && i < int(cameras.size()))
    return cameras[i];
  else
    return 0;
}

/* EOF */
