//  $Id: World.h,v 1.4 2004/08/11 11:27:21 grumbel Exp $
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

#ifndef HEADER_WORLD_H
#define HEADER_WORLD_H

#include <plib/ssg.h>
#include <vector>
#include "RaceSetup.h"

#define NUM_PROJECTILES  8
#define NUM_EXPLOSIONS   6

#include "Screen.h"

class Herring;
class KartDriver;
class Projectile;
class Explosion;
class Track;
class guUDPConnection;
class GFX;

/** This class keeps all the state of a race, scenegraph, time,
    etc. */
class World : public Screen
{
private:
  static World* current_;
public:
  static World* current() { return current_; }

public:
  ssgRoot      *scene;

  typedef std::vector<KartDriver*> Karts;
  Karts kart;

  Projectile *projectile [ NUM_PROJECTILES ] ;
  Explosion   *explosion [ NUM_EXPLOSIONS  ] ;
  ulClock     *fclock ;

private:
  RaceSetup raceSetup;
  Track        *track;
  guUDPConnection *net;
  int network_enabled;
  int network_testing;
  
  GFX* gfx;

  ssgBranch *trackBranch ;

  Herring *silver_h ;
  Herring *gold_h   ;
  Herring *red_h    ;
  Herring *green_h  ;

public:
  World(const RaceSetup& raceSetup);
  virtual ~World();

  void update();
  void restartRace();
private:
  void updateNetworkRead ();
  void updateLapCounter ( int k );
  void updateNetworkWrite ();
  void load_track(const char *fname );
  void load_players();
  void herring_command(char *s, char *str );
};

#endif

/* EOF */
