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
#include "network/rewind_manager.hpp"
#include "tracks/arena_graph.hpp"
#include "tracks/drive_graph.hpp"
#include "tracks/drive_node.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"

#include <IMeshSceneNode.h>
#include <ISceneManager.h>


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
    
// ----------------------------------------------------------------------------
/** Update the state of the item, called once per physics frame.
 *  \param ticks Number of ticks to simulate (typically 1).
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
    m_previous_owner = kart;
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

    if (race_manager->getMinorMode() == RaceManager::MINOR_MODE_BATTLE)
    {
        m_ticks_till_return *= 3;
    }
}   // collected

// ============================================================================
/** Constructor for an item.
 *  \param type Type of the item.
 *  \param xyz Location of the item.
 *  \param normal The normal upon which the item is placed (so that it can
 *         be aligned properly with the ground).
 *  \param mesh The mesh to be used for this item.
 *  \param is_predicted True if the creation of the item is predicted by
 *         a client. Only used in networking.
 */
Item::Item(ItemType type, const Vec3& xyz, const Vec3& normal,
           scene::IMesh* mesh, scene::IMesh* lowres_mesh, bool is_predicted)
    : ItemState(type)
{
    assert(type != ITEM_TRIGGER); // use other constructor for that

    m_was_available_previously = true;
    m_distance_2        = 1.2f;
    m_is_predicted      = is_predicted;
    initItem(type, xyz);

    m_original_rotation = shortestArcQuat(Vec3(0, 1, 0), normal);
    m_rotation_angle    = 0.0f;
    m_original_mesh     = mesh;
    m_original_lowmesh  = lowres_mesh;
    m_listener          = NULL;

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

#ifdef DEBUG
    std::string debug_name("item: ");
    debug_name += getType();
    m_node->setName(debug_name.c_str());
#endif
    m_node->setAutomaticCulling(scene::EAC_FRUSTUM_BOX);
    m_node->setPosition(xyz.toIrrVector());
    Vec3 hpr;
    hpr.setHPR(m_original_rotation);
    m_node->setRotation(hpr.toIrrHPR());
    m_node->grab();
}   // Item(type, xyz, normal, mesh, lowres_mesh)

//-----------------------------------------------------------------------------

/** \brief Constructor to create a trigger item.
  * Trigger items are invisible and can be used to trigger a behavior when
  * approaching a point.
  */
Item::Item(const Vec3& xyz, float distance, TriggerItemListener* trigger)
    : ItemState(ITEM_TRIGGER)
{
    m_is_predicted      = false;
    m_distance_2        = distance*distance;
    initItem(ITEM_TRIGGER, xyz);
    m_original_rotation = btQuaternion(0, 0, 0, 1);
    m_rotation_angle    = 0.0f;
    m_original_mesh     = NULL;
    m_original_lowmesh  = NULL;
    m_node              = NULL;
    m_listener          = trigger;
    m_was_available_previously = true;
}   // Item(xyz, distance, trigger)

//-----------------------------------------------------------------------------
/** Initialises the item. Note that m_distance_2 must be defined before calling
 *  this function, since it pre-computes some values based on this.
 *  \param type Type of the item.
 */
void Item::initItem(ItemType type, const Vec3 &xyz)
{
    ItemState::initItem(type, xyz);
    m_previous_owner    = NULL;
    m_rotate            = (getType()!=ITEM_BUBBLEGUM) && 
                          (getType()!=ITEM_TRIGGER    );
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
/** Sets the type of the item (and also derived attributes lile m_rotate
 *  \param type Type of the item.
 */
void Item::setType(ItemType type)
{
    ItemState::setType(type);
    m_rotate = (type!=ITEM_BUBBLEGUM) && (type!=ITEM_TRIGGER);
    
    if (m_node != NULL)
    {
        for (auto* node : m_node->getAllNodes())
        {
            SP::SPMeshNode* spmn = dynamic_cast<SP::SPMeshNode*>(node);
            if (spmn)
            {
                spmn->setGlowColor(ItemManager::get()->getGlowColor(type));
            }
        }
    }
}   // setType

//-----------------------------------------------------------------------------
/** Changes this item to be a new type for a certain amount of time.
 *  \param type New type of this item.
 *  \param mesh Mesh to use to display this item.
 */
void Item::switchTo(ItemType type, scene::IMesh *mesh, scene::IMesh *lowmesh)
{
    setMesh(mesh, lowmesh);
    ItemState::switchTo(type);
}   // switchTo

//-----------------------------------------------------------------------------
/** Switch  backs to the original item.
 */
void Item::switchBack()
{
    setMesh(m_original_mesh, m_original_lowmesh);
    
    if (ItemState::switchBack()) 
        return;

    if (m_node != NULL)
    {
        Vec3 hpr;
        hpr.setHPR(m_original_rotation);
        m_node->setRotation(hpr.toIrrHPR());
    }
}   // switchBack

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

//-----------------------------------------------------------------------------
/** Sets which karts dropped an item. This is used to avoid that a kart is
 *  affected by its own items.
 *  \param parent Kart that dropped the item.
 */
void Item::setParent(const AbstractKart* parent)
{
    m_previous_owner = parent;
    ItemState::setDeactivatedTicks(stk_config->time2Ticks(1.5f));
}   // setParent

// ----------------------------------------------------------------------------
/** Updated the item - rotates it, takes care of items coming back into
 *  the game after it has been collected.
 *  \param ticks Number of physics time steps - should be 1.
 */
void Item::updateGraphics(float dt)
{
    if (m_node == NULL)
        return;

    float time_till_return = stk_config->ticks2Time(getTicksTillReturn());
    bool is_visible = isAvailable() || time_till_return <= 1.0f || 
                      (getType() == ITEM_BUBBLEGUM && 
                       getOriginalType() == ITEM_NONE && !isUsedUp());

    m_node->setVisible(is_visible);

    if (!m_was_available_previously && isAvailable() )
    {
        // This item is now available again - make sure it is not
        // scaled anymore.
        m_node->setScale(core::vector3df(1, 1, 1));
    }

    if (!isAvailable() && time_till_return <= 1.0f)
    {
        // Make it visible by scaling it from 0 to 1:
        m_node->setVisible(true);
        m_node->setScale(core::vector3df(1, 1, 1)*(1 - time_till_return));
    }
    if (isAvailable() && m_rotate)
    {
        // have it rotate
        m_rotation_angle += dt * M_PI;
        if (m_rotation_angle > M_PI * 2) m_rotation_angle -= M_PI * 2;

        btMatrix3x3 m;
        m.setRotation(m_original_rotation);
        btQuaternion r = btQuaternion(m.getColumn(1), m_rotation_angle) *
                         m_original_rotation;

        Vec3 hpr;
        hpr.setHPR(r);
        m_node->setRotation(hpr.toIrrHPR());
    }   // if item is available
    m_was_available_previously = isAvailable();
}   // update

//-----------------------------------------------------------------------------
/** Is called when the item is hit by a kart.  It sets the flag that the item
 *  has been collected, and the time to return to the parameter.
 *  \param kart The kart that collected the item.
 */
void Item::collected(const AbstractKart *kart)
{
    ItemState::collected(kart);
    
    if (m_listener != NULL)
    {
        m_listener->onTriggerItemApproached();
    }

}   // isCollected

