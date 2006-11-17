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

#ifndef HEADER_ATTACHMENT_H
#define HEADER_ATTACHMENT_H

#include "physics_parameters.hpp"
class Kart;
class ssgEntity;

// Some loop in Attachment.cpp depend on PARACHUTE being the first element,
// and TINYTUX being the last one. So if new elemts are added, make sure
// to add them in between those values.
enum attachmentType { ATTACH_PARACHUTE,
#ifdef USE_MAGNET
                      ATTACH_MAGNET, ATTACH_MAGNET_BZZT,
#endif
                      ATTACH_ANVIL, ATTACH_TINYTUX,
                      ATTACH_MAX, ATTACH_NOTHING};


class Attachment
{
private:
    attachmentType  m_type;
    Kart           *m_kart;
    float           m_time_left;
    ssgSelector    *m_holder;    // where the attachment is put on the kart
public:
    Attachment(Kart* _kart);
    ~Attachment();

    void set (attachmentType _type, float time);
    void set (attachmentType _type) {set(_type, m_time_left); }

    void clear ()
    {
        m_type=ATTACH_NOTHING; m_time_left=0.0;
        m_holder->select(0);
    }

    attachmentType getType () {return m_type;      }
    float getTimeLeft () {return m_time_left; }

    float WeightAdjust () const
    {
        return m_type==ATTACH_ANVIL
               ?physicsParameters->m_anvil_weight:0.0f;
    }

    float AirResistanceAdjust () const
    {
        return m_type==ATTACH_PARACHUTE
               ?physicsParameters->m_parachute_friction:0.0f;
    }

    float SpeedAdjust () const
    {
        return m_type==ATTACH_ANVIL
               ?physicsParameters->m_anvil_speed_factor:1.0f;
    }
    void  hitGreenHerring();
    void  update (float dt, sgCoord *velocity);
};

#endif
