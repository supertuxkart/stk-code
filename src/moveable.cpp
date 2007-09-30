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

#include "world.hpp"
#include "player_kart.hpp"
#include "material_manager.hpp"
#include "material.hpp"
#include "user_config.hpp"
#include "history.hpp"

Moveable::Moveable (bool bHasHistory)
{
#ifdef BULLET
    m_body         = 0;
    m_motion_state = 0;
#endif
    m_shadow       = 0;
    m_first_time   = true ;
    m_model        = new ssgTransform();

    m_model->ref();

    sgZeroVec3 ( m_reset_pos.xyz ) ; sgZeroVec3 ( m_reset_pos.hpr ) ;

    reset ();
    if(bHasHistory)
    {
        m_history_velocity = new sgCoord[history->GetSize()];
        m_history_position = new sgCoord[history->GetSize()];
    }
    else
    {
        m_history_velocity = NULL;
        m_history_position = NULL;
    }
}   // Moveable

//-----------------------------------------------------------------------------
Moveable::~Moveable()
{
#ifdef BULLET
    if(m_body        ) delete m_body;
    if(m_motion_state) delete m_motion_state;
#endif
    if(m_history_velocity)
    {
        delete [] m_history_velocity;
        delete [] m_history_position;
    }
    // FIXME what about model?
}   // ~Moveable

//-----------------------------------------------------------------------------
void Moveable::reset ()
{
    m_on_ground        = true;
    m_collided         = false;
    m_crashed          = false;
    m_material_hot     = NULL;
    m_normal_hot       = NULL;

    sgZeroVec3 ( m_velocity.xyz ) ;
    sgZeroVec3 ( m_velocity.hpr ) ;
    sgCopyCoord ( &m_curr_pos, &m_reset_pos ) ;
    sgZeroVec3 ( m_abs_velocity ) ;

}   // reset

//-----------------------------------------------------------------------------
#ifdef BULLET
void Moveable::createBody(float mass, btTransform& position, 
                          btCollisionShape *shape, btVector3 inertia) {
    
    m_motion_state = new btDefaultMotionState(position);

    // Then create a rigid body
    // ------------------------
    m_body = new btRigidBody(mass, m_motion_state, 
                             shape, inertia);
}   // createBody
#endif
//-----------------------------------------------------------------------------
void Moveable::update (float dt)
{
    if(m_history_velocity)
    {
        if(user_config->m_replay_history)
        {
            sgCoord tmp;
            sgCopyCoord(&tmp, &(m_history_velocity[history->GetCurrentIndex()]));
            //printf("m_velocity=%f,%f,%f,%f,%f,%f\n",
            //     m_velocity.xyz[0],m_velocity.xyz[1],m_velocity.xyz[2],
            //     m_velocity.hpr[0],m_velocity.hpr[1],m_velocity.hpr[2]);
            //printf("tmp     =%f,%f,%f,%f,%f,%f\n",
            //     tmp.xyz[0],tmp.xyz[1],tmp.xyz[2],
            //     tmp.hpr[0],tmp.hpr[1],tmp.hpr[2]);

#undef IGNORE_Z_IN_HISTORY
#ifdef IGNORE_Z_IN_HISTORY
            const float DUMMY=m_velocity.xyz[2];
            sgCopyCoord(&m_velocity, &tmp);
            m_velocity.xyz[2]=DUMMY;
#else
            sgCopyCoord(&m_velocity, &tmp);
#endif
        }
        else
        {
            sgCopyCoord(&(m_history_velocity[history->GetCurrentIndex()]), &m_velocity);
        }
    }   // if m_history_velocity

    sgMat4 result;
    updatePosition(dt,result);

    sgVec3 start ; sgCopyVec3      (start, m_curr_pos.xyz      );
    sgVec3 end   ; sgCopyVec3      (end  , result[3]         );

    const float  HOT   = collectIsectData(start, end               );

    sgCopyVec3 (result[3], end) ;

    sgVec3 prev_pos;
    sgCopyVec3(prev_pos, m_curr_pos.xyz);
    sgSetCoord (&m_curr_pos, result);
    sgSubVec3  (m_abs_velocity, m_curr_pos.xyz, prev_pos);

    if(m_history_position)
    {
        if(user_config->m_replay_history)
        {
            sgCoord tmp;
            sgCopyCoord(&tmp, &(m_history_position[history->GetCurrentIndex()]));
            //printf("m_curr_pos=%f,%f,%f,%f,%f,%f\n",
            //     m_curr_pos.xyz[0],m_curr_pos.xyz[1],m_curr_pos.xyz[2],
            //     m_curr_pos.hpr[0],m_curr_pos.hpr[1],m_curr_pos.hpr[2]);
            //printf("tmp     =%f,%f,%f,%f,%f,%f --> %d\n",
            //     tmp.xyz[0],tmp.xyz[1],tmp.xyz[2],
            //     tmp.hpr[0],tmp.hpr[1],tmp.hpr[2],
            //     history->GetCurrentIndex());

#ifdef IGNORE_Z_IN_HISTORY
            const float DUMMY=m_curr_pos.xyz[2];
            sgCopyCoord(&m_curr_pos, &tmp);
            m_curr_pos.xyz[2]=DUMMY;
#else
            sgCopyCoord(&m_curr_pos, &tmp);
#endif

        }
        else
        {
            sgCopyCoord(&(m_history_position[history->GetCurrentIndex()]), &m_curr_pos);
        }
    }   // if m_history_position
    const float HAT = m_curr_pos.xyz[2]-HOT;

#ifdef BULLET
    m_on_ground = ( HAT <= 1.5 );
#else
    m_on_ground = ( HAT <= 0.01 );
#endif

    doCollisionAnalysis(dt, HOT);

    placeModel () ;

    m_first_time = false ;
}   // update

//-----------------------------------------------------------------------------
/**
 * Computes the new position and hpr of the kart after a single time step.
 */
void Moveable::updatePosition(float dt, sgMat4 result)
{
    sgCoord scaled_velocity ;
    sgMat4  delta, mat;
    /* Scale velocities to current time step. */
    sgScaleVec3    (scaled_velocity.xyz, m_velocity.xyz, dt);
    sgScaleVec3    (scaled_velocity.hpr, m_velocity.hpr, dt);
    sgMakeCoordMat4(delta, & scaled_velocity             );
    sgMakeCoordMat4(mat  , & m_curr_pos                    );
    sgMultMat4     (result, mat, delta                   );
}   // updatePosition

//-----------------------------------------------------------------------------
void Moveable::WriteHistory(char* s, int kartNumber, int indx)
{
    sprintf(s, "Kart %d: v=%f,%f,%f,%f,%f,%f, p=%f,%f,%f,%f,%f,%f", kartNumber,
            m_history_velocity[indx].xyz[0],
            m_history_velocity[indx].xyz[1],
            m_history_velocity[indx].xyz[2],
            m_history_velocity[indx].hpr[0],
            m_history_velocity[indx].hpr[1],
            m_history_velocity[indx].hpr[2],
            m_history_position[indx].xyz[0],
            m_history_position[indx].xyz[1],
            m_history_position[indx].xyz[2],
            m_history_position[indx].hpr[0],
            m_history_position[indx].hpr[1],
            m_history_position[indx].hpr[2]);
}   // WriteHistory

//-----------------------------------------------------------------------------
void Moveable::ReadHistory(char* s, int kartNumber, int indx)
{
    int k;
    sscanf(s, "Kart %d: v=%f,%f,%f,%f,%f,%f, p=%f,%f,%f,%f,%f,%f", &k,
           m_history_velocity[indx].xyz+0,
           m_history_velocity[indx].xyz+1,
           m_history_velocity[indx].xyz+2,
           m_history_velocity[indx].hpr+0,
           m_history_velocity[indx].hpr+1,
           m_history_velocity[indx].hpr+2,
           m_history_position[indx].xyz+0,
           m_history_position[indx].xyz+1,
           m_history_position[indx].xyz+2,
           m_history_position[indx].hpr+0,
           m_history_position[indx].hpr+1,
           m_history_position[indx].hpr+2);
    if(k!=kartNumber)
    {
        fprintf(stderr,"WARNING: tried reading data for kart %d, found:\n",
                kartNumber);
        fprintf(stderr,"%s\n",s);
        exit(-2);
    }
}   // ReadHistory

//-----------------------------------------------------------------------------
void Moveable::doCollisionAnalysis  ( float,float ) { /* Empty by Default. */ }

#define ISECT_STEP_SIZE         0.4f
#define COLLISION_SPHERE_RADIUS 0.6f

#define max(m,n) ((m)>(n) ? (m) : (n)) /* return highest number */

//-----------------------------------------------------------------------------
float Moveable::collectIsectData ( sgVec3 start, sgVec3 end )
{
    sgVec3 vel ;

    m_collided = m_crashed = false ;  /* Initial assumption */

    sgSubVec3 ( vel, end, start ) ;

    const float SPEED = sgLengthVec3 ( vel ) ;

    /*
      At higher speeds, we must test frequently so we can't
      pass through something thin by mistake.

      At very high speeds, this is getting costly...so beware!
    */

    int nsteps = (int) ceil ( SPEED / ISECT_STEP_SIZE ) ;

    if ( nsteps == 0 ) nsteps = 1 ;

    if ( nsteps > 100 )
    {
        fprintf(stderr, "WARNING: Speed too high for collision detection!\n"
                        "WARNING: Nsteps=%d, Speed=%f!\n"
                        "moveable %p, vel=%f,%f,%f\n",
                nsteps, SPEED, this, vel[0], vel[1], vel[2]);
        nsteps = 100 ;
    }

    sgScaleVec3 ( vel, vel, 1.0f / (float) nsteps ) ;

    sgVec3 pos1, pos2 ;

    sgCopyVec3 ( pos1, start ) ;

    float hot = -9999.0 ;

    for ( int i = 0 ; i < nsteps ; i++ )
    {
        sgAddVec3 ( pos2, pos1, vel ) ;
        float hot1 = getIsectData ( pos1, pos2 ) ;
        hot = max(hot, hot1);
        sgCopyVec3 ( pos1, pos2 ) ;
        if(m_collided) break;
    }

    sgCopyVec3 ( end, pos2 ) ;
    return hot ;
}   // collectIsectData

//-----------------------------------------------------------------------------

float Moveable::getIsectData ( sgVec3 start, sgVec3 end )
{
    int num_hits;

    sgSphere sphere;

    /*
      It's necessary to lift the center of the bounding sphere
      somewhat so that Player can stand on a slope.
    */

    sphere.setRadius ( COLLISION_SPHERE_RADIUS ) ;
    sphere.setCenter ( 0.0f, 0.0f, COLLISION_SPHERE_RADIUS + 0.3f) ;

    /* Do a bounding-sphere test for Player. */
    sgSetVec3 ( m_surface_avoidance_vector, 0.0f, 0.0f, 0.0f );

    // new collision  algorithm
    AllHits a;
    sphere.setCenter ( end[0],end[1],end[2]+ COLLISION_SPHERE_RADIUS + 0.3f) ;
    num_hits = world->Collision(&sphere, &a);
    for(AllHits::iterator i=a.begin(); i!=a.end(); i++)
    {
        if ( (*i)->m_plane[2]>0.4 ) continue;
        const float DIST = sphere.getRadius()-(*i)->m_dist;
        sgVec3 nrm ;
        sgCopyVec3  ( nrm, (*i)->m_plane ) ;
        sgScaleVec3 ( nrm, nrm, DIST ) ;

        sgAddVec3 ( m_surface_avoidance_vector, nrm ) ;

        sgVec3 tmp ;
        sgCopyVec3 ( tmp, sphere.getCenter() ) ;
        sgAddVec3 ( tmp, nrm ) ;
        sphere.setCenter ( tmp ) ;

        m_collided = true ;
        Material* m = material_manager->getMaterial( (*i)->m_leaf);
        if (m->isZipper   () ) m_collided = false ;
        if (m->isCrashable() ) m_crashed  = true  ;
        if (m->isReset    () ) OutsideTrack(1);
    }   // for i in a

    sgAddVec3(end, m_surface_avoidance_vector);

    // H.O.T == Height Of Terrain
    // ==========================
    const float TOP = COLLISION_SPHERE_RADIUS + max(start[2],end[2]);
    sgVec3 dstart; sgCopyVec3(dstart, end);
    sgVec3 dummy; sgCopyVec3(dummy, end);
    dummy[2]=TOP;
    ssgLeaf* m_leaf;
    const float HOT = world->GetHOT(dummy, dummy, &m_leaf, &m_normal_hot);
    if(m_leaf)
    {
        m_material_hot = material_manager->getMaterial(m_leaf);
        // Only rescue the kart if it (nearly) touches the reset-material,
        // not only when it is above it. The condition for touching
        // a material is coarser then for the m_on_ground condition
        // (which tests for <0.01) - since the kart might have been falling
        // for quite some time, it might be really fast, so I guess a somewhat
        // coarser test is better for that case.
        if(m_material_hot->isReset() &&
           fabs(TOP-COLLISION_SPHERE_RADIUS - HOT)<0.2) OutsideTrack(1);
        if(m_material_hot->isZipper()) handleZipper();
    }
    else
    {
        OutsideTrack(0);
    }

    if (end[2] < HOT )
    {
        end[2] = HOT ;
    }   // end[2]<HOT
    return HOT ;
}   // getIsectData

