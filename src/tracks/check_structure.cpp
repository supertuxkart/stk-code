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

#include "tracks/check_structure.hpp"

#include <algorithm>

#include "karts/kart.hpp"
#include "modes/linear_world.hpp"
#include "modes/world.hpp"
#include "race/race_manager.hpp"
#include "tracks/check_lap.hpp"
#include "tracks/check_manager.hpp"


CheckStructure::CheckStructure(CheckManager *check_manager, 
                               const XMLNode &node, unsigned int index)
{
    m_index              = index;
    m_check_manager      = check_manager;
    std::string kind;
    node.get("kind", &kind);
    if(kind=="lap")
        m_check_type = CT_NEW_LAP;
    else if(kind=="activate")
        m_check_type = CT_ACTIVATE;
    else if(kind=="toggle")
        m_check_type = CT_TOGGLE;
    else if(kind=="ambient-light")
        m_check_type = CT_AMBIENT_SPHERE;
    else
    {
        printf("Unknown check structure '%s' - ignored.\n", kind.c_str());
    }
    m_check_structures_to_change_state.clear();
    node.get("other-ids", &m_check_structures_to_change_state);
    // Backwards compatibility to tracks exported with older versions of
    // the track exporter
    if(m_check_structures_to_change_state.size()==0)
        node.get("other-id", &m_check_structures_to_change_state);

    m_same_group.clear();
    node.get("same-group", &m_same_group);
    // Make sure that the index of this check structure is included in
    // the same_group list. While this should be guaranteed by the
    // current track exporter, tracks exported with the old track
    // exporter will not have this.
    if(std::find(m_same_group.begin(), m_same_group.end(), (int)m_index)
            == m_same_group.end())
        m_same_group.push_back(m_index);

    // As a default, only lap lines are activated
    m_active_at_reset= m_check_type==CT_NEW_LAP;
    node.get("active", &m_active_at_reset);
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
    
    World *world = World::getWorld();
    for(unsigned int i=0; i<world->getNumKarts(); i++)
    {
        const Vec3 &xyz = world->getKart(i)->getXYZ();
        m_previous_position.push_back(xyz);
        
        // Activate all checkline
        m_is_active.push_back(m_active_at_reset);
    }   // for i<getNumKarts
}   // reset

// ----------------------------------------------------------------------------
/** Updates all check structures. Called one per time step.
 *  \param dt Time since last call.
 */
void CheckStructure::update(float dt)
{
    World *world = World::getWorld();
    for(unsigned int i=0; i<world->getNumKarts(); i++)
    {
        const Vec3 &xyz = world->getKart(i)->getXYZ();
        // Only check active checklines.
        if(m_is_active[i] && isTriggered(m_previous_position[i], xyz, i))
        {
            if(UserConfigParams::m_check_debug)
                printf("CHECK: Check structure %d triggered for kart %s.\n",
                       m_index, world->getKart(i)->getIdent().c_str());
            trigger(i);
        }
        m_previous_position[i] = xyz;
    }   // for i<getNumKarts
}   // update

// ----------------------------------------------------------------------------
/** Changes the status (active/inactive) of all check structures contained
 *  in the index list indices.
 *  \param indices List of index of check structures in check_manager that
 *                 are to be changed.
 *  \param int kart_index For which the status should be changed.
 *  \param change_state How to change the state (active, deactivate, toggle).
 */
void CheckStructure::changeStatus(const std::vector<int> indices, 
                                  int kart_index,
                                  ChangeState change_state)
{
    bool update_debug_colors = 
        UserConfigParams::m_check_debug &&
        kart_index == (int)World::getWorld()->getPlayerKart(0)->getWorldKartId();

    for(unsigned int i=0; i<indices.size(); i++)
    {
        CheckStructure *cs = 
            m_check_manager->getCheckStructure(indices[i]);
        if (cs == NULL) continue;
        
        switch(change_state)
        {
        case CS_DEACTIVATE: 
            cs->m_is_active[kart_index] = false; 
            if(UserConfigParams::m_check_debug)
            {
                printf("CHECK: Deactivating %d for %s.\n",
                       indices[i],
                       World::getWorld()->getKart(kart_index)->getIdent().c_str());
            }
            break;
        case CS_ACTIVATE:
            cs->m_is_active[kart_index] = true;
            if(UserConfigParams::m_check_debug)
            {
                printf("CHECK: Activating %d for %s.\n",
                       indices[i],
                       World::getWorld()->getKart(kart_index)->getIdent().c_str());
            }
            break;
        case CS_TOGGLE:
            if(UserConfigParams::m_check_debug)
            {
                // At least on gcc 4.3.2 we can't simply print 
                // cs->m_is_active[kart_index] ("cannot pass objects of
                // non-POD type 'struct std::_Bit_reference' through '...'; 
                // call will abort at runtime"). So we use this somewhat
                // unusual but portable construct.
                printf("CHECK: Toggling %d for %s from %d.\n",
                       indices[i],
                       World::getWorld()->getKart(kart_index)->getIdent().c_str(),
                       cs->m_is_active[kart_index]==true);
            }
            cs->m_is_active[kart_index] = !cs->m_is_active[kart_index]; 
        }   // switch
        if(update_debug_colors)
        {
            cs->changeDebugColor(cs->m_is_active[kart_index]);
        }
    }   // for i<indices.size()
    
    /*
    printf("--------\n");
    for (int n=0; n<m_check_manager->getCheckStructureCount(); n++)
    {
        CheckStructure *cs = m_check_manager->getCheckStructure(n);
        if (dynamic_cast<CheckLap*>(cs) != NULL)
            printf("Checkline %i (LAP) : %i\n", n, (int)cs->m_is_active[kart_index]);
        else
            printf("Checkline %i : %i\n", n, (int)cs->m_is_active[kart_index]);

    }
    */
    
}   //changeStatus

// ----------------------------------------------------------------------------
/** Is called when this check structure is triggered. This then can cause
 *  a lap to be counted, animation to be started etc.
 */
void CheckStructure::trigger(unsigned int kart_index)
{    
    World* w = World::getWorld();
    LinearWorld* lw = dynamic_cast<LinearWorld*>(w);
    if (lw != NULL)
    {
        lw->getTrackSector(kart_index).setLastTriggeredCheckline(m_index);
    }
    
    switch(m_check_type)
    {
    case CT_NEW_LAP : 
        World::getWorld()->newLap(kart_index); 
        if(UserConfigParams::m_check_debug)
        {
            printf("CHECK: %s new lap %d triggered\n",
                   World::getWorld()->getKart(kart_index)->getIdent().c_str(),
                   m_index);
        }
        changeStatus(m_check_structures_to_change_state, 
                     kart_index, CS_ACTIVATE);
        break;
    case CT_ACTIVATE: 
        changeStatus(m_check_structures_to_change_state,
                     kart_index, CS_ACTIVATE);
        break;
    case CT_TOGGLE:   
        changeStatus(m_check_structures_to_change_state,
                     kart_index, CS_TOGGLE);
        break;
    default: 
        break;
    }   // switch m_check_type
    changeStatus(m_same_group, kart_index, CS_DEACTIVATE);
}   // trigger
