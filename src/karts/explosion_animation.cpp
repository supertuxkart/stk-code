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

#include "karts/explosion_animation.hpp"

#include "audio/sfx_manager.hpp"
#include "graphics/camera.hpp"
#include "graphics/stars.hpp"
#include "guiengine/engine.hpp"
#include "items/attachment.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/kart_properties.hpp"
#include "modes/follow_the_leader.hpp"
#include "network/network_string.hpp"
#include "network/protocols/client_lobby.hpp"
#include "race/race_manager.hpp"
#include "tracks/track.hpp"
#include "mini_glm.hpp"

#include <cstring>

/** A static create function that does only create an explosion if
 *  the explosion happens to be close enough to affect the kart.
 *  Otherwise, NULL is returned.
 *  \param kart The kart that is exploded.
 *  \param pos The position where the explosion happened.
 *  \param direct_hit If the kart was hit directly.
 */
ExplosionAnimation *ExplosionAnimation::create(AbstractKart *kart,
                                               const Vec3 &pos,
                                               bool direct_hit)
{
    // When goal phase is happening karts is made stationary, so no animation
    // will be created
    if (kart->isInvulnerable() || World::getWorld()->isGoalPhase())
        return NULL;

    float r = kart->getKartProperties()->getExplosionRadius();

    // Ignore explosion that are too far away.
    if(!direct_hit && pos.distance2(kart->getXYZ())>r*r) return NULL;

    if(kart->isShielded())
    {
        kart->decreaseShieldTime();
        return NULL;
    }

    if (RaceManager::get()->isFollowMode())
    {
        FollowTheLeaderRace *ftl_world =
            dynamic_cast<FollowTheLeaderRace*>(World::getWorld());
        if(ftl_world->isLeader(kart->getWorldKartId()))
            ftl_world->leaderHit();
    }

    return new ExplosionAnimation(kart, direct_hit);
}   // create

// ----------------------------------------------------------------------------
/** A static create function that does only create an explosion if
 *  the explosion happens to be close enough to affect the kart.
 *  Otherwise, NULL is returned. */
ExplosionAnimation *ExplosionAnimation::create(AbstractKart *kart)
{
    if (kart->isInvulnerable() || World::getWorld()->isGoalPhase())
        return NULL;
    else if (kart->isShielded())
    {
        kart->decreaseShieldTime();
        return NULL;
    }
    return new ExplosionAnimation(kart, /*direct hit*/true);
}   // create

// ----------------------------------------------------------------------------
ExplosionAnimation::ExplosionAnimation(AbstractKart* kart, bool direct_hit)
                  : AbstractKartAnimation(kart, "ExplosionAnimation")
{
    memset(m_reset_trans_compressed, 0, 16);
    Vec3 normal = m_created_transform.getBasis().getColumn(1).normalized();
    // Put the kart back to its own flag base like rescue if direct hit in CTF
    bool reset = RaceManager::get()->getMinorMode() ==
        RaceManager::MINOR_MODE_CAPTURE_THE_FLAG && direct_hit;
    if (reset)
    {
        btTransform prev_trans = m_kart->getTrans();
        World::getWorld()->moveKartAfterRescue(m_kart);
        btTransform reset_trans = m_kart->getTrans();
        m_kart->getBody()->setCenterOfMassTransform(prev_trans);
        m_kart->setTrans(prev_trans);
        MiniGLM::compressbtTransform(reset_trans,
            m_reset_trans_compressed);
        init(direct_hit, normal, reset_trans);
    }
    else
    {
        init(direct_hit, normal,
             btTransform(btQuaternion(0.0f, 0.0f, 0.0f, 1.0f)));
    }

    float t = m_kart->getKartProperties()->getExplosionInvulnerabilityTime();
    m_kart->setInvulnerableTicks(stk_config->time2Ticks(t));
    m_kart->playCustomSFX(SFXManager::CUSTOM_EXPLODE);
    m_kart->getAttachment()->clear();
    // Clear powerups when direct hit in CTF
    if (reset)
        resetPowerUp();
}   // ExplosionAnimation

//-----------------------------------------------------------------------------
ExplosionAnimation::ExplosionAnimation(AbstractKart* kart, BareNetworkString* b)
                  : AbstractKartAnimation(kart, "ExplosionAnimation")
{
    restoreBasicState(b);
    restoreData(b);
}   // RescueAnimation

//-----------------------------------------------------------------------------
void ExplosionAnimation::restoreData(BareNetworkString* b)
{
    bool direct_hit = b->getUInt8() == 1;
    Vec3 normal = m_created_transform.getBasis().getColumn(1).normalized();
    btTransform reset_transform =
        btTransform(btQuaternion(0.0f, 0.0f, 0.0f, 1.0f));

    if (RaceManager::get()->getMinorMode() ==
        RaceManager::MINOR_MODE_CAPTURE_THE_FLAG && direct_hit)
    {
        m_reset_trans_compressed[0] = b->getInt24();
        m_reset_trans_compressed[1] = b->getInt24();
        m_reset_trans_compressed[2] = b->getInt24();
        m_reset_trans_compressed[3] = b->getUInt32();
        reset_transform =
            MiniGLM::decompressbtTransform(m_reset_trans_compressed);
    }
    init(direct_hit, normal, reset_transform);
}   // restoreData

//-----------------------------------------------------------------------------
ExplosionAnimation::~ExplosionAnimation()
{
    // Only play with physics and camera if the object is getting destroyed
    // because its time is up. If there is still time left when this gets
    // called, it means that the world is getting destroyed so we don't touch
    // these settings.
    if (m_end_ticks != std::numeric_limits<int>::max())
    {
        m_kart->getBody()->setLinearVelocity(btVector3(0,0,0));
        m_kart->getBody()->setAngularVelocity(btVector3(0,0,0));
        // Don't reset spectate camera
        auto cl = LobbyProtocol::get<ClientLobby>();
        if (!GUIEngine::isNoGraphics() && (!cl || !cl->isSpectator()))
        {
            for (unsigned i = 0; i < Camera::getNumCameras(); i++)
            {
                Camera *camera = Camera::getCamera(i);
                if (camera->getType() != Camera::CM_TYPE_END)
                    camera->setMode(Camera::CM_NORMAL);
            }
        }
    }
}   // ~ExplosionAnimation

// ----------------------------------------------------------------------------
void ExplosionAnimation::init(bool direct_hit, const Vec3& normal,
                              const btTransform& reset_trans)
{
    m_direct_hit = direct_hit;
    m_reset_ticks = -1;
    float timer = m_kart->getKartProperties()->getExplosionDuration();
    m_normal = normal;

    // Non-direct hits will be only affected half as much.
    if (!direct_hit)
    {
        timer *= 0.5f;
    }

    // Put the kart back to its own flag base like rescue if direct hit in CTF
    if (RaceManager::get()->getMinorMode() ==
        RaceManager::MINOR_MODE_CAPTURE_THE_FLAG && direct_hit)
    {
        m_reset_ticks = m_created_ticks +
            stk_config->time2Ticks(timer * 0.8f);
    }
    m_end_ticks = m_created_ticks + stk_config->time2Ticks(timer);

    if (m_reset_ticks != -1)
        m_reset_trans = reset_trans;
    else
        m_reset_trans = btTransform(btQuaternion(0.0f, 0.0f, 0.0f, 1.0f));

    // Half of the overall time is spent in raising, so only use
    // half of the explosion time here.
    // Velocity after t seconds is:
    // v(t) = m_velocity + t*gravity
    // Since v(explosion_time*0.5) = 0, the following forumla computes
    // the right initial velocity for a kart to land back after
    // the specified time.
    m_velocity = 0.5f * timer * Track::getCurrentTrack()->getGravity();

    // From moveable::updatePosition
    Vec3 forw_vec = m_created_transform.getBasis().getColumn(2);
    float heading = atan2f(forw_vec.getX(), forw_vec.getZ());
    Vec3 up = m_created_transform.getBasis().getColumn(1);
    float pitch = atan2(up.getZ(), fabsf(up.getY()));
    float roll = atan2(up.getX(), up.getY());

    m_curr_rotation.setHeading(heading);
    m_curr_rotation.setPitch(pitch);
    m_curr_rotation.setRoll(roll);

    const int max_rotation = direct_hit ? 2 : 1;
    // To get rotations in both directions for each axis we determine a
    // number calculated by world created ticks between
    // -(max_rotation-1) and +(max_rotation-1)
    float f = 2.0f * M_PI / timer;
    m_add_rotation.setHeading(
        ((m_created_ticks / 9) % (2 * max_rotation + 1) - max_rotation) * f);
    m_add_rotation.setPitch(
        ((m_created_ticks / 10) % (2 * max_rotation + 1) - max_rotation) * f);
    m_add_rotation.setRoll(
        ((m_created_ticks / 11) % (2 * max_rotation + 1) - max_rotation) * f);
}   // init

// ----------------------------------------------------------------------------
/** Updates the kart animation.
 *  \param ticks Number of time steps - should be 1.
 */
void ExplosionAnimation::update(int ticks)
{
    float dur = stk_config->ticks2Time(
        World::getWorld()->getTicksSinceStart() - m_created_ticks);

    float velocity = m_velocity -
        dur * 0.5f * Track::getCurrentTrack()->getGravity();
    Vec3 xyz = m_created_transform.getOrigin() + dur * velocity * m_normal;
    btQuaternion q = m_created_transform.getRotation();

    // Make sure the kart does not end up under the track
    if ((xyz - m_created_transform.getOrigin()).dot(m_normal) < 0.0f)
    {
        xyz = m_created_transform.getOrigin();
        m_end_ticks = World::getWorld()->getTicksSinceStart();
    }
    else if (getAnimationTimer() != 0.0f)
    {
        Vec3 curr_rotation = m_curr_rotation + dur * m_add_rotation;
        q = btQuaternion(curr_rotation.getHeading(), curr_rotation.getPitch(),
            curr_rotation.getRoll());
    }

    if (m_reset_ticks != -1 &&
        World::getWorld()->getTicksSinceStart() > m_reset_ticks)
    {
        Vec3 reset_xyz;
        const Vec3 reset_up = m_reset_trans.getBasis().getColumn(1);
        reset_xyz = m_reset_trans.getOrigin() + dur * velocity * reset_up;
        if ((reset_xyz - m_reset_trans.getOrigin()).dot(reset_up) < 0.0f)
            reset_xyz = m_reset_trans.getOrigin();
        m_kart->setXYZ(reset_xyz);
        m_kart->setRotation(m_reset_trans.getRotation());
    }
    else
    {
        m_kart->setXYZ(xyz);
        m_kart->setRotation(q);
    }

    AbstractKartAnimation::update(ticks);
}   // update

// ----------------------------------------------------------------------------
void ExplosionAnimation::updateGraphics(float dt)
{
    if (m_kart->getStarsEffect() && !m_kart->getStarsEffect()->isEnabled())
    {
        // Set graphical effects for invulnerable time in updateGraphics
        // to avoid issue with rewind
        float t =
            m_kart->getKartProperties()->getExplosionInvulnerabilityTime();
        m_kart->showStarEffect(t);
    }
    AbstractKartAnimation::updateGraphics(dt);
}   // updateGraphics

// ----------------------------------------------------------------------------
bool ExplosionAnimation::hasResetAlready() const
{
    return m_reset_ticks != -1 &&
        World::getWorld()->getTicksSinceStart() > m_reset_ticks;
}   // update

// ----------------------------------------------------------------------------
void ExplosionAnimation::saveState(BareNetworkString* buffer)
{
    AbstractKartAnimation::saveState(buffer);
    buffer->addUInt8(m_direct_hit ? 1 : 0);
    if (RaceManager::get()->getMinorMode() ==
        RaceManager::MINOR_MODE_CAPTURE_THE_FLAG && m_direct_hit)
    {
        buffer->addInt24(m_reset_trans_compressed[0])
            .addInt24(m_reset_trans_compressed[1])
            .addInt24(m_reset_trans_compressed[2])
            .addUInt32(m_reset_trans_compressed[3]);
    }
}   // saveState

// ----------------------------------------------------------------------------
void ExplosionAnimation::restoreState(BareNetworkString* buffer)
{
    AbstractKartAnimation::restoreState(buffer);
    restoreData(buffer);
}   // restoreState
