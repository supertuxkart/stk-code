//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2013 Joerg Henrichs
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

#include "tracks/track_object_manager.hpp"

#include "animations/ipo.hpp"
#include "animations/three_d_animation.hpp"
#include "graphics/lod_node.hpp"
#include "graphics/material_manager.hpp"
#include "io/xml_node.hpp"
#include "physics/physical_object.hpp"
#include "tracks/track_object.hpp"
#include "utils/log.hpp"

#include <IMeshSceneNode.h>
#include <ISceneManager.h>

TrackObjectManager::TrackObjectManager()
{
}   // TrackObjectManager

// ----------------------------------------------------------------------------
TrackObjectManager::~TrackObjectManager()
{
}   // ~TrackObjectManager

// ----------------------------------------------------------------------------
/** Adds an object to the track object manager. The type to add is specified
 *  in the xml_node.
 * \note If you add add any objects with LOD, don't forget to call
 *       TrackObjectManager::assingLodNodes after everything is loaded
 *       to finalize their creation.
 *
 * FIXME: all of this is horrible, just make the exporter write LOD definitions
 *        in a separate section that's read before everything and remove all this
 *        crap
 */
void TrackObjectManager::add(const XMLNode &xml_node, scene::ISceneNode* parent,
                             ModelDefinitionLoader& model_def_loader)
{
    try
    {
        TrackObject *obj = new TrackObject(xml_node, parent, model_def_loader);
        m_all_objects.push_back(obj);
        if(obj->isDriveable())
            m_driveable_objects.push_back(obj);
    }
    catch (std::exception& e)
    {
        Log::warn("TrackObjectManager", "Could not load track object. Reason : %s",
                  e.what());
    }
}   // add

// ----------------------------------------------------------------------------
/** Initialises all track objects.
 */
void TrackObjectManager::init()
{

    TrackObject* curr;
    for_in (curr, m_all_objects)
    {
        curr->init();
    }
}   // reset
// ----------------------------------------------------------------------------
/** Initialises all track objects.
 */
void TrackObjectManager::reset()
{
    TrackObject* curr;
    for_in (curr, m_all_objects)
    {
        curr->reset();
        if (!curr->isEnabled())
        {
            //PhysicalObjects may need to be added
            if (curr->getType() == "mesh")
            {
                if (curr->getPhysicalObject() != NULL)
                    curr->getPhysicalObject()->addBody();
            }
        }
        curr->setEnable(true);
    }
}   // reset

// ----------------------------------------------------------------------------
/** disables all track objects with a particular ID
 *  \param name Name or ID for disabling
 */
void TrackObjectManager::disable(std::string name)
{
     TrackObject* curr;
     for_in (curr,m_all_objects)
     {
        if (curr->getName() == (name) || curr->getID() == (name))
        {

			curr->setEnable(false);
            if (curr->getType() == "mesh")
            {
                if (curr->getPhysicalObject()!=NULL)
                    curr->getPhysicalObject()->removeBody();
            }
        }
     }
}
// ----------------------------------------------------------------------------
/** enables all track objects with a particular ID
 *  \param name Name or ID for enabling
 */
void TrackObjectManager::enable(std::string name)
{
    TrackObject* curr;
    for_in (curr,m_all_objects)
    {
        if (curr->getName() == (name) || curr->getID() == (name))
        {
            curr->reset();
            curr->setEnable(true);
            if (curr->getType() == "mesh")
            {
                if (curr->getPhysicalObject() != NULL)
                curr->getPhysicalObject()->addBody();
            }
        }
    }
}

// ----------------------------------------------------------------------------
/**  returns activation status for all track objects
 *   with a particular ID
 *   \param name Name or ID of track object
 */
bool TrackObjectManager::getStatus(std::string name)
{
     TrackObject* curr;
     for_in (curr,m_all_objects){
            if (curr->getName() == (name)||curr->getID()==(name))
            {

				return curr->isEnabled();
            
            }
     }
     //object not found
     return false;
}
// ----------------------------------------------------------------------------
/** returns a reference to the track object
 *  with a particular ID
 *  \param name Name or ID of track object
 */
TrackObject* TrackObjectManager::getTrackObject(std::string name)
{
    TrackObject* curr;
    for_in(curr, m_all_objects)
    {
        if (curr->getName() == (name) || curr->getID() == (name))
        {

            return curr;

        }
    }
    //object not found
    return NULL;
}
/** Handles an explosion, i.e. it makes sure that all physical objects are
 *  affected accordingly.
 *  \param pos  Position of the explosion.
 *  \param obj  If the hit was a physical object, this object will be affected
 *              more. Otherwise this is NULL.
 *  \param secondary_hits True if items that are not directly hit should
 *         also be affected.
 */

void TrackObjectManager::handleExplosion(const Vec3 &pos, const PhysicalObject *mp,
                                         bool secondary_hits)
{
    TrackObject* curr;
    for_in (curr, m_all_objects)
    {
        if(secondary_hits || mp == curr->getPhysicalObject())
            curr->handleExplosion(pos, mp == curr->getPhysicalObject());
    }
}   // handleExplosion

// ----------------------------------------------------------------------------
/** Updates all track objects.
 *  \param dt Time step size.
 */
void TrackObjectManager::update(float dt)
{
    TrackObject* curr;
    for_in (curr, m_all_objects)
    {
        curr->update(dt);
    }
}   // update

// ----------------------------------------------------------------------------
/** Does a raycast against all driveable objects. This way part of the track
 *  can be a physical object, and can e.g. be animated. A separate list of all
 *  driveable objects is maintained (in one case there were over 2000 bodies,
 *  but only one is driveable). The result of the raycast against the track
 *  mesh are the input parameter. It is then tested if the raycast against 
 *  a track object gives a 'closer' result. If so, the parameters hit_point,
 *  normal, and material will be updated.
 *  \param from/to The from and to position for the raycast.
 *  \param xyz The position in world where the ray hit.
 *  \param material The material of the mesh that was hit.
 *  \param normal The intrapolated normal at that position.
 *  \param interpolate_normal If true, the returned normal is the interpolated
 *         based on the three normals of the triangle and the location of the
 *         hit point (which is more compute intensive, but results in much
 *         smoother results).
 *  \return True if a triangle was hit, false otherwise (and no output
 *          variable will be set.
 
 */
void TrackObjectManager::castRay(const btVector3 &from, 
                                 const btVector3 &to, btVector3 *hit_point, 
                                 const Material **material,
                                 btVector3 *normal,
                                 bool interpolate_normal) const
{
    float distance = 9999.9f;
    // If there was a hit already, compute the current distance
    if(*material)
    {
        distance = hit_point->distance(from);
    }
    const TrackObject* curr;
    for_in (curr, m_driveable_objects)
    {
        btVector3 new_hit_point;
        const Material *new_material;
        btVector3 new_normal;
        if(curr->castRay(from, to, &new_hit_point, &new_material, &new_normal,
                      interpolate_normal))
        {
            float new_distance = new_hit_point.distance(from);
            // If the new hit is closer than the current hit, save
            // the data.
            if (new_distance < distance)
            {
                *material  = new_material;
                *hit_point = new_hit_point;
                *normal    = new_normal;
                distance   = new_distance;
            }   // if new_distance < distance
        }   // if hit
    }   // for all track objects.
}   // castRay

// ----------------------------------------------------------------------------
/** Enables or disables fog for a given scene node.
 *  \param node The node to adjust.
 *  \param enable True if fog is enabled, otherwise fog is disabled.
 */
void adjustForFog(scene::ISceneNode *node, bool enable)
{
    if (node->getType() == scene::ESNT_MESH   ||
        node->getType() == scene::ESNT_OCTREE ||
        node->getType() == scene::ESNT_ANIMATED_MESH)
    {
        scene::IMesh* mesh;
        if (node->getType() == scene::ESNT_ANIMATED_MESH) {
            mesh = ((scene::IAnimatedMeshSceneNode*)node)->getMesh();
        }
        else {
            mesh = ((scene::IMeshSceneNode*)node)->getMesh();
        }

        unsigned int n = mesh->getMeshBufferCount();
        for (unsigned int i=0; i<n; i++)
        {
            scene::IMeshBuffer *mb = mesh->getMeshBuffer(i);
            video::SMaterial &irr_material=mb->getMaterial();
            for (unsigned int j=0; j<video::MATERIAL_MAX_TEXTURES; j++)
            {
                video::ITexture* t = irr_material.getTexture(j);
                if (t) material_manager->adjustForFog(t, mb, node, enable);

            }   // for j<MATERIAL_MAX_TEXTURES
        }  // for i<getMeshBufferCount()
    }
    else
    {
        node->setMaterialFlag(video::EMF_FOG_ENABLE, enable);
    }

    if (node->getType() == scene::ESNT_LOD_NODE)
    {
        std::vector<scene::ISceneNode*>&
            subnodes = ((LODNode*)node)->getAllNodes();
        for (unsigned int n=0; n<subnodes.size(); n++)
        {
            adjustForFog(subnodes[n], enable);
        }
    }
}   // adjustForFog


// ----------------------------------------------------------------------------
/** Enable or disable fog on objects.
  */
void TrackObjectManager::enableFog(bool enable)
{
    TrackObject* curr;
    for_in (curr, m_all_objects)
    {
        TrackObjectPresentationMesh* meshPresentation =
            curr->getPresentation<TrackObjectPresentationMesh>();
        if (meshPresentation!= NULL)
        {
            adjustForFog(meshPresentation->getNode(), enable);
        }
    }
}   // enableFog

// ----------------------------------------------------------------------------

void TrackObjectManager::insertObject(TrackObject* object)
{
    m_all_objects.push_back(object);
}

// ----------------------------------------------------------------------------
/** Removes the object from the scene graph, bullet, and the list of
 *  track objects, and then frees the object.
 *  \param obj The physical object to remove.
 */
void TrackObjectManager::removeObject(TrackObject* obj)
{
    m_all_objects.remove(obj);
    delete obj;
}   // removeObject

// ----------------------------------------------------------------------------

/*
void TrackObjectManager::assingLodNodes(const std::vector<LODNode*>& lod_nodes)
{
    for (unsigned int n=0; n<lod_nodes.size(); n++)
    {
        std::vector<const XMLNode*>& queue = m_lod_objects[ lod_nodes[n]->getGroupName() ];
        assert( queue.size() > 0 );
        const XMLNode* xml = queue[ queue.size() - 1 ];

        TrackObject* obj = new TrackObject(*xml, lod_nodes[n]->getParent(), lod_nodes[n]);
        queue.erase( queue.end() - 1 );

        m_all_objects.push_back(obj);
    }

    m_lod_objects.clear();
}
*/
