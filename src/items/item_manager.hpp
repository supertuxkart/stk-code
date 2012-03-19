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

private:
    /** The vector of all items of the current track. */
    typedef std::vector<Item*> AllItemTypes;
    AllItemTypes m_all_items;

    /** This stores all item models. */
    scene::IMesh *m_item_mesh[Item::ITEM_LAST-Item::ITEM_FIRST+1];
    scene::IMesh *m_item_lowres_mesh[Item::ITEM_LAST-Item::ITEM_FIRST+1];

    /** Stores all meshes for all items. */
    std::map<std::string,scene::IMesh*> m_all_meshes;
    std::map<std::string,scene::IMesh*> m_all_low_meshes;

    std::string m_user_filename;

    /** What item is item is switched to. */
    std::vector<Item::ItemType> m_switch_to;

    /** Remaining time that items should remain switched. If the
     *  value is <0, it indicates that the items are not switched atm. */
    float m_switch_time;

public:
                   ItemManager();
                  ~ItemManager();
    void           loadDefaultItems();
    Item*          newItem         (Item::ItemType type, const Vec3& xyz, 
                                    const Vec3 &normal, 
                                    AbstractKart* parent=NULL);
    Item*          newItem         (const Vec3& xyz, float distance, 
                                    TriggerItemListener* listener);
    void           update          (float delta);
    void           checkItemHit    (AbstractKart* kart);
    void           cleanup         ();
    void           reset           ();
    void           removeTextures  ();
    void           setUserFilename (char *s) {m_user_filename=s;}
    void           collectedItem   (int item_id, AbstractKart *kart,
                                    int add_info=-1);
    void           switchItems     ();
    void           setSwitchItems(const std::vector<int> &switch_items);
    scene::IMesh*  getItemModel    (Item::ItemType type)
                                      {return m_item_mesh[type];}
};

extern ItemManager* item_manager;

#endif
