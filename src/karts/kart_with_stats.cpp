//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011  Joerg Henrichs
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

#include "karts/kart_with_stats.hpp"

#include "items/item.hpp"

KartWithStats::KartWithStats(const std::string& ident, Track* track, 
                             int position,  bool is_first_kart,
                             const btTransform& init_transform, 
                             RaceManager::KartType type)
             : Kart(ident, track, position, is_first_kart, 
                    init_transform, type)
{
}   // KartWithStats

// ----------------------------------------------------------------------------
/** Called at the start of each race, esp. in case of a restart.
 */
void KartWithStats::reset()
{
    m_top_speed         = 0.0f; 
    m_explosion_time    = 0.0f;
    m_explosion_count   = 0;
    m_skidding_time     = 0.0f;
    m_rescue_count      = 0;
    m_rescue_time       = 0.0f;
    m_bonus_count       = 0;
    m_banana_count      = 0;
    m_small_nitro_count = 0;
    m_large_nitro_count = 0;
    m_bubblegum_count   = 0;
    Kart::reset();
}   // reset

// ----------------------------------------------------------------------------
/** This function is called each timestep, and it collects most of the 
 *  statistics for this kart.
 *  \param dt Time step size.
 */
void KartWithStats::update(float dt)
{
    Kart::update(dt);
    if(getSpeed()>m_top_speed) m_top_speed = getSpeed();
    if(getControls().m_drift)
        m_skidding_time += dt;
}   // update

// ----------------------------------------------------------------------------
/** Called when an explosion should be triggered. If the explosion actually
 *  happens (i.e. the kart is neither invulnerable nor is it already playing
 *  an emergency animation), it increases the number of times a kart was
 *  exploded, and adds up the overall time spent in animation as well.
 *  \param pos The position of the explosion.
 *  \param direct_hit If the kart was hit directly, or if this is only a
 *         seconday hit.
 */
void KartWithStats::handleExplosion(const Vec3& pos, bool direct_hit)
{
    bool is_new_explosion = !playingEmergencyAnimation() && !isInvulnerable();
    Kart::handleExplosion(pos, direct_hit);
    // If a kart is too far away from an explosion to be affected, its timer
    // will be 0, and this should then not be counted as an explosion.
    if(is_new_explosion && EmergencyAnimation::m_timer>0)
    {
        m_explosion_count ++;
        m_explosion_time += EmergencyAnimation::m_timer;
    }

}   // handleExplosion

// ----------------------------------------------------------------------------
/** Called when a kart is being rescued. It counts the number of times a
 *  kart is being rescued, and sums up the time for rescue as well.
 *  \param is_auto_rescue True if this is an automatically triggered rescue.
 */
void KartWithStats::forceRescue(bool is_auto_rescue)
{
    bool is_new_rescue = !playingEmergencyAnimation();
    Kart::forceRescue(is_auto_rescue);

    // If there wasn't already a rescue happening, count this event:
    if(is_new_rescue)
    {
        m_rescue_count++;
        m_rescue_time += EmergencyAnimation::m_timer;
    }
}   // forceRescue

// ----------------------------------------------------------------------------
/** Called when an item is collected. It will increment private variables that
 *  represent counters for each type of item hit.
 *  \param item The item that was hit.
 *  \param add_info Additional info, used in networking games to force
 *         a specific item to be used (instead of a random item) to keep
 *         all karts in synch.
 */
void KartWithStats::collectedItem(Item *item, int add_info)
{
    Kart::collectedItem(item, add_info);
    const Item::ItemType type = item->getType();

    switch (type)
    {
    case Item::ITEM_BANANA:
        m_banana_count++;
        break;
    case Item::ITEM_NITRO_SMALL:
        m_small_nitro_count++;
        break;
    case Item::ITEM_NITRO_BIG:
        m_large_nitro_count++;
        break;
    case Item::ITEM_BONUS_BOX:
        m_bonus_count++;
        break;
    case Item::ITEM_BUBBLEGUM:
        m_bubblegum_count++;
        break;
    default        : break;
    }   // switch TYPE

}   // collectedItem

//-----------------------------------------------------------------------------
