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

#include <stdexcept>
#include <string>
#include <sstream>

#include "user_config.hpp"
#include "file_manager.hpp"
#include "loader.hpp"
#include "material_manager.hpp"
#include "material.hpp"
#include "string_utils.hpp"
#include "translation.hpp"
#include "items/item_manager.hpp"
#include "items/bubblegumitem.hpp"
#include "karts/kart.hpp"
#include "network/network_manager.hpp"

#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf _snprintf
#endif
/** Simple shadow class, only used here for default items. */
class Shadow
{
    ssgBranch *sh ;

public:
    Shadow ( float x1, float x2, float y1, float y2 ) ;
    ssgEntity *getRoot () { return sh ; }
}
;   // Shadow

//-----------------------------------------------------------------------------
Shadow::Shadow ( float x1, float x2, float y1, float y2 )
{
    ssgVertexArray   *va = new ssgVertexArray   () ; sgVec3 v ;
    ssgNormalArray   *na = new ssgNormalArray   () ; sgVec3 n ;
    ssgColourArray   *ca = new ssgColourArray   () ; sgVec4 c ;
    ssgTexCoordArray *ta = new ssgTexCoordArray () ; sgVec2 t ;

    sgSetVec4 ( c, 0.0f, 0.0f, 0.0f, 1.0f ) ; ca->add(c) ;
    sgSetVec3 ( n, 0.0f, 0.0f, 1.0f ) ; na->add(n) ;

    sgSetVec3 ( v, x1, y1, 0.10f ) ; va->add(v) ;
    sgSetVec3 ( v, x2, y1, 0.10f ) ; va->add(v) ;
    sgSetVec3 ( v, x1, y2, 0.10f ) ; va->add(v) ;
    sgSetVec3 ( v, x2, y2, 0.10f ) ; va->add(v) ;

    sgSetVec2 ( t, 0.0f, 0.0f ) ; ta->add(t) ;
    sgSetVec2 ( t, 1.0f, 0.0f ) ; ta->add(t) ;
    sgSetVec2 ( t, 0.0f, 1.0f ) ; ta->add(t) ;
    sgSetVec2 ( t, 1.0f, 1.0f ) ; ta->add(t) ;

    sh = new ssgBranch ;
    sh -> clrTraversalMaskBits ( SSGTRAV_ISECT|SSGTRAV_HOT ) ;

    sh -> setName ( "Shadow" ) ;

    ssgVtxTable *gs = new ssgVtxTable ( GL_TRIANGLE_STRIP, va, na, ta, ca ) ;

    gs -> clrTraversalMaskBits ( SSGTRAV_ISECT|SSGTRAV_HOT ) ;
    gs -> setState ( fuzzy_gst ) ;
    sh -> addKid ( gs ) ;
    sh -> ref () ; /* Make sure it doesn't get deleted by mistake */
}   // Shadow

//=============================================================================
ItemManager* item_manager;
typedef std::map<std::string,ssgEntity*>::const_iterator CI_type;

ItemManager::ItemManager()
{
    m_all_models.clear();
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

    for(CI_type i=m_all_models.begin(); i!=m_all_models.end(); ++i)
    {
        ssgDeRefDelete(i->second);
    }
    m_all_models.clear();
    callback_manager->clear(CB_ITEM);
}   // removeTextures

//-----------------------------------------------------------------------------
ItemManager::~ItemManager()
{
    for(CI_type i=m_all_models.begin(); i!=m_all_models.end(); ++i)
    {
        // We can't use ssgDeRefDelete here, since then the object would be
        // freed, and when m_all_models is deleted, we have invalid memory
        // accesses.
        i->second->deRef();
    }
    m_all_models.clear();
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
            if(!StringUtils::has_suffix(*i, ".ac")) continue;
            ssgEntity*  h         = loader->load(*i, CB_ITEM, 
                                                 /*optimise*/true, 
                                                 /*full_path*/true);
            std::string shortName = StringUtils::basename(StringUtils::without_extension(*i));
            h->ref();
            h->setName(shortName.c_str());
            m_all_models[shortName] = h;
        }   // for i


    // Load the old, internal only models
    // ----------------------------------
    sgVec3 yellow = { 1.0f, 1.0f, 0.4f }; createDefaultItem(yellow, "OLD_GOLD"  );
    sgVec3 cyan   = { 0.4f, 1.0f, 1.0f }; createDefaultItem(cyan  , "OLD_SILVER");
    sgVec3 red    = { 0.8f, 0.0f, 0.0f }; createDefaultItem(red   , "OLD_RED"   );
    sgVec3 green  = { 0.0f, 0.8f, 0.0f }; createDefaultItem(green , "OLD_GREEN" );

    setDefaultItemStyle();
}   // loadDefaultItems

//-----------------------------------------------------------------------------
void ItemManager::setDefaultItemStyle()
{
    // This should go in an internal, system wide configuration file
    std::string DEFAULT_NAMES[ITEM_LAST - ITEM_FIRST - 1];
    DEFAULT_NAMES[ITEM_BONUS_BOX]   = "bonusblock";
    DEFAULT_NAMES[ITEM_BANANA]      = "banana";
    DEFAULT_NAMES[ITEM_GOLD_COIN]   = "goldcoin";
    DEFAULT_NAMES[ITEM_SILVER_COIN] = "silvercoin";
    DEFAULT_NAMES[ITEM_BUBBLEGUM]   = "bubblegum";

    bool bError=0;
    char msg[MAX_ERROR_MESSAGE_LENGTH];
    for(int i=ITEM_FIRST+1; i<ITEM_LAST; i++)
    {
        m_item_model[i] = m_all_models[DEFAULT_NAMES[i]];
        if(!m_item_model[i])
        {
            snprintf(msg, sizeof(msg), 
                     "Item model '%s' is missing (see item_manager)!\n",
                     DEFAULT_NAMES[i].c_str());
            bError=1;
            break;
        }   // if !m_item_model
    }   // for i
    if(bError)
    {
        fprintf(stderr, "The following models are available:\n");
        for(CI_type i=m_all_models.begin(); i!=m_all_models.end(); ++i)
        {
            if(i->second)
            {
                if(i->first.substr(0,3)=="OLD")
                {
                    fprintf(stderr,"   %s internally only.\n",i->first.c_str());
                }
                else
                {
                    fprintf(stderr, "   %s in %s.ac.\n",
                            i->first.c_str(),
                            i->first.c_str());
                }
            }  // if i->second
        }
        throw std::runtime_error(msg);
        exit(-1);
    }   // if bError

}   // setDefaultItemStyle

//-----------------------------------------------------------------------------
Item* ItemManager::newItem(ItemType type, const Vec3& xyz, const Vec3 &normal,
                           Kart* parent)
{
    Item* h;
    if(type == ITEM_BUBBLEGUM)
        h = new BubbleGumItem(type, xyz, normal, m_item_model[type], 
                              m_all_items.size());
    else
        h = new Item(type, xyz, normal, m_item_model[type],
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
    item->isCollected();
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
        loadItemStyle(user_config->m_item_style);
    }
    catch(std::runtime_error)
    {
        fprintf(stderr,"The item style '%s' in your configuration file does not exist.\nIt is ignored.\n",
                user_config->m_item_style.c_str());
        user_config->m_item_style="";
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
    for(AllItemTypes::iterator i =m_all_items.begin();
        i!=m_all_items.end();  i++)
    {
        (*i)->reset();
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
void ItemManager::createDefaultItem(sgVec3 colour, std::string name)
{
    ssgVertexArray   *va = new ssgVertexArray   () ; sgVec3 v ;
    ssgNormalArray   *na = new ssgNormalArray   () ; sgVec3 n ;
    ssgColourArray   *ca = new ssgColourArray   () ; sgVec4 c ;
    ssgTexCoordArray *ta = new ssgTexCoordArray () ; sgVec2 t ;

    sgSetVec3(v, -0.5f, 0.0f, 0.0f ) ; va->add(v) ;
    sgSetVec3(v,  0.5f, 0.0f, 0.0f ) ; va->add(v) ;
    sgSetVec3(v, -0.5f, 0.0f, 0.5f ) ; va->add(v) ;
    sgSetVec3(v,  0.5f, 0.0f, 0.5f ) ; va->add(v) ;
    sgSetVec3(v, -0.5f, 0.0f, 0.0f ) ; va->add(v) ;
    sgSetVec3(v,  0.5f, 0.0f, 0.0f ) ; va->add(v) ;

    sgSetVec3(n,  0.0f,  1.0f,  0.0f ) ; na->add(n) ;

    sgCopyVec3 ( c, colour ) ; c[ 3 ] = 1.0f ; ca->add(c) ;

    sgSetVec2(t, 0.0f, 0.0f ) ; ta->add(t) ;
    sgSetVec2(t, 1.0f, 0.0f ) ; ta->add(t) ;
    sgSetVec2(t, 0.0f, 1.0f ) ; ta->add(t) ;
    sgSetVec2(t, 1.0f, 1.0f ) ; ta->add(t) ;
    sgSetVec2(t, 0.0f, 0.0f ) ; ta->add(t) ;
    sgSetVec2(t, 1.0f, 0.0f ) ; ta->add(t) ;


    ssgLeaf *gset = new ssgVtxTable ( GL_TRIANGLE_STRIP, va, na, ta, ca ) ;

    // FIXME - this method seems outdated
    //gset->setState(material_manager->getMaterial("herring.rgb")->getState()) ;

    Shadow* sh = new Shadow ( -0.5f, 0.5f, -0.25f, 0.25f ) ;

    ssgTransform* tr = new ssgTransform () ;

    tr -> addKid ( sh -> getRoot () ) ;
    tr -> addKid ( gset ) ;
    tr -> ref () ; /* Make sure it doesn't get deleted by mistake */
    m_all_models[name] = tr;

}   // createDefaultItem

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
        char msg[MAX_ERROR_MESSAGE_LENGTH];
        snprintf(msg, sizeof(msg), "Couldn't load map '%s': no item node.",
                 filename.c_str());
	delete root;
        throw std::runtime_error(msg);
        delete root;
    }
    setItem(item_node, "red",   ITEM_BONUS_BOX   );
    setItem(item_node, "green", ITEM_BANANA );
    setItem(item_node, "gold"  ,ITEM_GOLD_COIN  );
    setItem(item_node, "silver",ITEM_SILVER_COIN);
    delete root;
}   // loadItemStyle

//-----------------------------------------------------------------------------
void ItemManager::setItem(const lisp::Lisp *item_node,
                                const char *colour, ItemType type)
{
    std::string name;
    item_node->get(colour, name);
    if(name.size()>0)
    {
        m_item_model[type]=m_all_models[name];
    }
}   // setItem
