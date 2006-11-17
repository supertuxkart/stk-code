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
#include "kart.hpp"
#include "constants.hpp"
#include "loader.hpp"
#include "world.hpp"
#include "sound_manager.hpp"

Attachment::Attachment(Kart* _kart)
{
    m_type      = ATTACH_NOTHING;
    m_time_left = 0.0;
    m_kart      = _kart;
    m_holder    = new ssgSelector();
    m_kart->getModel()->addKid(m_holder);

    for(int i=ATTACH_PARACHUTE; i<=ATTACH_TINYTUX; i++)
    {
        ssgEntity *p=attachment_manager->getModel((attachmentType)i);
        m_holder->addKid(p);
    }
    m_holder->select(0);
}   // Attachmetn

//-----------------------------------------------------------------------------
Attachment::~Attachment()
{
    ssgDeRefDelete(m_holder);
}   // ~Attachment

//-----------------------------------------------------------------------------
void Attachment::set(attachmentType _type, float time)
{
    m_holder->selectStep(_type);
    m_type      = _type;
    m_time_left = time;
}   // set

// -----------------------------------------------------------------------------
void Attachment::hitGreenHerring()
{
    switch (rand()%2)
    {
    case 0: set( ATTACH_PARACHUTE, 4.0f ) ;
        // if ( m_kart == m_kart[0] )
        //   sound -> playSfx ( SOUND_SHOOMF ) ;
        break ;
    case 1: set( ATTACH_ANVIL, 2.0f ) ;
        // if ( m_kart == m_kart[0] )
        //   sound -> playSfx ( SOUND_SHOOMF ) ;
        // Reduce speed once (see description above), all other changes are
        // handled in Kart::updatePhysics
        m_kart->getVelocity()->xyz[1] *= physicsParameters->m_anvil_speed_factor;
        break ;
    }   // switch rand()%2
}   // hitGreenHerring

//-----------------------------------------------------------------------------
void Attachment::update(float dt, sgCoord *velocity)
{
    if(m_type==ATTACH_NOTHING) return;
    m_time_left -=dt;

    switch (m_type)
    {
    case ATTACH_PARACHUTE:  // handled in Kart::updatePhysics
    case ATTACH_ANVIL:      // handled in Kart::updatePhysics
    case ATTACH_NOTHING:   // Nothing to do, but complete all cases for switch
    case ATTACH_MAX:       break;
    case ATTACH_TINYTUX:   if(m_time_left<=0.0) m_kart->handleRescue();
        sgZeroVec3 ( velocity->xyz ) ;
        sgZeroVec3 ( velocity->hpr ) ;
        velocity->xyz[2] = 1.1f * GRAVITY * dt *10.0f;
        break;
#ifdef USE_MAGNET
    case ATTACH_MAGNET:    break;
    case ATTACH_MAGNET_BZZT: float cdist; int closest;
        m_kart->getClosestKart(&cdist, &closest);
        // if no closest kart, set type to
        // non-active magnet
        if(closest==-1)
        {
            if ( m_type == ATTACH_MAGNET_BZZT )
                set( ATTACH_MAGNET, m_time_left ) ;
            return;
        }
        // Otherwise: set type to active magnet.
        if(m_type==ATTACH_MAGNET)
        {
            if(m_kart->isPlayerKart() || closest==0)
            {
                sound_manager->playSfx(SOUND_BZZT);
            }
            set(ATTACH_MAGNET_BZZT,
                    m_time_left<4.0?4.0:m_time_left);
        }
        m_kart->handleMagnet(cdist, closest);
        break;
#endif

    }   // switch

    if ( m_time_left <= 0.0f)
    {
        clear();
    }   // if m_time_left<0
}   // update
