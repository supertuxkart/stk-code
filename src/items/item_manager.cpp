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

#include "items/item_manager.hpp"

#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/sp/sp_base.hpp"
#include "io/file_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/spare_tire_ai.hpp"
#include "modes/easter_egg_hunt.hpp"
#include "modes/profile_world.hpp"
#include "network/network_config.hpp"
#include "network/race_event_manager.hpp"
#include "physics/triangle_mesh.hpp"
#include "tracks/arena_graph.hpp"
#include "tracks/arena_node.hpp"
#include "tracks/track.hpp"
#include "utils/string_utils.hpp"

#include <IMesh.h>
#include <IAnimatedMesh.h>

#include <assert.h>
#include <stdexcept>
#include <sstream>
#include <string>


std::vector<scene::IMesh *>  ItemManager::m_item_mesh;
std::vector<scene::IMesh *>  ItemManager::m_item_lowres_mesh;
std::vector<video::SColorf>  ItemManager::m_glow_color;
std::vector<std::string>     ItemManager::m_icon;
bool                         ItemManager::m_disable_item_collection = false;
std::mt19937                 ItemManager::m_random_engine;
uint32_t                     ItemManager::m_random_seed = 0;

//-----------------------------------------------------------------------------
/** Loads the default item meshes (high- and low-resolution).
 */
void ItemManager::loadDefaultItemMeshes()
{
    m_item_mesh.clear();
    m_item_lowres_mesh.clear();
    m_glow_color.clear();
    m_icon.clear();
    m_item_mesh.resize(ItemState::ITEM_LAST-ItemState::ITEM_FIRST+1, NULL);
    m_glow_color.resize(ItemState::ITEM_LAST-ItemState::ITEM_FIRST+1,
                        video::SColorf(255.0f, 255.0f, 255.0f) );
    m_icon.resize(ItemState::ITEM_LAST-ItemState::ITEM_FIRST+1, "");

    m_item_lowres_mesh.resize(ItemState::ITEM_LAST-ItemState::ITEM_FIRST+1, NULL);

    // A temporary mapping of items to names used in the XML file:
    std::map<ItemState::ItemType, std::string> item_names;
    item_names[ItemState::ITEM_BANANA     ] = "banana";
    item_names[ItemState::ITEM_BONUS_BOX  ] = "bonus-box";
    item_names[ItemState::ITEM_BUBBLEGUM  ] = "bubblegum";
    item_names[ItemState::ITEM_NITRO_BIG  ] = "nitro-big";
    item_names[ItemState::ITEM_NITRO_SMALL] = "nitro-small";
    item_names[ItemState::ITEM_BUBBLEGUM_NOLOK] = "bubblegum-nolok";
    item_names[ItemState::ITEM_EASTER_EGG ] = "easter-egg";

    const std::string file_name = file_manager->getAsset("items.xml");
    const XMLNode *root         = file_manager->createXMLTree(file_name);
    for(unsigned int i=ItemState::ITEM_FIRST; i<=ItemState::ITEM_LAST; i++)
    {
        const std::string &name = item_names[(ItemState::ItemType)i];
        const XMLNode *node = root->getNode(name);
        if (!node)  continue;

        std::string model_filename;
        node->get("model", &model_filename);

        scene::IMesh *mesh = irr_driver->getAnimatedMesh(model_filename);
        if(!node || model_filename.size()==0 || !mesh)
        {
            Log::fatal("[ItemManager]", "Item model '%s' in items.xml could not be loaded "
                        "- aborting", name.c_str());
            exit(-1);
        }
#ifndef SERVER_ONLY
        SP::uploadSPM(mesh);
#endif
        mesh->grab();
        m_item_mesh[i]            = mesh;
        node->get("glow", &(m_glow_color[i]));

        std::string lowres_model_filename;
        node->get("lowmodel", &lowres_model_filename);
        m_item_lowres_mesh[i] = lowres_model_filename.size() == 0
                              ? NULL
                              : irr_driver->getMesh(lowres_model_filename);

        if (m_item_lowres_mesh[i])
        {
#ifndef SERVER_ONLY
            SP::uploadSPM(m_item_lowres_mesh[i]);
#endif
            m_item_lowres_mesh[i]->grab();
        }
        std::string icon = "icon-" + item_names[(ItemState::ItemType)i] + ".png";
        if (preloadIcon(icon))
            m_icon[i] = icon;
    }   // for i
    delete root;
    preloadIcon("item_spark.png");
}   // loadDefaultItemMeshes

//-----------------------------------------------------------------------------
/** Preload icon materials to avoid hangs when firstly insert item
 */
bool ItemManager::preloadIcon(const std::string& name)
{
    // From IrrDriver::addBillboard
    Material* m = material_manager->getMaterial(name, false/*full_path*/,
        /*make_permanent*/true, /*complain_if_not_found*/true,
        /*strip_path*/false, /*install*/false);
    return m->getTexture(true/*srgb*/, m->getShaderName() == "additive" ||
        m->getShaderName() == "alphablend" ? true : false/*premul_alpha*/) !=
        NULL;
}   // preloadIcon

//-----------------------------------------------------------------------------
/** Clean up all textures. This is necessary when switching resolution etc.
 */
void ItemManager::removeTextures()
{
    if (m_item_mesh.empty() && m_item_lowres_mesh.empty())
        return;
    for(unsigned int i=0; i<ItemState::ITEM_LAST-ItemState::ITEM_FIRST+1; i++)
    {
        if(m_item_mesh[i])
        {
            m_item_mesh[i]->drop();
            irr_driver->removeMeshFromCache(m_item_mesh[i]);
        }
        m_item_mesh[i] = NULL;
        if(m_item_lowres_mesh[i])
        {
            m_item_lowres_mesh[i]->drop();
            irr_driver->removeMeshFromCache(m_item_lowres_mesh[i]);
        }
        m_item_lowres_mesh[i] = NULL;
    }
}   // removeTextures


// ============================================================================
/** Creates a new instance of the item manager. This is done at startup
 *  of each race. */
ItemManager::ItemManager()
{
    m_switch_ticks = -1;
    // The actual loading is done in loadDefaultItems

    // Prepare the switch to array, which stores which item should be
    // switched to what other item. Initialise it with a mapping that
    // each item is switched to itself, so basically a no-op.
    m_switch_to.reserve(ItemState::ITEM_COUNT);
    for(unsigned int i=ItemState::ITEM_FIRST; i<ItemState::ITEM_COUNT; i++)
        m_switch_to.push_back((ItemState::ItemType)i);
    setSwitchItems(stk_config->m_switch_items);

    if(Graph::get())
    {
        m_items_in_quads = new std::vector<AllItemTypes>;
        // Entries 0 to n-1 are for the quads, entry
        // n is for all items that are not on a quad.
        m_items_in_quads->resize(Graph::get()->getNumNodes()+1);
    }
    else
    {
        m_items_in_quads = NULL;
    }
}   // ItemManager

//-----------------------------------------------------------------------------
/** Sets which objects is getting switched to what.
 *  \param switch A mapping of items types to item types for the mapping.
 *         must contain one entry for each item.
 */
void ItemManager::setSwitchItems(const std::vector<int> &switch_items)
{
    for(unsigned int i=ItemState::ITEM_FIRST; i<ItemState::ITEM_COUNT; i++)
        m_switch_to[i]=(ItemState::ItemType)switch_items[i];
}   // setSwitchItems

//-----------------------------------------------------------------------------
/** Destructor. Cleans up all items and meshes stored.
 */
ItemManager::~ItemManager()
{
    if(m_items_in_quads)
        delete m_items_in_quads;
    for(AllItemTypes::iterator i =m_all_items.begin();
                               i!=m_all_items.end();  i++)
    {
        if(*i)
            delete *i;
    }

    m_all_items.clear();
}   // ~ItemManager

//-----------------------------------------------------------------------------
/** Inserts the new item into the items management data structures, if possible
 *  reusing an existing, unused entry (e.g. due to a removed bubble gum). Then
 *  the item is also added to the quad-wise list of items.
 *  \param item The item to be added.
 *  \return Index of the newly added item in the list of all items.
 */
unsigned int ItemManager::insertItem(Item *item)
{
    // Find where the item can be stored in the index list: either in a
    // previously deleted entry, otherwise at the end.
    int index = -1;
    for(index=(int)m_all_items.size()-1; index>=0 && m_all_items[index]; index--) {}
    if (index == -1)
    {
        index = (int)m_all_items.size();
        m_all_items.push_back(item);
    }
    else
    {
        m_all_items[index] = item;
    }
    item->setItemId(index);
    insertItemInQuad(item);
    // Now insert into the appropriate quad list, if there is a quad list
    // (i.e. race mode has a quad graph).
    return index;
}   // insertItem

//-----------------------------------------------------------------------------
/** Insert into the appropriate quad list, if there is a quad list
 *  (i.e. race mode has a quad graph).
 */
void ItemManager::insertItemInQuad(Item *item)
{
    if(m_items_in_quads)
    {
        int graph_node = item->getGraphNode();
        // If the item is on the graph, store it at the appropriate index
        if(graph_node > -1)
        {
            (*m_items_in_quads)[graph_node].push_back(item);
        }
        else  // otherwise store it in the 'outside' index
            (*m_items_in_quads)[m_items_in_quads->size()-1].push_back(item);
    }   // if m_items_in_quads
}   // insertItemInQuad

//-----------------------------------------------------------------------------
/** Creates a new item at the location of the kart (e.g. kart drops a
 *  bubblegum).
 *  \param type Type of the item.
 *  \param kart The kart that drops the new item.
 *  \param server_xyz Can be used to overwrite the item location.
 *  \param server_normal The normal as seen on the server.
 */
Item* ItemManager::dropNewItem(ItemState::ItemType type,
                               const AbstractKart *kart, 
                               const Vec3 *server_xyz,
                               const Vec3 *server_normal)
{
    Vec3 normal, pos;
    const Material* material_hit;
    if (!server_xyz)
    {
        // We are doing a new drop locally, i.e. not based on
        // server data. So we need a raycast to find the correct
        // location and normal of the item:
        pos = server_xyz ? *server_xyz : kart->getXYZ();
        Vec3 to = pos + kart->getTrans().getBasis() * Vec3(0, -10000, 0);
        Vec3 hit_point;
        Track::getCurrentTrack()->getTriangleMesh().castRay(pos, to,
                                                            &hit_point,
                                                            &material_hit,
                                                            &normal);

        // We will get no material if the kart is 'over nothing' when dropping
        // the bubble gum. In most cases this means that the item does not need
        // to be created (and we just return NULL). 
        if (!material_hit) return NULL;
        normal.normalize();
        pos = hit_point + kart->getTrans().getBasis() * Vec3(0, -0.05f, 0);
    }
    else
    {
        // We are on a client which has received a new item event from the
        // server. So use the server's data for the new item:
        normal = *server_normal;
        pos    = *server_xyz;
    }

    ItemState::ItemType mesh_type = type;
    if (type == ItemState::ITEM_BUBBLEGUM && kart->getIdent() == "nolok")
    {
        mesh_type = ItemState::ITEM_BUBBLEGUM_NOLOK;
    }

    Item* item = new Item(type, pos, normal, m_item_mesh[mesh_type],
                          m_item_lowres_mesh[mesh_type], m_icon[mesh_type],
                          /*prev_owner*/kart);

    // restoreState in NetworkItemManager will handle the insert item
    if (!server_xyz)
        insertItem(item);
    if(m_switch_ticks>=0)
    {
        ItemState::ItemType new_type = m_switch_to[item->getType()];
        item->switchTo(new_type);
    }
    return item;
}   // dropNewItem

//-----------------------------------------------------------------------------
/** Places a new item on the track/arena. It is used for the initial placement
 *  of the items - either according to the scene.xml file, or random item
 *  placement.
 *  \param type Type of the item.
 *  \param xyz  Position of the item.
 *  \param normal The normal of the terrain to set roll and pitch.
 */
Item* ItemManager::placeItem(ItemState::ItemType type, const Vec3& xyz,
                             const Vec3 &normal)
{
    // Make sure this subroutine is not used otherwise (since networking
    // needs to be aware of items added to the track, so this would need
    // to be added).
    assert(World::getWorld()->getPhase() == WorldStatus::SETUP_PHASE ||
           ProfileWorld::isProfileMode()                               );
    ItemState::ItemType mesh_type = type;

    Item* item = new Item(type, xyz, normal, m_item_mesh[mesh_type],
                          m_item_lowres_mesh[mesh_type], m_icon[mesh_type],
                          /*prev_owner*/NULL);

    insertItem(item);
    if (m_switch_ticks >= 0)
    {
        ItemState::ItemType new_type = m_switch_to[item->getType()];
        item->switchTo(new_type);
    }
    return item;
}   // placeItem

//-----------------------------------------------------------------------------
/** Set an item as collected.
 *  This function is called on the server when an item is collected, or on
 *  the client upon receiving information about collected items.
 *  \param item The item that was collected.
 *  \param kart The kart that collected the item.
 */
void ItemManager::collectedItem(ItemState *item, AbstractKart *kart)
{
    assert(item);
    item->collected(kart);
    // Inform the world - used for Easter egg hunt
    World::getWorld()->collectedItem(kart, item);
    kart->collectedItem(item);
}   // collectedItem

//-----------------------------------------------------------------------------
/** Checks if any item was collected by the given kart. This function calls
 *  collectedItem if an item was collected.
 *  \param kart Pointer to the kart.
 */
void  ItemManager::checkItemHit(AbstractKart* kart)
{
    // We could use m_items_in_quads to to check for item hits: take the quad
    // of the graph node of the kart, and only check items in that quad. But
    // then we also need to check for any adjacent quads (since an item just
    // on the order of one quad might get hit from an adjacent quad). Then
    // it is possible that a quad is that short that we need to test adjacent
    // of adjacent quads. And check for items outside of the track.
    // Since at this stace item detection is by far not a bottle neck,
    // the original, simple and stable algorithm is left in place.

    /** Disable item collection detection for debug purposes. */
    if(m_disable_item_collection) return;

    // Spare tire karts don't collect items
    if ( dynamic_cast<SpareTireAI*>(kart->getController()) ) return;

    for(AllItemTypes::iterator i =m_all_items.begin();
                               i!=m_all_items.end();  i++)
    {
        // Ignore items that have been collected or are not available atm
        if ((!*i) || !(*i)->isAvailable() || (*i)->isUsedUp()) continue;

        // Shielded karts can simply drive over bubble gums without any effect
        if ( kart->isShielded() &&
             ( (*i)->getType() == ItemState::ITEM_BUBBLEGUM      ||
               (*i)->getType() == ItemState::ITEM_BUBBLEGUM_NOLOK  ) )
        {
            continue;
        }


        // To allow inlining and avoid including kart.hpp in item.hpp,
        // we pass the kart and the position separately.
        if((*i)->hitKart(kart->getXYZ(), kart))
        {
            collectedItem(*i, kart);
        }   // if hit
    }   // for m_all_items
}   // checkItemHit

//-----------------------------------------------------------------------------
/** Resets all items and removes bubble gum that is stuck on the track.
 *  This is done when a race is (re)started.
 */
void ItemManager::reset()
{
    // If items are switched, switch them back first.
    if(m_switch_ticks>=0)
    {
        for(AllItemTypes::iterator i =m_all_items.begin();
                                   i!=m_all_items.end(); i++)
        {
            if(*i) (*i)->switchBack();
        }
        m_switch_ticks = -1;

    }

    // We can't simply erase items in the list: in this case the indicies
    // stored in each item are invalid, and lead to incorrect result/crashes
    // in deleteItem
    AllItemTypes::iterator i=m_all_items.begin();
    while(i!=m_all_items.end())
    {
        if(!*i)
        {
            i++;
            continue;
        }
        if((*i)->canBeUsedUp() || (*i)->getType()==ItemState::ITEM_BUBBLEGUM)
        {
            deleteItem( *i );
            i++;
        }
        else
        {
            (*i)->reset();
            i++;
        }
    }  // whilem_all_items.end() i

    m_switch_ticks = -1;
}   // reset

//-----------------------------------------------------------------------------
/** Updates all items, and handles switching items back if the switch time
 *  is over.
 *  \param ticks Number of physics time steps - should be 1.
 */
void ItemManager::update(int ticks)
{
    // If switch time is over, switch all items back
    if(m_switch_ticks>=0)
    {
        m_switch_ticks -= ticks;
        if(m_switch_ticks<0)
        {
            for(AllItemTypes::iterator i = m_all_items.begin();
                                       i!= m_all_items.end();  i++)
            {
                if(*i) (*i)->switchBack();
            }   // for m_all_items
        }   // m_switch_ticks < 0
    }   // m_switch_ticks>=0

    for(AllItemTypes::iterator i =m_all_items.begin();
        i!=m_all_items.end();  i++)
    {
        if(*i)
        {
            (*i)->update(ticks);
            if( (*i)->isUsedUp())
            {
                deleteItem( *i );
            }   // if usedUp
        }   // if *i
    }   // for m_all_items
}   // update

//-----------------------------------------------------------------------------
/** Updates the graphics, called once per rendered frame.
 * \param dt Time based on frame rate.
 */
void ItemManager::updateGraphics(float dt)
{
    for (AllItemTypes::iterator i  = m_all_items.begin();
                                i != m_all_items.end();  i++)
    {
        if (*i) (*i)->updateGraphics(dt);
    }   // for m_all_items

}   // updateGraphics

//-----------------------------------------------------------------------------
/** Removes an items from the items-in-quad list, from the list of all
 *  items, and then frees the item itself.
 *  \param The item to delete.
 */
void ItemManager::deleteItem(ItemState *item)
{
    // First check if the item needs to be removed from the items-in-quad list
    deleteItemInQuad(item);
    int index = item->getItemId();
    m_all_items[index] = NULL;
    delete item;
}   // delete item

//-----------------------------------------------------------------------------
/** Removes an items from the items-in-quad list only
 *  \param The item to delete.
 */
void ItemManager::deleteItemInQuad(ItemState* item)
{
    if(m_items_in_quads)
    {
        int sector = item->getGraphNode();
        unsigned int indx = sector==Graph::UNKNOWN_SECTOR
                          ? (unsigned int) m_items_in_quads->size()-1
                          : sector;
        AllItemTypes &items = (*m_items_in_quads)[indx];
        AllItemTypes::iterator it = std::find(items.begin(), items.end(),item);
        assert(it!=items.end());
        items.erase(it);
    }   // if m_items_in_quads
}   // deleteItemInQuad

//-----------------------------------------------------------------------------
/** Switches all items: boxes become bananas and vice versa for a certain
 *  amount of time (as defined in stk_config.xml).
 */
void ItemManager::switchItems()
{
    switchItemsInternal(m_all_items);
}  // switchItems

//-----------------------------------------------------------------------------
/** Switches all items: boxes become bananas and vice versa for a certain
 *  amount of time (as defined in stk_config.xml).
 */
void ItemManager::switchItemsInternal(std::vector<ItemState*> &all_items)
{
    for(AllItemTypes::iterator i  = all_items.begin();
                               i != all_items.end();  i++)
    {
        if(!*i) continue;

        ItemState::ItemType new_type = m_switch_to[(*i)->getType()];

        if (new_type == (*i)->getType())
            continue;
        if(m_switch_ticks<0)
            (*i)->switchTo(new_type);
        else
            (*i)->switchBack();
    }   // for all_items

    // if the items are already switched (m_switch_ticks >=0)
    // then switch back, and set m_switch_ticks to -1 to indicate
    // that the items are now back to normal.
    m_switch_ticks = m_switch_ticks < 0 ? stk_config->m_item_switch_ticks : -1;

}   // switchItems

//-----------------------------------------------------------------------------
bool ItemManager::randomItemsForArena(const AlignedArray<btTransform>& pos)
{
    if (!UserConfigParams::m_random_arena_item) return false;
    if (!ArenaGraph::get()) return false;

    const ArenaGraph* ag = ArenaGraph::get();
    std::vector<int> used_location;
    std::vector<int> invalid_location;
    for (unsigned int i = 0; i < pos.size(); i++)
    {
        // Load all starting positions of arena, so no items will be near them
        int node = -1;
        ag->findRoadSector(pos[i].getOrigin(), &node, NULL, true);
        assert(node != -1);
        used_location.push_back(node);
        invalid_location.push_back(node);
    }

    const unsigned int ALL_NODES = ag->getNumNodes();
    const unsigned int MIN_DIST = int(sqrt(ALL_NODES));
    const unsigned int TOTAL_ITEM = MIN_DIST / 2;

    std::vector<uint32_t> random_numbers;
    Log::info("[ItemManager]","Creating %d random items for arena", TOTAL_ITEM);
    for (unsigned int i = 0; i < TOTAL_ITEM; i++)
    {
        int chosen_node = -1;
        while(true)
        {
            if (used_location.size() - pos.size() +
                invalid_location.size() == ALL_NODES)
            {
                Log::warn("[ItemManager]","Can't place more random items! "
                    "Use default item location.");
                return false;
            }
            uint32_t number = m_random_engine();
            Log::debug("[ItemManager]", "%u from random engine.", number);
            const int node = number % ALL_NODES;

            // Check if tried
            std::vector<int>::iterator it = std::find(invalid_location.begin(),
                invalid_location.end(), node);
            if (it != invalid_location.end())
                continue;

            // Check if near edge
            if (ag->getNode(node)->isNearEdge())
            {
                invalid_location.push_back(node);
                continue;
            }
            // Check if too close
            bool found = true;
            for (unsigned int j = 0; j < used_location.size(); j++)
            {
                if (!found) continue;
                float test_distance = ag->getDistance(used_location[j], node);
                found = test_distance > MIN_DIST;
            }
            if (found)
            {
                chosen_node = node;
                invalid_location.push_back(node);
                random_numbers.push_back(number);
                break;
            }
            else
                invalid_location.push_back(node);
        }

        assert(chosen_node != -1);
        used_location.push_back(chosen_node);
    }

    for (unsigned int i = 0; i < pos.size(); i++)
        used_location.erase(used_location.begin());

    assert(used_location.size() == TOTAL_ITEM);
    assert(random_numbers.size() == TOTAL_ITEM);

    // Hard-coded ratio for now
    const int BONUS_BOX = 4;
    const int NITRO_BIG = 2;
    const int NITRO_SMALL = 1;

    for (unsigned int i = 0; i < TOTAL_ITEM; i++)
    {
        const unsigned j = random_numbers[i] % 10;
        ItemState::ItemType type = (j > BONUS_BOX ? ItemState::ITEM_BONUS_BOX :
            j > NITRO_BIG ? ItemState::ITEM_NITRO_BIG :
            j > NITRO_SMALL ? ItemState::ITEM_NITRO_SMALL : ItemState::ITEM_BANANA);

        ArenaNode* an = ag->getNode(used_location[i]);
        Vec3 loc = an->getCenter();
        Vec3 quad_normal = an->getNormal();
        loc += quad_normal;

        // Do a raycast to help place it fully on the surface
        const Material* m;
        Vec3 normal;
        Vec3 hit_point;
        const TriangleMesh& tm = Track::getCurrentTrack()->getTriangleMesh();
        bool success = tm.castRay(loc, an->getCenter() + (-10000*quad_normal),
                                   &hit_point, &m, &normal);

        if (success)
        {
            placeItem(type, hit_point, normal);
        }
        else
        {
            Log::warn("[ItemManager]","Raycast to surface failed"
                      "from node %d", used_location[i]);
            placeItem(type, an->getCenter(), quad_normal);
        }
    }

    return true;
}   // randomItemsForArena
