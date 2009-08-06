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

#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "io/file_manager.hpp"
#include "items/bubblegumitem.hpp"
#include "karts/kart.hpp"
#include "network/network_manager.hpp"
#include "utils/string_utils.hpp"

ItemManager* item_manager;

//-----------------------------------------------------------------------------
typedef std::map<std::string,scene::IMesh*>::const_iterator CI_type;

ItemManager::ItemManager()
{
    m_all_meshes.clear();
    // The actual loading is done in loadDefaultItems
}   // ItemManager

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
        // FIXME: What about this plib comment:
        // We can't use ssgDeRefDelete here, since then the object would be
        // freed, and when m_all_models is deleted, we have invalid memory
        // accesses.
        i->second->drop();
    }
    m_all_meshes.clear();
}   // ~ItemManager

//-----------------------------------------------------------------------------
void ItemManager::loadDefaultItems()
{
    // Load all models. This can't be done in the constructor, 
    // since the file_manager isn't ready at that stage.
    // -------------------------------------------------------
    std::set<std::string> files;
    file_manager->listFiles(files, file_manager->getItemsDir(),
                            /*is_full_path*/true, 
                            /*make_full_path*/true);
    for(std::set<std::string>::iterator i  = files.begin();
            i != files.end();  ++i)
    {
        if(StringUtils::extension(*i)!="b3d") continue;
        scene::IMesh *mesh = irr_driver->getAnimatedMesh(*i);
        if(!mesh) continue;
        std::string shortName = StringUtils::basename(StringUtils::without_extension(*i));
        m_all_meshes[shortName] = mesh;
        mesh->grab();
    }   // for i

    setDefaultItemStyle();
}   // loadDefaultItems

//-----------------------------------------------------------------------------
void ItemManager::setDefaultItemStyle()
{
    // FIXME - This should go in an internal, system wide configuration file
    std::string DEFAULT_NAMES[Item::ITEM_LAST - Item::ITEM_FIRST +1];
    DEFAULT_NAMES[Item::ITEM_BONUS_BOX]   = "gift-box";
    DEFAULT_NAMES[Item::ITEM_BANANA]      = "banana";
    DEFAULT_NAMES[Item::ITEM_GOLD_COIN]   = "nitrotank-big";
    DEFAULT_NAMES[Item::ITEM_SILVER_COIN] = "nitrotank-small";
    DEFAULT_NAMES[Item::ITEM_BUBBLEGUM]   = "bubblegum";

    bool bError=0;
    std::ostringstream msg;
    for(int i=Item::ITEM_FIRST; i<=Item::ITEM_LAST; i++)
    {
        m_item_mesh[i] = m_all_meshes[DEFAULT_NAMES[i]];
        if(!m_item_mesh[i])
        {
            msg << "Item model '" << DEFAULT_NAMES[i] 
                << "' is missing (see item_manager)!\n";
            bError=1;
            break;
        }   // if !m_item_model
    }   // for i
    if(bError)
    {
        fprintf(stderr, "The following models are available:\n");
        for(CI_type i=m_all_meshes.begin(); i!=m_all_meshes.end(); ++i)
        {
            if(i->second)
            {
                fprintf(stderr, "   %s in %s.ac.\n",
                    i->first.c_str(),
                    i->first.c_str());
            }  // if i->second
        }
        throw std::runtime_error(msg.str());
        exit(-1);
    }   // if bError

}   // setDefaultItemStyle

//-----------------------------------------------------------------------------
Item* ItemManager::newItem(Item::ItemType type, const Vec3& xyz, 
                           const Vec3 &normal, Kart *parent)
{
    Item* h;
    if(type == Item::ITEM_BUBBLEGUM)
        h = new BubbleGumItem(type, xyz, normal, m_item_mesh[type], 
                              m_all_items.size());
    else
        h = new Item(type, xyz, normal, m_item_mesh[type],
                     m_all_items.size());
    if(parent != NULL) h->setParent(parent);
    
    m_all_items.push_back(h);
    return h;
}   // newItem

//-----------------------------------------------------------------------------
/** Set an item as collected.
 *  This function is called on the server when an item is collected, or on the
 *  client upon receiving information about collected items.                  */
void ItemManager::collectedItem(int item_id, Kart *kart, int add_info)
{
    Item *item=m_all_items[item_id];
    item->collected();
    kart->collectedItem(*item, add_info);
}   // collectedItem

//-----------------------------------------------------------------------------
void  ItemManager::hitItem(Kart* kart)
{
    // Only do this on the server
    if(network_manager->getMode()==NetworkManager::NW_CLIENT) return;

    for(AllItemTypes::iterator i =m_all_items.begin();
        i!=m_all_items.end();  i++)
    {
        if((*i)->wasCollected()) continue;
        if((*i)->hitKart(kart))
        {
            collectedItem(i-m_all_items.begin(), kart);
        }   // if hit
    }   // for m_all_items
}   // hitItem

//-----------------------------------------------------------------------------
/** Remove all item instances, and the track specific models. This is used
 *  just before a new track is loaded and a race is started.
 */
void ItemManager::cleanup()
{
    for(AllItemTypes::iterator i =m_all_items.begin();
        i!=m_all_items.end();  i++)
    {
        delete *i;
    }
    m_all_items.clear();

    setDefaultItemStyle();

    // FIXME - this seems outdated
    
    // Then load the default style from the user_config file
    // -----------------------------------------------------
    // This way if an item is not defined in the item-style-file, the
    // default (i.e. old herring) is used.
    try
    {
        // FIXME: This should go in a system-wide configuration file,
        //        and only one of this and the hard-coded settings in
        //        setDefaultItemStyle are necessary!!!
        loadItemStyle(UserConfigParams::m_item_style);
    }
    catch(std::runtime_error)
    {
        fprintf(stderr,"The item style '%s' in your configuration file does not exist.\nIt is ignored.\n",
                UserConfigParams::m_item_style.c_str());
        UserConfigParams::m_item_style="";
    }

    try
    {
        loadItemStyle(m_user_filename);
    }
    catch(std::runtime_error)
    {
        fprintf(stderr,"The item style '%s' specified on the command line does not exist.\nIt is ignored.\n",
                m_user_filename.c_str());
        m_user_filename="";  // reset to avoid further warnings.
    }

}   // cleanup

//-----------------------------------------------------------------------------
/** Remove all item instances, and the track specific models. This is used
 * just before a new track is loaded and a race is started
 */
void ItemManager::reset()
{
    std::vector<AllItemTypes::iterator> to_delete;
    AllItemTypes::iterator i=m_all_items.begin();
    while(i!=m_all_items.end())
    {
        if((*i)->getType()==Item::ITEM_BUBBLEGUM)
        {
            BubbleGumItem *b=static_cast<BubbleGumItem*>(*i);
            AllItemTypes::iterator i_next = m_all_items.erase(i); 
            delete b;
            i = i_next;
        }
        else
        {
            (*i)->reset();
            i++;
        }
    }  // for i
}   // reset

//-----------------------------------------------------------------------------
void ItemManager::update(float delta)
{
    for(AllItemTypes::iterator i =m_all_items.begin();
        i!=m_all_items.end();  i++)
    {
        (*i)->update(delta);
    }   // for m_all_items
}   // delta

//-----------------------------------------------------------------------------
void ItemManager::loadItemStyle(const std::string filename)
{
    if(filename.length()==0) return;
    const lisp::Lisp* root = 0;
    lisp::Parser parser;
    
    root = parser.parse(file_manager->getConfigFile(filename + ".items"));

    const lisp::Lisp* item_node = root->getLisp("item");
    if(!item_node)
    {
        std::ostringstream msg;
        msg << "Couldn't load map '" << filename << "': no item node.";
        delete root;
        throw std::runtime_error(msg.str());
        delete root;
    }
    setItem(item_node, "red",   Item::ITEM_BONUS_BOX   );
    setItem(item_node, "green", Item::ITEM_BANANA );
    setItem(item_node, "gold"  ,Item::ITEM_GOLD_COIN  );
    setItem(item_node, "silver",Item::ITEM_SILVER_COIN);
    delete root;
}   // loadItemStyle

//-----------------------------------------------------------------------------
void ItemManager::setItem(const lisp::Lisp *item_node,
                          const char *colour, Item::ItemType type)
{
    std::string name;
    item_node->get(colour, name);
    if(name.size()>0)
    {
        m_item_mesh[type]=m_all_meshes[name];
    }
}   // setItem
