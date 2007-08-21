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

Projectile::Projectile(Kart *kart, int collectable) : Moveable(false)
{
    init(kart, collectable);
    getModel()->clrTraversalMaskBits(SSGTRAV_ISECT|SSGTRAV_HOT);
}   // Projectile

//-----------------------------------------------------------------------------
void Projectile::init(Kart *kart, int collectable_)
{
    m_owner              = kart;
    m_type               = collectable_;
    m_has_hit_something    = false;
    m_last_radar_beep     = -1;
    m_speed              = collectable_manager->getSpeed(m_type);
    ssgTransform *m    = getModel();
    m->addKid(collectable_manager->getModel(m_type));

    sgCoord c;
    sgCopyCoord(&c, kart->getCoord());
    const float angle_terrain = c.hpr[0]*M_PI/180.0f;
    // The projectile should have the pitch of the terrain on which the kart
    // is - while this is not really realistic, it plays much better this way
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
    scene->add(m);
}   // init

//-----------------------------------------------------------------------------
Projectile::~Projectile()
{}   // ~Projectile

//-----------------------------------------------------------------------------
void Projectile::update (float dt)
{
    // we don't even do any physics here - just set the
    // velocity, and ignore everything else for projectiles.
    m_velocity.xyz[1] = m_speed;
    sgCopyCoord ( &m_last_pos, &m_curr_pos);
    Moveable::update(dt);
    doObjectInteractions();
}   // update

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

            if ( D < 2.0f )
            {
                explode(kart);
                return;
            }
            else if ( D < ndist )
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

//-----------------------------------------------------------------------------
void Projectile::explode(Kart *kart_hit)
{
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

    for ( unsigned int i = 0 ; i < world->getNumKarts() ; i++ )
    {
        Kart *kart = world -> getKart(i);
        // handle the actual explosion. Set a flag it if was a direct hit.
        kart->handleExplosion(m_curr_pos.xyz, kart==kart_hit);
    }

}   // explode

/* EOF */
