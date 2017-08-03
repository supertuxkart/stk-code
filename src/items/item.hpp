//  SuperTuxKart - a fun racing game with go-kart
//
//  Copyright (C) 2004-2015 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006-2015 SuperTuxKart-Team
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
  * Defines the various collectibles and weapons of STK.
  */


#include "utils/leak_check.hpp"
#include "utils/no_copy.hpp"
#include "utils/vec3.hpp"

#include <line3d.h>

class AbstractKart;
class Item;
class LODNode;

namespace irr
{
    namespace scene { class IMesh; class ISceneNode; }
}
using namespace irr;

// -----------------------------------------------------------------------------

/**
 * \ingroup items
 * \brief Listener class to go with Items of type ITEM_TRIGGER
 */
class TriggerItemListener
{
public:
    virtual ~TriggerItemListener() {}
    virtual void onTriggerItemApproached() = 0;
};

/**
  * \ingroup items
  */
class Item : public NoCopy
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
        ITEM_BUBBLEGUM_NOLOK,

        /** For easter egg mode only. */
        ITEM_EASTER_EGG,
        /** An invisible item that can be used to trigger some behavior when
          * approaching a point
          */
        ITEM_TRIGGER,
        ITEM_LAST = ITEM_TRIGGER,
        ITEM_COUNT,
        ITEM_NONE
    };

private:
    LEAK_CHECK();

    /** Item type. */
    ItemType      m_type;

    /** If the item is switched, this contains the original type.
     *  It is ITEM_NONE if the item is not switched. */
    ItemType      m_original_type;

    /** Stores the original rotation of an item. This is used in
     *  case of a switch to restore the rotation of a bubble gum
     *  (bubble gums don't rotate, but it will be replaced with
     *  a nitro which rotates, and so overwrites the original
     *  rotation). */
    btQuaternion  m_original_rotation;

    /** Used when rotating the item */
    float         m_rotation_angle;

    /** True if item was collected & is not displayed. */
    bool          m_collected;

    /** Time till a collected item reappears. */
    float         m_time_till_return;

    /** Scene node of this item. */
    LODNode *m_node;

    /** Stores the original mesh in order to reset it. */
    scene::IMesh *m_original_mesh;
    scene::IMesh *m_original_lowmesh;

    /** The original position - saves calls to m_node->getPosition()
     * and then converting this value to a Vec3. */
    Vec3          m_xyz;

    /** Index in item_manager field. */
    unsigned int  m_item_id;

    /** Set to false if item should not rotate. */
    bool          m_rotate;

    /** Optionally set this if this item was laid by a particular kart. in
     *  this case the 'm_deactive_time' will also be set - see below. */
    const AbstractKart   *m_event_handler;

    /** Kart that emitted this item if any */
    const AbstractKart   *m_emitter;

    /** Optionally if item was placed by a kart, a timer can be used to
     *  temporarly deactivate collision so a kart is not hit by its own item */
    float         m_deactive_time;

    /** Counts how often an item is used before it disappears. Used for
     *  bubble gum to make them disappear after a while. A value >0
     *  indicates that the item still exists, =0 that the item can be
     *  deleted, and <0 that the item will never be deleted. */
    int           m_disappear_counter;

    /** callback used if type == ITEM_TRIGGER */
    TriggerItemListener* m_listener;

    /** square distance at which item is collected */
    float         m_distance_2;

    /** The graph node this item is on. */
    int           m_graph_node;

    /** Distance from the center of the quad this item is in. This value is
     *  >0 if it is to the right of the center, and undefined if this quad
     *  is not on any quad. */
    float         m_distance_from_center;

    /** The closest point to the left and right of this item at which it
     *  would not be collected. Used by the AI to avoid items. */
    Vec3          *m_avoidance_points[2];


    void          initItem(ItemType type, const Vec3 &xyz);
    void          setType(ItemType type);

public:
                  Item(ItemType type, const Vec3& xyz, const Vec3& normal,
                       scene::IMesh* mesh, scene::IMesh* lowres_mesh);
                  Item(const Vec3& xyz, float distance,
                       TriggerItemListener* trigger);
    virtual       ~Item ();
    void          update  (float delta);
    virtual void  collected(const AbstractKart *kart, float t=2.0f);
    void          setParent(AbstractKart* parent);
    void          reset();
    void          switchTo(ItemType type, scene::IMesh *mesh, scene::IMesh *lowmesh);
    void          switchBack();

    const AbstractKart* getEmitter() const { return m_emitter; }

    // ------------------------------------------------------------------------
    /** Returns true if the Kart is close enough to hit this item, the item is
     *  not deactivated anymore, and it wasn't placed by this kart (this is
     *  e.g. used to avoid that a kart hits a bubble gum it just dropped).
     *  \param kart Kart to test.
     *  \param xyz Location of kart (avoiding to use kart->getXYZ() so that
     *         kart.hpp does not need to be included here).
     */
    bool hitKart(const Vec3 &xyz, const AbstractKart *kart=NULL) const
    {
        if (m_event_handler == kart && m_deactive_time > 0)
            return false;
        Vec3 lc = quatRotate(m_original_rotation, xyz - m_xyz);
        // Don't be too strict if the kart is a bit above the item
        lc.setY(lc.getY() / 2.0f);
        return lc.length2() < m_distance_2;
    }   // hitKart

protected:
    // ------------------------------------------------------------------------
    // Some convenient functions for the AI only
    friend class SkiddingAI;
    friend class TestAI;
    /** Returns true if the specified line segment would come close enough
     *  to this item so that this item would be collected.
     *  \param line The line segment which is tested if it is close enough
     *         to this item so that this item would be collected.
     */
    bool hitLine(const core::line3df &line,
                  const AbstractKart *kart=NULL) const
    {
        if(m_event_handler==kart && m_deactive_time >0) return false;

        Vec3 closest = line.getClosestPoint(m_xyz.toIrrVector());
        return hitKart(closest, kart);
    }   // hitLine

public:
    // ------------------------------------------------------------------------
    /** Sets the index of this item in the item manager list. */
    void          setItemId(unsigned int n)  { m_item_id = n; }
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
    /** Returns the XYZ position of the item. */
    const Vec3&   getXYZ() const { return m_xyz; }
    // ------------------------------------------------------------------------
    /** Returns the index of the graph node this item is on. */
    int           getGraphNode() const { return m_graph_node; }
    // ------------------------------------------------------------------------
    /** Returns the distance from center: negative means left of center,
     *  positive means right of center. */
    float getDistanceFromCenter() const { return m_distance_from_center; }
    // ------------------------------------------------------------------------
    /** Returns a point to the left or right of the item which will not trigger
     *  a collection of this item.
     *  \param left If true, return a point to the left, else a point to
     *         the right. */
    const Vec3 *getAvoidancePoint(bool left) const
    {
        if(left) return m_avoidance_points[0];
        return m_avoidance_points[1];
    }   // getAvoidancePoint

    // ------------------------------------------------------------------------
    scene::ISceneNode *getSceneNode()
    {
        return (scene::ISceneNode *) m_node;
    }
};   // class Item

#endif
