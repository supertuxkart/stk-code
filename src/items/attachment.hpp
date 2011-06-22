//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
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

#ifndef HEADER_ATTACHMENT_HPP
#define HEADER_ATTACHMENT_HPP

#include "config/stk_config.hpp"
#include "utils/no_copy.hpp"
#include "utils/random_generator.hpp"

class Kart;
class Item;

/**
  * \ingroup items
  */
class Attachment: public NoCopy
{
public:
    // Some loop in attachment.cpp depend on ATTACH_FIRST and ATTACH_MAX.
    // So if new elements are added, make sure to add them in between those values.
    // Also, please note that Attachment::Attachment relies on ATTACH_FIRST being 0.
    enum AttachmentType
    {
        ATTACH_FIRST = 0,
        ATTACH_PARACHUTE = 0,
        ATTACH_BOMB,
        ATTACH_ANVIL,
        ATTACH_SWATTER,
        ATTACH_TINYTUX,
        ATTACH_MAX,
        ATTACH_NOTHING
    };

private:
    /** Attachment type. */
    AttachmentType  m_type;

    /** Kart the attachment is attached to. */
    Kart           *m_kart;

    /** How often an attachment can be used (e.g. swatter). */
    int             m_count;

    /** Time left till attachment expires. */
    float           m_time_left;

    /** For parachutes only. */
    float           m_initial_speed;

    /** Scene node of the attachment, which will be attached to the kart's
     *  scene node. */
    scene::IAnimatedMeshSceneNode 
                   *m_node;

    /** Used by bombs so that it's not passed back to previous owner. */
    Kart           *m_previous_owner;

    /** State of a swatter animation. The swatter is either aiming (looking
     *  for a kart and/or swatting an incoming item), moving towards or back
     *  from a kart, or swatting left and right to hit an item - which has
     *  three phases: going left to an angle a (phae 1), then going to -a
     *  (phase 2), then going back to 0. */
    enum            {SWATTER_AIMING, SWATTER_TO_KART,
                     SWATTER_BACK_FROM_KART,
                     SWATTER_ITEM_1, SWATTER_ITEM_2, SWATTER_ITEM_3,
                     SWATTER_BACK_FROM_ITEM} m_animation_phase;

    /** Timer for swatter animation. */
    float           m_animation_timer;

    /** The kart the swatter is aiming at. */
    Kart           *m_animation_target;

    /** Rotation per second so that the swatter will hit the kart. */
    core::vector3df m_rot_per_sec;

    RandomGenerator m_random;

    void            aimSwatter();
    bool            isLeftSideOfKart(const Vec3 &xyz);
    void            checkForHitKart(bool isSwattingToLeft);

public:
          Attachment(Kart* kart);
         ~Attachment();
    void  clear ();
    void  hitBanana(Item *item, int new_attachment=-1);
    void  update (float dt);
    void  updateSwatter(float dt);
    void  moveBombFromTo(Kart *from, Kart *to);
    void  swatItem();

    void  set (AttachmentType type, float time, Kart *previous_kart=NULL);
    // ------------------------------------------------------------------------
    /** Sets the type of the attachment, but keeps the old time left value. */
    void  set (AttachmentType type) { set(type, m_time_left); }
    // ------------------------------------------------------------------------
    /** Returns the type of this attachment. */
    AttachmentType getType() const { return m_type; }
    // ------------------------------------------------------------------------
    /** Returns how much time is left before this attachment is removed. */
    float getTimeLeft() const { return m_time_left;      }
    // ------------------------------------------------------------------------
    /** Sets how long this attachment will remain attached. */
    void  setTimeLeft(float t){ m_time_left = t;         }
    // ------------------------------------------------------------------------
    /** Returns the previous owner of this attachment, used in bombs that
     *  are being passed between karts. */
    Kart* getPreviousOwner() const { return m_previous_owner; }
    // ------------------------------------------------------------------------
    /** Returns additional weight for the kart. */
    float weightAdjust() const { 
        return m_type==ATTACH_ANVIL ? stk_config->m_anvil_weight : 0.0f; }
    // ------------------------------------------------------------------------
    /** Returns if the swatter is currently aiming, i.e. can be used to
     *  swat an incoming projectile. */
    bool isSwatterReady() const {
        assert(m_type==ATTACH_SWATTER);
        return m_animation_phase == SWATTER_AIMING;
    }   // isSwatterReady
    // ------------------------------------------------------------------------
};   // Attachment

#endif
