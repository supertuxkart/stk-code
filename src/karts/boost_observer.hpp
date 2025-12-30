//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2024  SuperTuxKart-Team
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

#ifndef HEADER_BOOST_OBSERVER_HPP
#define HEADER_BOOST_OBSERVER_HPP

class Kart;

/**
 * \brief Observer interface for boost activation events.
 * \ingroup karts
 *
 * Classes implementing this interface can register with MaxSpeed to receive
 * notifications when a boost activates. This enables event-driven visual effects
 * instead of per-frame polling.
 *
 * Usage:
 * - KartGFX: Receives events for ALL karts to trigger particle bursts
 * - CameraNormal: Receives events but only acts on the camera's own kart
 *                 to trigger speed lines shader and camera pull-back
 */
class IBoostObserver
{
public:
    virtual ~IBoostObserver() = default;

    /** Called when a boost activates (NOT every frame - only on activation).
     *  Implementers should check if kart matches their target before acting.
     *
     *  @param kart The kart that activated the boost
     *  @param category The boost type (MS_INCREASE_NITRO, MS_INCREASE_ZIPPER, etc.)
     *  @param add_speed The speed increase value
     *  @param duration_ticks How long the boost lasts in game ticks
     */
    virtual void onBoostActivated(Kart* kart,
                                   unsigned int category,
                                   float add_speed,
                                   int duration_ticks) = 0;
};

#endif // HEADER_BOOST_OBSERVER_HPP
