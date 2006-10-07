//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
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

#include <SDL/SDL.h>

#include "world.hpp"
#include "widget_set.hpp"
#include "world_screen.hpp"
#include "sound_manager.hpp"
#include "camera.hpp"
#include "config.hpp"
#include "track_manager.hpp"
#include "track.hpp"
#include "gui/menu_manager.hpp"
#include "history.hpp"

WorldScreen* WorldScreen::current_ = 0;

WorldScreen::WorldScreen(const RaceSetup& raceSetup)
{
  // the constructor assigns this object to the global
  // variable world. Admittedly a bit ugly, but simplifies
  // handling of objects which get created in the constructor
  // and need world to be defined.
  new World(raceSetup);

  current_ = this;

  for(int i = 0; i < raceSetup.getNumPlayers(); ++i)
    cameras.push_back(new Camera(raceSetup.getNumPlayers(), i));
  fclock.reset();
  fclock.setMaxDelta(1.0);
  frameClock.reset();
  frameClock.setMaxDelta(100000.0);
  frameCount = 0;
}

WorldScreen::~WorldScreen()
{
  for (Cameras::iterator i = cameras.begin(); i != cameras.end(); ++i)
    delete *i;

  if(current() == this) {
  delete world;
  world = 0;
  }
}

void WorldScreen::update() {
  fclock.update();

  if ( ! widgetSet -> get_paused () ) {
    world->update(fclock.getDeltaTime());
  }

  for (Cameras::iterator i = cameras.begin(); i != cameras.end(); ++i)
    (*i)->update();

  draw();

  menu_manager->update();
  sound_manager->update() ;
  if(config->profile) {
    frameCount++;
    if (world->clock>config->profile) {
      // The actual timing for FPS has to be done with an external clock,
      // since world->clock might be modified by replaying a history file.
      frameClock.update();
      printf("Number of frames: %d time %f, Average FPS: %f\n",
	     frameCount, frameClock.getAbsTime(), 
	     (float)frameCount/frameClock.getAbsTime());
      if(!config->replayHistory) history->Save();
      exit(-2);
    }
  }   // if profile

  SDL_GL_SwapBuffers() ;
}

void
WorldScreen::draw()
{
  const Track* track = world->track;

  glEnable ( GL_DEPTH_TEST ) ;

  if (track->useFog())
    {
      glEnable ( GL_FOG ) ;

      glFogf ( GL_FOG_DENSITY, track->getFogDensity() ) ;
      glFogfv( GL_FOG_COLOR  , track->getFogColor() ) ;
      glFogf ( GL_FOG_START  , track->getFogStart() ) ;
      glFogf ( GL_FOG_END    , track->getFogEnd() ) ;
      glFogi ( GL_FOG_MODE   , GL_EXP2   ) ;
      glHint ( GL_FOG_HINT   , GL_NICEST ) ;

      /* Clear the screen */
      glClearColor (track->getFogColor()[0],
                    track->getFogColor()[1],
                    track->getFogColor()[2],
                    track->getFogColor()[3]);
    }
  else
    {
      /* Clear the screen */
      glClearColor (track->getSkyColor()[0],
                    track->getSkyColor()[1],
                    track->getSkyColor()[2],
                    track->getSkyColor()[3]);
    }

  glClear      ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

  for ( Cameras::iterator i = cameras.begin(); i != cameras.end(); ++i)
    {
      (*i) -> apply () ;
      world->draw() ;
    }

  if (track->useFog())
    {
      glDisable ( GL_FOG ) ;
    }

  glViewport ( 0, 0, config->width, config->height ) ;
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
