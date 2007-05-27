//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
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
#include "herring.hpp"
#include "kart.hpp"
#include "scene.hpp"

Herring::Herring(herringType _type, sgVec3* xyz, ssgEntity* model)
{
    sgSetVec3(m_coord.hpr, 0.0f, 0.0f, 0.0f);

    sgCopyVec3(m_coord.xyz, *xyz);
    m_root   = new ssgTransform();
    m_root->ref();
    m_root->setTransform(&m_coord);

    m_rotate = new ssgTransform();
    m_rotate->ref();
    m_rotate->addKid(model);
    m_root->addKid(m_rotate);
    scene->add(m_root);

    m_type           = _type;
    m_eaten         = false;
    m_rotation       = 0.0f;
    m_time_to_return = 0.0f;  // not strictly necessary, see isEaten()
}   // Herring

//-----------------------------------------------------------------------------
Herring::~Herring()
{
    ssgDeRefDelete(m_root);
    ssgDeRefDelete(m_rotate);
}   // ~Herring

//-----------------------------------------------------------------------------
void Herring::reset()
{
    m_eaten         = false;
    m_time_to_return = 0.0f;
    m_root->setTransform(&m_coord);
}   // reset

//-----------------------------------------------------------------------------
int Herring::hitKart(Kart* kart)
{
    return sgDistanceSquaredVec3 ( kart->getCoord()->xyz, m_coord.xyz ) < 0.8f;
}   // hitKart

//-----------------------------------------------------------------------------
void Herring::update(float delta)
{
    if(m_eaten)
    {
        const float T = m_time_to_return - world->m_clock;
        if ( T > 0 )
        {
            sgVec3 hell;
            sgCopyVec3(hell, m_coord.xyz);

            hell[2] = ( T > 1.0f ) ? -1000000.0f : m_coord.xyz[2] - T / 2.0f;
            m_root -> setTransform(hell);
        }
        else
        {
            m_eaten   = false;
            m_rotation = 0.0f;
            m_root -> setTransform(&m_coord);
        }   // T>0

    }
    else
    {   // not m_eaten
        sgCoord c = { { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } } ;
        c.hpr[0] = m_rotation;
        m_rotation += 180.0f*delta;
        m_rotate -> setTransform ( &c ) ;
    }
}   // update

//-----------------------------------------------------------------------------
void Herring::isEaten()
{
    m_eaten=true;
    m_time_to_return=world->m_clock+2.0f;
}

