//  $Id: World.h,v 1.1 2004/08/11 00:13:05 grumbel Exp $
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

#include "RaceSetup.h"

class Herring;

/** This class keeps all the state of a race, scenegraph, time,
    etc. */
class World
{
private:
  static World* current_;
public:
  static World* current() { return current_; }

private:
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
  World(RaceSetup& raceSetup);
  ~World();

  void run(RaceSetup& raceSetup);

private:
  void updateNetworkRead ();
  void updateLapCounter ( int k );
  void updateNetworkWrite ();
  void load_track(RaceSetup& raceSetup, const char *fname );
  void load_players();
  void herring_command (RaceSetup& raceSetup, char *s, char *str );
};

#endif

/* EOF */
