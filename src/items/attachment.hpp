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

#ifndef HEADER_ATTACHMENT_HPP
#define HEADER_ATTACHMENT_HPP

#include "config/stk_config.hpp"
#include "utils/random_generator.hpp"

class Kart;
class Item;

// Some loop in attachment.cpp depend on ATTACH_FIRST and ATTACH_MAX.
// So if new elements are added, make sure to add them in between those values.
// Also, please note that Attachment::Attachment relies on ATTACH_FIRST being 0.
enum attachmentType
{
    ATTACH_FIRST = 0,
    ATTACH_PARACHUTE = 0,
    ATTACH_BOMB,
    ATTACH_ANVIL,
    ATTACH_TINYTUX,
    ATTACH_MAX,
    ATTACH_NOTHING
};

/**
  * \ingroup items
  */
class Attachment
{
private:
    attachmentType  m_type;            //!< attachment type
    Kart           *m_kart;            //!< kart the attachment is attached to
    float           m_time_left;       //!< time left till attachment expires
    float           m_initial_speed;   //!< for parachutes only
    /** Scene node of the attachment, which will be attached to the kart's
     *  scene node. */
    scene::IAnimatedMeshSceneNode 
                   *m_node;
    Kart           *m_previous_owner;  //!< used by bombs so that it's not passed
                                       //!< back to previous owner
    RandomGenerator m_random;
public:
         Attachment(Kart* _kart);
        ~Attachment();
    void set (attachmentType _type, float time, Kart *previous_kart=NULL);
    void set (attachmentType _type) { set(_type, m_time_left); }
    void clear ();
    attachmentType getType () const { return m_type;           }
    float getTimeLeft      () const { return m_time_left;      }
    void setTimeLeft       (float t){ m_time_left = t;         }
    Kart* getPreviousOwner () const { return m_previous_owner; }

    float weightAdjust     () const {
                                      return m_type==ATTACH_ANVIL
                                          ?stk_config->m_anvil_weight:0.0f;
                                    }

    float airResistanceAdjust () const {
                                      return m_type==ATTACH_PARACHUTE
                                          ?stk_config->m_parachute_friction:0.0f;
                                    }

    float speedAdjust () const      {
                                      return m_type==ATTACH_ANVIL
                                          ?stk_config->m_anvil_speed_factor:1.0f;
                                    }

    /** Randomly selects the new attachment. For a server process, the
     *  attachment can be passed into this function.
     *  \param item The item that was collected.
     *  \param new_attachment Optional: only used on the clients, it
     *                        specifies the new attachment to use
     */
    void  hitBanana(Item *item, int new_attachment=-1);
    void  update (float dt);
    void  moveBombFromTo(Kart *from, Kart *to);
};

#endif
