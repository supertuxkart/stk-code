//  $Id: WorldScreen.cxx,v 1.2 2004/08/15 15:25:07 grumbel Exp $
//
//  TuxKart - a fun racing game with go-kart
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

#include <plib/ul.h>
#include "World.h"
#include "tuxkart.h"
#include "WidgetSet.h"
#include "RaceSetup.h"
#include "WorldScreen.h"
#include "WorldLoader.h"
#include "sound.h"
#include "Camera.h"
#include "TrackData.h"
#include "TrackManager.h"
#include "gfx.h"

WorldScreen::WorldScreen(const RaceSetup& raceSetup)
  : world(new World(raceSetup)),
    overtime(0)
{
  fclock = new ulClock;
  fclock->reset();
  
  Camera::Mode camera_mode;

  if (raceSetup.numPlayers == 1)
    camera_mode = Camera::ONE_SPLIT;
  else if (raceSetup.numPlayers == 2)
    camera_mode = Camera::TWO_SPLIT;
  else 
    camera_mode = Camera::FOUR_SPLIT; 

  for(int i = 0; i < raceSetup.numPlayers; ++i)
    cameras.push_back(new Camera(camera_mode, i));
}

WorldScreen::~WorldScreen()
{
  for (Cameras::iterator i = cameras.begin(); i != cameras.end(); ++i)
    delete *i;

  delete fclock;
  delete world;
}

void
WorldScreen::update()
{
  fclock->update    () ;
  //updateNetworkRead () ;

  float inc = 0.02f;
  float dt = fclock->getDeltaTime () + overtime;
  overtime = 0;

  for (Cameras::iterator i = cameras.begin(); i != cameras.end(); ++i)
    (*i)->update();

  while ( dt > inc )
    {
      if ( ! widgetSet -> get_paused () )
        world->update(inc);

      dt -= inc ;
    }

  if ( dt > 0.0f )
    overtime = dt;

  draw();

  pollEvents();
  kartInput (world->raceSetup) ;
  updateGUI(world->raceSetup);
  sound    -> update () ;

  updateWorld        () ;

  /* Swap graphics buffers last! */
  world->gfx      -> done   () ;
}

void 
WorldScreen::draw()
{
  TrackData& track_data = track_manager.tracks[World::current()->raceSetup.track];

  glEnable ( GL_DEPTH_TEST ) ;

  if (track_data.use_fog)
    {
      glEnable ( GL_FOG ) ;
      
      glFogf ( GL_FOG_DENSITY, 1.0f / 100.0f ) ;
      glFogfv( GL_FOG_COLOR  , track_data.fog_color ) ;
      glFogf ( GL_FOG_START  , 0.0       ) ;
      glFogf ( GL_FOG_END    , 1000.0      ) ;
      glFogi ( GL_FOG_MODE   , GL_EXP2   ) ;
      glHint ( GL_FOG_HINT   , GL_NICEST ) ;

      /* Clear the screen */
      glClearColor (track_data.fog_color[0], 
                    track_data.fog_color[1], 
                    track_data.fog_color[2], 
                    track_data.fog_color[3]);
    }
  else
    {
      /* Clear the screen */
      glClearColor (track_data.sky_color[0], 
                    track_data.sky_color[1], 
                    track_data.sky_color[2], 
                    track_data.sky_color[3]);
    }

  glClear      ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

  for ( Cameras::iterator i = cameras.begin(); i != cameras.end(); ++i)
    {
      (*i) -> apply () ;
      world->draw() ;
    }

  if (track_data.use_fog)
    {
      glDisable ( GL_FOG ) ;
    }

  glViewport ( 0, 0, getScreenWidth(), getScreenHeight() ) ;

}

/* EOF */
