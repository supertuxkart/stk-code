//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 Joerg Henrichs
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

#include "graphics/irr_driver.hpp"
#include "karts/abstract_kart.hpp"
#include "modes/world.hpp"

#include <ISceneManager.h>
#include <ISceneNode.h>
#include <ICameraSceneNode.h>

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
    setDebugMode((DebugModeType) ((m_debug_mode+1) % 3));
}
// -----------------------------------------------------------------------------

void IrrDebugDrawer::setDebugMode(DebugModeType mode)
{
    m_debug_mode = mode;
    World *world = World::getWorld();
    unsigned int num_karts = world->getNumKarts();
    for(unsigned int i=0; i<num_karts; i++)
    {
        AbstractKart *kart = world->getKart(i);
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

    //World::getWorld()->getCa

    if (from.distance2(m_camera_pos) > 10000) return;

    std::vector<float>& v = m_lines[c];
    v.push_back(from.getX());
    v.push_back(from.getY());
    v.push_back(from.getZ());
    v.push_back(to.getX());
    v.push_back(to.getY());
    v.push_back(to.getZ());
    //draw3DLine((const core::vector3df&)from, (const core::vector3df&)to, c);
}

// -----------------------------------------------------------------------------

void IrrDebugDrawer::beginNextFrame()
{
    for (std::map<video::SColor, std::vector<float> >::iterator it = m_lines.begin(); it != m_lines.end(); it++)
    {
        it->second.clear();
    }

    scene::ICameraSceneNode* camera = irr_driver->getSceneManager()->getActiveCamera();
    m_camera_pos = camera->getPosition();
}

/* EOF */

