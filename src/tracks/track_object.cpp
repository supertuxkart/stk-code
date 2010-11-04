//  $Id: track_object.cpp 4308 2009-12-17 00:22:29Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Joerg Henrichs
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

#include "tracks/track_object.hpp"

#include "graphics/irr_driver.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "modes/world.hpp"
#include "tracks/track.hpp"

/** A track object: any additional object on the track. This object implements
 *  a graphics-only representation, i.e. there is no physical representation.
 *  Derived classes can implement a physical representation (see 
 *  physics/physical_object) or animations.
 * \param xml_node The xml node from which the initial data is taken. This is
 *                 for now: initial position, initial rotation, name of the
 *                 model, enable/disable status, timer information.
 */
TrackObject::TrackObject(const XMLNode &xml_node)
{

    std::string model_name;
    xml_node.get("model", &model_name);
    std::string full_path = World::getWorld()->getTrack()->getTrackFile(model_name);
    m_animated_mesh = irr_driver->getAnimatedMesh(full_path);
    if(!m_animated_mesh)
    {
        // If the model isn't found in the track directory, look 
        // in STK's model directory.
        full_path = file_manager->getModelFile(model_name);
        m_animated_mesh = irr_driver->getAnimatedMesh(full_path);
        if(!m_animated_mesh)
        {
            fprintf(stderr, "Warning: '%s' in '%s' not found and is ignored.\n",
                    xml_node.getName().c_str(), model_name.c_str());
            return;
        }   // if(!m_animated_mesh)
    }
    m_animated_node = irr_driver->addAnimatedMesh(m_animated_mesh);
#ifdef DEBUG
    std::string debug_name = model_name+" (track-object)";
    m_animated_node->setName(debug_name.c_str());
#endif

    // Get the information from the xml node.
    m_enabled = true;
    xml_node.get("enabled", &m_enabled);

    m_is_looped = false;
    xml_node.get("looped", &m_is_looped);

    m_frame_start = m_animated_node->getStartFrame();
    xml_node.get("frame-start", &m_frame_start);

    m_frame_end = m_animated_node->getEndFrame();
    xml_node.get("frame-end", &m_frame_end);

    if(!m_enabled)
        m_animated_node->setVisible(false);

    m_init_xyz   = core::vector3df(0,0,0);
    int result   = xml_node.get("xyz", &m_init_xyz);
    m_init_hpr   = core::vector3df(0,0,0);
    result       = xml_node.get("hpr", &m_init_hpr);
    m_init_scale = core::vector3df(1,1,1);
    result       = xml_node.get("scale", &m_init_scale);
    if(!XMLNode::hasP(result) ||
       !XMLNode::hasR(result))   // Needs perhaps pitch and roll
    {
    }
    m_animated_node->setPosition(m_init_xyz);
    m_animated_node->setRotation(m_init_hpr);
    m_animated_node->setScale(m_init_scale);
    m_animated_node->setMaterialFlag(video::EMF_LIGHTING, false);
}   // TrackObject

// ----------------------------------------------------------------------------
TrackObject::~TrackObject()
{
    irr_driver->removeNode(m_animated_node);
    irr_driver->removeMesh(m_animated_mesh);
}   // ~TrackObject

// ----------------------------------------------------------------------------
/** Initialises an object before a race starts.
 */
void TrackObject::reset()
{
    m_animated_node->setPosition(m_init_xyz);
    m_animated_node->setRotation(m_init_hpr);
    m_animated_node->setScale(m_init_scale);
    m_animated_node->setLoopMode(false);

    if(m_is_looped)
    {
        m_animated_node->setFrameLoop(m_frame_start, m_frame_end);
        m_animated_node->setLoopMode(true);
    }
}   // reset
// ----------------------------------------------------------------------------
/** Enables or disables this object. This affects the visibility, i.e. 
 *  disabled objects will not be displayed anymore.
 *  \param mode Enable (true) or disable (false) this object.
 */
void TrackObject::setEnable(bool mode)
{
    m_enabled = mode;
    m_animated_node->setVisible(m_enabled);
}   // setEnable
// ----------------------------------------------------------------------------
/** This function is called from irrlicht when a (non-looped) animation ends.
 */
void TrackObject::OnAnimationEnd(scene::IAnimatedMeshSceneNode* node)
{
}   // OnAnimationEnd

// ----------------------------------------------------------------------------
void TrackObject::update(float dt)
{
}   // update
// ----------------------------------------------------------------------------
