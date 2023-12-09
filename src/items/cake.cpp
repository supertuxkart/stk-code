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

#include "items/cake.hpp"

#include "io/xml_node.hpp"
#include "karts/kart.hpp"
#include "utils/constants.hpp"
#include "utils/random_generator.hpp"

#include "utils/log.hpp"
#include <ISceneNode.h>


float Cake::m_st_max_distance_squared;
float Cake::m_gravity;

Cake::Cake (Kart *kart) : Flyable(kart, PowerupManager::POWERUP_CAKE)
{
    m_target = NULL;
}   // Cake

// -----------------------------------------------------------------------------
/** Initialises the object from an entry in the powerup.xml file.
 *  \param node The xml node for this object.
 *  \param cakde_model The mesh model of the cake.
 */
void Cake::init(const XMLNode &node, scene::IMesh *cake_model)
{
    Flyable::init(node, cake_model, PowerupManager::POWERUP_CAKE);
    float max_distance        = 80.0f;
    m_gravity                 = 9.8f;

    node.get("max-distance",    &max_distance  );
    m_st_max_distance_squared = max_distance*max_distance;
}   // init

// ----------------------------------------------------------------------------
/** Callback from the physics in case that a kart or physical object is hit.
 *  The cake triggers an explosion when hit.
 *  \param kart The kart hit (NULL if no kart was hit).
 *  \param object The object that was hit (NULL if none).
 *  \returns True if there was actually a hit (i.e. not owner, and target is
 *           not immune), false otherwise.
 */
bool Cake::hit(Kart* kart, PhysicalObject* obj)
{
    bool was_real_hit = Flyable::hit(kart, obj);
    if(was_real_hit)
    {
        if(kart && kart->isShielded())
        {
            kart->decreaseShieldTime();
            return false; //Not sure if a shield hit is a real hit.
        }
        if (!m_mini)
            explode(kart, obj);
        else
            explode(kart, obj, /* secondary hits */ false, /* indirect damage */ true);
    }

    return was_real_hit;
}   // hit

// ----------------------------------------------------------------------------
void Cake::onFireFlyable(bool mini)
{
    Flyable::onFireFlyable();
    setDoTerrainInfo(false);

    btVector3 gravity_vector;
    btQuaternion q = m_owner->getTrans().getRotation();
    gravity_vector = Vec3(0, -1, 0).rotate(q.getAxis(), q.getAngle());
    gravity_vector = gravity_vector.normalize() * m_gravity;
    const bool  backwards = m_owner->getControls().getLookBack();
    m_mini = mini;

    if (m_node != NULL) // It can be null when rewinding
    {
        if (m_mini)
            m_node->setScale(core::vector3df(0.6f,0.6f,0.6f));
        else
            m_node->setScale(core::vector3df(1.2f,1.2f,1.2f));
    }

    // A bit of a hack: the mass of this kinematic object is still 1.0
    // (see flyable), which enables collisions. I tried setting
    // collisionFilterGroup/mask, but still couldn't get this object to
    // collide with the track. By setting the mass to 1, collisions happen.
    // (if bullet is compiled with _DEBUG, a warning will be printed the first
    // time a homing-track collision happens).
    float forward_offset=m_owner->getKartLength()/2.0f + m_extend.getZ()/2.0f;

    // Mini cakes are slightly slower than normal cakes
    if(m_mini)
        m_speed = 0.9f * m_speed;

    float up_velocity = m_speed/6.3f;

    // Adds the speed of the kart owning the cake. m_speed is defined in flyable
    m_speed += (backwards ? -m_owner->getSpeed()
                          :  m_owner->getSpeed());

    btTransform trans = m_owner->getTrans();

    float heading=m_owner->getHeading();
    float pitch = m_owner->getTerrainPitch(heading);

    // Find closest kart in front of the current one
    const Kart *closest_kart=NULL;
    Vec3        direction;
    float       kart_dist_squared;
    getClosestKart(&closest_kart, &kart_dist_squared, &direction,
                   m_owner /* search in front of this kart */, backwards);

    // aim at this kart if 1) it's not too far, 2) if the aimed kart's speed
    // allows the projectile to catch up with it
    //
    // this code finds the correct angle and upwards velocity to hit an opponents'
    // vehicle if they were to continue travelling in the same direction and same speed
    // (barring any obstacles in the way of course)

    if(closest_kart != NULL && kart_dist_squared < m_st_max_distance_squared)
    {
        m_target = (Kart*)closest_kart;

        float fire_angle     = 0.0f;
        getLinearKartItemIntersection (m_owner->getXYZ(), closest_kart,
                                       m_speed, m_gravity, forward_offset,
                                       &fire_angle, &up_velocity);

        // apply transformation to the bullet object (without pitch)
        btQuaternion q;
        q = trans.getRotation() * btQuaternion(btVector3(0, 1, 0), fire_angle);
        trans.setRotation(q);
        m_initial_velocity = Vec3(0.0f, up_velocity, m_speed);

        createPhysics(forward_offset, m_initial_velocity,
                      new btCylinderShape(0.5f*m_extend),
                      0.5f /* restitution */, gravity_vector,
                      true /* rotation */, false /* backwards */, &trans);
    }
    else
    {
        m_target = NULL;
        // kart is too far to be hit. so throw the projectile in a generic way,
        // straight ahead, without trying to hit anything in particular
        trans = m_owner->getAlignedTransform(pitch);

        m_initial_velocity = Vec3(0.0f, up_velocity, m_speed);

        createPhysics(forward_offset, m_initial_velocity,
                      new btCylinderShape(0.5f*m_extend),
                      0.5f /* restitution */, gravity_vector,
                      true /* rotation */, backwards, &trans);
    }

    //do not adjust height according to terrain
    setAdjustUpVelocity(false);
    m_body->setActivationState(DISABLE_DEACTIVATION);
    m_body->clearForces();
    m_body->applyTorque(btVector3(5.0f, -3.0f, 7.0f));
}   // onFireFlyable
