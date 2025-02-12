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

/** \defgroup items
 *  Defines the various collectibles and weapons of STK.
 */

#include "utils/cpp2011.hpp"
#include "utils/leak_check.hpp"
#include "utils/log.hpp"
#include "utils/no_copy.hpp"
#include "utils/vec3.hpp"

#include <line3d.h>

class BareNetworkString;
class AbstractKart;
class LODNode;

namespace irr
{
    namespace scene { class IMesh; class ISceneNode; }
}
using namespace irr;

// ============================================================================
/** \ingroup items
 *  Contains the state information of an item, i.e. all non-visual information
 *  only, which also can change (e.g. position and AI information is constant
 *  and therefore not stored here). This class is used as a base class for
 *  item and for networking to save item states.
 */
class ItemState
{
    LEAK_CHECK();
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
        ITEM_LAST = ITEM_EASTER_EGG,
        ITEM_COUNT,
        ITEM_NONE
    };

private:
    /** Item type. */
    ItemType m_type;

    /** If the item is switched, this contains the original type.
    *  It is ITEM_NONE if the item is not switched. */
    ItemType m_original_type;

    /** Time till a collected item reappears. When this value is <=0 this
     *  means that the item is availabe to be collected. When the value is
     *  > 0 it means that the item is not available. */
    int m_ticks_till_return;

    /** Index in item_manager field. This field can also take on a negative
     *  value when used in the NetworkItemManager. */
    int  m_item_id;

    /** Optionally if item was placed by a kart, a timer can be used to
    *  temporarly deactivate collision so a kart is not hit by its own item */
    int m_deactive_ticks;

    /** Counts how often an item is used before it disappears. Used for
     *  bubble gum to make them disappear after a while. A value >0
     *  indicates that the item still exists, =0 that the item can be
     *  deleted, and <0 that the item will never be deleted, i.e. it 
     *  will always reappear after a while. */
    int m_used_up_counter;

    /** The position of this ItemState. */
    Vec3 m_xyz;

    /** The original rotation of the item. While this is technically a visual
     *  only value (atm, it could be used for collision detection), it is
     *  required to make sure a client can display items with the right normal
     *  (in case that a client would get a different (or no) normal from a
     *  raycast).
     */
    btQuaternion m_original_rotation;

    /** The 'owner' of the item, i.e. the kart that dropped this item.
    *  Is NULL if the item is part of the track. */
    const AbstractKart *m_previous_owner;

protected:

    friend class ItemManager;
    friend class NetworkItemManager;
    // ------------------------------------------------------------------------
    virtual void setType(ItemType type) { m_type = type; }
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
                 const AbstractKart *kart = NULL) const
    {
        if (getPreviousOwner() == kart && getDeactivatedTicks() > 0)
            return false;

        Vec3 closest = line.getClosestPoint(getXYZ().toIrrVector());
        return hitKart(closest, kart);
    }   // hitLine

public:
    // ------------------------------------------------------------------------
         ItemState(ItemType type, const AbstractKart *owner=NULL, int id = -1);
    // ------------------------------------------------------------------------
         ItemState(const BareNetworkString& buffer);
    // ------------------------------------------------------------------------
    void initItem(ItemType type, const Vec3& xyz, const Vec3& normal);
    void update(int ticks);
    void setDisappearCounter();
    virtual void collected(const AbstractKart *kart);
    // ------------------------------------------------------------------------
    virtual ~ItemState() {}
         
    // -----------------------------------------------------------------------
    /** Dummy implementation, causing an abort if it should be called to
     *  catch any errors early. */
    virtual void updateGraphics(float dt)
    {
        Log::fatal("ItemState", "updateGraphics() called for ItemState.");
    }   // updateGraphics

    // -----------------------------------------------------------------------
    virtual bool hitKart(const Vec3 &xyz,
                         const AbstractKart *kart = NULL) const
    {
        Log::fatal("ItemState", "hitKart() called for ItemState.");
        return false;
    }   // hitKart

    // -----------------------------------------------------------------------
    virtual int getGraphNode() const 
    {
        Log::fatal("ItemState", "getGraphNode() called for ItemState.");
        return 0;
    }   // getGraphNode

    // -----------------------------------------------------------------------
    virtual const Vec3 *getAvoidancePoint(bool left) const
    {
        Log::fatal("ItemState", "getAvoidancePoint() called for ItemState.");
        // Return doesn't matter, fatal aborts
        return &m_xyz;
    }   // getAvoidancePoint

    // -----------------------------------------------------------------------
    virtual float getDistanceFromCenter() const
    {
        Log::fatal("itemState",
                   "getDistanceFromCentre() called for ItemState.");
        return 0;
    }   // getDistanceFromCentre

    // -----------------------------------------------------------------------
    /** Resets an item to its start state. */
    virtual void reset()
    {
        m_deactive_ticks    = 0;
        m_ticks_till_return = 0;
        setDisappearCounter();
        // If the item was switched:
        if (m_original_type != ITEM_NONE)
        {
            setType(m_original_type);
            m_original_type = ITEM_NONE;
        }
    }   // reset

    // -----------------------------------------------------------------------
    /** Switches an item to be of a different type. Used for the switch
     *  powerup.
     *  \param type New type for this item.
     */
    virtual void switchTo(ItemType type)
    {
        // triggers and easter eggs should not be switched
        if (m_type == ITEM_EASTER_EGG) return;
        m_original_type = m_type;
        setType(type);
        return;
    }   // switchTo

    // ------------------------------------------------------------------------
    /** Returns true if this item was not actually switched (e.g. trigger etc)
     */
    virtual bool switchBack()
    {
        // If the item is not switched, do nothing. This can happen if a bubble
        // gum is dropped while items are switched - when switching back, this
        // bubble gum has no original type.
        if (m_original_type == ITEM_NONE)
            return true;
        setType(m_original_type);
        m_original_type = ITEM_NONE;
        return false;
    }   // switchBack

    // ------------------------------------------------------------------------
    /** Returns if this item is negative, i.e. a banana or bubblegum. */
    bool isNegativeItem() const
    {
        return m_type == ITEM_BANANA || m_type == ITEM_BUBBLEGUM ||
               m_type == ITEM_BUBBLEGUM_NOLOK;
    }
    // ------------------------------------------------------------------------
    /** Sets how long an item should be disabled. While item itself sets
     *  a default, this time is too short in case that a kart that has a bomb
     *  hits a banana: by the time the explosion animation is ended and the
     *  kart is back at its original position, the banana would be back again
     *  and therefore hit the kart again. See Attachment::hitBanana for more
     *  details.
     *  \param f Time till the item can be used again.
     */
    void setTicksTillReturn(int t) { m_ticks_till_return = t; }
    // ------------------------------------------------------------------------
    /** Returns the time the item is disabled for. */
    int getTicksTillReturn() const { return m_ticks_till_return; }
    // ------------------------------------------------------------------------
    /** Returns true if this item is currently collected. */
    bool isAvailable() const { return m_ticks_till_return <= 0; }
    // ------------------------------------------------------------------------
    /** Returns the type of this item. */
    ItemType getType() const { return m_type; }
    // ------------------------------------------------------------------------
    ItemType getGrahpicalType() const;
    // ------------------------------------------------------------------------
    /** Returns the original type of this item. */
    ItemType getOriginalType() const { return m_original_type; }
    // ------------------------------------------------------------------------
    /** Sets the index of this item in the item manager list. */
    void setItemId(unsigned int n) { m_item_id = n; }
    // ------------------------------------------------------------------------
    /** Returns the index of this item in the item manager list. */
    unsigned int getItemId() const { return m_item_id; }
    // ------------------------------------------------------------------------
    /** Returns true if this item is used up and can be removed. */
    bool isUsedUp() const { return m_used_up_counter == 0; }
    // ------------------------------------------------------------------------
    /** Returns true if this item can be used up, and therefore needs to
     *  be removed when the game is reset. */
    bool canBeUsedUp()  const { return m_used_up_counter>-1; }
    // ------------------------------------------------------------------------
    /** Returns the number of ticks during which the item is deactivated (i.e.
     *  it was collected). */
    int getDeactivatedTicks() const { return m_deactive_ticks; }
    // ------------------------------------------------------------------------
    /** Sets the number of ticks during which the item is deactivated (i.e.
     *  it was collected). */
    void setDeactivatedTicks(int ticks) { m_deactive_ticks = ticks; }
    // ------------------------------------------------------------------------
    /** Returns the kart that dropped this item (or NULL if the item was not
     *  dropped by a kart. */
    const AbstractKart *getPreviousOwner() const { return m_previous_owner; }
    // ------------------------------------------------------------------------
    void setXYZ(const Vec3& xyz) { m_xyz = xyz; }
    // ------------------------------------------------------------------------
    /** Returns the XYZ position of the item. */
    const Vec3& getXYZ() const { return m_xyz; }
    // ------------------------------------------------------------------------
    /** Returns the normal of the ItemState. */
    const Vec3 getNormal() const
    {
        return quatRotate(m_original_rotation, Vec3(0.0f, 1.0f, 0.0f));
    }
    // ------------------------------------------------------------------------
    /** Returns the original rotation of the item. */
    const btQuaternion& getOriginalRotation() const
    {
        return m_original_rotation;
    }
    // ------------------------------------------------------------------------
    void saveCompleteState(BareNetworkString* buffer) const;
};   // class ItemState

// ============================================================================
/**
  * \ingroup items
  */
class Item : public ItemState, public NoCopy
{

private:
    /** Scene node of this item. */
    LODNode *m_node;

    /** Graphical type of the mesh. */
    ItemType m_graphical_type;

    /** Vector containing the sparks */
    std::vector<scene::ISceneNode*> m_spark_nodes;

    /** Billboard that shows when the item is about to respawn */
    scene::ISceneNode* m_icon_node;

    /** Stores if the item was available in the previously rendered frame. */
    bool m_was_available_previously;

    /** square distance at which item is collected */
    float m_distance_2;

    /** The graph node this item is on. */
    int m_graph_node;

    /** Distance from the center of the quad this item is in. This value is
     *  >0 if it is to the right of the center, and undefined if this quad
     *  is not on any quad. */
    float m_distance_from_center;

    /** Time ticks since the item last respawned */
    int m_animation_start_ticks;

    /** The closest point to the left and right of this item at which it
     *  would not be collected. Used by the AI to avoid items. */
    Vec3 *m_avoidance_points[2];

    void          initItem(ItemType type, const Vec3 &xyz, const Vec3 &normal);
    void          setMesh(scene::IMesh* mesh, scene::IMesh* lowres_mesh);
    void          handleNewMesh(ItemType type);

public:
                  Item(ItemType type, const Vec3& xyz, const Vec3& normal,
                       scene::IMesh* mesh, scene::IMesh* lowres_mesh,
                       const std::string& icon, const AbstractKart *owner);
    virtual       ~Item ();
    virtual void  updateGraphics(float dt) OVERRIDE;
    virtual void  reset() OVERRIDE;

    //-------------------------------------------------------------------------
    /** Is called when the item is hit by a kart.  It sets the flag that the
     *  item has been collected, and the time to return to the parameter.
     *  \param kart The kart that collected the item.
     */
    virtual void collected(const AbstractKart *kart)  OVERRIDE
    {
        ItemState::collected(kart);
    }   // isCollected
    //-------------------------------------------------------------------------
    /** Switch backs to the original item. Returns true if the item was not
     *  actually switched (e.g. trigger, or bubblegum dropped during switch
     *  time). The return value is not actually used, but necessary in order
     *  to overwrite ItemState::switchBack()
     */
    virtual bool switchBack() OVERRIDE
    {
        if (ItemState::switchBack())
            return true;
        return false;
    }   // switchBack
    // ------------------------------------------------------------------------
    /** Returns true if the Kart is close enough to hit this item, the item is
     *  not deactivated anymore, and it wasn't placed by this kart (this is
     *  e.g. used to avoid that a kart hits a bubble gum it just dropped).
     *  \param kart Kart to test.
     *  \param xyz Location of kart (avoiding to use kart->getXYZ() so that
     *         kart.hpp does not need to be included here).
     */
    virtual bool hitKart(const Vec3 &xyz, const AbstractKart *kart=NULL) const
        OVERRIDE
    {
        if (getPreviousOwner() == kart && getDeactivatedTicks() > 0)
            return false;
        Vec3 lc = quatRotate(getOriginalRotation(), xyz - getXYZ());
        // Don't be too strict if the kart is a bit above the item
        lc.setY(lc.getY() / 2.0f);
        return lc.length2() < m_distance_2;
    }   // hitKart
    // ------------------------------------------------------------------------
    bool rotating() const               { return getType() != ITEM_BUBBLEGUM; }

public:
    // ------------------------------------------------------------------------
    /** Returns the index of the graph node this item is on. */
    virtual int getGraphNode() const OVERRIDE { return m_graph_node; }
    // ------------------------------------------------------------------------
    /** Returns the distance from center: negative means left of center,
     *  positive means right of center. */
    virtual float getDistanceFromCenter() const OVERRIDE
    {
        return m_distance_from_center;
    }   // getDistanceFromCenter
    // ------------------------------------------------------------------------
    /** Returns a point to the left or right of the item which will not trigger
     *  a collection of this item.
     *  \param left If true, return a point to the left, else a point to
     *         the right. */
    virtual const Vec3 *getAvoidancePoint(bool left) const OVERRIDE
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
