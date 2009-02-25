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

#ifndef HEADER_ITEMMANAGER_H
#define HEADER_ITEMMANAGER_H


#include <vector>
#include <map>
#include "items/item.hpp"
#include "lisp/lisp.hpp"

class Kart;
class ssgEntity;

class ItemManager
{

private:
    // The vector of all items of the current track
    typedef std::vector<Item*> AllItemTypes;
    AllItemTypes m_all_items;

    // This stores all item models
#ifdef HAVE_IRRLICHT
    // FIXME: why ITEM_SILVER_COINT+1 in plib??
    scene::IMesh *m_item_mesh[Item::ITEM_LAST-Item::ITEM_FIRST+1];
    std::map<std::string,scene::IMesh*> m_all_meshes;
#else
    ssgEntity *m_item_model[Item::ITEM_SILVER_COIN+1];

    // This is the active model. It gets determined by first loading the
    // default, then track models, user models, grand prix models. This means that
    // an item style specified in a track overwrites a command line option.
    std::map<std::string,ssgEntity*> m_all_models;
#endif

    std::string m_user_filename;
    void setDefaultItemStyle();
    void setItem(const lisp::Lisp *item_node, const char *colour,
                 Item::ItemType type);

public:
    ItemManager();
    ~ItemManager();
    void        loadDefaultItems();
    void        loadItemStyle   (const std::string filename);
    Item*       newItem         (Item::ItemType type, const Vec3& xyz, 
                                 const Vec3 &normal, Kart* parent=NULL);
    void        update          (float delta);
    void        hitItem         (Kart* kart);
    void        cleanup         ();
    void        reset           ();
    void        removeTextures  ();
    void        setUserFilename (char *s) {m_user_filename=s;}
    void        collectedItem   (int item_id, Kart *kart,
                                 int add_info=-1);
#ifdef HAVE_IRRLICHT
    scene::IMesh*  getItemModel (Item::ItemType type)
                                {return m_item_mesh[type];}
#else
    ssgEntity*  getItemModel (Item::ItemType type)
                                {return m_item_model[type];}
#endif
};

extern ItemManager* item_manager;


#endif
