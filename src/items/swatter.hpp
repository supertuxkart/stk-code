//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011 Joerg Henrichs
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

#ifndef HEADER_SWATTER_HPP
#define HEADER_SWATTER_HPP

#include <vector3d.h>

#include "config/stk_config.hpp"
#include "items/attachment_plugin.hpp"
#include "utils/no_copy.hpp"
#include "utils/random_generator.hpp"

class Kart;
class Item;
class Attachment;

/**
  * \ingroup items
  */
class Swatter : public NoCopy, public AttachmentPlugin
{

private:
    /** Kart the attachment is attached to. */
    Kart           *m_kart;

    /** How often an attachment can be used (e.g. swatter). */
    int             m_count;

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

    void            aimSwatter();
    bool            isLeftSideOfKart(const Vec3 &xyz);
    void            checkForHitKart(bool isSwattingToLeft);

public:
          Swatter(Attachment *attachment, Kart *kart);
         ~Swatter();
    bool  updateAndTestFinished(float dt);
    void  updateSwatter(float dt);
    void  swatItem();

    // ------------------------------------------------------------------------
    /** Returns if the swatter is currently aiming, i.e. can be used to
     *  swat an incoming projectile. */
    bool isSwatterReady() const {
        return m_animation_phase == SWATTER_AIMING;
    }   // isSwatterReady
    // ------------------------------------------------------------------------
};   // Swatter

#endif
