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

#ifndef HEADER_THREE_D_ANIMATION_HPP
#define HEADER_THREE_D_ANIMATION_HPP

#include <string>

#include <vector3d.h>
using namespace irr;

#include "btBulletDynamicsCommon.h"

#include "animations/animation_base.hpp"
#include "physics/user_pointer.hpp"

namespace irr
{
    namespace scene { class IAnimatedMesh; class ISceneNode; class IMesh; }
}

class TrackObject;
class BezierCurve;
class XMLNode;

/** \brief A virtual base class for all animations.
  * \ingroup animations
  */
class ThreeDAnimation : public AnimationBase
{
private:
    TrackObject          *m_object;

    /** True if a collision with this object should trigger
     *  rescuing a kart. */
    bool                  m_crash_reset;

    /** True if a collision with this object should trigger
     *  "exploding" a kart. */
    bool                  m_explode_kart;
    
    bool                  m_flatten_kart;

    /** True if animation is currently paused by scripts */
    bool                  m_is_paused;
    /** We have to store the rotation value as computed in blender, since
     *  irrlicht uses a different order, so for rotation animations we
     *  can not use the value returned by getRotation from a scene node. */
    Vec3                  m_hpr;

    /**
      * If true, play animation even when GFX are disabled
      */
    bool                  m_important_animation;

public:
                 ThreeDAnimation(const XMLNode &node, TrackObject* object);
    virtual     ~ThreeDAnimation();
    virtual void update(float dt) {}
    // ------------------------------------------------------------------------
    void updateWithWorldTicks(bool with_physics);
    // ------------------------------------------------------------------------
    /** Returns true if a collision with this object should
     * trigger a rescue. */
    bool isCrashReset() const { return m_crash_reset; }
    bool isExplodeKartObject() const { return m_explode_kart; }
    bool isFlattenKartObject() const { return m_flatten_kart; }
    void setPaused(bool mode){ m_is_paused = mode; }
    // ------------------------------------------------------------------------
    ThreeDAnimation* clone(TrackObject* obj);
};   // ThreeDAnimation
#endif

