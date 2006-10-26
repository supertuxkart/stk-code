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

#include "physics.hpp"
#include "ssg_help.hpp"

#ifdef BULLET

// -----------------------------------------------------------------------------
/// Initialise physics.
Physics::Physics(float gravity)
{
    m_dynamics_world = new btDiscreteDynamicsWorld();
    m_dynamics_world->setGravity(btVector3(0.0f, 0.0f, -gravity));

    //    btCollisionShape box = new btBoxShape(btVector3(10.0f, 10.0f, 10.0f));
}   // Physics

// -----------------------------------------------------------------------------
Physics::~Physics()
{
    delete m_dynamics_world;
}   // ~Physics

// -----------------------------------------------------------------------------
void Physics::addKart(Kart *kart)
{
    ssgEntity *model = kart->getModel();
    float x_min, x_max, y_min, y_max;
    MinMax(model, &x_min, &x_max, &y_min, &y_max);
    printf("Kart %s: x: %f - %f  y: %f - %f\n", kart->getName().c_str(), x_min, x_max, y_min, y_max);
}   // addKart

// -----------------------------------------------------------------------------

#endif
/* EOF */
  
