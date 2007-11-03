
//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006 Joerg Henrichs, Steve Baker
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

#include "constants.hpp"
#include "projectile.hpp"
#include "world.hpp"
#include "kart.hpp"
#include "projectile_manager.hpp"
#include "sound_manager.hpp"
#include "scene.hpp"
#include "ssg_help.hpp"

Projectile::Projectile(Kart *kart, int collectable) : Moveable(false)
{
    init(kart, collectable);
}   // Projectile

//-----------------------------------------------------------------------------
void Projectile::init(Kart *kart, int collectable_)
{
    m_owner              = kart;
    m_type               = collectable_;
    m_has_hit_something  = false;
    m_last_radar_beep    = -1;
    m_speed              = collectable_manager->getSpeed(m_type);
    ssgTransform *m      = getModel();
    m->addKid(collectable_manager->getModel(m_type));
    scene->add(m);

#ifdef BULLET
	m_exploded = false;
	float x_min, x_max, y_min, y_max, z_min, z_max;
	// getModel returns the transform node, so get the actual model node
	MinMax(getModel()->getKid(0), &x_min, &x_max, &y_min, &y_max, &z_min, &z_max);
	float half_length = 0.5f*(y_max-y_min);
    
    btCylinderShape* shape = new btCylinderShapeX(btVector3(half_length,
														    0.5f*(z_max-z_min),
															0.5f*(x_max-x_min)));

    // The rocket has to start ahead of the kart (not in), so it has to 
	// be moved forward by (at least) half a kart lengt .. we use one length here
	btTransform trans;
    kart->getTrans(&trans);

	// We should get heading from the kart, and pitch/roll from the terrain here
    btVector3 normal;
	if(world->getPhysics()->getTerrainNormal(trans.getOrigin(), &normal))
	{
		float m[4][4];
		trans.getOpenGLMatrix((float*)&m);
		sgCoord pos;
        sgSetCoord(&pos, m);
		btQuaternion q=trans.getRotation();
	    const float angle_terrain = DEGREE_TO_RAD(pos.hpr[0]);
	    // The projectile should have the pitch of the terrain on which the kart
        // is - while this is not really realistic, it plays much better this way
        const float X = -sin(angle_terrain);
        const float Y =  cos(angle_terrain);
        // Compute the angle between the normal of the plane and the line to
        // (x,y,0).  (x,y,0) is normalised, so are the coordinates of the plane,
        // simplifying the computation of the scalar product.
        float pitch = ( normal.getX()*X + normal.getY()*Y );  // use ( x,y,0)
        
        // The actual angle computed above is between the normal and the (x,y,0)
        // line, so to compute the actual angles 90 degrees must be subtracted.
        pos.hpr[1] = acosf(pitch)/M_PI*180.0f-90.0f;
		btQuaternion r=trans.getRotation();
		r.setEuler(DEGREE_TO_RAD(pos.hpr[1]),DEGREE_TO_RAD(pos.hpr[2]),DEGREE_TO_RAD(pos.hpr[0]));
		trans.setRotation(r);
	}   // if a normal exist

    m_initial_velocity = trans.getBasis()*btVector3(0.0f, m_speed, 0.0f);
	btTransform offset;
	offset.setOrigin(btVector3(0.0f, 2.0f*kart->getKartLength()+2.0f*half_length, 
							   kart->getKartHeight()));

	// The cylinder needs to be rotated by 90 degrees to face in the right direction:
	btQuaternion r90(0.0f, 0.0f, -NINETY_DEGREE_RAD);
	offset.setRotation(r90);
    
	// Apply rotation and offste
	trans *= offset;
	float mass=1.0f;

    createBody(mass, trans, shape, Moveable::MOV_PROJECTILE);
	// Simplified rockets: no gravity
	world->getPhysics()->addBody(getBody());
    m_body->setGravity(btVector3(0.0f, 0.0f, 0.0f));
	m_body->setLinearVelocity(m_initial_velocity);
	// FIXME: for now it is necessary to synch the graphical position with the 
	//        physical position, since 'hot' computation is done using the 
	//        graphical position (and hot can trigger an explosion when no
	//        terrain is under the rocket). Once hot is done with bullet as
	//        well, this shouldn't be necessary anymore.
    placeModel();

#else
    sgCoord c;
    sgCopyCoord(&c, kart->getCoord());
    const float angle_terrain = c.hpr[0]*M_PI/180.0f;
    // The projectile should have the pitch of the terrain on which the kart
    const sgVec4* normal      = kart->getNormalHOT();
    if(normal) 
    {
        const float X = -sin(angle_terrain);
        const float Y =  cos(angle_terrain);
        // Compute the angle between the normal of the plane and the line to
        // (x,y,0).  (x,y,0) is normalised, so are the coordinates of the plane,
        // simplifying the computation of the scalar product.
        float pitch = ( (*normal)[0]*X + (*normal)[1]*Y );  // use ( x,y,0)
        
        // The actual angle computed above is between the normal and the (x,y,0)
        // line, so to compute the actual angles 90 degrees must be subtracted.
        c.hpr[1] = acosf(pitch)/M_PI*180.0f-90.0f;
    }

    setCoord(&c);
#endif
}   // init

//-----------------------------------------------------------------------------
Projectile::~Projectile()
{}   // ~Projectile

//-----------------------------------------------------------------------------
void Projectile::update (float dt)
{
#ifdef BULLET
	if(m_exploded) return;
    m_body->setLinearVelocity(m_initial_velocity);	
#else
    // we don't even do any physics here - just set the
    // velocity, and ignore everything else for projectiles.
    m_velocity.xyz[1] = m_speed;
    sgCopyCoord ( &m_last_pos, &m_curr_pos);
#endif
    Moveable::update(dt);
    doObjectInteractions();
}   // update

// -----------------------------------------------------------------------------
#ifdef BULLET
void Projectile::placeModel()
{
	btTransform t;
	getTrans(&t);
	// FIXME: this 90 degrees rotation can be removed 
	//        if the 3d model is rotated instead
	btQuaternion r90(0.0f, 0.0f, NINETY_DEGREE_RAD);
	t.setRotation(t.getRotation()*r90);
    float m[4][4];
    t.getOpenGLMatrix((float*)&m);
    sgSetCoord(&m_curr_pos, m);
    const btVector3 &v=m_body->getLinearVelocity();
    sgSetVec3(m_velocity.xyz, v.x(), v.y(), v.z());
    m_model->setTransform(&m_curr_pos);
    
}  // placeModel
#endif
// -----------------------------------------------------------------------------
/** Returns true if this missile has hit something, otherwise false. */
void Projectile::doObjectInteractions ()
{
    float ndist = SG_MAX ;
    int nearest = -1 ;

    for ( unsigned int i = 0 ; i < world->getNumKarts() ; i++ )
    {
        sgCoord *pos ;

        Kart *kart = world -> getKart(i);
        pos        = kart  -> getCoord();

        if ( m_type != COLLECT_NOTHING && kart != m_owner )
        {
            const float D = sgDistanceSquaredVec3 ( pos->xyz, getCoord()->xyz ) ;
#ifndef BULLET
            if ( D < 2.0f )
            {
                explode(kart);
                return;
            }
            else 
#endif
            if ( D < ndist )
            {
                ndist = D ;
                nearest = i ;
            }  // if !D<2.0f
        }   // if m_type!=NOTHING &&kart!=m_owner
    }  // for i<getNumKarts
    if ( m_type == COLLECT_HOMING_MISSILE && nearest != -1 &&
         ndist < MAX_HOME_DIST_SQD                          )
    {
        sgVec3 delta;
        sgVec3 hpr;
        Kart * kart=world->getKart(nearest);
        if(m_last_radar_beep!=nearest && kart->isPlayerKart())
        {
            sound_manager->playSfx(SOUND_MISSILE_LOCK);
            m_last_radar_beep=nearest;
        }
        sgCoord *k = kart->getCoord() ;

        sgSubVec3 ( delta, k->xyz, m_curr_pos.xyz ) ;
#ifdef BULLET

        sgHPRfromVec3 ( hpr, delta ) ;

        m_curr_pos.hpr[1] = -hpr[1];

        sgSubVec3 ( hpr, m_curr_pos.hpr ) ;

        if ( hpr[0] >  180.0f ) hpr[0] -= 360.0f ;
        if ( hpr[0] < -180.0f ) hpr[0] += 360.0f ;

        if ( hpr[0] > 80.0f || hpr[0] < -80.0f )
            m_velocity.hpr[0] = 0.0f ;
        else
        {
            if      ( hpr[0] >  3.0f ) m_velocity.hpr[0] =  HOMING_MISSILE_TURN_RATE ;
            else if ( hpr[0] < -3.0f ) m_velocity.hpr[0] = -HOMING_MISSILE_TURN_RATE ;
            else                       m_velocity.hpr[0] =  0.0f ;
        }
#else
        delta[2] = 0.0f ;

        sgHPRfromVec3 ( hpr, delta ) ;

        sgSubVec3 ( hpr, m_curr_pos.hpr ) ;

        if ( hpr[0] >  180.0f ) hpr[0] -= 360.0f ;
        if ( hpr[0] < -180.0f ) hpr[0] += 360.0f ;
        if ( hpr[1] >  180.0f ) hpr[1] -= 360.0f ;
        if ( hpr[1] < -180.0f ) hpr[1] += 360.0f ;

        if ( hpr[0] > 80.0f || hpr[0] < -80.0f )
            m_velocity.hpr[0] = 0.0f ;
        else
        {
            if      ( hpr[0] >  3.0f ) m_velocity.hpr[0] =  HOMING_MISSILE_TURN_RATE ;
            else if ( hpr[0] < -3.0f ) m_velocity.hpr[0] = -HOMING_MISSILE_TURN_RATE ;
            else                       m_velocity.hpr[0] =  0.0f ;

            if      ( hpr[2] > 1.0f  ) m_velocity.hpr[1] = -HOMING_MISSILE_PITCH_RATE ;
            else if ( hpr[2] < -1.0f ) m_velocity.hpr[1] = HOMING_MISSILE_PITCH_RATE ;
            else                       m_velocity.hpr[1] = 0.0f ;
        }
#endif
    }
    else  // m_type!=HOMING||nearest==-1||ndist>MAX_HOME_DIST_SQD
        m_velocity.hpr[0] = m_velocity.hpr[1] = 0.0f ;
}   // doObjectInteractions

//-----------------------------------------------------------------------------
#ifndef BULLET
void Projectile::doCollisionAnalysis  ( float dt, float hot )
{
    if ( m_collided || m_crashed )
    {
        if ( m_type == COLLECT_SPARK )
        {
            sgVec3 bouncevec ;
            sgVec3 direction ;

            sgNormalizeVec3 ( bouncevec, m_surface_avoidance_vector ) ;
            sgSubVec3 ( direction, m_curr_pos.xyz, m_last_pos.xyz ) ;
            sgReflectInPlaneVec3 ( direction, bouncevec ) ;

            sgHPRfromVec3 ( m_curr_pos.hpr, direction ) ;
        }
        else if ( m_type != COLLECT_NOTHING )
        {
            explode(NULL);  // no kart was hit directly
        }
    }   // if m_collided||m_crashed
}   // doCollisionAnalysis
#endif
//-----------------------------------------------------------------------------
void Projectile::explode(Kart *kart_hit)
{
#ifdef BULLET
	if(m_exploded) return;

#endif
	m_has_hit_something=true;
    m_curr_pos.xyz[2] += 1.2f ;
    // Notify the projectile manager that this rocket has hit something.
    // The manager will create the appropriate explosion object, and
    // place this projectile into a list so that it can be reused later,
    // without the additional cost of creating the object again
    projectile_manager->explode();

    // Now remove this projectile from the graph:
    ssgTransform *m = getModel();
    m->removeAllKids();
    scene->remove(m);

#ifdef BULLET
    world->getPhysics()->removeBody(getBody());
	m_exploded=true;
#endif
    for ( unsigned int i = 0 ; i < world->getNumKarts() ; i++ )
    {
        Kart *kart = world -> getKart(i);
        // handle the actual explosion. Set a flag it if was a direct hit.
        kart->handleExplosion(m_curr_pos.xyz, kart==kart_hit);
    }

}   // explode

/* EOF */
