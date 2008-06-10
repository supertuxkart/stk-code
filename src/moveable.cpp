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
    if(m_body) m_body->setLinearVelocity(btVector3(0.0, 0.0, 0.0));
    sgCopyCoord( &m_curr_pos, &m_reset_pos );
    m_hpr = Vec3(m_curr_pos.hpr);
    m_hpr.degreeToRad();
}   // reset

//-----------------------------------------------------------------------------
void Moveable::createBody(float mass, btTransform& trans, 
                          btCollisionShape *shape) {
    
    btVector3 inertia;
    shape->calculateLocalInertia(mass, inertia);
    m_motion_state = new btDefaultMotionState(trans);

    btRigidBody::btRigidBodyConstructionInfo info(mass, m_motion_state, shape, inertia);
    info.m_restitution=0.5f;

    // Then create a rigid body
    // ------------------------
    m_body = new btRigidBody(info);
    // The value of user_pointer must be set from the actual class, otherwise this
    // is only a pointer to moveable, not to (say) kart, and virtual 
    // functions are not called correctly. So only init the pointer to zero.
    m_user_pointer.zero();
    m_body->setUserPointer(&m_user_pointer);
    const btMatrix3x3& basis=m_body->getWorldTransform().getBasis();
    m_hpr.setHPR(basis);
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
            m_velocityLC.setValue(tmp.xyz[0],tmp.xyz[1],tmp.xyz[2]);
#endif
        }
        else
        {
            m_history_velocity[history->GetCurrentIndex()].xyz[0]=m_velocityLC.getX();
            m_history_velocity[history->GetCurrentIndex()].xyz[1]=m_velocityLC.getY();
            m_history_velocity[history->GetCurrentIndex()].xyz[2]=m_velocityLC.getZ();
        }
    }   // if m_history_velocity

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

    m_velocityLC = getVelocity()*getTrans().getBasis();
    m_motion_state->getWorldTransform(m_transform);
    m_hpr.setHPR(m_transform.getBasis());

    placeModel();
    m_first_time = false ;
}   // update

//-----------------------------------------------------------------------------
void Moveable::placeModel()
{
    m_model_transform->setTransform(&m_curr_pos);
}   // placeModel

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
