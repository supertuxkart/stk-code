//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Joerg Henrichs
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

#ifndef HEADER_EMERGENCY_ANIMATION_HPP
#define HEADER_EMERGENCY_ANIMATION_HPP

#include "utils/no_copy.hpp"
#include "utils/vec3.hpp"

class Kart;
class Stars;

/** 
 * \brief This class is a 'mixin' for kart, and handles the animated explosion.
 *  I.e. it will throw the kart a certain amount in the air, rotate it
 *  randomly, and after the specified time period let it land at the
 *  same spot where it was hit, therefore avoiding any problems of 
 *  karts being pushed on wrong parts of the track, and making explosion
 *  more 'fair' (it can't happen that one explosion give you actually
 *  a benefit by pushing you forwards.
 *  The object is a base class for kart, but will only be used if an
 *  explosion happens.
 * \ingroup karts
 */
class EmergencyAnimation: public NoCopy
{
protected:
    /** The coordinates where the kart was hit originally. */
    Vec3 m_xyz;

    /** The kart's current rotation. */
    Vec3 m_curr_rotation;

    /** The artificial rotation to toss the kart around. It's in units
     *  of rotation per second. */
    Vec3 m_add_rotation;

    /** The upwards velocity. */
    float m_up_velocity;

    /** Timer for the explosion. */
    float m_timer;

    /** Duration for this explosion. This can potentially be set
     *  with different values for different karts, or depending
     *  on difficulty (so that on easy you can drive again earlier. */
    float m_duration;

    /** A pointer to the class to which this object belongs. */
    Kart *m_kart;

    /** True if this kart has been eliminated. */
    bool          m_eliminated;

    /** For stars rotating around head effect */
    Stars        *m_stars_effect;
    
    /** Different kart modes: normal racing, being rescued, showing end
     *  animation, explosions, kart eliminated. */
    enum {EA_NONE, EA_RESCUE, EA_EXPLOSION}
          m_kart_mode;
public:
                 EmergencyAnimation(Kart *kart);
    virtual     ~EmergencyAnimation();
    void         reset();
    virtual void handleExplosion(const Vec3& pos, bool direct_hit);
    virtual void forceRescue(bool is_auto_rescue=false);
    void         update(float dt);
    // ------------------------------------------------------------------------
    /** Returns true if an emergency animation is being played. */
    bool  playingEmergencyAnimation() const {return m_kart_mode!=EA_NONE; }
    
    /** Returns if a rescue animation is being shown. */
    bool  playingRescueAnimation() const {return m_kart_mode==EA_RESCUE; }

    /** Returns if an explosion animation is being shown. */
    bool  playingExplosionAnimation() const {return m_kart_mode==EA_EXPLOSION; }

    /** Returns the timer for the currently played animation. */
    const float getAnimationTimer() const {return m_timer;}
    /** Returns true if the kart is eliminated. */
    bool        isEliminated     () const {return m_eliminated;}
    /** Returns a pointer to the stars effect. */
    const Stars *getStarEffect   () const {return m_stars_effect; }
    void        eliminate        (bool remove);

};   // EmergencyAnimation
#endif
