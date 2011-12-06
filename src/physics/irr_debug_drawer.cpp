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
    m_debug_mode = DM_NONE;
}   // IrrDebugDrawer

// -----------------------------------------------------------------------------
/** Activates the next debug mode, or switches the mode off again.
 */
void IrrDebugDrawer::nextDebugMode()
{
    // Go to next debug mode. Note that debug mode 3 (
    m_debug_mode = (DebugModeType) ((m_debug_mode+1) % 3);
    World *world = World::getWorld();
    unsigned int num_karts = world->getNumKarts();
    for(unsigned int i=0; i<num_karts; i++)
    {
        Kart *kart = world->getKart(i);
        if(kart->isEliminated()) continue;
        kart->getNode()->setVisible(!(m_debug_mode & DM_NO_KARTS_GRAPHICS));
    }
}   // nextDebugMode

// -----------------------------------------------------------------------------
void IrrDebugDrawer::drawLine(const btVector3& from, const btVector3& to,
                              const btVector3& color)
{
    video::SColor c(255, (int)(color.getX()*255), (int)(color.getY()*255),
                         (int)(color.getZ()*255)                          );
    irr_driver->getVideoDriver()->draw3DLine((const core::vector3df&)from,
                                             (const core::vector3df&)to, c);
}

/* EOF */

