//  $Id: WorldScreen.cxx,v 1.1 2004/08/15 13:57:55 grumbel Exp $
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
#include "sound.h"
#include "gfx.h"

WorldScreen::WorldScreen(const RaceSetup& racesetup)
  : world(new World(racesetup)),
    overtime(0)
{
  fclock = new ulClock;
  fclock->reset();
}

WorldScreen::~WorldScreen()
{
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
  
  while ( dt > inc )
    {
      float delta = inc ;
      
      if ( ! widgetSet -> get_paused () )
        world->update(delta);

      dt -= inc ;
    }

  if ( dt > 0.0f )
    overtime = dt;

  updateGFX ( world->gfx ) ;

  pollEvents();
  kartInput (world->raceSetup) ;
  updateGUI(world->raceSetup);
  sound    -> update () ;

  /* Swap graphics buffers last! */
  world->gfx      -> done   () ;
}

/* EOF */
