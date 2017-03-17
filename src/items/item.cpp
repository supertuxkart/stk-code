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
#include "karts/abstract_kart.hpp"
#include "modes/easter_egg_hunt.hpp"
#include "modes/three_strikes_battle.hpp"
#include "modes/world.hpp"
#include "tracks/arena_graph.hpp"
#include "tracks/drive_graph.hpp"
#include "tracks/drive_node.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"
#include "utils/vec3.hpp"

#include <IMeshSceneNode.h>
#include <ISceneManager.h>

Item::Item(ItemType type, const Vec3& xyz, const Vec3& normal,
           scene::IMesh* mesh, scene::IMesh* lowres_mesh)
{
    assert(type != ITEM_TRIGGER); // use other constructor for that

    m_distance_2        = 1.2f;
    initItem(type, xyz);

    m_original_rotation = shortestArcQuat(Vec3(0, 1, 0), normal);
    m_rotation_angle    = 0.0f;
    m_original_mesh     = mesh;
    m_original_lowmesh  = lowres_mesh;
    m_listener          = NULL;

    LODNode* lodnode    = new LODNode("item",
                                      irr_driver->getSceneManager()->getRootSceneNode(),
                                      irr_driver->getSceneManager());
    scene::IMeshSceneNode* meshnode = 
        irr_driver->addMesh(mesh, StringUtils::insertValues("item_%i", (int)type));

    if (lowres_mesh != NULL)
    {
        lodnode->add(35, meshnode, true);
        scene::IMeshSceneNode* meshnode = 
            irr_driver->addMesh(lowres_mesh, 
                                StringUtils::insertValues("item_lo_%i", (int)type));
        lodnode->add(100, meshnode, true);
    }
    else
    {
        lodnode->add(100, meshnode, true);
    }

    m_node              = lodnode;

    //m_node             = irr_driver->addMesh(mesh);
#ifdef DEBUG
    std::string debug_name("item: ");
    debug_name += m_type;
    m_node->setName(debug_name.c_str());
#endif
    Track::getCurrentTrack()->adjustForFog(m_node);
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
{
    m_distance_2        = distance*distance;
    initItem(ITEM_TRIGGER, xyz);
    m_original_rotation = btQuaternion(0, 0, 0, 1);
    m_rotation_angle    = 0.0f;
    m_original_mesh     = NULL;
    m_original_lowmesh  = NULL;
    m_node              = NULL;
    m_listener          = trigger;
}   // Item(xyz, distance, trigger)

//-----------------------------------------------------------------------------
/** Initialises the item. Note that m_distance_2 must be defined before calling
 *  this function, since it pre-computes some values based on this.
 *  \param type Type of the item.
 */
void Item::initItem(ItemType type, const Vec3 &xyz)
{
    m_type              = type;
    m_xyz               = xyz;
    m_event_handler     = NULL;
    m_item_id           = -1;
    m_collected         = false;
    m_original_type     = ITEM_NONE;
    m_deactive_time     = 0;
    m_time_till_return  = 0.0f;  // not strictly necessary, see isCollected()
    m_emitter           = NULL;
    m_rotate            = (type!=ITEM_BUBBLEGUM) && (type!=ITEM_TRIGGER);
    switch(m_type)
    {
    case ITEM_BUBBLEGUM:
        m_disappear_counter = stk_config->m_bubblegum_counter; break;
    case ITEM_EASTER_EGG:
        m_disappear_counter = -1; break;
    default:
        m_disappear_counter = -1;
    }
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
        DriveGraph::get()->spatialToTrack(&distances, m_xyz, m_graph_node);
        m_distance_from_center = distances.getX();
        const DriveNode* dn = DriveGraph::get()->getNode(m_graph_node);
        const Vec3& right = dn->getRightUnitVector();
        // Give it 10% more space, since the kart will not always come
        // parallel to the drive line.
        Vec3 delta = right * sqrt(m_distance_2) * 1.3f;
        m_avoidance_points[0] = new Vec3(m_xyz + delta);
        m_avoidance_points[1] = new Vec3(m_xyz - delta);
    }

}   // initItem

//-----------------------------------------------------------------------------
/** Sets the type of the item (and also derived attributes lile m_rotate
 *  \param type Type of the item.
 */
void Item::setType(ItemType type)
{
    m_type   = type;
    m_rotate = (type!=ITEM_BUBBLEGUM) && (type!=ITEM_TRIGGER);
}   // setType

//-----------------------------------------------------------------------------
/** Changes this item to be a new type for a certain amount of time.
 *  \param type New type of this item.
 *  \param mesh Mesh to use to display this item.
 */
void Item::switchTo(ItemType type, scene::IMesh *mesh, scene::IMesh *lowmesh)
{
    // triggers and easter eggs should not be switched
    if (m_type == ITEM_TRIGGER || m_type == ITEM_EASTER_EGG) return;

    m_original_type = m_type;
    setType(type);

    scene::ISceneNode* node = m_node->getAllNodes()[0];
    ((scene::IMeshSceneNode*)node)->setMesh(mesh);
#ifndef SERVER_ONLY
    if (lowmesh != NULL)
    {
        node = m_node->getAllNodes()[1];
        ((scene::IMeshSceneNode*)node)->setMesh(lowmesh);
        irr_driver->applyObjectPassShader(m_node->getAllNodes()[1]);
    }

    irr_driver->applyObjectPassShader(m_node->getAllNodes()[0]);

    Track::getCurrentTrack()->adjustForFog(m_node);
#endif
}   // switchTo

//-----------------------------------------------------------------------------
/** Switch  backs to the original item.
 */
void Item::switchBack()
{
    // triggers should not be switched
    if (m_type == ITEM_TRIGGER) return;

    // If the item is not switched, do nothing. This can happen if a bubble
    // gum is dropped while items are switched - when switching back, this
    // bubble gum has no original type.
    if(m_original_type==ITEM_NONE)
        return;

    setType(m_original_type);
    m_original_type = ITEM_NONE;

    scene::ISceneNode* node = m_node->getAllNodes()[0];
    ((scene::IMeshSceneNode*)node)->setMesh(m_original_mesh);
    if (m_original_lowmesh != NULL)
    {
        node = m_node->getAllNodes()[1];
        ((scene::IMeshSceneNode*)node)->setMesh(m_original_lowmesh);
    }

    Track::getCurrentTrack()->adjustForFog(m_node);
    Vec3 hpr;
    hpr.setHPR(m_original_rotation);
    m_node->setRotation(hpr.toIrrHPR());
}   // switchBack

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
    m_collected         = false;
    m_time_till_return  = 0.0f;
    m_deactive_time     = 0.0f;
    switch(m_type)
    {
    case ITEM_BUBBLEGUM:
        m_disappear_counter = stk_config->m_bubblegum_counter; break;
    case ITEM_EASTER_EGG:
        m_disappear_counter = -1; break;
    default:
        m_disappear_counter = -1;
    }
    if(m_original_type!=ITEM_NONE)
    {
        setType(m_original_type);
        m_original_type = ITEM_NONE;
    }

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
void Item::setParent(AbstractKart* parent)
{
    m_event_handler = parent;
    m_emitter       = parent;
    m_deactive_time = 1.5f;
}   // setParent

//-----------------------------------------------------------------------------
/** Updated the item - rotates it, takes care of items coming back into
 *  the game after it has been collected.
 *  \param dt Time step size.
 */
void Item::update(float dt)
{
    if(m_deactive_time > 0) m_deactive_time -= dt;

    if(m_collected)
    {
        m_time_till_return -= dt;
        if(m_time_till_return<0)
        {
            m_collected=false;

            if (m_node != NULL)
            {
                m_node->setScale(core::vector3df(1,1,1));
            }
        }   // time till return <0 --> is fully visible again
        else if ( m_time_till_return <=1.0f )
        {
            if (m_node != NULL)
            {
                // Make it visible by scaling it from 0 to 1:
                m_node->setVisible(true);
                m_node->setScale(core::vector3df(1,1,1)*(1-m_time_till_return));
            }
        }   // time till return < 1
    }   // if collected
    else
    {   // not m_collected

        if(!m_rotate || m_node == NULL) return;
        // have it rotate
        m_rotation_angle += dt * M_PI ;
        if (m_rotation_angle > M_PI * 2) m_rotation_angle -= M_PI * 2;

        btMatrix3x3 m;
        m.setRotation(m_original_rotation);
        btQuaternion r = btQuaternion(m.getColumn(1), m_rotation_angle) *
            m_original_rotation;

        Vec3 hpr;
        hpr.setHPR(r);
        m_node->setRotation(hpr.toIrrHPR());
        return;
    }   // not m_collected
}   // update

//-----------------------------------------------------------------------------
/** Is called when the item is hit by a kart.  It sets the flag that the item
 *  has been collected, and the time to return to the parameter.
 *  \param t Time till the object reappears (defaults to 2 seconds).
 */
void Item::collected(const AbstractKart *kart, float t)
{
    m_collected     = true;
    m_event_handler = kart;
    if(m_type==ITEM_EASTER_EGG)
    {
        m_time_till_return=99999;
        EasterEggHunt *world = dynamic_cast<EasterEggHunt*>(World::getWorld());
        assert(world);
        world->collectedEasterEgg(kart);
        if (m_node != NULL)
        {
            m_node->setVisible(false);
        }
    }
    else if(m_type==ITEM_BUBBLEGUM && m_disappear_counter>0)
    {
        m_disappear_counter --;
        // Deactivates the item for a certain amount of time. It is used to
        // prevent bubble gum from hitting a kart over and over again (in each
        // frame) by giving it time to drive away.
        m_deactive_time = 0.5f;
        // Set the time till reappear to -1 seconds --> the item will
        // reappear immediately.
        m_time_till_return = -1;
    }
    else
    {
        // Note if the time is negative, in update the m_collected flag will
        // be automatically set to false again.
        m_time_till_return = t;
        if (m_node != NULL)
        {
            m_node->setVisible(false);
        }
    }

    if (m_listener != NULL)
    {
        m_listener->onTriggerItemApproached();
    }

    if (dynamic_cast<ThreeStrikesBattle*>(World::getWorld()) != NULL)
    {
        m_time_till_return *= 3;
    }
}   // isCollected

