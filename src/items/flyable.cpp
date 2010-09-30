//  $Id: flyable.cpp 1284 2007-11-08 12:31:54Z hikerstk $
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

#include <math.h>

#include "graphics/irr_driver.hpp"
#include "graphics/mesh_tools.hpp"
#include "io/xml_node.hpp"
#include "items/projectile_manager.hpp"
#include "karts/kart.hpp"
#include "modes/world.hpp"
#include "network/flyable_info.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"
#include "utils/string_utils.hpp"


const wchar_t* getCakeString()
{
    const int CAKE_STRINGS_AMOUNT = 3;

    RandomGenerator r;
    const int id = r.get(CAKE_STRINGS_AMOUNT);

    switch (id)
    {
        //I18N: shown when hit by cake. %1 is the attacker, %0 is the victim.
        case 0: return _("%0 eats too much of %1's cake");
        //I18N: shown when hit by cake. %1 is the attacker, %0 is the victim.
        case 1: return _("%0 is dubious of %1's cooking skills");
        //I18N: shown when hit by cake. %1 is the attacker, %0 is the victim.
        case 2: return _("%0 should not play with %1's lunch");
        default: assert(false); return L"";   // avoid compiler warning
    }
}


const wchar_t* getBowlingString()
{
    const int BOWLING_STRINGS_AMOUNT = 3;

    RandomGenerator r;
    const int id = r.get(BOWLING_STRINGS_AMOUNT);

    switch (id)
    {
        //I18N: shown when hit by bowling ball. %1 is the attacker, %0 is the victim.
        case 0 : return _("%0 will not play bowling with %1 again");
        //I18N: shown when hit by bowling ball. %1 is the attacker, %0 is the victim.
        case 1 : return _("%1 strikes %0");
        //I18N: shown when hit by bowling ball. %1 is the attacker, %0 is the victim.
        case 2 : return _("%0 is bowled over by %1");
        default: assert(false); return L"";  //  avoid compiler warning
    }
}


const wchar_t* getSelfBowlingString()
{
    const int SELFBOWLING_STRINGS_AMOUNT = 3;

    RandomGenerator r;
    const int id = r.get(SELFBOWLING_STRINGS_AMOUNT);

    switch (id)
    {
        //I18N: shown when hit by own bowling ball. %s is the kart.
        case 0 : return _("%s is practicing with a blue, big, spheric yo-yo");
        //I18N: shown when hit by own bowling ball. %s is the kart.
        case 1 : return _("%s is the world master of the boomerang ball");
        //I18N: shown when hit by own bowling ball. %s is the kart.
        case 2 : return _("%s should play (rubber) darts instead of bowling");
        default: assert(false); return L"";  //  avoid compiler warning
    }
}


// static variables:
float         Flyable::m_st_speed       [PowerupManager::POWERUP_MAX];
scene::IMesh* Flyable::m_st_model       [PowerupManager::POWERUP_MAX];
float         Flyable::m_st_min_height  [PowerupManager::POWERUP_MAX];
float         Flyable::m_st_max_height  [PowerupManager::POWERUP_MAX];
float         Flyable::m_st_force_updown[PowerupManager::POWERUP_MAX];
Vec3          Flyable::m_st_extend      [PowerupManager::POWERUP_MAX];
// ----------------------------------------------------------------------------

Flyable::Flyable(Kart *kart, PowerupManager::PowerupType type, float mass) 
       : Moveable()
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
    btTransform trans = ( custom_direction == NULL ? m_owner->getKartHeading()
                                                   : *custom_direction );

    // Apply offset
    btTransform offset_transform;
    offset_transform.setIdentity();
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
        m_body->setLinearVelocity(v);
        if(!rotates) m_body->setAngularFactor(0.0f);   // prevent rotations
    }
    m_body->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);

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
    btTransform tProjectile = (inFrontOf != NULL ? inFrontOf->getTrans() 
                                                 : getTrans());

    *minDistSquared = 999999.9f;
    *minKart = NULL;

    World *world = World::getWorld();
    for(unsigned int i=0 ; i<world->getNumKarts(); i++ )
    {
        Kart *kart = world->getKart(i);
        if(kart->isEliminated() || kart == m_owner || 
            kart->playingEmergencyAnimation() ) continue;
        btTransform t=kart->getTrans();

        Vec3 delta      = t.getOrigin()-tProjectile.getOrigin();
        float distance2 = delta.length2();

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
    float target_kart_speed = target_direction.length_2d() * target_kart->getSpeed();

    float target_kart_heading = target_kart->getHeading();

    float dist = -(target_kart_speed / item_XZ_speed) * (dx * cosf(target_kart_heading) -
                                                         dz * sinf(target_kart_heading));

    float fire_th = (dx*dist - dz * sqrtf(dx*dx + dz*dz - dist*dist)) / (dx*dx + dz*dz);
    fire_th = (((dist - dx*fire_th) / dz > 0) ? -acosf(fire_th): acosf(fire_th));

    float time = 0.0f;
    float a = item_XZ_speed * sinf (fire_th) + target_kart_speed * sinf (target_kart_heading);
    float b = item_XZ_speed * cosf (fire_th) + target_kart_speed * cosf (target_kart_heading);

    if (fabsf(a) > fabsf(b)) time = fabsf (dx / a);
    else if (b != 0.0f)      time = fabsf(dz / b);

    if (fire_th > M_PI)
        fire_th -= M_PI;
    else
        fire_th += M_PI;

    //createPhysics offset
    time -= forw_offset / sqrt(a*a+b*b);

    *fire_angle = fire_th;
    *up_velocity = (0.5f * time * gravity) + (dy / time) + (gy * target_kart->getSpeed());
}   // getLinearKartItemIntersection

//-----------------------------------------------------------------------------
void Flyable::update(float dt)
{
    m_time_since_thrown += dt;
    if(m_max_lifespan > -1 && m_time_since_thrown > m_max_lifespan) hit(NULL);

    if(m_exploded) return;

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
    if(xyz[0]<(*min)[0]+eps || xyz[2]<(*min)[2]+eps || xyz[1]<(*min)[1]+eps ||
       xyz[0]>(*max)[0]-eps || xyz[2]>(*max)[2]-eps || xyz[1]>(*max)[1]-eps   )
    {
        hit(NULL);    // flyable out of track boundary
        return;
    }

    TerrainInfo::update(xyz);

    if(m_adjust_up_velocity)
    {
        float hat = xyz.getY()-getHoT();

        // Use the Height Above Terrain to set the Z velocity.
        // HAT is clamped by min/max height. This might be somewhat
        // unphysical, but feels right in the game.

        float delta = m_average_height - std::max(std::min(hat, m_max_height), m_min_height);
        Vec3 v = getVelocity();
        float heading = atan2f(v.getX(), v.getZ());
        float pitch   = getTerrainPitch(heading);
        float vel_up = m_force_updown*(delta);
        if (hat < m_max_height) // take into account pitch of surface
            vel_up += v.length_2d()*tanf(pitch);
        v.setY(vel_up);
        setVelocity(v);
    }   // if m_adjust_up_velocity

    Moveable::update(dt);
}   // update

// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
void Flyable::hit(Kart *kart_hit, PhysicalObject* object)
{
    // the owner of this flyable should not be hit by his own flyable
    if(m_exploded || isOwnerImmunity(kart_hit)) return;

    if (kart_hit != NULL)
    {
        RaceGUIBase* gui = World::getWorld()->getRaceGUI();
        irr::core::stringw hit_message;
        switch(m_type)
        {
            case PowerupManager::POWERUP_CAKE:
            {
                hit_message += StringUtils::insertValues(getCakeString(),
                                                         kart_hit->getName().c_str(),
                                                         m_owner->getName().c_str()
                                                        ).c_str();
            }
            break;
            case PowerupManager::POWERUP_PLUNGER: 
                // Handled by plunger.cpp Plunger::hit
            break;
            case PowerupManager::POWERUP_BOWLING:
            {
                if (kart_hit == m_owner)
                {
                    hit_message += StringUtils::insertValues(getSelfBowlingString(),
                                                             m_owner->getName().c_str()
                                                            ).c_str();
                }
                else
                {
                    hit_message += StringUtils::insertValues(getBowlingString(),
                                                             kart_hit->getName().c_str(),
                                                             m_owner->getName().c_str()
                                                            ).c_str();
                }
            }
            break;
            default:
                printf("Failed message for %i\n", m_type);
                assert(false);
        }
        gui->addMessage(hit_message, NULL, 3.0f, 40, video::SColor(255, 255, 255, 255), false);
    }

    m_has_hit_something=true;
    // Notify the projectile manager that this rocket has hit something.
    // The manager will create the appropriate explosion object.
    projectile_manager->notifyRemove();

    m_exploded=true;

    if(!needsExplosion()) return;

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

/* EOF */
