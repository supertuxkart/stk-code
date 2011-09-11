//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010  Joerg Henrichs
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

#include "tracks/check_lap.hpp"

#include <string>

#include "io/xml_node.hpp"
#include "modes/linear_world.hpp"
#include "race/race_manager.hpp"
#include "tracks/check_manager.hpp"
#include "tracks/track.hpp"

/** Constructor for a lap line.
 *  \param check_manager Pointer to the check manager, which is needed when
 *         resetting e.g. new lap counters. 
 *  \param node XML node containing the parameters for this checkline.
 */
CheckLap::CheckLap(CheckManager *check_manager, const XMLNode &node, 
                   unsigned int index) 
         : CheckStructure(check_manager, node, index)
{
    // Note that when this is called the karts have not been allocated
    // in world, so we can't call world->getNumKarts()
    m_previous_distance.resize(race_manager->getNumberOfKarts());
}   // CheckLap

// ----------------------------------------------------------------------------
void CheckLap::reset(const Track &track)
{
    CheckStructure::reset(track);
    for(unsigned int i=0; i<m_previous_distance.size(); i++)
    {
        m_previous_distance[i] = 0;
    }
}   // reset

// ----------------------------------------------------------------------------
/** True if going from old_pos to new_pos crosses this checkline. This function
 *  is called from update (of the checkline structure).
 *  \param old_pos  Position in previous frame.
 *  \param new_pos  Position in current frame.
 *  \param indx     Index of the kart, can be used to store kart specific
 *                  additional data.
 */
bool CheckLap::isTriggered(const Vec3 &old_pos, const Vec3 &new_pos, int indx)
{
    float track_length = World::getWorld()->getTrack()->getTrackLength();
    float current_distance = 
        ((LinearWorld*)World::getWorld())->getDistanceDownTrackForKart(indx);
    bool result =(m_previous_distance[indx]>0.95f*track_length &&
                  current_distance<7.0f);
    if(UserConfigParams::m_check_debug && result)
    {
        printf("CHECK: Kart %s crossed start line from %f to %f.\n",
                World::getWorld()->getKart(indx)->getIdent().c_str(),
                m_previous_distance[indx], current_distance);
    }
    m_previous_distance[indx] = current_distance;
    
    
    // If a lap was triggered, reset all visit flags
    if (result)
    {
        CheckManager* cm = World::getWorld()->getTrack()->getCheckManager();
        for (int n=0; n<cm->getCheckStructureCount(); n++)
        {
            cm->getCheckStructure(n)->resetVisits();
        }
    }
    return result;
}   // isTriggered
