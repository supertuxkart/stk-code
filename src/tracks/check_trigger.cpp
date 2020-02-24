//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2019 SuperTuxKart-Team
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

#include "tracks/check_trigger.hpp"
#include "karts/abstract_kart.hpp"
#include "modes/world.hpp"
#include "utils/time.hpp"

/** Constructor for a check trigger.
 *  \param center Center point of this trigger
 *  \param distance Kart within it between center will trigger
 *  \param triggering_function callback function to be used when triggered
 */
CheckTrigger::CheckTrigger(const Vec3& center, float distance,
                           std::function<void(int)> triggering_function)
            : CheckStructure(),
              m_center(center), m_distance2(distance * distance),
              m_triggering_function(triggering_function)
{
    m_last_triggered_time = StkTime::getMonoTimeMs();
}   // CheckSphere

// ----------------------------------------------------------------------------
/** Copied from item state.
 */
bool CheckTrigger::isTriggered(const Vec3 &old_pos, const Vec3 &new_pos,
                               int kart_id)
{
    // kart_id will be -1 if called by CheckManager::getChecklineTriggering
    if (kart_id < 0 || kart_id >= (int)World::getWorld()->getNumKarts())
        return false;
    if (m_last_triggered_time + 2000 > StkTime::getMonoTimeMs())
        return false;
    AbstractKart* k = World::getWorld()->getKart(kart_id);
    if ((k->getXYZ() - m_center).length2() < m_distance2)
    {
        m_last_triggered_time = StkTime::getMonoTimeMs();
        return true;
    }
    return false;
}   // isTriggered
