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

#include <stdexcept>
#include <string>
#include <sstream>

#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "io/file_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/spare_tire_ai.hpp"
#include "network/network_config.hpp"
#include "network/race_event_manager.hpp"
#include "physics/triangle_mesh.hpp"
#include "tracks/arena_graph.hpp"
#include "tracks/arena_node.hpp"
#include "tracks/track.hpp"
#include "utils/string_utils.hpp"

#include <IMesh.h>
#include <IAnimatedMesh.h>

std::vector<scene::IMesh *> ItemManager::m_item_mesh;
std::vector<scene::IMesh *> ItemManager::m_item_lowres_mesh;
std::vector<video::SColorf> ItemManager::m_glow_color;
ItemManager *               ItemManager::m_item_manager = NULL;


//-----------------------------------------------------------------------------
/** Creates one instance of the item manager. */
void ItemManager::create()
{
    assert(!m_item_manager);
    m_item_manager = new ItemManager();
}   // create

//-----------------------------------------------------------------------------
/** Destroys the one instance of the item manager. */
void ItemManager::destroy()
{
    assert(m_item_manager);
    delete m_item_manager;
    m_item_manager = NULL;
}   // destroy

//-----------------------------------------------------------------------------
/** Loads the default item meshes (high- and low-resolution).
 */
void ItemManager::loadDefaultItemMeshes()
{
    m_item_mesh.resize(Item::ITEM_LAST-Item::ITEM_FIRST+1, NULL);
    m_glow_color.resize(Item::ITEM_LAST-Item::ITEM_FIRST+1,
                        video::SColorf(255.0f, 255.0f, 255.0f) );

    m_item_lowres_mesh.resize(Item::ITEM_LAST-Item::ITEM_FIRST+1, NULL);

    // A temporary mapping of items to names used in the XML file:
    std::map<Item::ItemType, std::string> item_names;
    item_names[Item::ITEM_BANANA     ] = "banana";
    item_names[Item::ITEM_BONUS_BOX  ] = "bonus-box";
    item_names[Item::ITEM_BUBBLEGUM  ] = "bubblegum";
    item_names[Item::ITEM_NITRO_BIG  ] = "nitro-big";
    item_names[Item::ITEM_NITRO_SMALL] = "nitro-small";
    item_names[Item::ITEM_TRIGGER    ] = "trigger";
    item_names[Item::ITEM_BUBBLEGUM_NOLOK] = "bubblegum-nolok";
    item_names[Item::ITEM_EASTER_EGG ] = "easter-egg";

    const std::string file_name = file_manager->getAsset("items.xml");
    const XMLNode *root         = file_manager->createXMLTree(file_name);
    for(unsigned int i=Item::ITEM_FIRST; i<=Item::ITEM_LAST; i++)
    {
        const std::string &name = item_names[(Item::ItemType)i];
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
        mesh->grab();
        m_item_mesh[i]            = mesh;
        node->get("glow", &(m_glow_color[i]));

        std::string lowres_model_filename;
        node->get("lowmodel", &lowres_model_filename);
        m_item_lowres_mesh[i] = lowres_model_filename.size() == 0
                              ? NULL
                              : irr_driver->getMesh(lowres_model_filename);

        if (m_item_lowres_mesh[i]) m_item_lowres_mesh[i]->grab();
    }   // for i
    delete root;
}   // loadDefaultItemMeshes

//-----------------------------------------------------------------------------
/** Clean up all textures. This is necessary when switching resolution etc.
 */
void ItemManager::removeTextures()
{
    for(unsigned int i=0; i<Item::ITEM_LAST-Item::ITEM_FIRST+1; i++)
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
    m_switch_time = -1.0f;
    // The actual loading is done in loadDefaultItems

    // Prepare the switch to array, which stores which item should be
    // switched to what other item. Initialise it with a mapping that
    // each item is switched to itself, so basically a no-op.
    m_switch_to.reserve(Item::ITEM_COUNT);
    for(unsigned int i=Item::ITEM_FIRST; i<Item::ITEM_COUNT; i++)
        m_switch_to.push_back((Item::ItemType)i);
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
    for(unsigned int i=Item::ITEM_FIRST; i<Item::ITEM_COUNT; i++)
        m_switch_to[i]=(Item::ItemType)stk_config->m_switch_items[i];
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
 */
void ItemManager::insertItem(Item *item)
{
    // Find where the item can be stored in the index list: either in a
    // previously deleted entry, otherwise at the end.
    int index = -1;
    for(index=(int)m_all_items.size()-1; index>=0 && m_all_items[index]; index--) {}

    if(index==-1) index = (int)m_all_items.size();

    if(index<(int)m_all_items.size())
        m_all_items[index] = item;
    else
        m_all_items.push_back(item);
    item->setItemId(index);

    // Now insert into the appropriate quad list, if there is a quad list
    // (i.e. race mode has a quad graph).
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
}   // insertItem

//-----------------------------------------------------------------------------
/** Creates a new item.
 *  \param type Type of the item.
 *  \param xyz  Position of the item.
 *  \param normal The normal of the terrain to set roll and pitch.
 *  \param parent In case of a dropped item used to avoid that a kart
 *         is affected by its own items.
 */
Item* ItemManager::newItem(Item::ItemType type, const Vec3& xyz,
                           const Vec3 &normal, AbstractKart *parent)
{
    Item::ItemType mesh_type = type;
    if (type == Item::ITEM_BUBBLEGUM && parent->getIdent() == "nolok")
    {
        mesh_type = Item::ITEM_BUBBLEGUM_NOLOK;
    }

    Item* item = new Item(type, xyz, normal, m_item_mesh[mesh_type],
                          m_item_lowres_mesh[mesh_type]);

    insertItem(item);
    if(parent != NULL) item->setParent(parent);
    if(m_switch_time>=0)
    {
        Item::ItemType new_type = m_switch_to[item->getType()];
        item->switchTo(new_type, m_item_mesh[(int)new_type],
                       m_item_lowres_mesh[(int)new_type]);
    }
    return item;
}   // newItem

//-----------------------------------------------------------------------------
/** Creates a new trigger item.
 *  \param xyz  Position of the item.
 */
Item* ItemManager::newItem(const Vec3& xyz, float distance,
                           TriggerItemListener* listener)
{
    Item* item;
    item = new Item(xyz, distance, listener);
    insertItem(item);

    return item;
}   // newItem

//-----------------------------------------------------------------------------
/** Set an item as collected.
 *  This function is called on the server when an item is collected, or on
 *  the client upon receiving information about collected items.             */
void ItemManager::collectedItem(Item *item, AbstractKart *kart, int add_info)
{
    assert(item);
    // Spare tire karts don't collect items
    if (dynamic_cast<SpareTireAI*>(kart->getController()) != NULL) return;
    if( (item->getType() == Item::ITEM_BUBBLEGUM || 
         item->getType() == Item::ITEM_BUBBLEGUM_NOLOK) && kart->isShielded())
    {
        // shielded karts can simply drive over bubble gums without any effect.
        return;
    }
    item->collected(kart);
    kart->collectedItem(item, add_info);
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

    for(AllItemTypes::iterator i =m_all_items.begin();
        i!=m_all_items.end();  i++)
    {
        if((!*i) || (*i)->wasCollected()) continue;
        // To allow inlining and avoid including kart.hpp in item.hpp,
        // we pass the kart and the position separately.
        if((*i)->hitKart(kart->getXYZ(), kart))
        {
            // if we're not playing online, pick the item.
            if (!RaceEventManager::getInstance()->isRunning())
                collectedItem(*i, kart);
            else if (NetworkConfig::get()->isServer())
            {
                // Only the server side detects item being collected
                // A client does the collection upon receiving the 
                // event from the server!
                collectedItem(*i, kart);
                RaceEventManager::getInstance()->collectedItem(*i, kart);
            }
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
    if(m_switch_time>=0)
    {
        for(AllItemTypes::iterator i =m_all_items.begin();
                                   i!=m_all_items.end(); i++)
        {
            if(*i) (*i)->switchBack();
        }
        m_switch_time = -1.0f;

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
        if((*i)->canBeUsedUp() || (*i)->getType()==Item::ITEM_BUBBLEGUM)
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

    m_switch_time = -1;
}   // reset

//-----------------------------------------------------------------------------
/** Updates all items, and handles switching items back if the switch time
 *  is over.
 *  \param dt Time step.
 */
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
                deleteItem( *i );
            }   // if usedUp
        }   // if *i
    }   // for m_all_items
}   // update

//-----------------------------------------------------------------------------
/** Removes an items from the items-in-quad list, from the list of all
 *  items, and then frees the item itself.
 *  \param The item to delete.
 */
void ItemManager::deleteItem(Item *item)
{
    // First check if the item needs to be removed from the items-in-quad list
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

    int index = item->getItemId();
    m_all_items[index] = NULL;
    delete item;
}   // delete item

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

        if ((*i)->getType() == Item::ITEM_BUBBLEGUM || (*i)->getType() == Item::ITEM_BUBBLEGUM_NOLOK)
        {
            if (race_manager->getAISuperPower() == RaceManager::SUPERPOWER_NOLOK_BOSS)
            {
                continue;
            }
        }

        Item::ItemType new_type = m_switch_to[(*i)->getType()];

        if(m_switch_time<0)
            (*i)->switchTo(new_type, m_item_mesh[(int)new_type], m_item_lowres_mesh[(int)new_type]);
        else
            (*i)->switchBack();
    }   // for m_all_items

    // if the items are already switched (m_switch_time >=0)
    // then switch back, and set m_switch_time to -1 to indicate
    // that the items are now back to normal.
    m_switch_time = m_switch_time < 0 ? stk_config->m_item_switch_time : -1;

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

    RandomGenerator random;
    const unsigned int ALL_NODES = ag->getNumNodes();
    const unsigned int MIN_DIST = int(sqrt(ALL_NODES));
    const unsigned int TOTAL_ITEM = MIN_DIST / 2;

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

            const int node = random.get(ALL_NODES);

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

    assert (used_location.size() == TOTAL_ITEM);

    // Hard-coded ratio for now
    const int BONUS_BOX = 4;
    const int NITRO_BIG = 2;
    const int NITRO_SMALL = 1;

    for (unsigned int i = 0; i < TOTAL_ITEM; i++)
    {
        const int j = random.get(10);
        Item::ItemType type = (j > BONUS_BOX ? Item::ITEM_BONUS_BOX :
            j > NITRO_BIG ? Item::ITEM_NITRO_BIG :
            j > NITRO_SMALL ? Item::ITEM_NITRO_SMALL : Item::ITEM_BANANA);

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
            newItem(type, hit_point, normal);
        }
        else
        {
            Log::warn("[ItemManager]","Raycast to surface failed"
                      "from node %d", used_location[i]);
            newItem(type, an->getCenter(), quad_normal);
        }
    }

    return true;
}   // randomItemsForArena
