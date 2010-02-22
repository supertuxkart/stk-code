//  $Id: irr_debug_drawer.cpp 839 2006-10-24 00:01:56Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
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

#include "physics/irr_debug_drawer.hpp"

#include "modes/world.hpp"

IrrDebugDrawer::IrrDebugDrawer()
{
    m_debug_mode = DBG_DrawAabb;
}   // IrrDebugDrawer

// -----------------------------------------------------------------------------
/** Activates the debug view. It makes all karts invisible (in irrlicht), so
 *  that the bullet view can be seen.
 */
void IrrDebugDrawer::activate()
{
    World *world = World::getWorld();
    unsigned int num_karts = world->getNumKarts();
    for(unsigned int i=0; i<num_karts; i++)
    {
        Kart *kart = world->getKart(i);
        if(kart->isEliminated()) continue;
        kart->getNode()->setVisible(false);
    }
}   // activate
// -----------------------------------------------------------------------------
/** Deactivates the bullet debug view, and makes all karts visible again.
 */
void IrrDebugDrawer::deactivate()
{
    World *world = World::getWorld();
    unsigned int num_karts = world->getNumKarts();
    for(unsigned int i=0; i<num_karts; i++)
    {
        Kart *kart = world->getKart(i);
        if(kart->isEliminated()) continue;
        kart->getNode()->setVisible(true);
    }
}   // deactivate
// -----------------------------------------------------------------------------
void IrrDebugDrawer::drawLine(const btVector3& from, const btVector3& to,
                              const btVector3& color)
{
    Vec3 f(from); 
    Vec3 t(to);
    video::SColor c(255, (int)(color.getX()*255), (int)(color.getY()*255),
                         (int)(color.getZ()*255)                          );
    irr_driver->getVideoDriver()->draw3DLine(f.toIrrVector(),
                                             t.toIrrVector(), c);
}

/* EOF */

