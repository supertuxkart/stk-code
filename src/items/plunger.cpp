//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007-2013 Joerg Henrichs
//
//  Physics improvements and linear intersection algorithm by
//  Copyright (C) 2009-2013 David Mikos.
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
#include "karts/abstract_kart.hpp"
#include "karts/controller/controller.hpp"
#include "karts/kart_properties.hpp"
#include "modes/world.hpp"
#include "physics/physical_object.hpp"
#include "physics/physics.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"
#include "utils/string_utils.hpp"

// -----------------------------------------------------------------------------
Plunger::Plunger(AbstractKart *kart)
       : Flyable(kart, PowerupManager::POWERUP_PLUNGER)
{
    const float gravity = 0.0f;

    float forward_offset = 0.5f*kart->getKartLength()+0.5f*m_extend.getZ();
    float up_velocity = 0.0f;
    float plunger_speed = 2 * m_speed;

    // if the kart is looking backwards, release from the back
    m_reverse_mode = kart->getControls().m_look_back;

    // find closest kart in front of the current one
    const AbstractKart *closest_kart=0;
    Vec3        direction;
    float       kart_dist_2;
    getClosestKart(&closest_kart, &kart_dist_2, &direction,
                   kart /* search in front of this kart */, m_reverse_mode);

    btTransform kart_transform = kart->getAlignedTransform();
    btMatrix3x3 kart_rotation = kart_transform.getBasis();
    // The current forward vector is rotation*(0,0,1), or:
    btVector3 forward(kart_rotation.getColumn(2));

    float heading =kart->getHeading();
    float pitch  = kart->getTerrainPitch(heading);

    // aim at this kart if it's not too far
    if(closest_kart != NULL && kart_dist_2 < 30*30)
    {
        float fire_angle     = 0.0f;
        getLinearKartItemIntersection (kart->getXYZ(), closest_kart,
                                       plunger_speed, gravity, forward_offset,
                                       &fire_angle, &up_velocity);

        btTransform trans = kart->getTrans();

        trans.setRotation(btQuaternion(btVector3(0, 1, 0), fire_angle));

        m_initial_velocity = btVector3(0.0f, up_velocity, plunger_speed);

        createPhysics(forward_offset, m_initial_velocity,
                      new btCylinderShape(0.5f*m_extend),
                      0.5f /* restitution */ , gravity,
                      /* rotates */false , /*turn around*/false, &trans);
    }
    else
    {
        createPhysics(forward_offset, btVector3(pitch, 0.0f, plunger_speed),
                      new btCylinderShape(0.5f*m_extend),
                      0.5f /* restitution */, gravity,
                      false /* rotates */, m_reverse_mode, &kart_transform);
    }

    //adjust height according to terrain
    setAdjustUpVelocity(false);

    // pulling back makes no sense in battle mode, since this mode is not a race.
    // so in battle mode, always hide view
    if( m_reverse_mode || race_manager->isBattleMode() )
        m_rubber_band = NULL;
    else
    {
        m_rubber_band = new RubberBand(this, kart);
    }
    m_keep_alive = -1;
}   // Plunger

// ----------------------------------------------------------------------------
Plunger::~Plunger()
{
    if(m_rubber_band)
        delete m_rubber_band;
}   // ~Plunger

// ----------------------------------------------------------------------------
void Plunger::init(const XMLNode &node, scene::IMesh *plunger_model)
{
    Flyable::init(node, plunger_model, PowerupManager::POWERUP_PLUNGER);
}   // init

// ----------------------------------------------------------------------------
/** Updates the bowling ball ineach frame. If this function returns true, the
 *  object will be removed by the projectile manager.
 *  \param dt Time step size.
 *  \returns True of this object should be removed.
 */
bool Plunger::updateAndDelete(float dt)
{
    // In keep-alive mode, just update the rubber band
    if(m_keep_alive >= 0)
    {
        m_keep_alive -= dt;
        if(m_keep_alive<=0)
        {
            setHasHit();
            return true;
        }
        if(m_rubber_band != NULL) m_rubber_band->update(dt);
        return false;
    }

    // Else: update the flyable and rubber band
    bool ret = Flyable::updateAndDelete(dt);
    if(m_rubber_band != NULL) m_rubber_band->update(dt);

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
    if(isOwnerImmunity(kart)) return false;
    
    // pulling back makes no sense in battle mode, since this mode is not a race.
    // so in battle mode, always hide view
    if( m_reverse_mode || race_manager->isBattleMode() )
    {
        if(kart)
        {
            kart->blockViewWithPlunger();
            if (kart->getController()->isPlayerController())
                SFXManager::get()->quickSound("plunger");
        }

        m_keep_alive = 0;
        // Make this object invisible.
        getNode()->setVisible(false);
        World::getWorld()->getPhysics()->removeBody(getBody());
    }
    else
    {
        m_keep_alive = m_owner->getKartProperties()->getRubberBandDuration();

        // Make this object invisible by placing it faaar down. Not that if this
        // objects is simply removed from the scene graph, it might be auto-deleted
        // because the ref count reaches zero.
        scene::ISceneNode *node = getNode();
        if(node)
        {
            node->setVisible(false);
        }
        World::getWorld()->getPhysics()->removeBody(getBody());

        if(kart)
        {
            m_rubber_band->hit(kart);
            return false;
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
    hit(NULL, NULL);
}   // hitTrack

// -----------------------------------------------------------------------------
