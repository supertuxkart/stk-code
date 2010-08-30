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

/**
  * \defgroup items
  */

#include "irrlicht.h"
using namespace irr;

#include "karts/kart.hpp"

// -----------------------------------------------------------------------------

/**
  * \ingroup items
  */
class Item
{
public:
    /**
      * The list of all items. Important for the switch item function:
      * bubblegum must be the last item (since bubble gum can't be
      * switched with any other item, since it's a different objecct).
      */
    enum ItemType
    {
        ITEM_FIRST,
        ITEM_BONUS_BOX = ITEM_FIRST,
        ITEM_BANANA,
        ITEM_NITRO_BIG,
        ITEM_NITRO_SMALL,
        ITEM_BUBBLEGUM,
        ITEM_LAST = ITEM_BUBBLEGUM,
        ITEM_COUNT,
        ITEM_NONE
    };

private:
    /** Item type. */
    ItemType      m_type;

    /** If the item is switched, this contains the original type.
     *  It is ITEM_NONE if the item is not switched. */
    ItemType      m_original_type;

    /** True if item was collected & is not displayed. */
    bool          m_collected;

    /** Time till a collected item reappears. */
    float         m_time_till_return;

    /** Scene node of this item. */
    scene::IMeshSceneNode *m_node;

    /** Stores the original mesh in order to reset it. */
    scene::IMesh *m_original_mesh;

    /** The original position - saves calls to m_node->getPosition() 
     * and then converting this value to a Vec3. */
    Vec3          m_xyz;

    /** Index in item_manager field. */
    unsigned int  m_item_id;

    /** Set to false if item should not rotate. */
    bool          m_rotate;       
    
    /** Optionally set this if this item was laid by a particular kart. in 
     *  this case the 'm_deactive_time' will also be set - see below. */ 
    const Kart   *m_event_handler;

    /** Optionally if item was placed by a kart, a timer can be used to 
     *  temporarly deactivate collision so a kart is not hit by its own item */
    float         m_deactive_time;

    /** Counts how often an item is used before it disappears. Used for 
     *  bubble gum to make them disappear after a while. A value >0
     *  indicates that the item still exists, =0 that the item can be
     *  deleted, and <0 that the item will never be deleted. */
    int           m_disappear_counter;

    void          setType(ItemType type);
public:
                  Item (ItemType type, const Vec3& xyz, const Vec3& normal,
                        scene::IMesh* mesh, unsigned int item_id);
    virtual       ~Item ();
    void          update  (float delta);
    virtual void  collected(const Kart *kart, float t=2.0f);
    
    // ------------------------------------------------------------------------
    /** Returns true if the Kart is close enough to hit this item, and
     *  the item is not deactivated anymore.
     *  \param kart Kart to test.
     */
    bool hitKart (Kart* kart ) const
    {
        return (m_event_handler!=kart || m_deactive_time <=0) &&
               (kart->getXYZ()-m_xyz).length2()<0.8f;
    }   // hitKart

    // ------------------------------------------------------------------------
    /** Returns the index of this item in the item manager list. */
    unsigned int  getItemId()    const { return m_item_id;  }
    // ------------------------------------------------------------------------
    /** Returns the type of this item. */
    ItemType      getType()      const { return m_type;     }
    // ------------------------------------------------------------------------
    /** Returns true if this item is currently collected. */
    bool          wasCollected() const { return m_collected;}    
    // ------------------------------------------------------------------------
    /** Returns true if this item is used up and can be removed. */
    bool          isUsedUp()     const {return m_disappear_counter==0; }
    // ------------------------------------------------------------------------
    /** Returns true if this item can be used up, and therefore needs to
     *  be removed when the game is reset. */
    bool          canBeUsedUp()  const {return m_disappear_counter>-1; }
    // ------------------------------------------------------------------------
    /** Sets how long an item should be disabled. While item itself sets
     *  a default, this time is too short in case that a kart that has a bomb
     *  hits a banana: by the time the explosion animation is ended and the
     *  kart is back at its original position, the banana would be back again
     *  and therefore hit the kart again. See Attachment::hitBanana for more
     *  details. 
     *  \param f Time till the item can be used again. 
     */
    void          setDisableTime(float f) { m_time_till_return = f; }
    // ------------------------------------------------------------------------
    /** Returns the time the item is disabled for. */
    float         getDisableTime() const { return m_time_till_return; }
    // ------------------------------------------------------------------------
    void          setParent(Kart* parent);
    void          reset();
    void          switchTo(ItemType type, scene::IMesh *mesh);
    void          switchBack();
};   // class Item

#endif
