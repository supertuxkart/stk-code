//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007 Joerg Henrichs
//
//  Linear item-kart intersection function written by
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

#include "items/flyable.hpp"

#if defined(WIN32) && !defined(__CYGWIN__)
#  define isnan _isnan
#endif
#include <math.h>

#include <IMeshManipulator.h>
#include <IMeshSceneNode.h>

#include "graphics/explosion.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/mesh_tools.hpp"
#include "graphics/stars.hpp"
#include "io/xml_node.hpp"
#include "items/projectile_manager.hpp"
#include "karts/kart.hpp"
#include "modes/world.hpp"
#include "network/flyable_info.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"
#include "utils/string_utils.hpp"

// static variables:
float         Flyable::m_st_speed       [PowerupManager::POWERUP_MAX];
scene::IMesh* Flyable::m_st_model       [PowerupManager::POWERUP_MAX];
float         Flyable::m_st_min_height  [PowerupManager::POWERUP_MAX];
float         Flyable::m_st_max_height  [PowerupManager::POWERUP_MAX];
float         Flyable::m_st_force_updown[PowerupManager::POWERUP_MAX];
Vec3          Flyable::m_st_extend      [PowerupManager::POWERUP_MAX];
// ----------------------------------------------------------------------------

Flyable::Flyable(Kart *kart, PowerupManager::PowerupType type, float mass) 
       : Moveable(), TerrainInfo()
{
    // get the appropriate data from the static fields
    m_speed                        = m_st_speed[type];
    m_extend                       = m_st_extend[type];
    m_max_height                   = m_st_max_height[type];
    m_min_height                   = m_st_min_height[type];
    m_average_height               = (m_min_height+m_max_height)/2.0f;
    m_force_updown                 = m_st_force_updown[type];
    m_owner                        = kart;
    m_type                         = type;
    m_has_hit_something            = false;
    m_exploded                     = false;
    m_shape                        = NULL;
    m_mass                         = mass;
    m_adjust_up_velocity           = true;
    m_time_since_thrown            = 0;
    m_position_offset              = Vec3(0,0,0);
    m_owner_has_temporary_immunity = true;
    m_max_lifespan = -1;

    // Add the graphical model
    setNode(irr_driver->addMesh(m_st_model[type]));
#ifdef DEBUG
    std::string debug_name("flyable: ");
    debug_name += type;
    getNode()->setName(debug_name.c_str());
#endif

}   // Flyable

// ----------------------------------------------------------------------------
/** Creates a bullet physics body for the flyable item.
 *  \param forw_offset How far ahead of the kart the flyable should be 
 *         positioned. Necessary to avoid exploding a rocket inside of the
 *         firing kart.
 *  \param velocity Initial velocity of the flyable.
 *  \param shape Collision shape of the flyable.
 *  \param gravity Gravity to use for this flyable.
 *  \param rotates True if the item should rotate, otherwise the angular factor
 *         is set to 0 preventing rotations from happening.
 *  \param turn_around True if the item is fired backwards.
 *  \param custom_direction If defined the initial heading for this item, 
 *         otherwise the kart's heading will be used.
 */
void Flyable::createPhysics(float forw_offset, const Vec3 &velocity,
                            btCollisionShape *shape, const float gravity,
                            const bool rotates, const bool turn_around,
                            const btTransform* custom_direction)
{
    // Get Kart heading direction
    btTransform trans = ( !custom_direction ? m_owner->getAlignedTransform()
                                            : *custom_direction          );

    // Apply offset
    btTransform offset_transform;
    offset_transform.setIdentity();
    assert(!isnan(m_average_height));
    assert(!isnan(forw_offset));
    offset_transform.setOrigin(Vec3(0,m_average_height,forw_offset));

    // turn around
    if(turn_around)
    {
        btTransform turn_around_trans;
        //turn_around_trans.setOrigin(trans.getOrigin());
        turn_around_trans.setIdentity();
        turn_around_trans.setRotation(btQuaternion(btVector3(0, 1, 0), M_PI));
        trans  *= turn_around_trans;
    }

    trans  *= offset_transform;

    m_shape = shape;
    createBody(m_mass, trans, m_shape);
    m_user_pointer.set(this);
    World::getWorld()->getPhysics()->addBody(getBody());

    m_body->setGravity(btVector3(0.0f, gravity, 0));

    // Rotate velocity to point in the right direction
    btVector3 v=trans.getBasis()*velocity;

    if(m_mass!=0.0f)  // Don't set velocity for kinematic or static objects
    {
        assert(!isnan(v.getX()));
        assert(!isnan(v.getY()));
        assert(!isnan(v.getZ()));
        m_body->setLinearVelocity(v);
        if(!rotates) m_body->setAngularFactor(0.0f);   // prevent rotations
    }
    m_body->setCollisionFlags(m_body->getCollisionFlags() | 
                              btCollisionObject::CF_NO_CONTACT_RESPONSE);

}   // createPhysics
// -----------------------------------------------------------------------------
void Flyable::init(const XMLNode &node, scene::IMesh *model,
                   PowerupManager::PowerupType type)
{
    m_st_speed[type]        = 25.0f;
    m_st_max_height[type]   = 1.0f;
    m_st_min_height[type]   = 3.0f;
    m_st_force_updown[type] = 15.0f;
    node.get("speed",           &(m_st_speed[type])       );
    node.get("min-height",      &(m_st_min_height[type])  );
    node.get("max-height",      &(m_st_max_height[type])  );
    node.get("force-updown",    &(m_st_force_updown[type]));
    core::vector3df scale(1.0f, 1.0f, 1.0f);
    if(node.get("scale",        &scale))
    {
        irr::scene::IMeshManipulator *mani = 
            irr_driver->getVideoDriver()->getMeshManipulator();
        mani->scale(model, scale);
    }
    
    // Store the size of the model
    Vec3 min, max;
    MeshTools::minMax3D(model, &min, &max);
    m_st_extend[type] = btVector3(max-min);
    m_st_model[type]  = model;
}   // init

//-----------------------------------------------------------------------------
Flyable::~Flyable()
{
    if(m_shape) delete m_shape;
    World::getWorld()->getPhysics()->removeBody(getBody());
}   // ~Flyable

//-----------------------------------------------------------------------------
/** Returns information on what is the closest kart and at what distance it is.
 *  All 3 parameters first are of type 'out'. 'inFrontOf' can be set if you 
 *  wish to know the closest kart in front of some karts (will ignore those 
 *  behind). Useful e.g. for throwing projectiles in front only.
 */

void Flyable::getClosestKart(const Kart **minKart, float *minDistSquared,
                             Vec3 *minDelta, const Kart* inFrontOf, 
                             const bool backwards) const
{
    btTransform trans_projectile = (inFrontOf != NULL ? inFrontOf->getTrans()
                                                      : getTrans());

    *minDistSquared = 999999.9f;
    *minKart = NULL;

    World *world = World::getWorld();
    for(unsigned int i=0 ; i<world->getNumKarts(); i++ )
    {
        Kart *kart = world->getKart(i);
        // If a kart has star effect shown, the kart is immune, so
        // it is not considered a target anymore.
        if(kart->isEliminated() || kart == m_owner || 
            kart->isInvulnerable()                 ||
            kart->playingEmergencyAnimation() ) continue;
        btTransform t=kart->getTrans();

        Vec3 delta      = t.getOrigin()-trans_projectile.getOrigin();
        // the Y distance is added again because karts above or below should//
        // not be prioritized when aiming
        float distance2 = delta.length2() + abs(t.getOrigin().getY() 
                        - trans_projectile.getOrigin().getY())*2;

        if(inFrontOf != NULL)
        {
            // Ignore karts behind the current one
            Vec3 to_target       = kart->getXYZ() - inFrontOf->getXYZ();
            const float distance = to_target.length();
            if(distance > 50) continue; // kart too far, don't aim at it

            btTransform trans = inFrontOf->getTrans();
            // get heading=trans.getBasis*(0,0,1) ... so save the multiplication:
            Vec3 direction(trans.getBasis().getColumn(2));
            // Originally it used angle = to_target.angle( backwards ? -direction : direction );
            // but sometimes due to rounding errors we get an acos(x) with x>1, causing
            // an assertion failure. So we remove the whole acos() test here and copy the
            // code from to_target.angle(...)
            Vec3  v = backwards ? -direction : direction;
            float s = sqrt(v.length2() * to_target.length2());
            float c = to_target.dot(v)/s;
            // Original test was: fabsf(acos(c))>1,  which is the same as
            // c<cos(1) (acos returns values in [0, pi] anyway) 
            if(c<0.54) continue;
        }

        if(distance2 < *minDistSquared)
        {
            *minDistSquared = distance2;
            *minKart  = kart;
            *minDelta = delta;
        }
    }  // for i<getNumKarts

}   // getClosestKart

//-----------------------------------------------------------------------------
/** Returns information on the parameters needed to hit a target kart moving 
 *  at constant velocity and direction for a given speed in the XZ-plane.
 *  \param origin Location of the kart shooting the item.
 *  \param target_kart Which kart to target.
 *  \param item_xz_speed Speed of the item projected in XZ plane.
 *  \param gravity The gravity used for this item.
 *  \param forw_offset How far ahead of the kart the item is shot (so that
 *         the item does not originate inside of the shooting kart.
 *  \param fire_angle Returns the angle to fire the item at.
 *  \param up_velocity Returns the upwards velocity to use for the item.
 */
void Flyable::getLinearKartItemIntersection (const Vec3 &origin, 
                                             const Kart *target_kart,
                                             float item_XZ_speed, 
                                             float gravity, float forw_offset,
                                             float *fire_angle, 
                                             float *up_velocity)
{
    Vec3 relative_target_kart_loc = target_kart->getXYZ() - origin;

    btTransform trans = target_kart->getTrans();
    Vec3 target_direction(trans.getBasis().getColumn(2));

    float dx = relative_target_kart_loc.getX();
    float dy = relative_target_kart_loc.getY();
    float dz = relative_target_kart_loc.getZ();

    float gy = target_direction.getY();

    //Projected onto X-Z plane
    float target_kart_speed = target_direction.length_2d() 
                            * target_kart->getSpeed();

    float target_kart_heading = target_kart->getHeading();

    float dist = -(target_kart_speed / item_XZ_speed) 
               * (dx * cosf(target_kart_heading) -
                  dz * sinf(target_kart_heading)   );

    float fire_th = (dx*dist - dz * sqrtf(dx*dx + dz*dz - dist*dist))
                  / (dx*dx + dz*dz);
    fire_th = (((dist - dx*fire_th) / dz > 0) ? -acosf(fire_th)
                                              :  acosf(fire_th));

    float time = 0.0f;
    float a = item_XZ_speed     * sinf (fire_th) 
            + target_kart_speed * sinf (target_kart_heading);
    float b = item_XZ_speed     * cosf (fire_th) 
            + target_kart_speed * cosf (target_kart_heading);

    if (fabsf(a) > fabsf(b)) time = fabsf (dx / a);
    else if (b != 0.0f)      time = fabsf(dz / b);

    if (fire_th > M_PI)
        fire_th -= M_PI;
    else
        fire_th += M_PI;

    //createPhysics offset
    time -= forw_offset / sqrt(a*a+b*b);

    *fire_angle = fire_th;
    *up_velocity = (0.5f * time * gravity) + (dy / time) 
                 + (gy * target_kart->getSpeed());
}   // getLinearKartItemIntersection

//-----------------------------------------------------------------------------
/** Updates this flyable. It calls Moveable::update. If this function returns
 *  true, the flyable will be deleted by the projectile manager.
 *  \param dt Time step size.
 *  \returns True if this object can be deleted.
 */
bool Flyable::updateAndDelete(float dt)
{
    m_time_since_thrown += dt;
    if(m_max_lifespan > -1 && m_time_since_thrown > m_max_lifespan) 
        hit(NULL);

    if(m_exploded) return true;

    Vec3 xyz=getBody()->getWorldTransform().getOrigin();
    // Check if the flyable is outside of the track. If so, explode it.
    const Vec3 *min, *max;
    World::getWorld()->getTrack()->getAABB(&min, &max);

    // I have seen that the bullet AABB can be slightly different from the 
    // one computed here - I assume due to minor floating point errors
    // (e.g. 308.25842 instead of 308.25845). To avoid a crash with a bullet
    // assertion (see bug 3058932) I add an epsilon here - but admittedly
    // that does not really explain the bullet crash, since bullet tests
    // against its own AABB, and should therefore not cause the assertion.
    // But since we couldn't reproduce the problem, and the epsilon used
    // here does not hurt, I'll leave it in.
    float eps = 0.1f;
    assert(!isnan(xyz.getX()));
    assert(!isnan(xyz.getY()));
    assert(!isnan(xyz.getZ()));
    if(xyz[0]<(*min)[0]+eps || xyz[2]<(*min)[2]+eps || xyz[1]<(*min)[1]+eps ||
       xyz[0]>(*max)[0]-eps || xyz[2]>(*max)[2]-eps || xyz[1]>(*max)[1]-eps   )
    {
        hit(NULL);    // flyable out of track boundary
        return true;
    }

    // Add the position offset so that the flyable can adjust its position 
    // (usually to do the raycast from a slightly higher position to avoid
    // problems finding the terrain in steep uphill sections).
    TerrainInfo::update(xyz+m_position_offset);

    // Remove flyable if its
    if(TerrainInfo::getMaterial()==NULL)
        return true;

    if(m_adjust_up_velocity)
    {
        float hat = xyz.getY()-getHoT();

        // Use the Height Above Terrain to set the Z velocity.
        // HAT is clamped by min/max height. This might be somewhat
        // unphysical, but feels right in the game.

        float delta = m_average_height - std::max(std::min(hat, m_max_height),
                                                  m_min_height);
        Vec3 v = getVelocity();
        assert(!isnan(v.getX()));
        assert(!isnan(v.getX()));
        assert(!isnan(v.getX()));
        float heading = atan2f(v.getX(), v.getZ());
        assert(!isnan(heading));
        float pitch   = getTerrainPitch(heading);
        float vel_up = m_force_updown*(delta);
        if (hat < m_max_height) // take into account pitch of surface
            vel_up += v.length_2d()*tanf(pitch);
        assert(!isnan(vel_up));
        v.setY(vel_up);
        setVelocity(v);
    }   // if m_adjust_up_velocity

    Moveable::update(dt);

    return false;
}   // updateAndDelete

// ----------------------------------------------------------------------------
/** Updates the position of a projectile based on information received frmo the
 *  server.
 */
void Flyable::updateFromServer(const FlyableInfo &f, float dt)
{
    setXYZ(f.m_xyz);
    setRotation(f.m_rotation);
    // m_exploded is not set here, since otherwise when explode() is called,
    // the rocket is considered to be already exploded.
    // Update the graphical position
    Moveable::update(dt);
}   // updateFromServer

// ----------------------------------------------------------------------------
/** Returns true if the item hit the kart who shot it (to avoid that an item
 *  that's too close to the shoter hits the shoter).
 *  \param kart Kart who was hit.
 */
bool Flyable::isOwnerImmunity(const Kart* kart_hit) const
{
    return m_owner_has_temporary_immunity &&
           kart_hit == m_owner            &&
           m_time_since_thrown < 2.0f;
}   // isOwnerImmunity

// ----------------------------------------------------------------------------
/** Callback from the physics in case that a kart or physical object is hit. 
 *  kart The kart hit (NULL if no kart was hit).
 *  object The object that was hit (NULL if none).
 */
void Flyable::hit(Kart *kart_hit, PhysicalObject* object)
{
    // the owner of this flyable should not be hit by his own flyable
    if(m_exploded || isOwnerImmunity(kart_hit)) return;

    if (kart_hit != NULL)
    {
        RaceGUIBase* gui = World::getWorld()->getRaceGUI();
        irr::core::stringw hit_message =
            StringUtils::insertValues(getHitString(kart_hit),
                                      core::stringw(kart_hit->getName()),
                                      core::stringw(m_owner ->getName())
                                                                         );
        if(hit_message.size()>0)
            gui->addMessage(translations->fribidize(hit_message), NULL, 3.0f,
                            40, video::SColor(255, 255, 255, 255), false);
    }

    m_has_hit_something=true;

    m_exploded=true;
    
    return;

}   // hit

// ----------------------------------------------------------------------------
/** Creates the explosion physical effect, i.e. pushes the karts and ph
 *  appropriately. The corresponding visual/sfx needs to be added manually!
 */
void Flyable::explode(Kart *kart_hit, PhysicalObject *object)
{
    // Apply explosion effect
    // ----------------------
    World *world = World::getWorld();
    for ( unsigned int i = 0 ; i < world->getNumKarts() ; i++ )
    {
        Kart *kart = world->getKart(i);
        // Handle the actual explosion. The kart that fired a flyable will
        // only be affected if it's a direct hit. This allows karts to use
        // rockets on short distance.
        if(m_owner!=kart || m_owner==kart_hit)
        {
            // Set a flag it if was a direct hit.
            kart->handleExplosion(getXYZ(), kart==kart_hit);
            if(kart==kart_hit && world->getTrack()->isArena())
            {
                world->kartHit(kart->getWorldKartId());
            }
        }
    }
    world->getTrack()->handleExplosion(getXYZ(), object);
}   // hit
// ----------------------------------------------------------------------------
/** Returns the hit effect object to use when this objects hits something.
 *  \returns The hit effect object, or NULL if no hit effect should be played.
 */
HitEffect* Flyable::getHitEffect() const
{
    return new Explosion(getXYZ(), "explosion");
}   // getHitEffect

/* EOF */
