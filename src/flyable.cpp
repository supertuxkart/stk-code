//  $Id: flyable.cpp 1284 2007-11-08 12:31:54Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007 Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

Flyable::Flyable(Kart *kart, CollectableType type) : Moveable(false)
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
    m_mass              = 1.0f;

    // Add the graphical model
    ssgTransform *m     = getModelTransform();
    m->addKid(m_st_model[type]);
    scene->add(m);
}   // Flyable
// ----------------------------------------------------------------------------
void Flyable::createPhysics(float y_offset, const btVector3 velocity,
                            btCollisionShape *shape)
{
    // The actual transform is determined as follows:
    // 1) Compute the heading of the kart
    // 2) Compute the pitch of the terrain. This avoids the problem of the
    //    rocket hitting the floor (e.g. if the kart is braking and therefore
    //    pointing downwards).
    btTransform trans = m_owner->getTrans();

    // get heading=trans.getBasis*(0,1,0) ... so save the multiplication:
    btVector3 direction(trans.getBasis()[0][1],
                        trans.getBasis()[1][1],
                        trans.getBasis()[2][1]);
    float heading=atan2(-direction.getX(), direction.getY());

    TerrainInfo::update(m_owner->getPos());
    float pitch = getTerrainPitch(heading);

    btMatrix3x3 m;
    m.setEulerZYX(pitch, 0.0f, heading);
    trans.setBasis(m);

    // Apply rotation and offset
    btTransform offset_transform;
    offset_transform.setIdentity();
    btVector3 offset=btVector3(0,y_offset,m_average_height);
    offset_transform.setOrigin(offset);
        
    trans  *= offset_transform;

    m_shape = shape;
    createBody(m_mass, trans, m_shape);
    m_user_pointer.set(this);
    world->getPhysics()->addBody(getBody());

    // Simplified rockets: no gravity
    m_body->setGravity(btVector3(0.0f, 0.0f, 0.0f));

    // Rotate velocity to point in the right direction
    btVector3 v=trans.getBasis()*velocity;

    if(m_mass!=0.0f)  // Don't set velocity for kinematic or static objects
    {
        m_body->setLinearVelocity(v);
        m_body->setAngularFactor(0.0f);   // prevent rotations
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
void Flyable::getClosestKart(const Kart **minKart, float *minDist, btVector3 *minDelta) const
{
    btTransform tProjectile=getTrans();
    *minDist = 99999.9f;
    for(unsigned int i=0 ; i<race_manager->getNumKarts(); i++ )
    {
        Kart *kart = world -> getKart(i);
        if(kart->isEliminated() || kart == m_owner) continue;
        btTransform t=kart->getTrans();
       
        btVector3 delta = t.getOrigin()-tProjectile.getOrigin();
        float distance2 = delta.length2();

        if(distance2 < *minDist)
        {
            *minDist  = sqrt(distance2);
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
void Flyable::placeModel()
{
	btTransform t=getTrans();
    float m[4][4];
    t.getOpenGLMatrix((float*)&m);
    sgSetCoord(&m_curr_pos, m);
    Moveable::placeModel();
}  // placeModel

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
    btVector3 pos_explosion=getPos();
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
            kart->handleExplosion(getPos(), kart==kart_hit);
        }
    }
    callback_manager->handleExplosion(pos_explosion, moving_physics);
}   // explode

/* EOF */
