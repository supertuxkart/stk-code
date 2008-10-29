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

#ifndef HEADER_COLLECTABLE_H
#define HEADER_COLLECTABLE_H

#define MAX_COLLECTABLES 5

#include "items/collectable_manager.hpp"  // needed for collectable_type
#include "utils/random_generator.hpp"

class Kart;
class Item;
class SFXBase;

class Collectable
{
private:
    RandomGenerator            m_random;
    SFXBase                   *m_sound_shot;
    SFXBase                   *m_sound_use_anvil;
    SFXBase                   *m_sound_use_parachute;

protected:
    Kart*                      m_owner;
    CollectableType            m_type;
    int                        m_number;

public:
                    Collectable  (Kart* kart_);
                   ~Collectable  ();
    void            set          (CollectableType _type, int n=1);
    void            reset        ();
    int             getNum       () const {return m_number;}
    CollectableType getType      () const {return m_type;  }
    void            hitBonusBox  (int n, const Item &item, int newC=-1);
    Material*       getIcon      ();
    void            use          ();
};

#endif
