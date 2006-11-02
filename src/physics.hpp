//  $Id: physics.hpp 839 2006-10-24 00:01:56Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006 SuperTuxKart-Team, Joerg Henrichs, Steve Baker
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

#ifndef HEADER_PHYSICS_H
#define HEADER_PHYSICS_H

#include "kart.hpp"

#ifdef BULLET
#include "bullet/btBulletDynamicsCommon.h"
#include "../bullet/Demos/OpenGL/GLDebugDrawer.h"
class Physics 
{
protected:
    btDynamicsWorld *m_dynamics_world;
    Kart            *m_kart;
    GLDebugDrawer   *m_debug_drawer;
public:
         Physics  (float gravity);
        ~Physics  ();
    void addKart  (Kart *k, btRaycastVehicle **v, 
                   btRaycastVehicle::btVehicleTuning **t);
    void update   (float dt);
    void set_track(ssgEntity *track);
};

// For non-bullet version: empty object
#else
class Physics 
{
public:
         Physics(float gravity) {};
        ~Physics() {};
    void update(float dt) {};
    void set_track(ssgEntity *track) {};
};

#endif

#endif
/* EOF */
  
