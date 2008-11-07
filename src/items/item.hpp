//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
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

#ifndef HEADER_ITEM_H
#define HEADER_ITEM_H

// num_players triggers  'already defined' messages without the WINSOCKAPI define. Don't ask me :(
#define _WINSOCKAPI_
#include <plib/sg.h>
#include "coord.hpp"

class Kart;
class ssgTransform;
class ssgEntity;

enum ItemType
{
    ITEM_FIRST = -1,
    
    ITEM_BONUS_BOX = 0,
    ITEM_BANANA,
    ITEM_GOLD_COIN,
    ITEM_SILVER_COIN,
    ITEM_BUBBLEGUM,
    
    ITEM_LAST
};

// -----------------------------------------------------------------------------
class Item
{
protected:
    ItemType      m_type;         // Item type
    bool          m_collected;        // true if item was collected & is not displayed
    float         m_time_till_return;  // time till a collected item reappears
    Coord         m_coord;        // Original coordinates, used mainly when
                                  // collected items reappear.
    ssgTransform* m_root;         // The actual root of the item
    unsigned int  m_item_id;      // index in item_manager field

    bool          m_rotate;       // set to false if item should not rotate
    
    Kart*         m_parent;        // optional, if item was placed by a kart, a timer
    float         m_immunity_timer; // can be used so it's not hit by its own item
    
public:
                  Item (ItemType type, const Vec3& xyz, ssgEntity* model,
                           unsigned int item_id);
                 ~Item ();
    unsigned int  getItemId() const {return m_item_id; }
    void          update  (float delta);
    void          isCollected ();
    bool          hitKart (Kart* kart );
    void          reset   ();
    ssgTransform* getRoot () const {return m_root;}
    ItemType      getType () const {return m_type;}
    bool          wasCollected() const {return m_collected;}
    
    void          setParent(Kart* parent);
}
;   // class Item

#endif
