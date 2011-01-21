//  $Id$
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
    m_init_xyz   = core::vector3df(0,0,0);
    m_init_hpr   = core::vector3df(0,0,0);
    m_init_scale = core::vector3df(1,1,1);
    m_enabled    = true;
    m_is_looped  = false;

    xml_node.get("xyz",     &m_init_xyz  );
    xml_node.get("hpr",     &m_init_hpr  );
    xml_node.get("scale",   &m_init_scale);
    xml_node.get("enabled", &m_enabled   );
    xml_node.get("looped",  &m_is_looped );
    std::string model_name;
    xml_node.get("model",   &model_name  );

    // Some animated objects (billboards) don't use this scene node
    if(model_name=="")
    {
        m_node = NULL;
    }
    else
    {
        std::string full_path = World::getWorld()->getTrack()->getTrackFile(model_name);
        scene::IAnimatedMesh *mesh = irr_driver->getAnimatedMesh(full_path);
        if(!mesh)
        {
            // If the model isn't found in the track directory, look 
            // in STK's model directory.
            full_path = file_manager->getModelFile(model_name);
            mesh      = irr_driver->getAnimatedMesh(full_path);
            if(!mesh)
            {
                fprintf(stderr, "Warning: '%s' in '%s' not found and is ignored.\n",
                       xml_node.getName().c_str(), model_name.c_str());
                return;
            }   // if(!mesh)
        }

        scene::IAnimatedMeshSceneNode *node=irr_driver->addAnimatedMesh(mesh);
        m_node = node;
#ifdef DEBUG
        std::string debug_name = model_name+" (track-object)";
        m_node->setName(debug_name.c_str());
#endif
        m_frame_start = node->getStartFrame();
        xml_node.get("frame-start", &m_frame_start);

        m_frame_end = node->getEndFrame();
        xml_node.get("frame-end", &m_frame_end);

        if(!m_enabled)
            m_node->setVisible(false);

        m_node->setPosition(m_init_xyz);
        m_node->setRotation(m_init_hpr);
        m_node->setScale(m_init_scale);
    }
    reset();
}   // TrackObject

// ----------------------------------------------------------------------------
TrackObject::~TrackObject()
{
    if(m_node)
        irr_driver->removeNode(m_node);
}   // ~TrackObject

// ----------------------------------------------------------------------------
/** Initialises an object before a race starts.
 */
void TrackObject::reset()
{
    if(!m_node) return;
    if(m_node->getType()==scene::ESNT_ANIMATED_MESH)
    {
        scene::IAnimatedMeshSceneNode *a_node =
            (scene::IAnimatedMeshSceneNode*)m_node;

        a_node->setPosition(m_init_xyz);
        a_node->setRotation(m_init_hpr);
        a_node->setScale(m_init_scale);
        a_node->setLoopMode(m_is_looped);

        if(m_is_looped)
        {
            a_node->setFrameLoop(m_frame_start, m_frame_end);
        }
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
    if(m_node)
        m_node->setVisible(m_enabled);
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
