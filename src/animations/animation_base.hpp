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

#ifndef HEADER_ANIMATION_BASE_HPP
#define HEADER_ANIMATION_BASE_HPP

/**
 * \defgroup animations
 * This module manages interpolation-based animation (of position, rotation
 * and/or scale)
 */

#include <vector>

// Note that ipo.hpp is included here in order that PtrVector<Ipo> can call
// the proper destructor!
#include "animations/ipo.hpp"
#include "utils/ptr_vector.hpp"

#include <algorithm>

class XMLNode;

/**
  * \brief A base class for all animations.
  * \ingroup animations
  */
class AnimationBase
{
private:
    /** Two types of animations: cyclic ones that play all the time, and
    *  one time only (which might get triggered more than once). */
    enum AnimTimeType { ATT_CYCLIC, ATT_CYCLIC_ONCE } m_anim_type;

    /** The current time used in the IPOs. */
    float m_current_time;

    /** The inital position of this object. */
    Vec3 m_initial_xyz;

    /** The initial rotation of this object. */
    Vec3 m_initial_hpr;

protected:
    /** All IPOs for this animation. */
    PtrVector<Ipo>  m_all_ipos;

    /** True if the animation is currently playing. */
    bool  m_playing;
    
public:
                 AnimationBase(const XMLNode &node);
                 AnimationBase(Ipo *ipo);
    virtual      ~AnimationBase() {}
    virtual void update(float dt,  Vec3 *xyz=NULL, Vec3 *hpr=NULL,
                                   Vec3 *scale=NULL);
    virtual void getAt(float time, Vec3 *xyz = NULL, Vec3 *hpr = NULL,
                                   Vec3 *scale = NULL);
    virtual void getDerivativeAt(float time, Vec3 *xyz);
    /** This needs to be implemented by the inheriting classes. It is called
     *  once per frame from the track. It has a dummy implementation that
     *  just asserts so that this class can be instantiated in
     *  CannonAnimation. */
    virtual void update(float dt) {assert(false); };
    void         setInitialTransform(const Vec3 &xyz,
                                     const Vec3 &hpr);
    void         reset();
    // ------------------------------------------------------------------------
    /** Disables or enables an animation. */
    void         setPlaying(bool playing) {m_playing = playing; }

    // ------------------------------------------------------------------------

    float getAnimationDuration() const
    {
        float duration = -1;

        for (const Ipo* currIpo : m_all_ipos)
        {
            duration = std::max(duration, currIpo->getEndTime());
        }

        return duration;
    }

};   // AnimationBase

#endif

