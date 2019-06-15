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

#include "items/item.hpp"

#include "graphics/irr_driver.hpp"
#include "graphics/lod_node.hpp"
#include "graphics/sp/sp_mesh.hpp"
#include "graphics/sp/sp_mesh_node.hpp"
#include "items/item_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "modes/world.hpp"
#include "network/network_string.hpp"
#include "network/rewind_manager.hpp"
#include "tracks/arena_graph.hpp"
#include "tracks/drive_graph.hpp"
#include "tracks/drive_node.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"
#include "utils/string_utils.hpp"

#include <IMeshSceneNode.h>
#include <ISceneManager.h>


// ----------------------------------------------------------------------------
/** Constructor.
 *  \param type Type of the item.
 *  \param owner If not NULL it is the kart that dropped this item; NULL
 *         indicates an item that's part of the track.
 *  \param id Index of this item in the array of all items.
 */
ItemState::ItemState(ItemType type, const AbstractKart *owner, int id)
{
    setType(type);
    m_item_id = id;
    m_previous_owner = owner;
    m_used_up_counter = -1;
     if (owner)
         setDeactivatedTicks(stk_config->time2Ticks(1.5f));
    else
        setDeactivatedTicks(0);
}   // ItemState(ItemType)

//-----------------------------------------------------------------------------
/** Constructor to restore item state at current ticks in client for live join
 */
ItemState::ItemState(const BareNetworkString& buffer)
{
    m_type = (ItemType)buffer.getUInt8();
    m_original_type = (ItemType)buffer.getUInt8();
    m_ticks_till_return = buffer.getUInt32();
    m_item_id = buffer.getUInt32();
    m_deactive_ticks = buffer.getUInt32();
    m_used_up_counter = buffer.getUInt32();
    m_xyz = buffer.getVec3();
    m_original_rotation = buffer.getQuat();
    m_previous_owner = NULL;
    int8_t kart_id = buffer.getUInt8();
    if (kart_id != -1)
        m_previous_owner = World::getWorld()->getKart(kart_id);
}   // ItemState(const BareNetworkString& buffer)

// ------------------------------------------------------------------------
/** Sets the disappear counter depending on type.  */
void ItemState::setDisappearCounter()
{
    switch (m_type)
    {
    case ITEM_BUBBLEGUM:
        m_used_up_counter = stk_config->m_bubblegum_counter; break;
    case ITEM_EASTER_EGG:
        m_used_up_counter = -1; break;
    default:
        m_used_up_counter = -1;
    }   // switch
}   // setDisappearCounter

// -----------------------------------------------------------------------
/** Initialises an item.
 *  \param type Type for this item.
 *  \param xyz The position for this item.
 *  \param normal The normal for this item.
 */
void ItemState::initItem(ItemType type, const Vec3& xyz, const Vec3& normal)
{
    m_xyz               = xyz;
    m_original_rotation = shortestArcQuat(Vec3(0, 1, 0), normal);
    m_original_type     = ITEM_NONE;
    m_ticks_till_return = 0;
    setDisappearCounter();
}   // initItem

// ----------------------------------------------------------------------------
/** Update the state of the item, called once per physics frame.
 *  \param ticks Number of ticks to simulate. While this value is 1 when
 *         called during the normal game loop, during a rewind this value
 *         can be (much) larger than 1.
 */
void ItemState::update(int ticks)
{
    if (m_deactive_ticks > 0) m_deactive_ticks -= ticks;
    if (m_ticks_till_return>0)
    {
        m_ticks_till_return -= ticks;
    }   // if collected

}   // update

// ----------------------------------------------------------------------------
/** Called when the item is collected.
 *  \param kart The kart that collected the item.
 */
void ItemState::collected(const AbstractKart *kart)
{
    if (m_type == ITEM_EASTER_EGG)
    {
        // They will disappear 'forever'
        m_ticks_till_return = stk_config->time2Ticks(99999);
    }
    else if (m_used_up_counter > 0)
    {
        m_used_up_counter--;
        // Deactivates the item for a certain amount of time. It is used to
        // prevent bubble gum from hitting a kart over and over again (in each
        // frame) by giving it time to drive away.
        m_deactive_ticks = stk_config->time2Ticks(0.5f);
        // Set the time till reappear to -1 seconds --> the item will
        // reappear immediately.
        m_ticks_till_return = -1;
    }
    else
    {
        m_ticks_till_return = stk_config->time2Ticks(2.0f);
    }

    if (race_manager->isBattleMode())
    {
        m_ticks_till_return *= 3;
    }
}   // collected

// ----------------------------------------------------------------------------
/** Returns the graphical type of this item should be using (takes nolok into
 *  account). */
Item::ItemType ItemState::getGrahpicalType() const
{
    return m_previous_owner && m_previous_owner->getIdent() == "nolok" &&
        getType() == ITEM_BUBBLEGUM ?
        ITEM_BUBBLEGUM_NOLOK : getType();
}   // getGrahpicalType

//-----------------------------------------------------------------------------
/** Save item state at current ticks in server for live join
 */
void ItemState::saveCompleteState(BareNetworkString* buffer) const
{
    buffer->addUInt8((uint8_t)m_type).addUInt8((uint8_t)m_original_type)
        .addUInt32(m_ticks_till_return).addUInt32(m_item_id)
        .addUInt32(m_deactive_ticks).addUInt32(m_used_up_counter)
        .add(m_xyz).add(m_original_rotation)
        .addUInt8(m_previous_owner ?
            (int8_t)m_previous_owner->getWorldKartId() : (int8_t)-1);
}   // saveCompleteState

// ============================================================================
/** Constructor for an item.
 *  \param type Type of the item.
 *  \param xyz Location of the item.
 *  \param normal The normal upon which the item is placed (so that it can
 *         be aligned properly with the ground).
 *  \param mesh The mesh to be used for this item.
 *  \param owner 'Owner' of this item, i.e. the kart that drops it. This is
 *         used to deactivate this item for the owner, i.e. avoid that a kart
 *         'collects' its own bubble gum. NULL means no owner, and the item
 *         can be collected immediatley by any kart.
 */
Item::Item(ItemType type, const Vec3& xyz, const Vec3& normal,
           scene::IMesh* mesh, scene::IMesh* lowres_mesh,
           const AbstractKart *owner)
    : ItemState(type, owner)
{
    m_was_available_previously = true;
    m_distance_2        = 1.2f;
    initItem(type, xyz, normal);
    m_graphical_type    = getGrahpicalType();

    LODNode* lodnode =
        new LODNode("item", irr_driver->getSceneManager()->getRootSceneNode(),
                    irr_driver->getSceneManager());
    scene::ISceneNode* meshnode =
        irr_driver->addMesh(mesh, StringUtils::insertValues("item_%i", (int)type));

    if (lowres_mesh != NULL)
    {
        lodnode->add(35, meshnode, true);
        scene::ISceneNode* meshnode =
            irr_driver->addMesh(lowres_mesh,
                                StringUtils::insertValues("item_lo_%i", (int)type));
        lodnode->add(100, meshnode, true);
    }
    else
    {
        lodnode->add(100, meshnode, true);
    }
    m_node              = lodnode;
    setType(type);
    handleNewMesh(getGrahpicalType());

#ifdef DEBUG
    std::string debug_name("item: ");
    debug_name += getType();
    m_node->setName(debug_name.c_str());
#endif
    m_node->setAutomaticCulling(scene::EAC_FRUSTUM_BOX);
    m_node->setPosition(xyz.toIrrVector());
    Vec3 hpr;
    hpr.setHPR(getOriginalRotation());
    m_node->setRotation(hpr.toIrrHPR());
    m_node->grab();
}   // Item(type, xyz, normal, mesh, lowres_mesh)

//-----------------------------------------------------------------------------
/** Initialises the item. Note that m_distance_2 must be defined before calling
 *  this function, since it pre-computes some values based on this.
 *  \param type Type of the item.
 *  \param xyz Position of this item.
 *  \param normal Normal for this item.
 */
void Item::initItem(ItemType type, const Vec3 &xyz, const Vec3&normal)
{
    ItemState::initItem(type, xyz, normal);
    // Now determine in which quad this item is, and its distance
    // from the center within this quad.
    m_graph_node = Graph::UNKNOWN_SECTOR;
    m_distance_from_center = 9999.9f;
    m_avoidance_points[0] = NULL;
    m_avoidance_points[1] = NULL;

    // Check that Graph exist (it might not in battle mode without navmesh)
    if (Graph::get())
    {
        Graph::get()->findRoadSector(xyz, &m_graph_node);
    }
    if (DriveGraph::get() && m_graph_node != Graph::UNKNOWN_SECTOR)
    {
        // Item is on drive graph. Pre-compute the distance from center
        // of this item, which is used by the AI (mostly for avoiding items)
        Vec3 distances;
        DriveGraph::get()->spatialToTrack(&distances, getXYZ(), m_graph_node);
        m_distance_from_center = distances.getX();
        const DriveNode* dn = DriveGraph::get()->getNode(m_graph_node);
        const Vec3& right = dn->getRightUnitVector();
        // Give it 10% more space, since the kart will not always come
        // parallel to the drive line.
        Vec3 delta = right * sqrt(m_distance_2) * 1.3f;
        m_avoidance_points[0] = new Vec3(getXYZ() + delta);
        m_avoidance_points[1] = new Vec3(getXYZ() - delta);
    }

}   // initItem

//-----------------------------------------------------------------------------
void Item::setMesh(scene::IMesh* mesh, scene::IMesh* lowres_mesh)
{
#ifndef SERVER_ONLY
    if (m_node == NULL)
        return;

    unsigned i = 0;
    for (auto* node : m_node->getAllNodes())
    {
        scene::IMesh* m = i == 0 ? mesh : lowres_mesh;
        if (m == NULL)
        {
            continue;
        }
        SP::SPMeshNode* spmn = dynamic_cast<SP::SPMeshNode*>(node);
        if (spmn)
        {
            spmn->setMesh(static_cast<SP::SPMesh*>(m));
        }
        else
        {
            ((scene::IMeshSceneNode*)node)->setMesh(m);
        }
        i++;
    }
#endif
}   // setMesh

//-----------------------------------------------------------------------------
/** Removes an item.
 */
Item::~Item()
{
    if (m_node != NULL)
    {
        irr_driver->removeNode(m_node);
        m_node->drop();
    }
    if(m_avoidance_points[0])
        delete m_avoidance_points[0];
    if(m_avoidance_points[1])
        delete m_avoidance_points[1];
}   // ~Item

//-----------------------------------------------------------------------------
/** Resets before a race (esp. if a race is restarted).
 */
void Item::reset()
{
    m_was_available_previously = true;
    ItemState::reset();

    if (m_node != NULL)
    {
        m_node->setScale(core::vector3df(1,1,1));
        m_node->setVisible(true);
    }

}   // reset

// ----------------------------------------------------------------------------
void Item::handleNewMesh(ItemType type)
{
#ifndef SERVER_ONLY
    if (m_node == NULL)
        return;
    setMesh(ItemManager::get()->getItemModel(type),
        ItemManager::get()->getItemLowResolutionModel(type));
    for (auto* node : m_node->getAllNodes())
    {
        SP::SPMeshNode* spmn = dynamic_cast<SP::SPMeshNode*>(node);
        if (spmn)
            spmn->setGlowColor(ItemManager::get()->getGlowColor(type));
    }
    Vec3 hpr;
    hpr.setHPR(getOriginalRotation());
    m_node->setRotation(hpr.toIrrHPR());
#endif
}   // handleNewMesh

// ----------------------------------------------------------------------------
/** Updated the item - rotates it, takes care of items coming back into
 *  the game after it has been collected.
 *  \param ticks Number of physics time steps - should be 1.
 */
void Item::updateGraphics(float dt)
{
    if (m_node == NULL)
        return;

    if (m_graphical_type != getGrahpicalType())
    {
        handleNewMesh(getGrahpicalType());
        m_graphical_type = getGrahpicalType();
    }

    float time_till_return = stk_config->ticks2Time(getTicksTillReturn());
    bool is_visible = isAvailable() || time_till_return <= 1.0f ||
                      (getType() == ITEM_BUBBLEGUM &&
                       getOriginalType() == ITEM_NONE && !isUsedUp());

    m_node->setVisible(is_visible);
    m_node->setPosition(getXYZ().toIrrVector());

    if (!m_was_available_previously && isAvailable())
    {
        // This item is now available again - make sure it is not
        // scaled anymore.
        m_node->setScale(core::vector3df(1, 1, 1));
    }

    if (!isAvailable() && time_till_return <= 1.0f)
    {
        // Make it visible by scaling it from 0 to 1:
        if (rotating())
        {
            float angle =
                fmodf((float)(World::getWorld()->getTicksSinceStart() +
                getTicksTillReturn()) / 40.0f, M_PI * 2);
            btMatrix3x3 m;
            m.setRotation(getOriginalRotation());
            btQuaternion r = btQuaternion(m.getColumn(1), angle) *
                getOriginalRotation();
            Vec3 hpr;
            hpr.setHPR(r);
            m_node->setRotation(hpr.toIrrHPR());
        }
        m_node->setVisible(true);
        m_node->setScale(core::vector3df(1, 1, 1)*(1 - time_till_return));
    }
    if (isAvailable())
    {
        Vec3 hpr;
        if (rotating())
        {
            // have it rotate
            float angle =
                fmodf((float)World::getWorld()->getTicksSinceStart() / 40.0f,
                M_PI * 2);

            btMatrix3x3 m;
            m.setRotation(getOriginalRotation());
            btQuaternion r = btQuaternion(m.getColumn(1), angle) *
                getOriginalRotation();
            hpr.setHPR(r);
        }
        else
            hpr.setHPR(getOriginalRotation());
        m_node->setRotation(hpr.toIrrHPR());
    }   // if item is available
    m_was_available_previously = isAvailable();
}   // updateGraphics
