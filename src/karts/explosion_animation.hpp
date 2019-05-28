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

#ifndef HEADER_EXPLOSION_ANIMATION_HPP
#define HEADER_EXPLOSION_ANIMATION_HPP

#include "karts/abstract_kart_animation.hpp"
#include "utils/vec3.hpp"

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
class ExplosionAnimation: public AbstractKartAnimation
{
protected:
friend class KartRewinder;
    /** The normal of kart when it started to explode. */
    Vec3 m_normal;

    /** The kart's current rotation. */
    Vec3 m_curr_rotation;

    /** The artificial rotation to toss the kart around. It's in units
     *  of rotation per second. */
    Vec3 m_add_rotation;

    /** The velocity with which the kart is moved. */
    float m_velocity;

    /** If not -1, when > world count up ticks it will use m_reset_trans below
     *  for animation. */
    int m_reset_ticks;

    /** Used for reset kart back to flag base in CTF. */
    btTransform m_reset_trans;

    /* Compressed values for server to send to avoid compressing everytime. */
    int m_reset_trans_compressed[4];

    bool m_direct_hit;

    // ------------------------------------------------------------------------
    void restoreData(BareNetworkString* b);
    // ------------------------------------------------------------------------
    void init(bool direct_hit, const Vec3& normal,
              const btTransform& reset_trans);
    // ------------------------------------------------------------------------
    ExplosionAnimation(AbstractKart* kart, BareNetworkString* buffer);
    // ------------------------------------------------------------------------
    ExplosionAnimation(AbstractKart* kart, bool direct_hit);
public:
    // ------------------------------------------------------------------------
    static ExplosionAnimation *create(AbstractKart* kart, const Vec3 &pos,
                                      bool direct_hit);
    // ------------------------------------------------------------------------
    static ExplosionAnimation *create(AbstractKart *kart);
    // ------------------------------------------------------------------------
    virtual ~ExplosionAnimation();
    // ------------------------------------------------------------------------
    virtual void update(int ticks);
    // ------------------------------------------------------------------------
    virtual void updateGraphics(float dt);
    // ------------------------------------------------------------------------
    virtual KartAnimationType getAnimationType() const
                                                      { return KAT_EXPLOSION; }
    // ------------------------------------------------------------------------
    virtual void saveState(BareNetworkString* buffer);
    // ------------------------------------------------------------------------
    virtual void restoreState(BareNetworkString* buffer);
    // ------------------------------------------------------------------------
    bool hasResetAlready() const;

};   // ExplosionAnimation
#endif
