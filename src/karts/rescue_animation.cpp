//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 Joerg Henrichs
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

#include "karts/rescue_animation.hpp"

#include "graphics/callbacks.hpp"
#include "graphics/camera.hpp"
#include "graphics/referee.hpp"
#include "items/attachment.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/kart_properties.hpp"
#include "modes/three_strikes_battle.hpp"
#include "modes/world_with_rank.hpp"
#include "physics/physics.hpp"
#include "physics/triangle_mesh.hpp"
#include "tracks/drive_graph.hpp"
#include "tracks/quad.hpp"
#include "tracks/track.hpp"
#include "tracks/track_sector.hpp"

#include "ISceneNode.h"

#include <algorithm>

/** The constructor stores a pointer to the kart this object is animating,
 *  and initialised the timer.
 *  \param kart Pointer to the kart which is animated.
 */
RescueAnimation::RescueAnimation(AbstractKart *kart, bool is_auto_rescue)
               : AbstractKartAnimation(kart, "RescueAnimation")
{
    m_referee     = new Referee(*m_kart);
    m_kart->getNode()->addChild(m_referee->getSceneNode());
    m_timer       = m_kart->getKartProperties()->getRescueDuration();
    m_up_vector   = m_kart->getTrans().getBasis().getColumn(1);
    m_xyz         = m_kart->getXYZ();
    m_orig_rotation = m_kart->getRotation();
    m_kart->getAttachment()->clear();

    // Determine maximum rescue height with up-raycast
    float max_height = m_kart->getKartProperties()->getRescueHeight();
    float hit_dest = 9999999.9f;
    Vec3 hit;
    const Material* m = NULL;
    Vec3 to = m_up_vector * 10000.0f;
    const TriangleMesh &tm = Track::getCurrentTrack()->getTriangleMesh();
    if (tm.castRay(m_xyz, to, &hit, &m, NULL/*normal*/, /*interpolate*/true))
    {
        hit_dest = (hit - m_xyz).length();
        hit_dest -= Referee::getHeight();
        if (hit_dest < 1.0f)
        {
            hit_dest = 1.0f;
        }
    }
    max_height = std::min(hit_dest, max_height);
    m_velocity = max_height / m_timer;

    // Determine the rotation that will rotate the kart from the current
    // up direction to the right up direction it should have according to
    // the last vaild quad of the kart
    WorldWithRank* wwr = dynamic_cast<WorldWithRank*>(World::getWorld());
    if (DriveGraph::get() && wwr)
    {
        const int sector = wwr->getTrackSector(m_kart->getWorldKartId())
            ->getCurrentGraphNode();
        const Vec3& quad_normal = DriveGraph::get()->getQuad(sector)
            ->getNormal();
        btQuaternion angle_rot(btVector3(0, 1, 0),
            Track::getCurrentTrack()->getAngle(sector));
        m_des_rotation = shortestArcQuat(Vec3(0, 1, 0), quad_normal) * angle_rot;
        m_des_rotation.normalize();
    }
    else
    {
        m_des_rotation = m_orig_rotation;
    }

    // Add a hit unless it was auto-rescue
    if (race_manager->getMinorMode()==RaceManager::MINOR_MODE_3_STRIKES &&
        !is_auto_rescue)
    {
        ThreeStrikesBattle *world=(ThreeStrikesBattle*)World::getWorld();
        world->kartHit(m_kart->getWorldKartId());
        if (UserConfigParams::m_arena_ai_stats)
            world->increaseRescueCount();
    }
};   // RescueAnimation

//-----------------------------------------------------------------------------
/** This object is automatically destroyed when the timer expires.
 */
RescueAnimation::~RescueAnimation()
{
    // If m_timer >=0, this object is deleted because the kart
    // is deleted (at the end of a race), which means that
    // world is in the process of being deleted. In this case
    // we can't call removeKartAfterRescue() or getPhysics anymore.
    if(m_timer < 0)
        World::getWorld()->moveKartAfterRescue(m_kart);
    m_kart->getNode()->removeChild(m_referee->getSceneNode());
    delete m_referee;
    m_referee = NULL;
    if(m_timer < 0)
    {
        m_kart->getBody()->setLinearVelocity(btVector3(0,0,0));
        m_kart->getBody()->setAngularVelocity(btVector3(0,0,0));
        Physics::getInstance()->addKart(m_kart);
        for(unsigned int i=0; i<Camera::getNumCameras(); i++)
        {
            Camera *camera = Camera::getCamera(i);
            if(camera && camera->getKart()==m_kart &&
                camera->getType() != Camera::CM_TYPE_END)
                camera->setMode(Camera::CM_NORMAL);
        }
    }
}   // ~RescueAnimation

// ----------------------------------------------------------------------------
/** Updates the kart animation.
 *  \param dt Time step size.
 *  \return True if the explosion is still shown, false if it has finished.
 */
void RescueAnimation::update(float dt)
{
    m_xyz += dt * m_velocity * m_up_vector;
    m_kart->setXYZ(m_xyz);
    btQuaternion result = m_des_rotation.slerp(m_orig_rotation,
        m_timer / m_kart->getKartProperties()->getRescueDuration());
    result.normalize();
    m_kart->setRotation(result);
    AbstractKartAnimation::update(dt);

}   // update
