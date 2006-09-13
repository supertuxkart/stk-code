//  $Id$
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

#ifndef HEADER_WORLD_H
#define HEADER_WORLD_H

#include <vector>
#include "race_setup.hpp"
#include "static_ssg.hpp"
#include "track.hpp"
#include "player_kart.hpp"

class ssgRoot;
class ssgBranch;
class ssgLeaf;
class ssgEntity;

class Herring;
class guUDPConnection;
class Kart;

/** This class keeps all the state of a race, scenegraph, time,
    etc. */
class World {
public:
  ssgRoot      *scene;

  typedef std::vector<Kart*> Karts;
private:
  Karts      kart;
  StaticSSG* staticSSG;
  float      finishDelayStartTime;
  int*       numberCollisions;
public:
  float      clock;

  /** resources, this should be put in a separate class or replaced by a smart
   * resource manager
   */
  RaceSetup raceSetup;

  enum Phase { 
    // The traffic light is shown and all players stay in position
    START_PHASE,
    // The traffic light turned green and all driver, drive around the track
    RACE_PHASE,
    // All players have finished, now wait a certain amount of time for AI 
    // karts to finish. If they do not finish in that time, finish the race
    DELAY_FINISH_PHASE,
    // The player crossed the finishing line and his and the time of
    // the other players is displayed, controll is automatic
    FINISH_PHASE 
  };

  int ready_set_go;

  const Track* track;
private:
  Phase phase;
public:
  /** debug text that will be overlaid to the screen */
  std::string debugtext[10];
  
  /** Reference to the track inside the scene */
  ssgBranch *trackBranch ;

  World(const RaceSetup& raceSetup);
  virtual ~World();
  float GetHOT(sgVec3 start, sgVec3 end, ssgLeaf** leaf, sgVec4** nrm) 
                              {return staticSSG->hot(start, end, leaf, nrm);}
  int   Collision(sgSphere* s, AllHits *a) 
                              {return staticSSG->collision(s,a);       }

  void draw();
  void update(float delta);
  void restartRace();
  
  PlayerKart* getPlayerKart(int player);
  Kart* getKart(int kartId)                 {assert(kartId >= 0 && 
						    kartId < int(kart.size()));
                                             return kart[kartId];              }
  int  getNumKarts() const                  {return (int)kart.size();          }
  void addToScene(ssgEntity *kid)           {scene->addKid(kid);               }
  void removeFromScene(ssgEntity *kid)      {scene->removeKid(kid);            }
  void addCollisions(int kartNumber, int n) {numberCollisions[kartNumber]+=n;  }

  /** Returns the phase of the game */
  Phase getPhase() const { return phase; }
  float getGravity() const { return track->getGravity();}
private:
  void updateRacePosition( int k );
  void loadTrack();
  void loadPlayers();
  void herring_command(sgVec3* loc, char htype, int bNeedHeight);
  void checkRaceStatus();
};

extern World* world;

#endif

/* EOF */
