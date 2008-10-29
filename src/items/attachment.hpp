//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
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

#include "stk_config.hpp"
#include "utils/random_generator.hpp"

class Kart;
class Item;

// Some loop in Attachment.cpp depend on PARACHUTE being the first element,
// and TINYTUX being the last one. So if new elemts are added, make sure
// to add them in between those values.
enum attachmentType { ATTACH_PARACHUTE,
                      ATTACH_BOMB,
                      ATTACH_ANVIL, ATTACH_TINYTUX,
                      ATTACH_MAX, ATTACH_NOTHING};


class Attachment
{
private:
    attachmentType  m_type;            // attachment type
    Kart           *m_kart;            // kart the attachment is attached to
    float           m_time_left;       // time left till attachment expires
    float           m_initial_speed;   // for parachutes only
    ssgSelector    *m_holder;          // where the attachment is put on the kart
    Kart           *m_previous_owner;  // used by bombs so that it's not passed
                                       // back to previous owner
    RandomGenerator m_random;
public:
    Attachment(Kart* _kart);
    ~Attachment();

    void set (attachmentType _type, float time, Kart *previous_kart=NULL);
    void set (attachmentType _type) { set(_type, m_time_left); }
    void clear ()                   {
                                      m_type=ATTACH_NOTHING; 
                                      m_time_left=0.0;
                                      m_holder->select(0);
                                    }
    attachmentType getType () const { return m_type;           }
    float getTimeLeft      () const { return m_time_left;      }
    void setTimeLeft       (float t){ m_time_left = t;         }
    Kart* getPreviousOwner () const { return m_previous_owner; }
    float WeightAdjust     () const {
                                      return m_type==ATTACH_ANVIL
                                          ?stk_config->m_anvil_weight:0.0f;
                                    }

    float AirResistanceAdjust () const {
                                      return m_type==ATTACH_PARACHUTE
                                          ?stk_config->m_parachute_friction:0.0f;
                                    }

    float SpeedAdjust () const      {
                                      return m_type==ATTACH_ANVIL
                                          ?stk_config->m_anvil_speed_factor:1.0f;
                                    }
    /** Randomly selects the new attachment. For a server process, the
     *  attachment can be passed into this function.
        \param item The item that was collected.
        \param random_attachment Optional: only used on the clients, it
                                 specifies the new attachment to use
     */
    void  hitBanana(const Item &item, int random_attachment=-1);
    void  update (float dt);
    void  moveBombFromTo(Kart *from, Kart *to);
};

#endif
