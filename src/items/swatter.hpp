//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011-2015 Joerg Henrichs
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

#include "config/stk_config.hpp"
#include "items/attachment_plugin.hpp"
#include "utils/cpp2011.hpp"
#include "utils/no_copy.hpp"

#include <limits>
#include <set>
#include <vector3d.h>

namespace irr
{
    namespace scene
    {
        class IAnimatedMeshSceneNode;
    }
}

using namespace irr;

class Attachment;
class AbstractKart;
class Item;
class Moveable;
class SFXBase;

/**
  * \ingroup items
  */
class Swatter : public NoCopy, public AttachmentPlugin
{

private:
    /** State of the animation, the swatter is successively:
     *   - aiming (default state) => it's turning to the nearest target
     *   - going down to the target
     *   - going up from the target
     */
    enum AnimationPhase : uint8_t
    {
        SWATTER_AIMING = 0,
        SWATTER_TO_TARGET = 1,
        SWATTER_FROM_TARGET = 2
    };
    AnimationPhase m_animation_phase;

    /** The kart the swatter is aiming at. */
    AbstractKart      *m_closest_kart;

    SFXBase           *m_swat_sound;

    /** Set the end ticks to complete the removing an attached bomb animation. */

    /** The scene node of the attachment. */
    scene::IAnimatedMeshSceneNode *m_scene_node;

    /** The scene node where a bomb is saved (in case that the swatter
     *  replaces a bomb. */
    scene::IAnimatedMeshSceneNode *m_bomb_scene_node;

    int                m_discard_ticks;

    int                m_swatter_duration;

    /** Set the bomb remaing ticks so we can set the timer on the removing
     *  bomb animation. */
    int16_t            m_bomb_remaining;

    int16_t            m_swatter_animation_ticks;

    /** True if the swatter will be discarded now. */
    bool               m_discard_now;

    /** True if the swatter animation has been played. */
    bool               m_played_swatter_animation;
public:
             Swatter(AbstractKart *kart, int16_t bomb_ticks, int ticks,
                     Attachment* attachment);
    virtual ~Swatter();
    void     updateGraphics(float dt) OVERRIDE;
    bool     updateAndTestFinished() OVERRIDE;

    // ------------------------------------------------------------------------
    /** Returns if the swatter is currently aiming, i.e. can be used to
     *  swat an incoming projectile. */
    bool isSwatterReady() const
    {
        return m_animation_phase == SWATTER_AIMING;
    }   // isSwatterReady
    // ------------------------------------------------------------------------
    virtual void restoreState(BareNetworkString *buffer) OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void saveState(BareNetworkString *buffer) const OVERRIDE;

private:
    /** Determine the nearest kart or item and update the current target accordingly */
    void    chooseTarget();

    /** If there is a current target, point to it, otherwise adopt the default position */
    void    pointToTarget();

    /** Squash karts or items that are around the end position (determined using a joint) of the swatter */
    void    squashThingsAround();
};   // Swatter

#endif
