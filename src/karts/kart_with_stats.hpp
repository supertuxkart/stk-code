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

#ifndef HEADER_KART_WITH_STATS_HPP
#define HEADER_KART_WITH_STATS_HPP

#include "karts/kart.hpp"

/** \defgroup karts */


/** This class implements a kart that collects statistics about a race,
 *  which is used in profiling mode. Example are maximum speed, number
 *  of times it got hit, ...
 */
class KartWithStats : public Kart
{
private:
    /** The maximum speed of this kart had. */
    float        m_top_speed;

    /** How long this kart spent in explosions. */
    float        m_explosion_time;

    /** How often that kart was hit. */
    unsigned int m_explosion_count;

    /** How often a kart was rescued. */
    unsigned int m_rescue_count;

    /** How many bonuses were taken */
    unsigned int m_bonus_count;

    /** How many bananas were taken */
    unsigned int m_banana_count;

    /** How many small nitro tanks were taken */
    unsigned int m_small_nitro_count;

    /** How many large nitro tanks were taken */
    unsigned int m_large_nitro_count;

    /** How many bubblegums were taken */
    unsigned int m_bubblegum_count;

    /** How often the kart braked. */
    unsigned int m_brake_count;

    /** How often the kart was off-track. */
    unsigned int m_off_track_count;

    /** How much time was spent in rescue. */
    float        m_rescue_time;

    /** How much time this kart was skidding. */
    float        m_skidding_time;

public:
                 KartWithStats(const std::string& ident,
                               unsigned int world_kart_id,
                               int position,
                               const btTransform& init_transform,
                               PerPlayerDifficulty difficulty);
    virtual void update(float dt);
    virtual void reset();
    virtual void collectedItem(Item *item, int add_info);
    virtual void setKartAnimation(AbstractKartAnimation *ka);

    /** Returns the top speed of this kart. */
    float getTopSpeed() const { return m_top_speed; }
    // ------------------------------------------------------------------------
    /** Returns how much time this kart spent in explosion animations. */
    float getExplosionTime() const { return m_explosion_time; }
    // ------------------------------------------------------------------------
    /** Returns how often this kart was hit by an explosion. */
    unsigned int getExplosionCount() const { return m_explosion_count; }
    // ------------------------------------------------------------------------
    /** Returns how much time this kart spent skidding. */
    float getSkiddingTime() const { return m_skidding_time; }
    // ------------------------------------------------------------------------
    /** Returns how often the kart braked. */
    unsigned int getBrakeCount() const { return m_brake_count; }
    // ------------------------------------------------------------------------
    /** Returns how often a kart was rescued. */
    unsigned int getRescueCount() const { return m_rescue_count; }
    // ------------------------------------------------------------------------
    /** Returns the number of bonuses that were taken */
    unsigned int getBonusCount() const { return m_bonus_count; }
    // ------------------------------------------------------------------------
    /** Returns the number of bananas that were taken */
    unsigned int getBananaCount() const { return m_banana_count; }
    // ------------------------------------------------------------------------
    /** Returns the number of small nitro tanks that were taken */
    unsigned int getSmallNitroCount() const { return m_small_nitro_count; }
    // ------------------------------------------------------------------------
    /** Returns the number of large nitro tanks that were taken */
    unsigned int getLargeNitroCount() const { return m_large_nitro_count; }
    // ------------------------------------------------------------------------
    /** Returns the number of bubblegums that were taken */
    unsigned int getBubblegumCount() const { return m_bubblegum_count; }
    // ------------------------------------------------------------------------
    /** Returns how long a kart was rescued all in all. */
    float getRescueTime() const { return m_rescue_time; }
    // ------------------------------------------------------------------------
    /** Returns how often the kart was off track. */
    unsigned int getOffTrackCount() const { return m_off_track_count; }
    // ------------------------------------------------------------------------

};   // KartWithStats
#endif
