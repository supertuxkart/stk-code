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

#ifndef HEADER_ITEMMANAGER_HPP
#define HEADER_ITEMMANAGER_HPP

#include "LinearMath/btTransform.h"

#include "items/item.hpp"
#include "utils/aligned_array.hpp"
#include "utils/no_copy.hpp"
#include "utils/vec3.hpp"

#include <SColor.h>

#include <assert.h>
#include <algorithm>

#include <map>
#include <memory>
#include <random>
#include <string>
#include <vector>

class Kart;
class STKPeer;

/**
  * \ingroup items
  */
class ItemManager : public NoCopy
{
    // Some static data and functions to initialise it:
private:
    /** Stores the glow color for all items. */
    static std::vector<video::SColorf> m_glow_color;

    /** Disable item collection (for debugging purposes). */
    static bool m_disable_item_collection;

    static std::mt19937 m_random_engine;

    static uint32_t m_random_seed;

    static bool preloadIcon(const std::string& name);
public:
    static void loadDefaultItemMeshes();
    static void removeTextures();
    static void updateRandomSeed(uint32_t seed_number)
    {
        m_random_engine.seed(seed_number);
        m_random_seed = seed_number;
    }   // updateRandomSeed
    // ------------------------------------------------------------------------
    static uint32_t getRandomSeed()
    {
        return m_random_seed;
    }   // getRandomSeed

    // ------------------------------------------------------------------------

    /** Disable item collection, useful to test client mispreditions or
     *  client/server disagreements. */
    static void disableItemCollection()
    {
        m_disable_item_collection = true;
    }   // disableItemCollection

    // ------------------------------------------------------------------------
    /** Returns the mesh for a certain item. */
    static scene::IMesh* getItemModel(ItemState::ItemType type)
                                      { return m_item_mesh[type]; }
    // ------------------------------------------------------------------------
    /** Returns the low resolution mesh for a certain item. */
    static scene::IMesh* getItemLowResolutionModel(ItemState::ItemType type)
                                      { return m_item_lowres_mesh[type]; }
    // ------------------------------------------------------------------------
    /** Returns the mesh for a certain item. */
    static std::string getIcon(ItemState::ItemType type)
                                      { return m_icon[type]; }
    // ------------------------------------------------------------------------
    /** Returns the glow color for an item. */
    static video::SColorf& getGlowColor(ItemState::ItemType type)
                                      { return m_glow_color[type]; }

    // ========================================================================
protected:
    /** The vector of all items of the current track. */
    typedef std::vector<ItemState*> AllItemTypes;
    AllItemTypes m_all_items;

    /** What item this item is switched to. */
    std::vector<ItemState::ItemType> m_switch_to;

private:
    /** Stores which items are on which quad. m_items_in_quads[#quads]
     *  contains all items that are not on a quad. Note that this
     *  field is undefined if no Graph exist, e.g. arena without navmesh. */
    std::vector< AllItemTypes > *m_items_in_quads;

    /** Stores all item models. */
    static std::vector<scene::IMesh *> m_item_mesh;

    /** Stores all low-resolution item models. */
    static std::vector<scene::IMesh *> m_item_lowres_mesh;

    /** Stores all item models. */
    static std::vector<std::string> m_icon;

protected:
    /** Remaining time that items should remain switched. If the
     *  value is <0, it indicates that the items are not switched atm. */
    int m_switch_ticks;

    void deleteItem(ItemState *item);
    void switchItemsInternal(std::vector < ItemState*> &all_items);
    void setSwitchItems(const std::vector<int> &switch_items);
    void insertItemInQuad(Item *item);
    void deleteItemInQuad(ItemState *item);
public:
             ItemManager();
    virtual ~ItemManager();

    virtual Item*  placeItem       (ItemState::ItemType type, const Vec3& xyz,
                                    const Vec3 &normal);
    virtual Item*  dropNewItem     (ItemState::ItemType type,
                                    const AbstractKart* parent,
                                    const Vec3 *server_xyz = NULL,
                                    const Vec3 *normal = NULL);
    void           update          (int ticks);
    void           updateGraphics  (float dt);
    void           checkItemHit    (AbstractKart* kart);
    void           reset           ();
    virtual void   collectedItem   (ItemState *item, AbstractKart *kart);
    virtual void   switchItems     ();
    bool           randomItemsForArena(const AlignedArray<btTransform>& pos);

    // ------------------------------------------------------------------------
    /** Returns true if the items are switched atm. */
    bool           areItemsSwitched() { return (m_switch_ticks > 0); }
    // ------------------------------------------------------------------------
    /** Only used in the NetworkItemManager. */
    virtual void setItemConfirmationTime(std::weak_ptr<STKPeer> peer,
                                         int ticks)
    {
        assert(false);
    }
    // ------------------------------------------------------------------------
    /** Returns the number of items. */
    unsigned int   getNumberOfItems() const
    {
        return (unsigned int) m_all_items.size();
    }
    // ------------------------------------------------------------------------
    /** Returns a pointer to the n-th item. */
    const ItemState* getItem(unsigned int n) const
    { 
        return dynamic_cast<Item*>(m_all_items[n]);
    };
    // ------------------------------------------------------------------------
    /** Returns a pointer to the n-th item. */
    ItemState* getItem(unsigned int n)
    {
        return dynamic_cast<Item*>(m_all_items[n]);
    }
    // ------------------------------------------------------------------------
    bool itemExists(const ItemState* is) const
    {
        if (!is)
            return false;
        auto it = std::find(m_all_items.begin(), m_all_items.end(), is);
        return it != m_all_items.end();
    }
    // ------------------------------------------------------------------------
    /** Returns a reference to the array of all items on the specified quad.
     */
    const AllItemTypes& getItemsInQuads(unsigned int n) const
    {
        assert(m_items_in_quads);
        assert(n<(*m_items_in_quads).size());
        return (*m_items_in_quads)[n];
    }   // getItemsInQuads
    // ------------------------------------------------------------------------
    /** Returns the first item (NULL if none) on the specified quad
     */
    Item* getFirstItemInQuad(unsigned int n) const
    {
        assert(m_items_in_quads);
        assert(n < m_items_in_quads->size());
        return ((*m_items_in_quads)[n]).empty()
              ? NULL 
             : dynamic_cast<Item*>((*m_items_in_quads)[n].front());
    }   // getFirstItemInQuad
    // ------------------------------------------------------------------------
    unsigned int insertItem(Item *item);
};   // ItemManager

#endif
