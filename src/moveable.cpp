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
    m_body            = 0;
    m_motion_state    = 0;
    m_shadow          = 0;
    m_first_time      = true ;
    m_model_transform = new ssgTransform();

    m_model_transform->ref();

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
    // The body is being removed from the world in kart/projectile
    if(m_body)         delete m_body;
    if(m_motion_state) delete m_motion_state;
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
void Moveable::createBody(float mass, btTransform& trans, 
                          btCollisionShape *shape, MoveableType m) {
    
    btVector3 inertia;
    m_transform = trans;
    shape->calculateLocalInertia(mass, inertia);
    m_motion_state = new btDefaultMotionState(trans);

    // Then create a rigid body
    // ------------------------
    m_body = new btRigidBody(mass, m_motion_state, 
                             shape, inertia);
    m_body->setUserPointer(this);
    setMoveableType(m);
}   // createBody

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
    
    placeModel();
    m_first_time = false ;
}   // update

//-----------------------------------------------------------------------------
void Moveable::placeModel()
{
    m_motion_state->getWorldTransform(m_transform);
    m_model_transform->setTransform(&m_curr_pos);
}   // placeModel

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
