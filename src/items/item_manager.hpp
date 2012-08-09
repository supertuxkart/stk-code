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

#ifndef HEADER_ITEMMANAGER_HPP
#define HEADER_ITEMMANAGER_HPP


#include <assert.h>
#include <map>
#include <string>
#include <vector>

#include "items/item.hpp"
#include "utils/no_copy.hpp"

class Kart;

/**
  * \ingroup items
  */
class ItemManager : public NoCopy
{
    // Some static data and functions to initialise it:
private:
    /** Stores all item models. */
    static std::vector<scene::IMesh *> m_item_mesh;
    
    /** Stores all low-resolution item models. */
    static std::vector<scene::IMesh *> m_item_lowres_mesh;

    /** The instance of ItemManager while a race is on. */
    static ItemManager *m_item_manager;
public:
    static void loadDefaultItemMeshes();
    static void removeTextures();
    static void create();
    static void destroy();

    /** Return an instance of the item manager (it does not automatically
     *  create one, call create for that). */
    static ItemManager *get() { 
        assert(m_item_manager); 
        return m_item_manager;
    }   // get
    
    // ========================================================================
private:
    /** The vector of all items of the current track. */
    typedef std::vector<Item*> AllItemTypes;
    AllItemTypes m_all_items;

    /** What item this item is switched to. */
    std::vector<Item::ItemType> m_switch_to;

    /** Remaining time that items should remain switched. If the
     *  value is <0, it indicates that the items are not switched atm. */
    float m_switch_time;

    void  insertItem(Item *item);

    // Make those private so only create/destroy functions can call them.
                   ItemManager();
                  ~ItemManager();
    void           setSwitchItems(const std::vector<int> &switch_items);

public:
    Item*          newItem         (Item::ItemType type, const Vec3& xyz, 
                                    const Vec3 &normal, 
                                    AbstractKart* parent=NULL);
    Item*          newItem         (const Vec3& xyz, float distance, 
                                    TriggerItemListener* listener);
    void           update          (float delta);
    void           checkItemHit    (AbstractKart* kart);
    void           reset           ();
    void           collectedItem   (int item_id, AbstractKart *kart,
                                    int add_info=-1);
    void           switchItems     ();
    // ------------------------------------------------------------------------
    scene::IMesh*  getItemModel    (Item::ItemType type)
                                      {return m_item_mesh[type];}
    // ------------------------------------------------------------------------
    /** Returns the number of items. */
    unsigned int   getNumberOfItems() const { return m_all_items.size(); }
    // ------------------------------------------------------------------------
    /** Returns a pointer to the n-th item. */
    const Item *   getItem(unsigned int n) const { return m_all_items[n]; };
};   // ItemManager

#endif
