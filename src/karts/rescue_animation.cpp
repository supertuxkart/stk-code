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

#include "config/user_config.hpp"
#include "graphics/referee.hpp"
#include "items/attachment.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/kart_properties.hpp"
#include "modes/three_strikes_battle.hpp"
#include "network/network_config.hpp"
#include "physics/btKart.hpp"
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
    btTransform prev_trans = kart->getTrans();
    // Get the required final physicial transform for network, then reset back
    // to the original transform
    World::getWorld()->moveKartAfterRescue(kart);

    m_end_transform = kart->getTrans();
    kart->getBody()->setCenterOfMassTransform(prev_trans);
    kart->setTrans(prev_trans);

    m_referee     = new Referee(*m_kart);
    m_kart->getNode()->addChild(m_referee->getSceneNode());
    float timer = m_kart->getKartProperties()->getRescueDuration();
    m_rescue_moment = stk_config->time2Ticks(timer * 0.6f);
    m_timer       = stk_config->time2Ticks(timer);
    m_up_vector   = m_kart->getTrans().getBasis().getColumn(1);
    m_xyz         = m_kart->getXYZ();
    m_kart->getAttachment()->clear();
    m_kart->getVehicle()->resetGroundHeight();

    if (NetworkConfig::get()->isNetworking() &&
        NetworkConfig::get()->isServer())
    {
        m_end_ticks = m_timer + World::getWorld()->getTicksSinceStart() + 1;
    }

    // Determine maximum rescue height with up-raycast
    float max_height = m_kart->getKartProperties()->getRescueHeight();
    float hit_dest = maximumHeight();

    max_height = std::min(hit_dest, max_height);
    m_velocity = max_height / timer;

    // Add a hit unless it was auto-rescue
    if (race_manager->getMinorMode()==RaceManager::MINOR_MODE_BATTLE &&
        !is_auto_rescue)
    {
        World::getWorld()->kartHit(m_kart->getWorldKartId());
        if (UserConfigParams::m_arena_ai_stats)
        {
            ThreeStrikesBattle* tsb = dynamic_cast<ThreeStrikesBattle*>
                (World::getWorld());
            if (tsb)
                tsb->increaseRescueCount();
        }
    }

    // Clear powerups when rescue in CTF
    addNetworkAnimationChecker(race_manager->getMajorMode() ==
        RaceManager::MAJOR_MODE_CAPTURE_THE_FLAG);
}   // RescueAnimation

//-----------------------------------------------------------------------------
/** This object is automatically destroyed when the timer expires.
 */
RescueAnimation::~RescueAnimation()
{
    m_kart->getBody()->setLinearVelocity(btVector3(0, 0, 0));
    m_kart->getBody()->setAngularVelocity(btVector3(0, 0, 0));
    m_kart->getNode()->removeChild(m_referee->getSceneNode());
    delete m_referee;
    m_referee = NULL;
}   // ~RescueAnimation

//-----------------------------------------------------------------------------
/** Determine maximum rescue height with up-raycast
 */
float RescueAnimation::maximumHeight() 
{
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
    return hit_dest;
}   // maximumHeight

// ----------------------------------------------------------------------------
/** Updates the kart animation.
 *  \param ticks Number of time steps - should be 1.
 */
void RescueAnimation::update(int ticks)
{
    float dt = stk_config->ticks2Time(ticks);
    if (m_timer <= m_rescue_moment)
    {
        if (m_kart_on_track == false)
        {
            m_kart_on_track = true;
            m_kart->getBody()->setCenterOfMassTransform(m_end_transform);
            m_kart->setTrans(m_end_transform);
            m_up_vector = m_kart->getTrans().getBasis().getColumn(1);
            m_xyz = m_kart->getXYZ();

            float hit_dest = maximumHeight();
            float max_height = std::min(hit_dest,
                m_kart->getKartProperties()->getRescueHeight()) * 0.6f;
            m_xyz += max_height * m_up_vector;
        }

        m_xyz -= dt * m_velocity * m_up_vector;
        m_kart->setXYZ(m_xyz);
    } 
    else 
    {
        m_xyz += dt * m_velocity * m_up_vector;
        m_kart->setXYZ(m_xyz);
    }

    AbstractKartAnimation::update(ticks);

}   // update
