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

#include "config/stk_config.hpp"
#include "items/attachment_plugin.hpp"
#include "utils/no_copy.hpp"
#include "utils/random_generator.hpp"

#include <vector3d.h>
#include <IAnimatedMeshSceneNode.h>

using namespace irr;

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
    enum    {SWATTER_AIMING, SWATTER_TO_TARGET, SWATTER_FROM_TARGET}
                    m_animation_phase;

    /** The kart the swatter is aiming at. */
    Moveable          *m_target;

    SFXBase           *m_swat_sound;
    
    /** True if the swatter is removing an attached bomb. */
    bool               m_removing_bomb;

    /** The scene node of the attachment. */
    scene::IAnimatedMeshSceneNode *m_scene_node;

    /** The scene node where a bomb is saved (in case that the swatter
     *  replaces a bomb. */
    scene::ISceneNode *m_bomb_scene_node;
    
    /** For some reason the built-in animation system doesn't work correctly here?? */
    float              m_swat_bomb_frame;
    
public:
             Swatter(AbstractKart *kart, bool was_bomb,
                     scene::ISceneNode* bomb_scene_node);
    virtual ~Swatter();
    bool     updateAndTestFinished(float dt);

    // ------------------------------------------------------------------------
    /** Returns if the swatter is currently aiming, i.e. can be used to
     *  swat an incoming projectile. */
    bool isSwatterReady() const {
        return m_animation_phase == SWATTER_AIMING;
    }   // isSwatterReady
    // ------------------------------------------------------------------------
    virtual void onAnimationEnd();
    // ------------------------------------------------------------------------
    
private:
    /** Determine the nearest kart or item and update the current target accordingly */
    void    chooseTarget();
    
    /** If there is a current target, point to it, otherwise adopt the default position */
    void    pointToTarget();
    
    /** Squash karts or items that are around the end position (determined using a joint) of the swatter */
    void    squashThingsAround();
};   // Swatter

#endif
