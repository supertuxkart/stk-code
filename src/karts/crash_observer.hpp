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

#ifndef HEADER_CRASH_OBSERVER_HPP
#define HEADER_CRASH_OBSERVER_HPP

class Kart;
class Material;

/**
 * \brief Observer interface for crash/collision events.
 * \ingroup karts
 *
 * Classes implementing this interface can register with a Kart to receive
 * notifications when a collision occurs. This enables event-driven visual effects
 * like camera shake.
 *
 * Usage:
 * - CameraNormal: Receives crash events for its kart to trigger camera shake
 */
class ICrashObserver
{
public:
    virtual ~ICrashObserver() = default;

    /** Called when a kart collides with another kart.
     *  @param kart The kart that crashed
     *  @param other_kart The kart that was hit
     *  @param intensity Collision intensity based on relative velocity [0.0, 1.0]
     */
    virtual void onKartCrash(Kart* kart, Kart* other_kart, float intensity) = 0;

    /** Called when a kart collides with the track/wall.
     *  @param kart The kart that crashed
     *  @param material The material that was hit (may be NULL)
     *  @param intensity Collision intensity based on velocity [0.0, 1.0]
     */
    virtual void onTrackCrash(Kart* kart, const Material* material, float intensity) = 0;
};

#endif // HEADER_CRASH_OBSERVER_HPP
