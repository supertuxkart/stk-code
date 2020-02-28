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

#include "animations/three_d_animation.hpp"

#include <stdio.h>

#include "audio/sfx_base.hpp"
#include "animations/ipo.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/mesh_tools.hpp"
#include "io/xml_node.hpp"
#include "modes/world.hpp"
#include "physics/kart_motion_state.hpp"
#include "physics/physics.hpp"
#include "physics/physical_object.hpp"
#include "physics/triangle_mesh.hpp"
#include "tracks/bezier_curve.hpp"
#include "tracks/track_object.hpp"
#include "utils/constants.hpp"
#include <ISceneManager.h>
#include <IMeshSceneNode.h>

ThreeDAnimation::ThreeDAnimation(const XMLNode &node, TrackObject* object) : AnimationBase(node)
{
    m_object = object;

    m_is_paused = false;
    m_crash_reset  = false;
    m_explode_kart = false;
    m_flatten_kart = false;
    node.get("reset", &m_crash_reset);
    node.get("explode", &m_explode_kart);
    node.get("flatten", &m_flatten_kart);

    m_important_animation = (World::getWorld()->getIdent() == IDENT_CUTSCENE);
    node.get("important", &m_important_animation);

    /** Save the initial position and rotation in the base animation object. */
    setInitialTransform(object->getInitXYZ(),
                        object->getInitRotation() );
    m_hpr = object->getInitRotation();

    assert(!std::isnan(m_hpr.getX()));
    assert(!std::isnan(m_hpr.getY()));
    assert(!std::isnan(m_hpr.getZ()));
}   // ThreeDAnimation

// ----------------------------------------------------------------------------
/** Destructor. */
ThreeDAnimation::~ThreeDAnimation()
{
}   // ~ThreeDAnimation

// ----------------------------------------------------------------------------
/** Updates position and rotation of this model. Called once per time step.
 */
void ThreeDAnimation::updateWithWorldTicks(bool has_physics)
{
    const bool track_object_with_physics =
        m_object->getPhysicalObject() != nullptr;

    if ((has_physics && !track_object_with_physics) ||
        (!has_physics && track_object_with_physics))
        return;

    Vec3 xyz   = m_object->getPosition();
    Vec3 scale = m_object->getScale();

    if (!m_is_paused)
    {
        int cur_ticks = World::getWorld()->getTicksSinceStart();
        m_current_time = stk_config->ticks2Time(cur_ticks);
    }

    AnimationBase::getAt(m_current_time, &xyz, &m_hpr, &scale);     //updates all IPOs
    //m_node->setPosition(xyz.toIrrVector());
    //m_node->setScale(scale.toIrrVector());

    if (!m_playing) return;

    // Note that the rotation order of irrlicht is different from the one
    // in blender. So in order to reproduce the blender IPO rotations
    // correctly, we have to get the rotations around each axis and combine
    // them in the right order for irrlicht
    core::matrix4 m;
    m.makeIdentity();
    core::matrix4 mx;
    assert(!std::isnan(m_hpr.getX()));
    assert(!std::isnan(m_hpr.getY()));
    assert(!std::isnan(m_hpr.getZ()));
    mx.setRotationDegrees(core::vector3df(m_hpr.getX(), 0, 0));
    core::matrix4 my;
    my.setRotationDegrees(core::vector3df(0, m_hpr.getY(), 0));
    core::matrix4 mz;
    mz.setRotationDegrees(core::vector3df(0, 0, m_hpr.getZ()));
    m = my*mz*mx;
    core::vector3df hpr = m.getRotationDegrees();
    //m_node->setRotation(hpr);

    if (m_object)
    {
        m_object->move(xyz.toIrrVector(), hpr, scale.toIrrVector(), true, false);
    }
}   // update

// ----------------------------------------------------------------------------
/** Copying to child process of track object.
 */
ThreeDAnimation* ThreeDAnimation::clone(TrackObject* obj)
{
    ThreeDAnimation* animation = new ThreeDAnimation(*this);
    animation->m_object = obj;
    for (unsigned i = 0; i < m_all_ipos.size(); i++)
        animation->m_all_ipos[i] = m_all_ipos[i]->clone();
    return animation;
}   // clone
