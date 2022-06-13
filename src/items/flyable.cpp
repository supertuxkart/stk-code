//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007-2015 Joerg Henrichs
//
//  Linear item-kart intersection function written by
//  Copyright (C) 2009-2015 David Mikos
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

#include <cmath>

#include <IMeshManipulator.h>
#include <IMeshSceneNode.h>

#include "audio/sfx_base.hpp"
#include "achievements/achievements_status.hpp"
#include "config/player_manager.hpp"
#include "graphics/explosion.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material.hpp"
#include "graphics/mesh_tools.hpp"
#include "guiengine/engine.hpp"
#include "io/xml_node.hpp"
#include "items/projectile_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/cannon_animation.hpp"
#include "karts/controller/controller.hpp"
#include "karts/explosion_animation.hpp"
#include "modes/linear_world.hpp"
#include "network/compress_network_body.hpp"
#include "network/network_config.hpp"
#include "network/network_string.hpp"
#include "network/rewind_manager.hpp"
#include "physics/physics.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"
#include "utils/string_utils.hpp"
#include "utils/vs.hpp"

#include <typeinfo>

// static variables:
float         Flyable::m_st_speed       [PowerupManager::POWERUP_MAX];
scene::IMesh* Flyable::m_st_model       [PowerupManager::POWERUP_MAX];
float         Flyable::m_st_min_height  [PowerupManager::POWERUP_MAX];
float         Flyable::m_st_max_height  [PowerupManager::POWERUP_MAX];
float         Flyable::m_st_force_updown[PowerupManager::POWERUP_MAX];
Vec3          Flyable::m_st_extend      [PowerupManager::POWERUP_MAX];
// ----------------------------------------------------------------------------

Flyable::Flyable(AbstractKart *kart, PowerupManager::PowerupType type,
                 float mass)
       : Moveable(), TerrainInfo(), m_mass(mass)
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
    m_shape                        = NULL;
    m_animation                    = NULL;
    m_adjust_up_velocity           = true;
    m_ticks_since_thrown           = 0;
    m_position_offset              = Vec3(0,0,0);
    m_owner_has_temporary_immunity = true;
    m_do_terrain_info              = true;
    m_deleted_once                 = false;
    m_max_lifespan                 = -1;
    m_compressed_gravity_vector    = 0;
    // It will be reset for each state restore
    m_has_server_state = true;
    m_last_deleted_ticks = -1;

    // Add the graphical model
#ifndef SERVER_ONLY
    if (!GUIEngine::isNoGraphics())
    {
        setNode(irr_driver->addMesh(m_st_model[type],
            StringUtils::insertValues("flyable_%i", (int)type)));
#ifdef DEBUG
        std::string debug_name("flyable: ");
        debug_name += type;
        getNode()->setName(debug_name.c_str());
#endif
    }
#endif
    // Smooth network body for flyable doesn't seem to be needed, most of the
    // time it rewinds almost the same
    SmoothNetworkBody::setEnable(false);
    m_created_ticks = World::getWorld()->getTicksSinceStart();
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
                            btCollisionShape *shape,
                            float restitution, const btVector3& gravity,
                            const bool rotates, const bool turn_around,
                            const btTransform* custom_direction)
{
    // Remove previously physics data if any
    removePhysics();
    // Get Kart heading direction
    btTransform trans = ( !custom_direction ? m_owner->getAlignedTransform()
                                            : *custom_direction          );

    // Apply offset
    btTransform offset_transform;
    offset_transform.setIdentity();
    assert(!std::isnan(m_average_height));
    assert(!std::isnan(forw_offset));
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
    createBody(m_mass, trans, m_shape, restitution);
    m_user_pointer.set(this);
    Physics::get()->addBody(getBody());

    m_body->setGravity(gravity);
    if (gravity.length2() != 0.0f && m_do_terrain_info)
    {
        m_compressed_gravity_vector = MiniGLM::compressVector3(
            Vec3(m_body->getGravity().normalized()).toIrrVector());
    }

    // Rotate velocity to point in the right direction
    btVector3 v=trans.getBasis()*velocity;

    if(m_mass!=0.0f)  // Don't set velocity for kinematic or static objects
    {
#ifdef DEBUG
        // Just to get some additional information if the assert is triggered
        if(std::isnan(v.getX()) || std::isnan(v.getY()) || std::isnan(v.getZ()))
        {
            Log::debug("[Flyable]", "vel %f %f %f v %f %f %f",
                        velocity.getX(),velocity.getY(),velocity.getZ(),
                        v.getX(),v.getY(),v.getZ());
        }
#endif
        assert(!std::isnan(v.getX()));
        assert(!std::isnan(v.getY()));
        assert(!std::isnan(v.getZ()));
        m_body->setLinearVelocity(v);
        if(!rotates) m_body->setAngularFactor(0.0f);   // prevent rotations
    }
}   // createPhysics

// -----------------------------------------------------------------------------
/** Initialises the static members of this class for a certain type with
 *  default values and with settings from powerup.xml.
 *  \param The xml node containing settings.
 *  \param model The mesh to use.
 *  \param type The type of flyable.
 */
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
    removePhysics();
    if (m_animation)
    {
        m_animation->handleResetRace();
        delete m_animation;
    }
}   // ~Flyable

//-----------------------------------------------------------------------------
/* Called when delete this flyable or re-firing during rewind. */
void Flyable::removePhysics()
{
    if (m_shape)
    {
        delete m_shape;
        m_shape = NULL;
    }
    if (m_body.get())
    {
        Physics::get()->removeBody(m_body.get());
        m_body.reset();
    }
}   // removePhysics

//-----------------------------------------------------------------------------
/** Returns information on what is the closest kart and at what distance it is.
 *  All 3 parameters first are of type 'out'. 'inFrontOf' can be set if you
 *  wish to know the closest kart in front of some karts (will ignore those
 *  behind). Useful e.g. for throwing projectiles in front only.
 */

void Flyable::getClosestKart(const AbstractKart **minKart,
                             float *minDistSquared, Vec3 *minDelta,
                             const AbstractKart* inFrontOf,
                             const bool backwards) const
{
    btTransform trans_projectile = (inFrontOf != NULL ? inFrontOf->getTrans()
                                                      : getTrans());

    *minDistSquared = 999999.9f;
    *minKart = NULL;

    World *world = World::getWorld();
    for(unsigned int i=0 ; i<world->getNumKarts(); i++ )
    {
        AbstractKart *kart = world->getKart(i);
        // If a kart has star effect shown, the kart is immune, so
        // it is not considered a target anymore.
        if(kart->isEliminated() || kart == m_owner ||
            kart->isInvulnerable()                 ||
            kart->getKartAnimation()                   ) continue;

        // Don't hit teammates in team world
        if (world->hasTeam() &&
            world->getKartTeam(kart->getWorldKartId()) ==
            world->getKartTeam(m_owner->getWorldKartId()))
            continue;

        btTransform t=kart->getTrans();

        Vec3 delta      = t.getOrigin()-trans_projectile.getOrigin();
        // the Y distance is added again because karts above or below should//
        // not be prioritized when aiming
        float distance2 = delta.length2() + std::abs(t.getOrigin().getY()
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
                                             const AbstractKart *target_kart,
                                             float item_XZ_speed,
                                             float gravity, float forw_offset,
                                             float *fire_angle,
                                             float *up_velocity)
{
    // Transform the target into the firing kart's frame of reference
    btTransform inv_trans = m_owner->getTrans().inverse();
    
    Vec3 relative_target_kart_loc = inv_trans(target_kart->getXYZ());
    
    // Find the direction target is moving in
    btTransform trans = target_kart->getTrans();
    Vec3 target_direction(trans.getBasis().getColumn(2));

    // Now rotate it to the firing kart's frame of reference
    btQuaternion inv_rotate = inv_trans.getRotation();
    target_direction = 
        target_direction.rotate(inv_rotate.getAxis(), inv_rotate.getAngle());
    
    // Now we try to find the angle to aim at to hit the target. 
    // Warning : Funky math stuff going on below. To understand, see answer by 
    // Jeffrey Hantin here : 
    // http://stackoverflow.com/questions/2248876/2d-game-fire-at-a-moving-target-by-predicting-intersection-of-projectile-and-u
    
    float target_x_speed = target_direction.getX()*target_kart->getSpeed();
    float target_z_speed = target_direction.getZ()*target_kart->getSpeed();
    float target_y_speed = target_direction.getY()*target_kart->getSpeed();

    float a = (target_x_speed*target_x_speed) + (target_z_speed*target_z_speed) -
                (item_XZ_speed*item_XZ_speed);
    float b = 2 * (target_x_speed * (relative_target_kart_loc.getX())
                    + target_z_speed * (relative_target_kart_loc.getZ()));
    float c = relative_target_kart_loc.getX()*relative_target_kart_loc.getX()
                + relative_target_kart_loc.getZ()*relative_target_kart_loc.getZ();
    
    float discriminant = b*b - 4 * a*c;
    if (discriminant < 0) discriminant = 0;

    float t1 = (-b + sqrt(discriminant)) / (2 * a);
    float t2 = (-b - sqrt(discriminant)) / (2 * a);
    float time;
    if (t1 >= 0 && t1<t2) time = t1;
    else time = t2;

    //createPhysics offset
    time -= forw_offset / item_XZ_speed;

    float aimX = time*target_x_speed + relative_target_kart_loc.getX();
    float aimZ = time*target_z_speed + relative_target_kart_loc.getZ();

    assert(time!=0);
    float angle = atan2f(aimX, aimZ);
    
    *fire_angle = angle;

    // Now find the up_velocity. This is an application of newton's equation.
    *up_velocity = (0.5f * time * gravity) + (relative_target_kart_loc.getY() / time)
                 + ( target_y_speed);
}   // getLinearKartItemIntersection

//-----------------------------------------------------------------------------

void Flyable::setAnimation(AbstractKartAnimation *animation)
{
    if (animation)
    {
        assert(m_animation == NULL);
        // add or removeBody currently breaks animation rewind
        moveToInfinity(/*set_moveable_trans*/false);
    }
    else   // animation = NULL
    {
        assert(m_animation != NULL);
        m_body->setWorldTransform(getTrans());
    }
    m_animation = animation;
}   // addAnimation

//-----------------------------------------------------------------------------
/** Called once per rendered frame. It is used to only update any graphical
 *  effects.
 *  \param dt Time step size (since last call).
 */
void Flyable::updateGraphics(float dt)
{
    Moveable::updateSmoothedGraphics(dt);
    Moveable::updateGraphics();
}   // updateGraphics

//-----------------------------------------------------------------------------
/** Updates this flyable. It calls Moveable::update. If this function returns
 *  true, the flyable will be deleted by the projectile manager.
 *  \param dt Time step size.
 *  \returns True if this object can be deleted.
 */
bool Flyable::updateAndDelete(int ticks)
{
    if (!m_has_server_state)
        return false;

    if (hasAnimation())
    {
        m_animation->update(ticks);
        Moveable::update(ticks);
        // Move the physical body to infinity so it doesn't interact with
        // game objects (for easier rewind)
        moveToInfinity(/*set_moveable_trans*/false);
        return false;
    }   // if animation

    // 32767 for max m_ticks_since_thrown so the last bit for animation save
    if (m_ticks_since_thrown < 32767)
        m_ticks_since_thrown += ticks;
    if(m_max_lifespan > -1 && (int)m_ticks_since_thrown > m_max_lifespan)
        hit(NULL);

    if(m_has_hit_something) return true;

    // Round values in network for better synchronization
    if (NetworkConfig::get()->roundValuesNow())
        CompressNetworkBody::compress(m_body.get(), m_motion_state.get());
    // Save the compressed values if done in client
    Moveable::update(ticks);

    //Vec3 xyz=getBody()->getWorldTransform().getOrigin();
    const Vec3 &xyz=getXYZ();
    // Check if the flyable is outside of the track. If so, explode it.
    const Vec3 *min, *max;
    Track::getCurrentTrack()->getAABB(&min, &max);

    // I have seen that the bullet AABB can be slightly different from the
    // one computed here - I assume due to minor floating point errors
    // (e.g. 308.25842 instead of 308.25845). To avoid a crash with a bullet
    // assertion (see bug 3058932) I add an epsilon here - but admittedly
    // that does not really explain the bullet crash, since bullet tests
    // against its own AABB, and should therefore not cause the assertion.
    // But since we couldn't reproduce the problem, and the epsilon used
    // here does not hurt, I'll leave it in.
    float eps = 0.1f;
    assert(!std::isnan(xyz.getX()));
    assert(!std::isnan(xyz.getY()));
    assert(!std::isnan(xyz.getZ()));
    if(xyz[0]<(*min)[0]+eps || xyz[2]<(*min)[2]+eps || xyz[1]<(*min)[1]+eps ||
       xyz[0]>(*max)[0]-eps || xyz[2]>(*max)[2]-eps || xyz[1]>(*max)[1]-eps   )
    {
        hit(NULL);    // flyable out of track boundary
        return true;
    }

    if (m_do_terrain_info)
    {
        Vec3 towards = MiniGLM::decompressVector3(m_compressed_gravity_vector);
        // Add the position offset so that the flyable can adjust its position
        // (usually to do the raycast from a slightly higher position to avoid
        // problems finding the terrain in steep uphill sections).
        // Towards is a unit vector. so we can multiply -towards to offset the
        // position by one unit.
        TerrainInfo::update(xyz + m_position_offset*(-towards), towards);

        // Make flyable anti-gravity when the it's projected on such surface
        const Material* m = TerrainInfo::getMaterial();
        if (m && m->hasGravity())
        {
            getBody()->setGravity(TerrainInfo::getNormal() * -70.0f);
        }
        else
        {
            getBody()->setGravity(Vec3(0, 1, 0) * -70.0f);
        }
        m_compressed_gravity_vector = MiniGLM::compressVector3(
            Vec3(m_body->getGravity().normalized()).toIrrVector());
    }

    if(m_adjust_up_velocity)
    {
        float hat = (xyz - getHitPoint()).length();

        // Use the Height Above Terrain to set the Z velocity.
        // HAT is clamped by min/max height. This might be somewhat
        // unphysical, but feels right in the game.

        float delta = m_average_height - std::max(std::min(hat, m_max_height),
                                                  m_min_height);
        Vec3 v = getVelocity();
        assert(!std::isnan(v.getX()));
        assert(!std::isnan(v.getX()));
        assert(!std::isnan(v.getX()));
        float heading = atan2f(v.getX(), v.getZ());
        assert(!std::isnan(heading));
        float pitch   = getTerrainPitch(heading);
        float vel_up = m_force_updown*(delta);
        if (hat < m_max_height) // take into account pitch of surface
            vel_up += v.length_2d()*tanf(pitch);
        assert(!std::isnan(vel_up));
        v.setY(vel_up);
        setVelocity(v);
    }   // if m_adjust_up_velocity
    return false;
}   // updateAndDelete

// ----------------------------------------------------------------------------
/** Returns true if the item hit the kart who shot it (to avoid that an item
 *  that's too close to the shooter hits the shooter).
 *  \param kart Kart who was hit.
 */
bool Flyable::isOwnerImmunity(const AbstractKart* kart_hit) const
{
    return m_owner_has_temporary_immunity &&
        kart_hit == m_owner            &&
        (int)m_ticks_since_thrown < stk_config->time2Ticks(2.0f);
}   // isOwnerImmunity

// ----------------------------------------------------------------------------
/** Callback from the physics in case that a kart or physical object is hit.
 *  \param kart The kart hit (NULL if no kart was hit).
 *  \param object The object that was hit (NULL if none).
 *  \return True if there was actually a hit (i.e. not owner, and target is
 *          not immune), false otherwise.
 */
bool Flyable::hit(AbstractKart *kart_hit, PhysicalObject* object)
{
    if (!m_has_server_state || hasAnimation())
        return false;
    // the owner of this flyable should not be hit by his own flyable
    if(isOwnerImmunity(kart_hit)) return false;
    m_has_hit_something=true;

    return true;

}   // hit

// ----------------------------------------------------------------------------
/** Creates the explosion physical effect, i.e. pushes the karts and ph
 *  appropriately. The corresponding visual/sfx needs to be added manually!
 *  \param kart_hit If non-NULL a kart that was directly hit.
 *  \param object If non-NULL a physical item that was hit directly.
 *  \param secondary_hits True if items that are not directly hit should
 *         also be affected.
 */
void Flyable::explode(AbstractKart *kart_hit, PhysicalObject *object,
                      bool secondary_hits)
{
    // Apply explosion effect
    // ----------------------
    World *world = World::getWorld();
    for ( unsigned int i = 0 ; i < world->getNumKarts() ; i++ )
    {
        AbstractKart *kart = world->getKart(i);
        // Don't explode teammates in team world
        if (world->hasTeam() &&
            world->getKartTeam(kart->getWorldKartId()) ==
            world->getKartTeam(m_owner->getWorldKartId()))
            continue;

        if (kart->isGhostKart()) continue;

        // If no secondary hits should be done, only hit the
        // direct hit kart.
        if(!secondary_hits && kart!=kart_hit)
            continue;

        // Handle the actual explosion. The kart that fired a flyable will
        // only be affected if it's a direct hit. This allows karts to use
        // rockets on short distance.
        if( (m_owner!=kart || m_owner==kart_hit) && !kart->getKartAnimation())
        {
            // The explosion animation will register itself with the kart
            // and will free it later.
            ExplosionAnimation::create(kart, getXYZ(), kart==kart_hit);
            if (kart == kart_hit)
            {
                world->kartHit(kart->getWorldKartId(),
                    m_owner->getWorldKartId());

                if (m_owner->getController()->canGetAchievements())
                {
                    if (m_owner->getWorldKartId() != kart->getWorldKartId())
                        PlayerManager::addKartHit(kart->getWorldKartId());
                    PlayerManager::increaseAchievement(AchievementsStatus::ALL_HITS, 1);
                    if (RaceManager::get()->isLinearRaceMode())
                        PlayerManager::increaseAchievement(AchievementsStatus::ALL_HITS_1RACE, 1);
                }

                // Rumble!
                Controller* controller = kart->getController();
                if (controller && controller->isLocalPlayerController())
                {
                    controller->rumble(0, 0.8f, 500);
                }
            }
        }
    }
    Track::getCurrentTrack()->handleExplosion(getXYZ(), object,secondary_hits);
}   // explode

// ----------------------------------------------------------------------------
/** Returns the hit effect object to use when this objects hits something.
 *  \returns The hit effect object, or NULL if no hit effect should be played.
 */
HitEffect* Flyable::getHitEffect() const
{
    if (GUIEngine::isNoGraphics())
        return NULL;
    return m_deleted_once ? NULL :
        new Explosion(getXYZ(), "explosion", "explosion_cake.xml");
}   // getHitEffect

// ----------------------------------------------------------------------------
unsigned int Flyable::getOwnerId()
{
    return m_owner->getWorldKartId();
}   // getOwnerId

// ----------------------------------------------------------------------------
/** It's called when undoing the creation or destruction of flyables, so that
 *  it will not affected the current game, and it will be deleted in
 *  computeError.
 *  \param set_moveable_trans If true, set m_transform in moveable, so the
 *  graphical node will have the same transform, otherwise only the physical
 *  body will be moved to infinity
 */
void Flyable::moveToInfinity(bool set_moveable_trans)
{
    const Vec3 *min, *max;
    Track::getCurrentTrack()->getAABB(&min, &max);
    btTransform t = m_body->getWorldTransform();
    t.setOrigin(*max * 2.0f);
    m_body->proceedToTransform(t);
    if (set_moveable_trans)
        setTrans(t);
    else
        m_motion_state->setWorldTransform(t);
}   // moveToInfinity

// ----------------------------------------------------------------------------
BareNetworkString* Flyable::saveState(std::vector<std::string>* ru)
{
    if (m_has_hit_something)
        return NULL;

    ru->push_back(getUniqueIdentity());

    BareNetworkString* buffer = new BareNetworkString();
    uint16_t ticks_since_thrown_animation = (m_ticks_since_thrown & 32767) |
        (hasAnimation() ? 32768 : 0);
    buffer->addUInt16(ticks_since_thrown_animation);
    if (m_do_terrain_info)
        buffer->addUInt32(m_compressed_gravity_vector);

    if (hasAnimation())
        m_animation->saveState(buffer);
    else
    {
        CompressNetworkBody::compress(
            m_body.get(), m_motion_state.get(), buffer);
    }
    return buffer;
}   // saveState

// ----------------------------------------------------------------------------
void Flyable::restoreState(BareNetworkString *buffer, int count)
{
    uint16_t ticks_since_thrown_animation = buffer->getUInt16();
    bool has_animation_in_state =
        (ticks_since_thrown_animation >> 15 & 1) == 1;
    if (m_do_terrain_info)
        m_compressed_gravity_vector = buffer->getUInt32();

    if (has_animation_in_state)
    {
        // At the moment we only have cannon animation for rubber ball
        if (!m_animation)
        {
            try
            {
                CannonAnimation* ca = new CannonAnimation(this, buffer);
                setAnimation(ca);
            }
            catch (const KartAnimationCreationException& kace)
            {
                Log::error("Flyable", "Kart animation creation error: %s",
                    kace.what());
                buffer->skip(kace.getSkippingOffset());
            }
        }
        else
            m_animation->restoreState(buffer);
    }
    else
    {
        if (hasAnimation())
        {
            // Delete unconfirmed animation, destructor of cannon animation
            // will set m_animation to null
            delete m_animation;
        }
        CompressNetworkBody::decompress(
            buffer, m_body.get(), m_motion_state.get());
        m_transform = m_body->getWorldTransform();
    }
    m_ticks_since_thrown = ticks_since_thrown_animation & 32767;
    m_has_server_state = true;
    m_has_hit_something = false;
}   // restoreState

// ----------------------------------------------------------------------------
void Flyable::addForRewind(const std::string& uid)
{
    Rewinder::setUniqueIdentity(uid);
    Rewinder::rewinderAdd();
}   // addForRewind

// ----------------------------------------------------------------------------
void Flyable::saveTransform()
{
    // It will be overwritten in restoreState (so it's confirmed by server) or
    // onFireFlyable (before the game state all flyables are assumed to be
    // sucessfully created)
    moveToInfinity();
    m_has_server_state = false;
    m_last_deleted_ticks = -1;
}   // saveTransform

// ----------------------------------------------------------------------------
void Flyable::computeError()
{
    // Remove the flyable if it doesn't exist or failed to create on server
    // For each saveTransform it will call moveToInfinity so the invalid
    // flyables won't affect the current game
    const int state_ticks = RewindManager::get()->getLatestConfirmedState();
    if (!m_has_server_state && (m_last_deleted_ticks == -1 ||
        state_ticks > m_last_deleted_ticks))
    {
        const std::string& uid = getUniqueIdentity();
        Log::debug("Flyable", "Flyable %s by %s created at %d "
            "doesn't exist on server, remove it.",
            typeid(*this).name(), StringUtils::wideToUtf8(
            m_owner->getController()->getName()).c_str(), m_created_ticks);
        ProjectileManager::get()->removeByUID(uid);
    }
}   // computeError

// ----------------------------------------------------------------------------
/** Call when the item is (re-)fired (during rewind if needed) by
 *  projectile_manager. */
void Flyable::onFireFlyable()
{
    if (m_animation)
    {
        m_animation->handleResetRace();
        delete m_animation;
        m_animation = NULL;
    }

    m_ticks_since_thrown = 0;
    m_has_hit_something = false;
    m_has_server_state = true;
    m_deleted_once = false;
    m_last_deleted_ticks = -1;
    // Reset the speed each time for (re-)firing, so subclass access with same
    // initial value
    m_speed = m_st_speed[m_type];
    m_extend = m_st_extend[m_type];
    m_max_height = m_st_max_height[m_type];
    m_min_height = m_st_min_height[m_type];
    m_average_height = (m_min_height + m_max_height) / 2.0f;
    m_force_updown = m_st_force_updown[m_type];
}   // onFireFlyable

// ----------------------------------------------------------------------------
/* Call when deleting the flyable locally and save the deleted world ticks. */
void Flyable::onDeleteFlyable()
{
    m_deleted_once = true;
    m_last_deleted_ticks = World::getWorld()->getTicksSinceStart();
    m_has_server_state = false;
    moveToInfinity();
}   // onDeleteFlyable

// ----------------------------------------------------------------------------
/* Make specifc sfx lower volume if needed in splitscreen multiplayer. */
void Flyable::fixSFXSplitscreen(SFXBase* sfx)
{
    if (sfx && RaceManager::get()->getNumLocalPlayers() > 1)
        sfx->setVolume(1.0f / (float)RaceManager::get()->getNumLocalPlayers());
}   // fixSFXSplitscreen

/* EOF */
