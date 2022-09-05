//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 Joerg Henrichs
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

#include "karts/kart_rewinder.hpp"

#include "audio/sfx_manager.hpp"
#include "items/attachment.hpp"
#include "items/powerup.hpp"
#include "guiengine/message_queue.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/cannon_animation.hpp"
#include "karts/explosion_animation.hpp"
#include "karts/rescue_animation.hpp"
#include "karts/controller/player_controller.hpp"
#include "karts/kart_properties.hpp"
#include "karts/max_speed.hpp"
#include "karts/skidding.hpp"
#include "modes/world.hpp"
#include "network/compress_network_body.hpp"
#include "network/protocols/client_lobby.hpp"
#include "network/rewind_manager.hpp"
#include "network/network_string.hpp"
#include "physics/btKart.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"
#include "utils/vec3.hpp"

#include <ISceneNode.h>
#include <string.h>

KartRewinder::KartRewinder(const std::string& ident,
                           unsigned int world_kart_id, int position,
                           const btTransform& init_transform,
                           HandicapLevel handicap,
                           std::shared_ptr<GE::GERenderInfo> ri)
            : Rewinder(
              {
                  RN_KART,
                  static_cast<char>(world_kart_id)
              })
            , Kart(ident, world_kart_id, position, init_transform, handicap,
                   ri)
{
    m_steering_smoothing_dt = -1.0f;
    m_prev_steering = m_steering_smoothing_time = 0.0f;
}   // KartRewinder

// ----------------------------------------------------------------------------
/** Resets status in case of a resetart.
 */
void KartRewinder::reset()
{
    Kart::reset();
    Rewinder::reset();
    // Can be null for the first time
    if (getController() && !getController()->isLocalPlayerController())
    {
        SmoothNetworkBody::setEnable(true);
        SmoothNetworkBody::setSmoothRotation(true);
        SmoothNetworkBody::setAdjustVerticalOffset(true);
    }
    else
        SmoothNetworkBody::setEnable(false);
    m_has_server_state = false;
}   // reset

// ----------------------------------------------------------------------------
/** This function is called immediately before a rewind is done and saves
 *  the current transform for the kart. The difference between this saved
 *  transform and the new transform after rewind is the error that needs
 *  (smoothly) be applied to the graphical position of the kart. 
 */
void KartRewinder::saveTransform()
{
    if (!m_kart_animation)
    {
        Moveable::prepareSmoothing();
        m_skidding->prepareSmoothing();
    }

    m_prev_steering = getSteerPercent();
    m_has_server_state = false;
}   // saveTransform

// ----------------------------------------------------------------------------
void KartRewinder::computeError()
{
    if (m_kart_animation == NULL)
    {
        Moveable::checkSmoothing();
        m_skidding->checkSmoothing();
    }

    float diff = fabsf(m_prev_steering - AbstractKart::getSteerPercent());
    if (diff > 0.05f)
    {
        m_steering_smoothing_time = getTimeFullSteer(diff) / 2.0f;
        m_steering_smoothing_dt = 0.0f;
    }
    else
        m_steering_smoothing_dt = -1.0f;

    if (!m_has_server_state && !isEliminated())
    {
        const int kartid = getWorldKartId();
        Log::debug("KartRewinder", "Kart id %d disconnected.", kartid);

        SFXManager::get()->quickSound("appear");
        core::stringw player_name = getController()->getName();
        // I18N: Message shown in game to tell player left the game in network
        core::stringw msg = _("%s left the game.", player_name);

        MessageQueue::add(MessageQueue::MT_FRIEND, msg);
        World::getWorld()->eliminateKart(kartid,
            false/*notify_of_elimination*/);
        setPosition(World::getWorld()->getCurrentNumKarts() + 1);
        finishedRace(World::getWorld()->getTime(), true/*from_server*/);
        if (RaceManager::get()->supportsLiveJoining())
        {
            RemoteKartInfo& rki = RaceManager::get()->getKartInfo(kartid);
            rki.makeReserved();
        }
    }
    else if (m_has_server_state && isEliminated())
    {
        if (auto cl = LobbyProtocol::get<ClientLobby>())
        {
            Log::debug("KartRewinder", "Kart id %d connected.",
                getWorldKartId());
            cl->requestKartInfo((uint8_t)getWorldKartId());
            // New live join kart, hide the node until new kart info is received
            // see ClientLobby::handleKartInfo
            World::getWorld()->addReservedKart(getWorldKartId());
            reset();
            // Final ticks come from server
            m_live_join_util = std::numeric_limits<int>::max();
            if (getNode())
                getNode()->setVisible(false);
        }
    }
}   // computeError

// ----------------------------------------------------------------------------
/** Saves all state information for a kart in a memory buffer. The memory
 *  is allocated here and the address returned. It will then be managed
 *  by the RewindManager.
 *  \param[out] ru The unique identity of rewinder writing to.
 *  \return The address of the memory buffer with the state.
 */
BareNetworkString* KartRewinder::saveState(std::vector<std::string>* ru)
{
    if (m_eliminated)
        return nullptr;

    ru->push_back(getUniqueIdentity());
    const int MEMSIZE = 17*sizeof(float) + 9+3;

    BareNetworkString *buffer = new BareNetworkString(MEMSIZE);

    // 1) Steering and other player controls
    // -------------------------------------
    getControls().saveState(buffer);
    bool sign_neg = getController()->saveState(buffer);

    // 2) Boolean handling to determine if need saving
    const bool has_animation = m_kart_animation != NULL;
    uint8_t bool_for_each_data = 0;
    if (m_fire_clicked)
        bool_for_each_data |= 1;
    if (m_bubblegum_ticks > 0)
        bool_for_each_data |= (1 << 1);
    if (m_view_blocked_by_plunger > 0)
        bool_for_each_data |= (1 << 2);
    if (m_invulnerable_ticks > 0)
        bool_for_each_data |= (1 << 3);
    if (getEnergy() > 0.0f)
        bool_for_each_data |= (1 << 4);
    if (has_animation)
        bool_for_each_data |= (1 << 5);
    if (m_vehicle->getTimedRotationTicks() > 0)
        bool_for_each_data |= (1 << 6);
    if (m_vehicle->getCentralImpulseTicks() > 0)
        bool_for_each_data |= (1 << 7);
    buffer->addUInt8(bool_for_each_data);

    uint8_t bool_for_each_data_2 = 0;
    if (sign_neg)
        bool_for_each_data_2 |= 1;
    if (m_bounce_back_ticks > 0)
        bool_for_each_data_2 |= (1 << 1);
    if (getAttachment()->getType() != Attachment::ATTACH_NOTHING)
        bool_for_each_data_2 |= (1 << 2);
    if (getPowerup()->getType() != PowerupManager::POWERUP_NOTHING)
        bool_for_each_data_2 |= (1 << 3);
    if (m_bubblegum_torque_sign)
        bool_for_each_data_2 |= (1 << 4);
    buffer->addUInt8(bool_for_each_data_2);

    if (m_bubblegum_ticks > 0)
        buffer->addUInt16(m_bubblegum_ticks);
    if (m_view_blocked_by_plunger > 0)
        buffer->addUInt16(m_view_blocked_by_plunger);
    if (m_invulnerable_ticks > 0)
        buffer->addUInt16(m_invulnerable_ticks);
    if (getEnergy() > 0.0f)
        buffer->addFloat(getEnergy());

    // 3) Kart animation status or physics values (transform and velocities)
    // -------------------------------------------
    if (has_animation)
    {
        buffer->addUInt8(m_kart_animation->getAnimationType());
        m_kart_animation->saveState(buffer);
    }
    else
    {
        CompressNetworkBody::compress(
            m_body.get(), m_motion_state.get(), buffer);

        if (m_vehicle->getTimedRotationTicks() > 0)
        {
            buffer->addUInt16(m_vehicle->getTimedRotationTicks());
            buffer->addFloat(m_vehicle->getTimedRotation());
        }

        // For collision rewind
        if (m_bounce_back_ticks > 0)
            buffer->addUInt8(m_bounce_back_ticks);
        if (m_vehicle->getCentralImpulseTicks() > 0)
        {
            buffer->addUInt16(m_vehicle->getCentralImpulseTicks());
            buffer->add(m_vehicle->getAdditionalImpulse());
        }
    }

    // 4) Attachment, powerup, nitro
    // -----------------------------
    if (getAttachment()->getType() != Attachment::ATTACH_NOTHING)
        getAttachment()->saveState(buffer);
    if (getPowerup()->getType() != PowerupManager::POWERUP_NOTHING)
        getPowerup()->saveState(buffer);

    // 5) Max speed info
    // ------------------
    m_max_speed->saveState(buffer);

    // 6) Skidding
    // -----------
    m_skidding->saveState(buffer);

    return buffer;
}   // saveState

// ----------------------------------------------------------------------------
/** Actually rewind to the specified state. 
 *  \param buffer The buffer with the state info.
 *  \param count Number of bytes that must be used up in this function (not
 *         used).
 */
void KartRewinder::restoreState(BareNetworkString *buffer, int count)
{
    m_has_server_state = true;

    // 1) Steering and other controls
    // ------------------------------
    getControls().rewindTo(buffer);
    getController()->rewindTo(buffer);

    // 2) Boolean handling to determine if need saving
    // -----------
    uint8_t bool_for_each_data = buffer->getUInt8();
    m_fire_clicked = (bool_for_each_data & 1) == 1;
    bool read_bubblegum = ((bool_for_each_data >> 1) & 1) == 1;
    bool read_plunger = ((bool_for_each_data >> 2) & 1) == 1;
    bool read_invulnerable = ((bool_for_each_data >> 3) & 1) == 1;
    bool read_energy =  ((bool_for_each_data >> 4) & 1) == 1;
    bool has_animation_in_state = ((bool_for_each_data >> 5) & 1) == 1;
    bool read_timed_rotation =  ((bool_for_each_data >> 6) & 1) == 1;
    bool read_impulse = ((bool_for_each_data >> 7) & 1) == 1;

    uint8_t bool_for_each_data_2 = buffer->getUInt8();
    bool controller_steer_sign = (bool_for_each_data_2 & 1) == 1;
    if (controller_steer_sign)
    {
        PlayerController* pc = dynamic_cast<PlayerController*>(m_controller);
        if (pc)
            pc->m_steer_val = pc->m_steer_val * -1;
    }
    bool read_bounce_back = ((bool_for_each_data_2 >> 1) & 1) == 1;
    bool read_attachment = ((bool_for_each_data_2 >> 2) & 1) == 1;
    bool read_powerup = ((bool_for_each_data_2 >> 3) & 1) == 1;
    m_bubblegum_torque_sign = ((bool_for_each_data_2 >> 4) & 1) == 1;

    if (read_bubblegum)
        m_bubblegum_ticks = buffer->getUInt16();
    else
        m_bubblegum_ticks = 0;

    if (read_plunger)
        m_view_blocked_by_plunger = buffer->getUInt16();
    else
        m_view_blocked_by_plunger = 0;

    if (read_invulnerable)
        m_invulnerable_ticks = buffer->getUInt16();
    else
        m_invulnerable_ticks = 0;

    if (read_energy)
    {
        float nitro = buffer->getFloat();
        setEnergy(nitro);
    }
    else
        setEnergy(0.0f);

    // 3) Kart animation status or transform and velocities
    // -----------
    if (has_animation_in_state)
    {
        KartAnimationType kat = (KartAnimationType)(buffer->getUInt8());
        if (!m_kart_animation ||
            m_kart_animation->getAnimationType() != kat)
        {
            delete m_kart_animation;
            m_kart_animation = NULL;
            try
            {
                switch (kat)
                {
                case KAT_RESCUE:
                    new RescueAnimation(this, buffer);
                    break;
                case KAT_EXPLOSION:
                    new ExplosionAnimation(this, buffer);
                    break;
                case KAT_CANNON:
                    new CannonAnimation(this, buffer);
                    break;
                }
            }
            catch (const KartAnimationCreationException& kace)
            {
                Log::error("KartRewinder", "Kart animation creation error: %s",
                    kace.what());
                buffer->skip(kace.getSkippingOffset());
                m_kart_animation = NULL;
            }
        }
        else
            m_kart_animation->restoreState(buffer);
    }
    else
    {
        if (m_kart_animation)
        {
            // Delete unconfirmed kart animation
            delete m_kart_animation;
            m_kart_animation = NULL;
        }

        // Clear any forces applied (like by plunger or bubble gum torque)
        m_body->clearForces();
        CompressNetworkBody::decompress(
            buffer, m_body.get(), m_motion_state.get());
        // Update kart transform in case that there are access to its value
        // before Moveable::update() is called (which updates the transform)
        m_transform = m_body->getWorldTransform();

        if (read_timed_rotation)
        {
            uint16_t time_rot = buffer->getUInt16();
            float timed_rotation_y = buffer->getFloat();
            // Set timed rotation divides by time_rot
            m_vehicle->setTimedRotation(time_rot,
                stk_config->ticks2Time(time_rot) * timed_rotation_y);
        }
        else
            m_vehicle->setTimedRotation(0, 0.0f);

        // Collision rewind
        if (read_bounce_back)
            m_bounce_back_ticks = buffer->getUInt8();
        else
            m_bounce_back_ticks = 0;
        if (read_impulse)
        {
            uint16_t central_impulse_ticks = buffer->getUInt16();
            Vec3 additional_impulse = buffer->getVec3();
            m_vehicle->setTimedCentralImpulse(central_impulse_ticks,
                additional_impulse, true/*rewind*/);
        }
        else
            m_vehicle->setTimedCentralImpulse(0, Vec3(0.0f), true/*rewind*/);

        // For the raycast to determine the current material under the kart
        // the m_hardPointWS of the wheels is used. So after a rewind we
        // must restore the m_hardPointWS to the new values, otherwise they
        // would still point at the kart position at the previous rewind
        // (i.e. different terrain --> different slowdown).
        m_vehicle->updateAllWheelTransformsWS();
    }

    // 4) Attachment, powerup, nitro
    // ------------------------------
    if (read_attachment)
        getAttachment()->rewindTo(buffer);
    else
        getAttachment()->clear();
    // Required for going back to anvil when rewinding
    updateWeight();

    if (read_powerup)
        getPowerup()->rewindTo(buffer);
    else
        getPowerup()->set(PowerupManager::POWERUP_NOTHING, 0);

    // 5) Max speed info
    // ------------------
    m_max_speed->rewindTo(buffer);

    // 6) Skidding
    // -----------
    m_skidding->rewindTo(buffer);

}   // restoreState

// ----------------------------------------------------------------------------
/** Called once a frame. It will add a new kart control event to the rewind
 *  manager if any control values have changed.
 */
void KartRewinder::update(int ticks)
{
    Kart::update(ticks);
}   // update

// ----------------------------------------------------------------------------
std::function<void()> KartRewinder::getLocalStateRestoreFunction()
{
    if (m_eliminated)
        return nullptr;

    // Variable can be saved locally if its adjustment only depends on the kart
    // itself
    int brake_ticks = m_brake_ticks;
    int8_t min_nitro_ticks = m_min_nitro_ticks;

    // Controller local state
    int steer_val_l = 0;
    int steer_val_r = 0;
    PlayerController* pc = dynamic_cast<PlayerController*>(m_controller);
    if (pc)
    {
        steer_val_l = pc->m_steer_val_l;
        steer_val_r = pc->m_steer_val_r;
    }

    // Max speed local state (terrain)
    float current_fraction = m_max_speed->m_speed_decrease
        [MaxSpeed::MS_DECREASE_TERRAIN].m_current_fraction;
    uint16_t max_speed_fraction = m_max_speed->m_speed_decrease
        [MaxSpeed::MS_DECREASE_TERRAIN].m_max_speed_fraction;

    // Skidding local state
    float remaining_jump_time = m_skidding->m_remaining_jump_time;

    return [brake_ticks, min_nitro_ticks,
        steer_val_l, steer_val_r, current_fraction,
        max_speed_fraction, remaining_jump_time, this]()
    {
        m_brake_ticks = brake_ticks;
        m_min_nitro_ticks = min_nitro_ticks;
        PlayerController* pc = dynamic_cast<PlayerController*>(m_controller);
        if (pc)
        {
            pc->m_steer_val_l = steer_val_l;
            pc->m_steer_val_r = steer_val_r;
        }
        m_max_speed->m_speed_decrease[MaxSpeed::MS_DECREASE_TERRAIN]
            .m_current_fraction = current_fraction;
        m_max_speed->m_speed_decrease[MaxSpeed::MS_DECREASE_TERRAIN]
            .m_max_speed_fraction = max_speed_fraction;
        m_skidding->m_remaining_jump_time = remaining_jump_time;
    };
}   // getLocalStateRestoreFunction
