//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015  Joerg Henrichs
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

#include "config/user_config.hpp"
#include "io/xml_node.hpp"
#include "karts/kart.hpp"
#include "modes/linear_world.hpp"
#include "modes/world.hpp"
#include "network/network_string.hpp"
#include "race/race_manager.hpp"
#include "tracks/check_lap.hpp"
#include "tracks/check_manager.hpp"
#include "tracks/track.hpp"

#include <algorithm>


CheckStructure::CheckStructure(const XMLNode &node, unsigned int index)
{
    m_index              = index;
    m_check_type         = CT_NEW_LAP;

    // This structure is actually filled by the check manager (necessary
    // in order to support track reversing).
    m_next_check_structures.clear();
    m_previous_check_structures.clear();
    m_same_group.clear();

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
    // Cannons don't have a kind specified, so test for the name in this case
    else if(node.getName()=="cannon")
        m_check_type = CT_CANNON;
    else if(node.getName()=="goal")
        m_check_type = CT_GOAL;
    else
        Log::warn("CheckStructure", "Unknown check structure '%s' - ignored.", kind.c_str());

    node.get("same-group", &m_same_group);
    // Make sure that the index of this check structure is included in
    // the same_group list. While this should be guaranteed by the
    // current track exporter, tracks exported with the old track
    // exporter will not have this.
    if(std::find(m_same_group.begin(), m_same_group.end(), (int)m_index)
            == m_same_group.end())
        m_same_group.push_back(m_index);

    // As a default, only lap lines, cannons and goals are activated
    m_active_at_reset= m_check_type==CT_NEW_LAP || m_check_type==CT_CANNON || m_check_type==CT_GOAL;
    node.get("active", &m_active_at_reset);
}   // CheckStructure

// ----------------------------------------------------------------------------
CheckStructure::CheckStructure()
              : m_active_at_reset(true),
                m_index(Track::getCurrentTrack()->getCheckManager()
                ->getCheckStructureCount()),
                m_check_type(CT_TRIGGER)
{
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
    m_backwards_active.clear();

    World *world = World::getWorld();
    for(unsigned int i=0; i<world->getNumKarts(); i++)
    {
        const Vec3 &xyz = world->getKart(i)->getXYZ();
        m_previous_position.push_back(xyz);

        // Activate all relevant checklines
        m_is_active.push_back(m_active_at_reset);
        m_backwards_active.push_back(false);
    }   // for i<getNumKarts
}   // reset

// ----------------------------------------------------------------------------
/** Updates all check structures. Called one per time step.
 *  \param dt Time since last call.
 */
void CheckStructure::update(float dt)
{
    World *world = World::getWorld();
    LinearWorld* lw = dynamic_cast<LinearWorld*>(World::getWorld());
    for(unsigned int i=0; i<world->getNumKarts(); i++)
    {
        const Vec3 &xyz = world->getKart(i)->getFrontXYZ();
        if(world->getKart(i)->getKartAnimation()) continue;

        // Check active checklines that the kart needs to cross to progress,
        // and also the previous checkline to prevent lap completion going backwards
        if((m_is_active[i] || m_backwards_active[i])
            && isTriggered(m_previous_position[i], xyz, i))
        {
            if(UserConfigParams::m_check_debug)
                Log::info("CheckStructure",
                          "Check structure %d triggered for kart %s at %f.",
                          m_index, world->getKart(i)->getIdent().c_str(),
                          World::getWorld()->getTime());
            trigger(i);
            if (triggeringCheckline() && lw)
                lw->updateCheckLinesServer(getIndex(), i);
        }
        m_previous_position[i] = xyz;
    }   // for i<getNumKarts
}   // update

// ----------------------------------------------------------------------------
// TODO : a lot of this code is specific to checklines, but it's in a class
//        which also serves as a parent for many other objects.
//        Evaluate the opportunity of getting rid of inheritance here to
//        consolidate things.
/** Changes the status (active/inactive) of all check structures contained
 *  in the index list indices.
 *  \param indices List of index of check structures in the CheckManager that
 *                 are to be changed.
 *  \param int kart_index For which the status should be changed.
 *  \param change_state How to change the state (active, deactivate, toggle).
 */
void CheckStructure::changeStatus(const std::vector<int> &indices,
                                  int kart_index, ChangeState change_state)
{
    int player_kart_index = 0;
    if (World::getWorld()->getPlayerKart(0))
        player_kart_index = World::getWorld()->getPlayerKart(0)->getWorldKartId();
    bool update_debug_colors =
        UserConfigParams::m_check_debug && RaceManager::get()->getNumPlayers()>0 &&
        kart_index == player_kart_index;
    bool activate_exclusive_done = false;

    CheckManager* cm = Track::getCurrentTrack()->getCheckManager();
    for(unsigned int i=0; i<indices.size(); i++)
    {
        CheckStructure *cs = cm->getCheckStructure(indices[i]);
        if (cs == NULL) continue;

        switch(change_state)
        {
        case CS_DEACTIVATE:
            cs->m_is_active[kart_index] = false;
            // Fall-through
        case CS_BACKWARDS_DEACTIVATE:
            cs->m_backwards_active[kart_index] = false;
            if(UserConfigParams::m_check_debug)
            {
                Log::info("CheckStructure", "Deactivating %d for %s.",
                          indices[i],
                          World::getWorld()->getKart(kart_index)->getIdent().c_str());
            }
            break;
        case CS_ACTIVATE_EXCLUSIVE:
            // Say checkline 2 was triggered then checkline 1 was triggered backwards,
            // we want to activate checkline 2 back but also ensure that checkline 3
            // is properly desactivated.
            // As all the checklines in a group should share the same successors,
            // we only need to do this once
            if (!activate_exclusive_done)
            {
                changeStatus(cs->m_next_check_structures, kart_index, CS_DEACTIVATE);
                activate_exclusive_done = true;
            }
            // Fall-through
        case CS_ACTIVATE:
            cs->m_is_active[kart_index] = true;
            if(UserConfigParams::m_check_debug)
            {
                Log::info("CheckStructure", "Activating %d for %s.",
                          indices[i],
                          World::getWorld()->getKart(kart_index)->getIdent().c_str());
            }
            break;
        case CS_BACKWARDS_ACTIVATE:
            // We don't want to call CheckLap's isTriggered function to avoid
            // incorrect lap validations. Laplines also don't check for being crossed
            // physically like other checklines do.
            if (cs->m_check_type == CT_NEW_LAP)
                break;

            cs->m_backwards_active[kart_index] = true;
            if(UserConfigParams::m_check_debug)
            {
                Log::info("CheckStructure", "Activating backward driving monitoring at %d for %s.",
                          indices[i],
                          World::getWorld()->getKart(kart_index)->getIdent().c_str());
            }
            // Stop backwards driving monitoring for the checkline group even before the one we just activated
            if (!activate_exclusive_done)
            {
                changeStatus(cs->m_previous_check_structures, kart_index, CS_BACKWARDS_DEACTIVATE);
                activate_exclusive_done = true;
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
                Log::info("CheckStructure", "Toggling %d for %s from %d.",
                          indices[i],
                          World::getWorld()->getKart(kart_index)->getIdent().c_str(),
                          cs->m_is_active[kart_index]==true);
            }
            cs->m_is_active[kart_index] = !cs->m_is_active[kart_index];
        }   // switch
        if(update_debug_colors)
        {
            cs->changeDebugColor(cs->m_is_active[kart_index], cs->m_backwards_active[kart_index]);
        }
    }   // for i<indices.size()

    if(UserConfigParams::m_check_debug)
    {
        printf("--------\n");
        for (unsigned int n=0; n<cm->getCheckStructureCount(); n++)
        {
            CheckStructure *cs = cm->getCheckStructure(n);
            if (cs->getType() == CT_CANNON)
                printf("Checkline %i (CANNON) - always active, doesn't count for lap validation\n", n);
            else if (dynamic_cast<CheckLap*>(cs) != NULL)
                printf("Checkline %i (LAP) : %i\n", n, (int)cs->m_is_active[kart_index] + 2*(int)cs->m_backwards_active[kart_index]);
            else
                printf("Checkline %i : %i\n", n, (int)cs->m_is_active[kart_index] + 2*(int)cs->m_backwards_active[kart_index]);
        }
    }

}   //changeStatus

// ----------------------------------------------------------------------------
/** Is called when this check structure is triggered. This then can cause
 *  a lap to be counted, animation to be started etc.
 */
void CheckStructure::trigger(unsigned int kart_index)
{
    switch(m_check_type)
    {
    case CT_NEW_LAP :
        World::getWorld()->newLap(kart_index);
        if(UserConfigParams::m_check_debug)
        {
            Log::info("CheckStructure", "%s new lap %d triggered",
                      World::getWorld()->getKart(kart_index)->getIdent().c_str(),
                      m_index);
        }
        changeStatus(m_next_check_structures, kart_index, CS_ACTIVATE);
        // Remove any backward-active status remaining
        // (the lapline doesn't have a predecessor)
        for (unsigned int n=0; n < Track::getCurrentTrack()->getCheckManager()->getCheckStructureCount(); n++)
        {
            CheckStructure *cs = Track::getCurrentTrack()->getCheckManager()->getCheckStructure(n);
            if (cs->m_backwards_active[kart_index])
            {
                std::vector<int> indice;
                indice.push_back(n);
                changeStatus(indice, kart_index, CS_BACKWARDS_DEACTIVATE);
            }
        }
        break;

    case CT_ACTIVATE:
        if (m_backwards_active[kart_index])
            changeStatus(m_next_check_structures, kart_index, CS_ACTIVATE_EXCLUSIVE);
        else
            changeStatus(m_next_check_structures, kart_index, CS_ACTIVATE);

        changeStatus(m_previous_check_structures, kart_index, CS_BACKWARDS_ACTIVATE);
        break;

    case CT_TOGGLE:
        changeStatus(m_next_check_structures, kart_index, CS_TOGGLE);
        break;

    default:
        break;
    }   // switch m_check_type
    changeStatus(m_same_group, kart_index, CS_DEACTIVATE);
}   // trigger

// ----------------------------------------------------------------------------
void CheckStructure::saveCompleteState(BareNetworkString* bns)
{
    World* world = World::getWorld();
    for (unsigned int i = 0; i < world->getNumKarts(); i++)
        bns->add(m_previous_position[i]).addUInt8(m_is_active[i]      ? 1 :
                                                  m_backwards_active[i] ? 2 : 0);
}   // saveCompleteState

// ----------------------------------------------------------------------------
void CheckStructure::restoreCompleteState(const BareNetworkString& b)
{
    m_previous_position.clear();
    m_is_active.clear();
    m_backwards_active.clear();
    World* world = World::getWorld();
    for (unsigned int i = 0; i < world->getNumKarts(); i++)
    {
        Vec3 xyz = b.getVec3();
        m_previous_position.push_back(xyz);
        uint8_t active_value = b.getUInt8();
        m_is_active.push_back(active_value == 1);
        m_backwards_active.push_back(active_value == 2);
    }
}   // restoreCompleteState

// ----------------------------------------------------------------------------
void CheckStructure::saveIsActive(int kart_id, BareNetworkString* bns)
{
    bns->addUInt8(m_is_active[kart_id]      ? 1 :
                  m_backwards_active[kart_id] ? 2 : 0);
}   // saveIsActive

// ----------------------------------------------------------------------------
void CheckStructure::restoreIsActive(int kart_id, const BareNetworkString& b)
{
    uint8_t active_value = b.getUInt8();
    m_is_active.at(kart_id) = (active_value == 1);
    m_backwards_active.at(kart_id) = (active_value == 2);
}   // restoreIsActive
