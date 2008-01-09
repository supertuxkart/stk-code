//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
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

#include <plib/ssg.h>

#include "attachment.hpp"
#include "attachment_manager.hpp"
#include "projectile_manager.hpp"
#include "kart.hpp"
#include "constants.hpp"
#include "loader.hpp"
#include "world.hpp"
#include "sound_manager.hpp"
#include "stk_config.hpp"

Attachment::Attachment(Kart* _kart)
{
    m_type           = ATTACH_NOTHING;
    m_time_left      = 0.0;
    m_kart           = _kart;
    m_holder         = new ssgSelector();
    m_previous_owner = NULL;
    m_kart->getModelTransform()->addKid(m_holder);

    for(int i=ATTACH_PARACHUTE; i<=ATTACH_TINYTUX; i++)
    {
        ssgEntity *p=attachment_manager->getModel((attachmentType)i);
        m_holder->addKid(p);
    }
    m_holder->select(0);
}   // Attachment

//-----------------------------------------------------------------------------
Attachment::~Attachment()
{
    ssgDeRefDelete(m_holder);
}   // ~Attachment

//-----------------------------------------------------------------------------
void Attachment::set(attachmentType _type, float time, Kart *current_kart)
{
    m_holder->selectStep(_type);
    m_type           = _type;
    m_time_left      = time;
    m_previous_owner = current_kart;
}   // set

// -----------------------------------------------------------------------------
void Attachment::hitGreenHerring()
{
    int random_attachment;
    float leftover_time   = 0.0f;
    switch(getType())   // If there already is an attachment, make it worse :)
    {
    case ATTACH_BOMB:  projectile_manager->newExplosion(m_kart->getCoord());
                       // Best solution would probably be to trigger the
                       // explosion, and then to attach a new, random
                       // attachment. Unfortunately, handleExplosion() is not
                       // really severe enough, and forceRescue() attaches
                       // tinytux, so that the new attachment is immediately lost.
                       //m_kart->handleExplosion(m_kart->getCoord()->xyz, true);
                       m_kart->forceRescue();
                       clear();
                       random_attachment = rand()%3;
                       break;
    case ATTACH_ANVIL :// if the kart already has an anvil, attach a new anvil, 
                       // and increase the overall time 
                       random_attachment = 2;
                       leftover_time     = m_time_left;
                       break;
    case ATTACH_PARACHUTE:
                       random_attachment = 2;  // anvil
                       leftover_time     = m_time_left;
                       break;
    default:           random_attachment = rand()%3;
    }   // switch
    
    switch (random_attachment)
    {
    case 0: set( ATTACH_PARACHUTE, stk_config->m_parachute_time+leftover_time);
        // if ( m_kart == m_kart[0] )
        //   sound -> playSfx ( SOUND_SHOOMF ) ;
        break ;
    case 1: set( ATTACH_BOMB, stk_config->m_bomb_time+leftover_time);
        // if ( m_kart == m_kart[0] )
        //   sound -> playSfx ( SOUND_SHOOMF ) ;
        break ;
    case 2: set( ATTACH_ANVIL, stk_config->m_anvil_time+leftover_time);
        // if ( m_kart == m_kart[0] )
        //   sound -> playSfx ( SOUND_SHOOMF ) ;
        // Reduce speed once (see description above), all other changes are
        // handled in Kart::updatePhysics
        m_kart->adjustSpeedWeight(stk_config->m_anvil_speed_factor);
        break ;
    }   // switch rand()%3
}   // hitGreenHerring

//-----------------------------------------------------------------------------
//** Moves a bomb from kart FROM to kart TO.
void Attachment::moveBombFromTo(Kart *from, Kart *to)
{
    to->setAttachmentType(ATTACH_BOMB,
                          from->getAttachment()->getTimeLeft()+
                          stk_config->m_bomb_time_increase, from);
    from->getAttachment()->clear();
}   // moveBombFromTo

//-----------------------------------------------------------------------------
void Attachment::update(float dt)
{
    if(m_type==ATTACH_NOTHING) return;
    m_time_left -=dt;

    switch (m_type)
    {
    case ATTACH_PARACHUTE:  // handled in Kart::updatePhysics
    case ATTACH_ANVIL:      // handled in Kart::updatePhysics
    case ATTACH_NOTHING:   // Nothing to do, but complete all cases for switch
    case ATTACH_MAX:       break;
    case ATTACH_BOMB:      if(m_time_left<=0.0) 
                           {
                               projectile_manager->newExplosion(m_kart->getCoord());
                               m_kart->forceRescue();
                           }
                           break;
    case ATTACH_TINYTUX:   if(m_time_left<=0.0) m_kart->endRescue();
                           break;
    }   // switch

    // Detach attachment if its time is up.
    if ( m_time_left <= 0.0f)
    {
        if(m_type==ATTACH_ANVIL) 
        {
            // Resets the weight, and multiplies the velocity by 1.0, 
            // i.e. no change of velocity.
            m_kart->getAttachment()->clear();
            m_kart->adjustSpeedWeight(1.0f);
        }
        clear();
    }   // if m_time_left<0
}   // update
//-----------------------------------------------------------------------------
