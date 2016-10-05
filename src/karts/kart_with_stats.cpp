//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011-2015  Joerg Henrichs
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

#include "graphics/render_info.hpp"
#include "karts/explosion_animation.hpp"
#include "karts/rescue_animation.hpp"
#include "items/item.hpp"
#include "modes/linear_world.hpp"

KartWithStats::KartWithStats(const std::string& ident,
                             unsigned int world_kart_id,
                             int position, const btTransform& init_transform,
                             PerPlayerDifficulty difficulty)
             : Kart(ident, world_kart_id, position,
                    init_transform, difficulty, KRT_DEFAULT)
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
    m_brake_count       = 0;
    m_off_track_count   = 0;
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
    if(getSpeed()>m_top_speed        ) m_top_speed = getSpeed();
    if(getControls().getSkidControl()) m_skidding_time += dt;
    if(getControls().getBrake()      ) m_brake_count ++;
    LinearWorld *world = dynamic_cast<LinearWorld*>(World::getWorld());
    if(world && !world->isOnRoad(getWorldKartId()))
        m_off_track_count ++;
}   // update

// ----------------------------------------------------------------------------
/** Overloading setKartAnimation with a kind of listener function in order
 *  to gather statistics about rescues and explosions.
 */
void KartWithStats::setKartAnimation(AbstractKartAnimation *ka)
{
    bool is_new = !getKartAnimation() && !isInvulnerable() && !isShielded();
    Kart::setKartAnimation(ka);
    // Nothing to count if it's not a new animation
    if(!is_new) return;

    // We can't use a dynamic cast here, since this function is called from
    // constructor of AbstractKartAnimation, which is the base class for all
    // animations. So at this stage ka is only an AbstractKartAnimation, not
    // any of the derived classes.
    if(ka && ka->getName()=="ExplosionAnimation")
    {
        m_explosion_count ++;
        m_explosion_time += ka->getAnimationTimer();
    }
    else if(ka && ka->getName()=="RescueAnimation")
    {
        m_rescue_count ++;
        m_rescue_time += ka->getAnimationTimer();
    }
}   // setKartAnimation

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
