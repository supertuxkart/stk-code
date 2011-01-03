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

#include "items/item_manager.hpp"

#include <stdexcept>
#include <string>
#include <sstream>

#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "io/file_manager.hpp"
#include "karts/kart.hpp"
#include "network/network_manager.hpp"
#include "tracks/track.hpp"
#include "utils/string_utils.hpp"

ItemManager* item_manager;

//-----------------------------------------------------------------------------
typedef std::map<std::string,scene::IMesh*>::const_iterator CI_type;

ItemManager::ItemManager()
{
    m_switch_time = -1.0f;
    m_all_meshes.clear();
    // The actual loading is done in loadDefaultItems

    // Prepare the switch to array, which stores which item should be
    // switched to what other item. Initialise it with a mapping that
    // each item is switched to itself, so basically a no-op.
    m_switch_to.reserve(Item::ITEM_COUNT);
    for(unsigned int i=Item::ITEM_FIRST; i<Item::ITEM_COUNT; i++)
        m_switch_to.push_back((Item::ItemType)i);
    setSwitchItems(stk_config->m_switch_items);
}   // ItemManager

//-----------------------------------------------------------------------------
/** Sets which objects is getting switched to what.
 *  \param switch A mapping of items types to item types for the mapping.
 *         must contain one entry for each item.
 */
void ItemManager::setSwitchItems(const std::vector<int> &switch_items)
{
    for(unsigned int i=Item::ITEM_FIRST; i<Item::ITEM_COUNT; i++)
        m_switch_to[i]=(Item::ItemType)stk_config->m_switch_items[i];
}   // setSwitchItems

//-----------------------------------------------------------------------------
void ItemManager::removeTextures()
{
    for(AllItemTypes::iterator i =m_all_items.begin();
        i!=m_all_items.end();  i++)
    {
        delete *i;
    }
    m_all_items.clear();

    for(CI_type i=m_all_meshes.begin(); i!=m_all_meshes.end(); ++i)
    {
        i->second->drop();
    }
    m_all_meshes.clear();
}   // removeTextures

//-----------------------------------------------------------------------------
ItemManager::~ItemManager()
{
    for(CI_type i=m_all_meshes.begin(); i!=m_all_meshes.end(); ++i)
    {
        i->second->drop();
    }
    m_all_meshes.clear();
}   // ~ItemManager

//-----------------------------------------------------------------------------
void ItemManager::loadDefaultItems()
{
    // The names must be given in the order of the definition of ItemType
    // in item.hpp. Note that bubblegum strictly isn't an item,
    // it is implemented as one, and so loaded here, too.
    static const std::string item_names[] = {"bonus-box", "banana",
                                             "nitro-big", "nitro-small", 
                                             "bubblegum" };
    const std::string file_name = file_manager->getDataFile("items.xml");
    const XMLNode *root         = file_manager->createXMLTree(file_name);
    for(unsigned int i=Item::ITEM_FIRST; i<=Item::ITEM_LAST; i++)
    {
        const XMLNode *node = root->getNode(item_names[i]);
        std::string model_filename;
        if (node)
            node->get("model", &model_filename);
        scene::IMesh *mesh = irr_driver->getAnimatedMesh(model_filename);
        if(!node || model_filename.size()==0 || !mesh)
        {
            fprintf(stderr, "Item model '%s' in items.xml could not be loaded - aborting",
                    item_names[i].c_str());
            exit(-1);
        }
        // If lighting would be enabled certain items (esp. bananas)
        // don't look smooth, so for now generally disable lighting
        // FIXME : re-export models with normals instead
        mesh->setMaterialFlag(video::EMF_LIGHTING, false);
        mesh->setMaterialFlag(video::EMF_FOG_ENABLE, true);
        std::string shortName = 
            StringUtils::getBasename(StringUtils::removeExtension(model_filename));
        m_all_meshes[shortName] = mesh;
        m_item_mesh[i]          = mesh;
        mesh->grab();
    }   // for i
    delete root;
}   // loadDefaultItems

//-----------------------------------------------------------------------------
/** Creates a new item.
 *  \param type Type of the item.
 *  \param xyz  Position of the item.
 *  \param normal The normal of the terrain to set roll and pitch.
 *  \param parent In case of a dropped item used to avoid that a kart
 *         is affected by its own items.
 */
Item* ItemManager::newItem(Item::ItemType type, const Vec3& xyz, 
                           const Vec3 &normal, Kart *parent)
{
    // Find where the item can be stored in the index list: either in a
    // previously deleted entry, otherwise at the end.
    int index = -1;
    for(index=m_all_items.size()-1; index>=0 && m_all_items[index]; index--) {}

    if(index==-1) index = m_all_items.size();
    Item* item;
    item = new Item(type, xyz, normal, m_item_mesh[type], index);
    if(parent != NULL) item->setParent(parent);
    if(m_switch_time>=0)
    {
        Item::ItemType new_type = m_switch_to[item->getType()];
        item->switchTo(new_type, m_item_mesh[(int)new_type]);
    }
    if(index<(int)m_all_items.size())
        m_all_items[index] = item;
    else
        m_all_items.push_back(item);

    return item;
}   // newItem

//-----------------------------------------------------------------------------
/** Set an item as collected.
 *  This function is called on the server when an item is collected, or on the
 *  client upon receiving information about collected items.                  */
void ItemManager::collectedItem(int item_id, Kart *kart, int add_info)
{
    Item *item=m_all_items[item_id];
    assert(item);
    item->collected(kart);
    kart->collectedItem(item, add_info);
}   // collectedItem

//-----------------------------------------------------------------------------
/** Checks if any item was collected by the given kart. This function calls
 *  collectedItem if an item was collected.
 *  \param kart Pointer to the kart. 
 */
void  ItemManager::checkItemHit(Kart* kart)
{
    // Only do this on the server
    if(network_manager->getMode()==NetworkManager::NW_CLIENT) return;

    for(AllItemTypes::iterator i =m_all_items.begin();
        i!=m_all_items.end();  i++)
    {
        if((!*i) || (*i)->wasCollected()) continue;
        if((*i)->hitKart(kart))
        {
            collectedItem(i-m_all_items.begin(), kart);
        }   // if hit
    }   // for m_all_items
}   // checkItemHit

//-----------------------------------------------------------------------------
/** Remove all item instances, and the track specific models. This is used
 *  just before a new track is loaded and a race is started.
 */
void ItemManager::cleanup()
{
    for(AllItemTypes::iterator i =m_all_items.begin();
        i!=m_all_items.end();  i++)
    {
        if(*i)
            delete *i;
    }
    m_all_items.clear();
}   // cleanup

//-----------------------------------------------------------------------------
/** Resets all items and removes bubble gum that is stuck on the track.
 *  This is necessary in case that a race is restarted.
 */
void ItemManager::reset()
{
    // If items are switched, switch them back first.
    if(m_switch_time>=0)
    {
        for(AllItemTypes::iterator i =m_all_items.begin(); 
                                   i!=m_all_items.end(); i++)
        {
            if(*i) (*i)->switchBack();
        }
        m_switch_time = -1.0f;

    }
    AllItemTypes::iterator i=m_all_items.begin();
    while(i!=m_all_items.end())
    {
        if(!*i) 
        {
            i++;
            continue;
        }
        if((*i)->canBeUsedUp() || (*i)->getType()==Item::ITEM_BUBBLEGUM)
        {
            Item *b=*i;
            AllItemTypes::iterator i_next = m_all_items.erase(i); 
            delete b;
            i = i_next;
        }
        else
        {
            (*i)->reset();
            i++;
        }
    }  // whilem_all_items.end() i

    m_switch_time = -1;
}   // reset

//-----------------------------------------------------------------------------
void ItemManager::update(float dt)
{
    // If switch time is over, switch all items back
    if(m_switch_time>=0)
    {
        m_switch_time -= dt;
        if(m_switch_time<0)
        {
            for(AllItemTypes::iterator i =m_all_items.begin();
                i!=m_all_items.end();  i++)
            {   
                if(*i) (*i)->switchBack();
            }   // for m_all_items
        }   // m_switch_time < 0
    }   // m_switch_time>=0

    for(AllItemTypes::iterator i =m_all_items.begin();
        i!=m_all_items.end();  i++)
    {
        if(*i)
        {
            (*i)->update(dt);
            if( (*i)->isUsedUp())
            {
                delete *i;
                m_all_items[i-m_all_items.begin()] = NULL;
            }   // if usedUp
        }   // if *i
    }   // for m_all_items
}   // update

//-----------------------------------------------------------------------------
/** Switches all items: boxes become bananas and vice versa for a certain
 *  amount of time (as defined in stk_config.xml.
 */
void ItemManager::switchItems()
{
    for(AllItemTypes::iterator i =m_all_items.begin();
        i!=m_all_items.end();  i++)
    {
        if(!*i) continue;
        Item::ItemType new_type = m_switch_to[(*i)->getType()];

        if(m_switch_time<0)
            (*i)->switchTo(new_type, m_item_mesh[(int)new_type]);
        else
            (*i)->switchBack();
    }   // for m_all_items

    // if the items are already switched (m_switch_time >=0)
    // then switch back, and set m_switch_time to -1 to indicate
    // that the items are now back to normal.
    m_switch_time = m_switch_time < 0 ? stk_config->m_item_switch_time : -1;

}   // switchItems
