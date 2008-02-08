//  $Id: moving_texture.cpp 796 2006-09-27 07:06:34Z hiker $
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

#include "constants.hpp"
#include "moving_texture.hpp"
#include "string_utils.hpp"
#include "world.hpp"
#include "translation.hpp"

MovingTexture::MovingTexture(char *data, ssgBranch *branch)
{
    m_branch = branch;
    branch->setUserData(new ssgBase());
    branch->setName("MovingTexture");

    m_branch->ref();
    m_phase  = 0.0f;
    m_mode   = MODE_FORWARD;
    m_cycle  = 30.0f;
    sgSetCoord(&m_now,   0, 0, 0, 0, 0, 0 ) ;
    sgSetCoord(&m_delta, 0, 0, 0, 0, 0, 0 ) ;
    parseData(data);


}   // MovingTexture

//-----------------------------------------------------------------------------
MovingTexture::~MovingTexture()
{
    ssgDeRefDelete(m_branch);

}   // ~MovingTexture

//-----------------------------------------------------------------------------
void MovingTexture::parseData(char *data)
{
    char *s = data;

    // convert to uppercase
    while(*s!='\0')
    {
        if( *s>='a' && *s<='z' ) *s = *s - 'a' + 'A' ;
        s++;
    }
    s = data;

    while ( s != NULL && *s != '\0' )
    {
        while ( *s > ' '                ) s++ ;     /* Skip previous token */
        while ( *s <= ' ' && *s != '\0' ) s++ ;     /* Skip spaces         */

        if ( *s == '\0' ) break ;

        float f ;

        if ( sscanf ( s,  "H=%f", & f ) == 1 ) m_delta.hpr[0] = f ; else
        if ( sscanf ( s,  "P=%f", & f ) == 1 ) m_delta.hpr[1] = f ; else
        if ( sscanf ( s,  "R=%f", & f ) == 1 ) m_delta.hpr[2] = f ; else
        if ( sscanf ( s,  "X=%f", & f ) == 1 ) m_delta.xyz[0] = f ; else
        if ( sscanf ( s,  "Y=%f", & f ) == 1 ) m_delta.xyz[1] = f ; else
        if ( sscanf ( s,  "Z=%f", & f ) == 1 ) m_delta.xyz[2] = f ; else
        if ( sscanf ( s,  "C=%f", & f ) == 1 ) m_cycle        = f ; else
        if ( sscanf ( s,  "M=%f", & f ) == 1 ) m_mode         = (int) f ; else
        if ( sscanf ( s,  "O=%f", & f ) == 1 ) m_phase        = f ; else
            fprintf ( stderr, _("Unrecognised @autodcs string: '%s'\n"), data );
    }   // while s!=NULL&&s!='\0'
}   // parseData

//-----------------------------------------------------------------------------
void MovingTexture::update(float dt)
{
    sgCoord add;


    float timer = world->getTime() + m_phase ;

    if ( m_cycle != 0.0 && m_mode != MODE_FORWARD )
    {
        if ( m_mode == MODE_SHUTTLE )
        {
            const float CTIMER = fmod ( timer, m_cycle ) ;

            if ( CTIMER > m_cycle / 2.0f )
                timer = m_cycle - CTIMER ;
            else
                timer = CTIMER ;
        }
        else
        {
            if ( m_mode == MODE_SINESHUTTLE )
                timer = sin ( timer * 2.0f * M_PI / m_cycle ) * m_cycle / 2.0f ;
            else
                timer = fmod ( timer, m_cycle ) ;
        }
    }   // m_mode!=MODE_FORWARD

    sgCopyCoord(&add, &m_delta  );
    sgScaleVec3(add.xyz, dt);
    sgScaleVec3(add.hpr, dt);

    sgAddVec3(m_now.xyz, add.xyz);
    sgAddVec3(m_now.hpr, add.hpr);

    /*
      To avoid roundoff problems with very large values
      accumulated after long runs all rotations
      can be modulo-360 degrees.
    */

    m_now.hpr[0] = fmod ( m_now.hpr[0], 360.0f);
    m_now.hpr[1] = fmod ( m_now.hpr[1], 360.0f);
    m_now.hpr[2] = fmod ( m_now.hpr[2], 360.0f);

    if ( m_branch->isAKindOf(ssgTypeTexTrans()) )
    {
        m_now.xyz[0] = fmod( m_now.xyz[0], 1.0f);
        m_now.xyz[1] = fmod( m_now.xyz[1], 1.0f);
        m_now.xyz[2] = fmod( m_now.xyz[2], 1.0f);
    }
    ((ssgBaseTransform *) m_branch) -> setTransform(&m_now);
}   // update


