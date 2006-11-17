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
#include <plib/ssg.h>
#include "race_setup.hpp"
#include "static_ssg.hpp"
#include "track.hpp"
#include "player_kart.hpp"
#include "physics.hpp"
#include "kart.hpp"


/** This class keeps all the state of a race, scenegraph, time,
    etc. */
class World
{
public:
    ssgRoot      *m_scene;

    typedef std::vector<Kart*> Karts;
private:
    Karts      m_kart;
    StaticSSG* m_static_ssg;
    float      m_finish_delay_start_time;
    int*       m_number_collisions;
    Physics*   m_physics;
public:
    float      m_clock;

    /** resources, this should be put in a separate class or replaced by a smart
     * resource manager
     */
    RaceSetup m_race_setup;

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
        FINISH_PHASE,
        // The state after finish where no calculations are done.
        LIMBO_PHASE,
    };

    int m_ready_set_go;

    const Track* m_track;
private:
    Phase m_phase;

public:
    /** debug text that will be overlaid to the screen */
    std::string m_debug_text[10];

    /** Reference to the track inside the scene */
    ssgBranch *m_track_branch ;

    World(const RaceSetup& raceSetup);
    virtual ~World();
    float GetHOT(sgVec3 start, sgVec3 end, ssgLeaf** leaf, sgVec4** nrm)
    {return m_static_ssg->hot(start, end, leaf, nrm);}
    int   Collision(sgSphere* s, AllHits *a)
    {return m_static_ssg->collision(s,a);       }

    void draw();
    void update(float delta);
    void restartRace();
    void disableRace(); // Put race into limbo phase

    PlayerKart* getPlayerKart(int player);
    Kart* getKart(int kartId)
    {
        assert(kartId >= 0 &&
               kartId < int(m_kart.size()));
        return m_kart[kartId];
    }
    int  getNumKarts() const                  {return (int)m_kart.size();          }
    void addToScene(ssgEntity *kid)           {m_scene->addKid(kid);               }
    void removeFromScene(ssgEntity *kid)      {m_scene->removeKid(kid);            }
    void addCollisions(int kartNumber, int n) {m_number_collisions[kartNumber]+=n;  }

    /** Returns the phase of the game */
    Phase getPhase() const                    { return m_phase;                    }
    float getGravity() const                  { return m_track->getGravity();      }
    Physics *getPhysics() const               { return m_physics;                  }

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
