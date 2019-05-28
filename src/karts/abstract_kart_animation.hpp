//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2015 Joerg Henrichs
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

#ifndef HEADER_ABSTRACT_KART_ANIMATION_HPP
#define HEADER_ABSTRACT_KART_ANIMATION_HPP

#include "LinearMath/btTransform.h"

#include "config/stk_config.hpp"
#include "utils/no_copy.hpp"
#include "utils/vec3.hpp"

#include <exception>
#include <limits>
#include <string>

class AbstractKart;
class BareNetworkString;

enum KartAnimationType : uint8_t
{
    KAT_RESCUE = 0,
    KAT_EXPLOSION = 1,
    KAT_CANNON = 2
};

/** Exception for kart animation creation in networking, so if thrown it will
 *  tell the num of bytes skipping in the game state. */
class KartAnimationCreationException : public std::exception
{
public:
    virtual int getSkippingOffset() const = 0;
};

/** The base class for all kart animation, like rescue, explosion, or cannon.
 *  Kart animations are done by removing the physics body from the physics
 *  world, and instead modifying the rotation and position of the kart
 *  directly. They are registered with the kart, and only one can be
 *  used at the same time. The memory is handled by the kart object, so
 *  there is no need to manage it. Sample usage:
 *    new ExplosionAnimation(kart);
 *  The object does not need to be stored.
 */
class AbstractKartAnimation: public NoCopy
{
private:
    /** Name of this animation, used for debug prints only. */
    std::string m_name;

protected:
   /** A pointer to the kart which is animated by this class. */
    AbstractKart *m_kart;

    /** Time in ticks for the animation which ends in world count up ticks. */
    int m_end_ticks;

    /** Time in ticks for the animation creation. */
    int m_created_ticks;

    /** Transformation by the time the animation was created, used for rewind
     *  to recreate the animation with the same one. */
    btTransform m_created_transform;

    /* Compressed values for server to send to avoid compressing everytime. */
    int m_created_transform_compressed[4];

    void resetPowerUp();
    // ------------------------------------------------------------------------
    void restoreBasicState(BareNetworkString* buffer);
    // ------------------------------------------------------------------------
    float getMaximumHeight(const Vec3& up_vector, float height_remove);

public:
                 AbstractKartAnimation(AbstractKart* kart,
                                       const std::string &name);
    virtual     ~AbstractKartAnimation();
    virtual void update(int ticks);
    // ------------------------------------------------------------------------
    virtual void updateGraphics(float dt);
    // ------------------------------------------------------------------------
    virtual float getAnimationTimer() const;
    // ------------------------------------------------------------------------
    /** To easily allow printing the name of the animation being used atm.
     *  Used in AstractKart in case of an incorrect sequence of calls. */
    virtual const std::string &getName() const { return m_name; }
    // ------------------------------------------------------------------------
    virtual KartAnimationType getAnimationType() const = 0;
    // ------------------------------------------------------------------------
    /* Used to ignore adding karts back to physics when destroying world. */
    void handleResetRace()   { m_end_ticks = std::numeric_limits<int>::max(); }
    // ------------------------------------------------------------------------
    /* Used to recreate animation in \ref KartRewinder. */
    virtual void saveState(BareNetworkString* buffer);
    // ------------------------------------------------------------------------
    /* Called when kart animation is the same in kart state, which make sure
     * for example the end or created ticks are the same. */
    virtual void restoreState(BareNetworkString* buffer)
                                                 { restoreBasicState(buffer); }
};   // AbstractKartAnimation

#endif
