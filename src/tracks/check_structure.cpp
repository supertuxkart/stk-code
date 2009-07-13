//  $Id: check_structure.cpp 1681 2008-04-09 13:52:48Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009  Joerg Henrichs
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

#include "karts/kart.hpp"
#include "modes/world.hpp"
#include "race/race_manager.hpp"
#include "tracks/check_manager.hpp"
#include "tracks/check_structure.hpp"


CheckStructure::CheckStructure(CheckManager *check_manager, 
                               const XMLNode &node)
{
    m_check_manager = check_manager;
    std::string type;
    node.get("type", &type);
    if(type=="new-lap")
        m_check_type = CT_NEW_LAP;
    else if(type=="reset-new-lap")
        m_check_type = CT_RESET_NEW_LAP;
    else
    {
        printf("Unknown check structure '%s' - ignored.\n", type.c_str());
    }
}   // CheckStructure

// ----------------------------------------------------------------------------
/** Initialises the 'previous positions' of all karts with the start position
 *  defined for this track.
 *  \param track The track object defining the start positions.
 */
void CheckStructure::reset(const Track &track)
{
	m_previous_position.clear();
    m_is_active.clear();
	for(unsigned int i=0; i<race_manager->getNumKarts(); i++)
	{
		const Vec3 &xyz = race_manager->getKart(i)->getXYZ();
		m_previous_position.push_back(xyz);
        
        // Deactivate all lap counters, activate everything else
        m_is_active.push_back(m_check_type!=CT_NEW_LAP);
	}   // for i<getNumKarts
}   // reset

// ----------------------------------------------------------------------------
/** Updates all check structures. Called one per time step.
 *  \param dt Time since last call.
 */
void CheckStructure::update(float dt)
{
	for(unsigned int i=0; i<race_manager->getNumKarts(); i++)
	{
		const Vec3 &xyz = race_manager->getKart(i)->getXYZ();
        // Only check active checklines.
        if(m_is_active[i] && isTriggered(m_previous_position[i], xyz, i))
        {
            trigger(i);
        }
        m_previous_position[i] = xyz;
	}   // for i<getNumKarts
}   // update

// ----------------------------------------------------------------------------
/** Is called when this check structure is triggered. This then can cause
 *  a lap to be counted, animation to be started etc.
 */
void CheckStructure::trigger(unsigned int kart_index)
{
    switch(m_check_type)
    {
    case CT_NEW_LAP : RaceManager::getWorld()->newLap(kart_index); 
                      m_is_active[kart_index] = false;
                      break;
    case CT_RESET_NEW_LAP : 
                      m_check_manager->activateNewLapChecks(kart_index);
                      break;
    default:          break;
    }   // switch m_check_type
}   // trigger

// ----------------------------------------------------------------------------
/** If this check structure is a new-lap check, activate it again.
 *  \param kart_index Index of the kart for which the lap check is activated.
 */
void CheckStructure::activateNewLapCheck(int kart_index)
{
    if(m_check_type==CT_NEW_LAP)
        m_is_active[kart_index] = true;
}   // resetNewLap
