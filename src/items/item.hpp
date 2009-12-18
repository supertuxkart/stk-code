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

#ifndef HEADER_ITEM_HPP
#define HEADER_ITEM_HPP

#include "irrlicht.h"
using namespace irr;

#include "karts/kart.hpp"
#include "utils/coord.hpp"

// -----------------------------------------------------------------------------
class Item
{
public:
    enum ItemType
    {
        ITEM_FIRST,
        ITEM_BONUS_BOX = ITEM_FIRST,
        ITEM_BANANA,
        ITEM_GOLD_COIN,
        ITEM_SILVER_COIN,
        ITEM_BUBBLEGUM,
        ITEM_LAST = ITEM_BUBBLEGUM
    };

private:
    ItemType      m_type;         // Item type
    bool          m_collected;        // true if item was collected & is not displayed
    float         m_time_till_return;  // time till a collected item reappears
    Coord         m_coord;        // Original coordinates, used mainly when
                                  // collected items reappear.
    /** Scene node of this item. */
    scene::ISceneNode *m_node;
    unsigned int  m_item_id;      // index in item_manager field

    bool          m_rotate;       // set to false if item should not rotate
    
    /** optionally, set this if this item was laid by a particular kart. in this case,
        the 'm_deactive_time' will also be set - see below. */ 
    Kart*         m_event_handler;
    /** optionally, if item was placed by a kart, a timer can be used to temporarly
       deactivate collision so a kart is not hit by its own item */
    float         m_deactive_time;
    
public:
                  Item (ItemType type, const Vec3& xyz, const Vec3& normal,
                        scene::IMesh* mesh, unsigned int item_id,
                        bool rotate=true);
    virtual       ~Item ();
    void          update  (float delta);
    virtual void  collected(float t=2.0f);
    
    // ------------------------------------------------------------------------
    /** Returns true if the Kart is close enough to hit this item, and
     *  the item is not deactivated anymore.
     *  \param kart Kart to test.
     */
    bool hitKart (Kart* kart ) const
    {
        return m_deactive_time <=0 &&
               (kart->getXYZ()-m_coord.getXYZ()).length2()<0.8f;
    }   // hitKart

    // ------------------------------------------------------------------------
    /** Deactivates the item for a certain amount of time. It is used to
     *  prevent bubble gum from hitting a kart over and over again (in each
     *  frame) by giving it time to drive away.
     *  \param t Time the item is deactivated.
     */
    void          deactivate(float t)  { m_deactive_time=t; }
    // ------------------------------------------------------------------------
    unsigned int  getItemId()    const { return m_item_id;  }
    ItemType      getType()      const { return m_type;     }
    bool          wasCollected() const { return m_collected;}    
    void          setParent(Kart* parent);
    void          reset();
};   // class Item

#endif
