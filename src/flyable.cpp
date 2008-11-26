//  $Id: flyable.cpp 1284 2007-11-08 12:31:54Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007 Joerg Henrichs
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

#include <math.h>

#include "flyable.hpp"
#include "track.hpp"
#include "callback_manager.hpp"
#include "scene.hpp"
#include "race_manager.hpp"
#include "items/projectile_manager.hpp"
#include "karts/kart.hpp"
#include "modes/world.hpp"
#include "modes/linear_world.hpp"
#include "network/flyable_info.hpp"
#include "utils/ssg_help.hpp"

// static variables:
float      Flyable::m_st_speed[POWERUP_MAX];
ssgEntity* Flyable::m_st_model[POWERUP_MAX];
float      Flyable::m_st_min_height[POWERUP_MAX];
float      Flyable::m_st_max_height[POWERUP_MAX];
float      Flyable::m_st_force_updown[POWERUP_MAX];
btVector3  Flyable::m_st_extend[POWERUP_MAX];
// ----------------------------------------------------------------------------

Flyable::Flyable(Kart *kart, PowerupType type, float mass) : Moveable()
{
    // get the appropriate data from the static fields
    m_speed             = m_st_speed[type];
    m_extend            = m_st_extend[type];
    m_max_height        = m_st_max_height[type];
    m_min_height        = m_st_min_height[type];
    m_average_height    = (m_min_height+m_max_height)/2.0f;
    m_force_updown      = m_st_force_updown[type];

    m_owner             = kart;
    m_has_hit_something = false;
    m_exploded          = false;
    m_shape             = NULL;
    m_mass              = mass;
    m_adjust_z_velocity = true;

	m_time_since_thrown = 0;
	m_owner_has_temporary_immunity = true;
	m_max_lifespan = -1;
	
    // Add the graphical model
    ssgTransform *m     = getModelTransform();
    m->addKid(m_st_model[type]);
    scene->add(m);
}   // Flyable
// ----------------------------------------------------------------------------
void Flyable::createPhysics(float y_offset, const btVector3 velocity,
                            btCollisionShape *shape, const bool gravity,
                            const bool rotates, const bool turn_around, const btTransform* customDirection)
{
    // Get Kart heading direction
    btTransform trans = ( customDirection == NULL ? m_owner->getKartHeading() : *customDirection );

    // Apply offset
    btTransform offset_transform;
    offset_transform.setIdentity();
    btVector3 offset=btVector3(0,y_offset,m_average_height);
    offset_transform.setOrigin(offset);
        
    // turn around
    if(turn_around)
    {
        btTransform turn_around_trans;
        //turn_around_trans.setOrigin(trans.getOrigin());
        turn_around_trans.setIdentity();
        turn_around_trans.setRotation(btQuaternion(btVector3(0, 0, 1), M_PI));
        trans  *= turn_around_trans;
    }
    
    trans  *= offset_transform;

    m_shape = shape;
    createBody(m_mass, trans, m_shape);
    m_user_pointer.set(this);
    RaceManager::getWorld()->getPhysics()->addBody(getBody());

    if(gravity) m_body->setGravity(btVector3(0.0f, 0.0f, -9.8f));
    else m_body->setGravity(btVector3(0.0f, 0.0f, 0.0f));

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
void Flyable::init(const lisp::Lisp* lisp, ssgEntity *model, 
                   PowerupType type)
{
    m_st_speed[type]        = 25.0f;
    m_st_max_height[type]   = 1.0f;
    m_st_min_height[type]   = 3.0f;
    m_st_force_updown[type] = 15.0f;
    lisp->get("speed",           m_st_speed[type]       );
    lisp->get("min-height",      m_st_min_height[type]  );
    lisp->get("max-height",      m_st_max_height[type]  );
    lisp->get("force-updown",    m_st_force_updown[type]);

    // Store the size of the model
    Vec3 min, max;
    
    SSGHelp::MinMax(model, &min, &max);
    m_st_extend[type] = btVector3(max-min);
    m_st_model[type]  = model;
}   // init

//-----------------------------------------------------------------------------
Flyable::~Flyable()
{
    if(m_shape) delete m_shape;
    RaceManager::getWorld()->getPhysics()->removeBody(getBody());
}   // ~Flyable

//-----------------------------------------------------------------------------
void Flyable::getClosestKart(const Kart **minKart, float *minDistSquared,
                             btVector3 *minDelta, const Kart* inFrontOf) const
{
    btTransform tProjectile = (inFrontOf != NULL ? inFrontOf->getTrans() : getTrans());
    
    *minDistSquared = -1.0f;
    *minKart = NULL;
    
    for(unsigned int i=0 ; i<race_manager->getNumKarts(); i++ )
    {
        Kart *kart = RaceManager::getKart(i);
        if(kart->isEliminated() || kart == m_owner || (!kart->isOnGround()) ) continue;
        btTransform t=kart->getTrans();
       
        btVector3 delta = t.getOrigin()-tProjectile.getOrigin();
        float distance2 = delta.length2();
        
        if(inFrontOf != NULL)
        {
            // Ignore karts behind the current one
            btVector3 to_target = kart->getXYZ() - inFrontOf->getXYZ();
            const float distance = to_target.length();
            if(distance > 50) continue; // kart too far, don't aim at it
            
            btTransform trans = inFrontOf->getTrans();
            // get heading=trans.getBasis*(0,1,0) ... so save the multiplication:
            btVector3 direction(trans.getBasis()[0][1],
                                trans.getBasis()[1][1],
                                trans.getBasis()[2][1]);
            
            float angle = to_target.angle(direction);
            
            if(fabsf(angle) > 1) continue;
        }
        
        if(distance2 < *minDistSquared || *minDistSquared < 0 /* not yet set */)
        {
            *minDistSquared = distance2;
            *minKart  = kart;
            *minDelta = delta;
        }
    }  // for i<getNumKarts
    
}   // getClosestKart

//-----------------------------------------------------------------------------
void Flyable::update (float dt)
{
	m_time_since_thrown += dt;
	if(m_max_lifespan > -1 && m_time_since_thrown > m_max_lifespan) explode(NULL);
	
    if(m_exploded) return;
	
    Vec3 pos=getBody()->getWorldTransform().getOrigin();
    TerrainInfo::update(pos);
    if(getHoT()==Track::NOHIT) 
    {
        explode(NULL);    // flyable out of track boundary
        return;
    }
    if(m_adjust_z_velocity)
    {
        float hat = pos.getZ()-getHoT();

        // Use the Height Above Terrain to set the Z velocity.
        // HAT is clamped by min/max height. This might be somewhat
        // unphysical, but feels right in the game.
        hat = std::max(std::min(hat, m_max_height) , m_min_height);
        float delta = m_average_height - hat;
        btVector3 v=getVelocity();
        v.setZ(m_force_updown*delta);
        setVelocity(v);
    }   // if m_adjust_z_velocity

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
void Flyable::explode(Kart *kart_hit, MovingPhysics* moving_physics)
{
	if(m_exploded) return;

	// the owner of this flyable should not be hit by his own flyable
	if(m_owner_has_temporary_immunity && kart_hit == m_owner && m_time_since_thrown < 2.0f) return;
	
    m_has_hit_something=true;
    // Notify the projectile manager that this rocket has hit something.
    // The manager will create the appropriate explosion object.
    projectile_manager->explode();

    // Now remove this projectile from the graph:
    ssgTransform *m = getModelTransform();
    m->removeAllKids();
    scene->remove(m);

    // The explosion is a bit higher in the air
    Vec3 pos_explosion=getXYZ();
    pos_explosion.setZ(pos_explosion.getZ()+1.2f);
    RaceManager::getWorld()->getPhysics()->removeBody(getBody());
	m_exploded=true;

    for ( unsigned int i = 0 ; i < race_manager->getNumKarts() ; i++ )
    {
        Kart *kart = RaceManager::getKart(i);
        // Handle the actual explosion. The kart that fired a flyable will 
        // only be affected if it's a direct hit. This allows karts to use
        // rockets on short distance.
        if(m_owner!=kart || m_owner==kart_hit) 
        {
            // Set a flag it if was a direct hit.
            kart->handleExplosion(getXYZ(), kart==kart_hit);
            if(kart==kart_hit && RaceManager::getTrack()->isArena())
            {
                RaceManager::getWorld()->kartHit(kart->getWorldKartId());
            }
        }
    }
    callback_manager->handleExplosion(pos_explosion, moving_physics);
}   // explode

/* EOF */
