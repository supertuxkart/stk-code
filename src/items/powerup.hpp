//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 Joerg Henrichs
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

#ifndef HEADER_POWERUP_HPP
#define HEADER_POWERUP_HPP

#define MAX_POWERUPS 5

#include "items/powerup_manager.hpp"  // needed for powerup_type
#include "utils/no_copy.hpp"

#include <set>

class AbstractKart;
class BareNetworkString;
class ItemState;
class SFXBase;

/**
  * \ingroup items
  */
class Powerup : public NoCopy
{
private:
    /** Sound effect that is being played. */
    SFXBase                    *m_sound_use;

    /** The powerup type. */
    PowerupManager::PowerupType m_type;

    /** Number of collected powerups. */
    int                         m_number;

    /** The owner (kart) of this powerup. */
    AbstractKart*               m_kart;

    std::set<int>               m_played_sound_ticks;

public:
                    Powerup      (AbstractKart* kart_);
                   ~Powerup      ();
    void            set          (PowerupManager::PowerupType _type, int n = 1);
    void            setNum       (int n = 1);
    void            reset        ();
    Material*       getIcon      () const;
    void            adjustSound  ();
    void            use          ();
    void            hitBonusBox (const ItemState &item);
    void            saveState(BareNetworkString *buffer) const;
    void            rewindTo(BareNetworkString *buffer);
    void            update(int ticks);

    /** Returns the number of powerups. */
    int             getNum       () const {return m_number;}
    // ------------------------------------------------------------------------
    /** Returns the type of this powerup. */
    PowerupManager::PowerupType
                    getType      () const {return m_type;  }
    // ------------------------------------------------------------------------
};

#endif
