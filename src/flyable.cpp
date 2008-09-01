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
#include "world.hpp"
#include "kart.hpp"
#include "projectile_manager.hpp"
#include "callback_manager.hpp"
#include "sound_manager.hpp"
#include "scene.hpp"
#include "ssg_help.hpp"

// static variables:
float      Flyable::m_st_speed[COLLECT_MAX];
ssgEntity* Flyable::m_st_model[COLLECT_MAX];
float      Flyable::m_st_min_height[COLLECT_MAX];
float      Flyable::m_st_max_height[COLLECT_MAX];
float      Flyable::m_st_force_updown[COLLECT_MAX];
btVector3  Flyable::m_st_extend[COLLECT_MAX];
// ----------------------------------------------------------------------------

Flyable::Flyable(Kart *kart, CollectableType type, float mass) : Moveable(false)
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
    m_last_radar_beep   = -1;
    m_exploded          = false;
    m_shape             = NULL;
    m_mass              = mass;

    // Add the graphical model
    ssgTransform *m     = getModelTransform();
    m->addKid(m_st_model[type]);
    scene->add(m);
}   // Flyable
// ----------------------------------------------------------------------------
void Flyable::createPhysics(float y_offset, const btVector3 velocity,
                            btCollisionShape *shape, const bool gravity,
                            const bool rotates, const btTransform* customDirection)
{
    // Get Kart heading direction
    btTransform trans = ( customDirection == NULL ? m_owner->getKartHeading() : *customDirection );

    // Apply offset
    btTransform offset_transform;
    offset_transform.setIdentity();
    btVector3 offset=btVector3(0,y_offset,m_average_height);
    offset_transform.setOrigin(offset);
        
    trans  *= offset_transform;

    m_shape = shape;
    createBody(m_mass, trans, m_shape);
    m_user_pointer.set(this);
    world->getPhysics()->addBody(getBody());

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
                   CollectableType type)
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
    float x_min, x_max, y_min, y_max, z_min, z_max;
    MinMax(model, &x_min, &x_max, &y_min, &y_max, &z_min, &z_max);
    m_st_extend[type] = btVector3(x_max-x_min,y_max-y_min, z_max-z_min);
    m_st_model[type]  = model;
}   // init

//-----------------------------------------------------------------------------
Flyable::~Flyable()
{
    if(m_shape) delete m_shape;
    world->getPhysics()->removeBody(getBody());
}   // ~Flyable

//-----------------------------------------------------------------------------
void Flyable::getClosestKart(Kart **minKart, float *minDistSquared,
                             btVector3 *minDelta, const Kart* inFrontOf) const
{
    btTransform tProjectile=getTrans();
    
    *minDistSquared = -1.0f;
    *minKart = NULL;
    
    for(unsigned int i=0 ; i<race_manager->getNumKarts(); i++ )
    {
        Kart *kart = world -> getKart(i);
        if(kart->isEliminated() || kart == m_owner || (!kart->isOnGround()) ) continue;
        btTransform t=kart->getTrans();
       
        btVector3 delta = t.getOrigin()-tProjectile.getOrigin();
        float distance2 = delta.length2();
        
        if(inFrontOf != NULL)
        {
            // Ignore karts behind the current one
            float distance =  kart->getDistanceDownTrack() - inFrontOf->getDistanceDownTrack();
            if(distance<0) distance += world->m_track->getTrackLength();
            
            //std::cout << "distance for " << kart->getName().c_str() << " : " << distance << std::endl;
            
            if(distance > 50){ std::cout << kart->getName().c_str() << " is behind" << std::endl; continue; } 
            
            /*
            // get the angle between the current kart and the target kart.
            // ignore karts that are not within an angle range
            

            btMatrix3x3 thisKartDirMatrix = kart->getKartHeading().getBasis();
            btVector3 thisKartDirVector(thisKartDirMatrix[0][1],
                                        thisKartDirMatrix[1][1],
                                        0);
            
            btVector3 targetLoc = inFrontOf->getTrans().getOrigin();
            btVector3 toClosestKart(targetLoc.getX() - kart->getTrans().getOrigin().getX(),
                                    targetLoc.getY() - kart->getTrans().getOrigin().getY(),
                                    0);
            float angle = toClosestKart.angle(thisKartDirVector);
            std::cout << angle << " (angle)" << std::endl;
            //if( angle>1.4f || angle<-1.4f )
            //    continue;
            */
            /*
            float angle = atan2(-(kart->getTrans().getOrigin().getX() - inFrontOf->getTrans().getOrigin().getX()),
                                  kart->getTrans().getOrigin().getY() - inFrontOf->getTrans().getOrigin().getY() );
            
            if( angle>1.4f || angle<-1.4f )
                continue;
             */
        }
        
        if(distance2 < *minDistSquared || *minDistSquared < 0 /* not yet set */)
        {
            std::cout << "keeping " << kart->getName().c_str() << " for now" << std::endl;
            *minDistSquared = distance2;
            *minKart  = kart;
            *minDelta = delta;
        }
    }  // for i<getNumKarts
    
}   // getClosestKart

//-----------------------------------------------------------------------------
void Flyable::update (float dt)
{
    if(m_exploded) return;
	
    Vec3 pos=getBody()->getWorldTransform().getOrigin();
    TerrainInfo::update(pos);
    if(getHoT()==Track::NOHIT) 
    {
        explode(NULL);    // flyable out of track boundary
        return;
    }

    float hat = pos.getZ()-getHoT();

    // Use the Height Above Terrain to set the Z velocity.
    // HAT is clamped by min/max height. This might be somewhat
    // unphysical, but feels right in the game.
    hat = std::max(std::min(hat, m_max_height) , m_min_height);
    float delta = m_average_height - hat;
    btVector3 v=getVelocity();
    v.setZ(m_force_updown*delta);
    setVelocity(v);

    Moveable::update(dt);
}   // update

// -----------------------------------------------------------------------------
void Flyable::explode(Kart *kart_hit, MovingPhysics* moving_physics)
{
	if(m_exploded) return;

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
    world->getPhysics()->removeBody(getBody());
	m_exploded=true;

    for ( unsigned int i = 0 ; i < race_manager->getNumKarts() ; i++ )
    {
        Kart *kart = world->getKart(i);
        // Handle the actual explosion. The kart that fired a flyable will 
        // only be affected if it's a direct hit. This allows karts to use
        // rockets on short distance.
        if(m_owner!=kart || m_owner==kart_hit) 
        {
            // Set a flag it if was a direct hit.
            kart->handleExplosion(getXYZ(), kart==kart_hit);
        }
    }
    callback_manager->handleExplosion(pos_explosion, moving_physics);
}   // explode

/* EOF */
