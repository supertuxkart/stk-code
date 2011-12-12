//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007 Joerg Henrichs
//
//  Physics improvements and linear intersection algorithm by
//  by David Mikos. Copyright (C) 2009.
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

float Cake::m_st_max_distance_squared;
float Cake::m_gravity;

Cake::Cake (Kart *kart) : Flyable(kart, PowerupManager::POWERUP_CAKE)
{
    m_target = NULL;

    // A bit of a hack: the mass of this kinematic object is still 1.0
    // (see flyable), which enables collisions. I tried setting
    // collisionFilterGroup/mask, but still couldn't get this object to
    // collide with the track. By setting the mass to 1, collisions happen.
    // (if bullet is compiled with _DEBUG, a warning will be printed the first
    // time a homing-track collision happens).
    float forward_offset=kart->getKartLength()/2.0f + m_extend.getZ()/2.0f;

    float up_velocity = m_speed/7.0f;

    // give a speed proportional to kart speed. m_speed is defined in flyable
    m_speed *= kart->getSpeed() / 23.0f;

    //when going backwards, decrease speed of cake by less
    if (kart->getSpeed() < 0) m_speed /= 3.6f; 

    m_speed += 16.0f;

    if (m_speed < 1.0f) m_speed = 1.0f;

    btTransform trans = kart->getTrans();

    float heading=kart->getHeading();
    float pitch = kart->getTerrainPitch(heading);

    // Find closest kart in front of the current one
    const bool  backwards = kart->getControls().m_look_back;
    const Kart *closest_kart=NULL;
    Vec3        direction;
    float       kart_dist_squared;
    getClosestKart(&closest_kart, &kart_dist_squared, &direction, 
                   kart /* search in front of this kart */, backwards);

    // aim at this kart if 1) it's not too far, 2) if the aimed kart's speed
    // allows the projectile to catch up with it
    //
    // this code finds the correct angle and upwards velocity to hit an opponents'
    // vehicle if they were to continue travelling in the same direction and same speed
    // (barring any obstacles in the way of course)
    if(closest_kart != NULL && kart_dist_squared < m_st_max_distance_squared &&
        m_speed>closest_kart->getSpeed())
    {
        m_target = (Kart*)closest_kart;

        float fire_angle     = 0.0f;
        getLinearKartItemIntersection (kart->getXYZ(), closest_kart,
                                       m_speed, m_gravity, forward_offset,
                                       &fire_angle, &up_velocity);

        // apply transformation to the bullet object (without pitch)
        trans.setRotation(btQuaternion(btVector3(0,1,0), fire_angle));
        
        m_initial_velocity = Vec3(0.0f, up_velocity, m_speed);

        createPhysics(forward_offset, m_initial_velocity,
                      new btCylinderShape(0.5f*m_extend), 
                      0.5f /* restitution */, -m_gravity,
                      true /* rotation */, false /* backwards */, &trans);
    }
    else
    {
        m_target = NULL;
        // kart is too far to be hit. so throw the projectile in a generic way,
        // straight ahead, without trying to hit anything in particular
        trans = kart->getAlignedTransform(pitch);
        
        m_initial_velocity = Vec3(0.0f, up_velocity, m_speed);

        createPhysics(forward_offset, m_initial_velocity,
                      new btCylinderShape(0.5f*m_extend), 
                      0.5f /* restitution */, -m_gravity,
                      true /* rotation */, backwards, &trans);
    }


    //do not adjust height according to terrain
    setAdjustUpVelocity(false);

    m_body->setActivationState(DISABLE_DEACTIVATION);

    m_body->applyTorque( btVector3(5,-3,7) );

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

    if (m_gravity < 0) m_gravity *= -1.0f;

    node.get("max-distance",    &max_distance  );
    m_st_max_distance_squared = max_distance*max_distance;
}   // init

// ----------------------------------------------------------------------------
/** Picks a random message to be displayed when a kart is hit by a cake.
 *  \param The kart that was hit (ignored here).
 *  \returns The string to display.
 */
const core::stringw Cake::getHitString(const Kart *kart) const
{
    const int CAKE_STRINGS_AMOUNT = 3;
    RandomGenerator r;
    switch (r.get(CAKE_STRINGS_AMOUNT))
    {
        //I18N: shown when hit by cake. %1 is the attacker, %0 is the victim.
        case 0: return _LTR("%0 eats too much of %1's cake");
        //I18N: shown when hit by cake. %1 is the attacker, %0 is the victim.
        case 1: return _LTR("%0 is dubious of %1's cooking skills");
        //I18N: shown when hit by cake. %1 is the attacker, %0 is the victim.
        case 2: return _LTR("%0 should not play with %1's lunch");
        default: assert(false); return L"";   // avoid compiler warning
    }
}   // getHitString
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
        explode(kart, obj);

    return was_real_hit;
}   // hit
