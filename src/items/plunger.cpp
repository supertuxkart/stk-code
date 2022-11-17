//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007-2015 Joerg Henrichs
//
//  Physics improvements and linear intersection algorithm by
//  Copyright (C) 2009-2015 David Mikos.
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

#include "items/plunger.hpp"

#include "audio/sfx_manager.hpp"
#include "io/xml_node.hpp"
#include "items/rubber_band.hpp"
#include "items/projectile_manager.hpp"
#include "graphics/central_settings.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/controller.hpp"
#include "karts/kart_properties.hpp"
#include "network/network_string.hpp"
#include "physics/physical_object.hpp"
#include "physics/physics.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"
#include "utils/string_utils.hpp"

#include <ISceneNode.h>

// -----------------------------------------------------------------------------
Plunger::Plunger(AbstractKart *kart)
       : Flyable(kart, PowerupManager::POWERUP_PLUNGER)
{
    m_has_locally_played_sound = false;
    m_moved_to_infinity = false;
    m_reverse_mode = false;
    m_rubber_band = NULL;
}   // Plunger

// ----------------------------------------------------------------------------
Plunger::~Plunger()
{
    if(m_rubber_band)
        delete m_rubber_band;
}   // ~Plunger

// ----------------------------------------------------------------------------
void Plunger::onFireFlyable()
{
    Flyable::onFireFlyable();
    m_has_locally_played_sound = false;
    m_moved_to_infinity = false;
    const float gravity = 0.0f;

    setDoTerrainInfo(false);

    float forward_offset = 0.5f*m_owner->getKartLength()+0.5f*m_extend.getZ();
    float up_velocity = 0.0f;
    float plunger_speed = 2 * m_speed;

    // if the kart is looking backwards, release from the back
    m_reverse_mode = m_owner->getControls().getLookBack();

    // find closest kart in front of the current one
    const AbstractKart *closest_kart=0;
    Vec3        direction;
    float       kart_dist_2;
    getClosestKart(&closest_kart, &kart_dist_2, &direction,
                   m_owner /* search in front of this kart */, m_reverse_mode);

    btTransform kart_transform = m_owner->getAlignedTransform();

    float heading =m_owner->getHeading();
    float pitch  = m_owner->getTerrainPitch(heading);

    // aim at this kart if it's not too far
    if(closest_kart != NULL && kart_dist_2 < 30*30)
    {
        float fire_angle     = 0.0f;
        getLinearKartItemIntersection (m_owner->getXYZ(), closest_kart,
                                       plunger_speed, gravity, forward_offset,
                                       &fire_angle, &up_velocity);

        btTransform trans = m_owner->getTrans();
        btQuaternion q;
        q = trans.getRotation()*(btQuaternion(btVector3(0, 1, 0), fire_angle));
        trans.setRotation(q);

        m_initial_velocity = btVector3(0.0f, up_velocity, plunger_speed);

        createPhysics(forward_offset, m_initial_velocity,
                      new btCylinderShape(0.5f*m_extend),
                      0.5f /* restitution */ , btVector3(.0f,gravity,.0f),
                      /* rotates */false , /*turn around*/false, &trans);
    }
    else
    {
        createPhysics(forward_offset, btVector3(pitch, 0.0f, plunger_speed),
                      new btCylinderShape(0.5f*m_extend),
                      0.5f /* restitution */, btVector3(.0f,gravity,.0f),
                      false /* rotates */, m_reverse_mode, &kart_transform);
    }

    //adjust height according to terrain
    setAdjustUpVelocity(false);

    const bool create_rubber_band =
        !(m_reverse_mode || RaceManager::get()->isBattleMode());
    if (create_rubber_band && !m_rubber_band)
        m_rubber_band = new RubberBand(this, m_owner);
    else if (!create_rubber_band && m_rubber_band)
    {
        delete m_rubber_band;
        m_rubber_band = NULL;
    }

    if (m_rubber_band)
        m_rubber_band->reset();

    m_keep_alive = -1;
    m_moved_to_infinity = false;
}   // onFireFlyable

// ----------------------------------------------------------------------------
void Plunger::init(const XMLNode &node, scene::IMesh *plunger_model)
{
    Flyable::init(node, plunger_model, PowerupManager::POWERUP_PLUNGER);
}   // init

// ----------------------------------------------------------------------------
void Plunger::updateGraphics(float dt)
{
    Flyable::updateGraphics(dt);
#ifndef SERVER_ONLY
    scene::ISceneNode *node = getNode();
    if (node && m_moved_to_infinity)
        node->setVisible(false);
    else if (node && !m_moved_to_infinity)
        node->setVisible(true);
#endif
    if (m_rubber_band)
        m_rubber_band->updateGraphics(dt);
}   // updateGraphics

// ----------------------------------------------------------------------------
/** Updates the plunger in each frame. If this function returns true, the
 *  object will be removed by the projectile manager.
 *  \param dt Time step size.
 *  \returns True of this object should be removed.
 */
bool Plunger::updateAndDelete(int ticks)
{
    // In keep-alive mode, just update the rubber band
    if(m_keep_alive >= 0)
    {
        m_keep_alive -= ticks;
        if(m_keep_alive<=0)
        {
            setHasHit();
            return true;
        }
        if(m_rubber_band != NULL) m_rubber_band->update(ticks);
        return false;
    }

    // Else: update the flyable and rubber band
    bool ret = Flyable::updateAndDelete(ticks);
    if(m_rubber_band != NULL) m_rubber_band->update(ticks);

    return ret;

}   // updateAndDelete
// -----------------------------------------------------------------------------
/** Virtual function called when the plunger hits something.
 *  The plunger is special in that it is not deleted when hitting an object.
 *  Instead it stays around (though not as a graphical or physical object)
 *  till the rubber band expires.
 *  \param kart Pointer to the kart hit (NULL if not a kart).
 *  \param obj  Pointer to PhysicalObject object if hit (NULL otherwise).
 *  \returns True if there was actually a hit (i.e. not owner, and target is
 *           not immune), false otherwise.
 */
bool Plunger::hit(AbstractKart *kart, PhysicalObject *obj)
{
    if (isOwnerImmunity(kart) || m_moved_to_infinity || !m_has_server_state)
        return false;

    // pulling back makes no sense in battle mode, since this mode is not a race.
    // so in battle mode, always hide view
    if( m_reverse_mode || RaceManager::get()->isBattleMode() )
    {
        if(kart)
        {
            kart->blockViewWithPlunger();
            if (kart->getController()->isLocalPlayerController() &&
                !m_has_locally_played_sound)
            {
                m_has_locally_played_sound = true;
                SFXManager::get()->quickSound("plunger");
            }
        }

        m_keep_alive = 0;
        // Previously removeBody will break rewind
        moveToInfinity(false/*set_moveable_trans*/);
        m_moved_to_infinity = true;
    }
    else
    {
        m_keep_alive = (int16_t)stk_config->time2Ticks(m_owner->getKartProperties()
            ->getPlungerBandDuration());
        if(kart)
        {
            m_rubber_band->hit(kart);
        }
        else if(obj)
        {
            Vec3 pos(obj->getBody()->getWorldTransform().getOrigin());
            m_rubber_band->hit(NULL, &pos);
        }
        else
        {
            m_rubber_band->hit(NULL, &(getXYZ()));
        }
        // Previously removeBody will break rewind
        moveToInfinity(false/*set_moveable_trans*/);
        m_moved_to_infinity = true;
    }

    // Rubber band attached.
    return false;
}   // hit

// -----------------------------------------------------------------------------
/** Called when the plunger hits the track. In this case, notify the rubber
 *  band, and remove the plunger (but keep it alive).
 */
void Plunger::hitTrack()
{
    if (m_moved_to_infinity || !m_has_server_state)
        return;
    hit(NULL, NULL);
}   // hitTrack

// ----------------------------------------------------------------------------
BareNetworkString* Plunger::saveState(std::vector<std::string>* ru)
{
    BareNetworkString* buffer = Flyable::saveState(ru);
    if (!buffer)
        return NULL;

    buffer->addUInt16(m_keep_alive);
    if (m_rubber_band)
        buffer->addUInt8(m_rubber_band->get8BitState());
    else
        buffer->addUInt8(255);
    return buffer;
}   // saveState

// ----------------------------------------------------------------------------
void Plunger::restoreState(BareNetworkString *buffer, int count)
{
    Flyable::restoreState(buffer, count);
    m_keep_alive = buffer->getUInt16();
    // Restore position base on m_keep_alive in Plunger::hit
    if (m_keep_alive == -1)
        m_moved_to_infinity = false;
    else
    {
        moveToInfinity(false/*set_moveable_trans*/);
        m_moved_to_infinity = true;
    }

    uint8_t bit_state = buffer->getUInt8();
    if (bit_state == 255 && m_rubber_band)
    {
        delete m_rubber_band;
        m_rubber_band = NULL;
        if (!m_reverse_mode)
            m_reverse_mode = true;
    }
    else if (bit_state != 255 && !m_rubber_band)
    {
        m_rubber_band = new RubberBand(this, m_owner);
        if (m_reverse_mode)
            m_reverse_mode = false;
    }
    if (bit_state != 255)
        m_rubber_band->set8BitState(bit_state);
}   // restoreState

// ----------------------------------------------------------------------------
void Plunger::onDeleteFlyable()
{
    Flyable::onDeleteFlyable();
    if (m_rubber_band)
        m_rubber_band->remove();
}   // onDeleteFlyable
