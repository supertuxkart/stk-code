//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 Joerg Henrichs, Marianne Gagnon
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

#include "animations/three_d_animation.hpp"
#include "graphics/irr_driver.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "input/device_manager.hpp"
#include "items/item_manager.hpp"
#include "physics/physical_object.hpp"
#include "race/race_manager.hpp"


/** A track object: any additional object on the track. This object implements
 *  a graphics-only representation, i.e. there is no physical representation.
 *  Derived classes can implement a physical representation (see 
 *  physics/physical_object) or animations.
 * \param xml_node The xml node from which the initial data is taken. This is
 *                 for now: initial position, initial rotation, name of the
 *                 model, enable/disable status, timer information.
 * \param lod_node Lod node (defaults to NULL).
 */
TrackObject::TrackObject(const XMLNode &xml_node, LODNode* lod_node)
{
    init(xml_node, lod_node);
}

// ----------------------------------------------------------------------------
/**
 * \param is_dynamic Only if interaction == 'movable', i.e. the object is
 *        affected by physics
 * \param physics_settings If interaction != 'ghost'
 */
TrackObject::TrackObject(const core::vector3df& xyz, const core::vector3df& hpr,
                         const core::vector3df& scale, const char* interaction,
                         TrackObjectPresentation* presentation,
                         bool is_dynamic, 
                         const PhysicalObject::Settings* physics_settings)
{
    m_init_xyz   = xyz;
    m_init_hpr   = hpr;
    m_init_scale = scale;
    m_enabled    = true;
    m_presentation = NULL;
    m_animator = NULL;
    m_rigid_body = NULL;
    m_interaction = interaction;
    
    m_presentation = presentation;
    
    if (m_interaction != "ghost" && m_interaction != "none" && 
        physics_settings )
    {
        m_rigid_body = new PhysicalObject(is_dynamic,
                                          *physics_settings,
                                          this);
    }
    
    reset();
}   // TrackObject

// ----------------------------------------------------------------------------

void TrackObject::init(const XMLNode &xml_node, LODNode* lod_node)
{
    m_init_xyz   = core::vector3df(0,0,0);
    m_init_hpr   = core::vector3df(0,0,0);
    m_init_scale = core::vector3df(1,1,1);
    m_enabled    = true;
    m_presentation = NULL;
    m_animator = NULL;
    
    m_rigid_body = NULL;
    
    xml_node.get("xyz",     &m_init_xyz  );
    xml_node.get("hpr",     &m_init_hpr  );
    xml_node.get("scale",   &m_init_scale);
    xml_node.get("enabled", &m_enabled   );

    m_interaction = "static";
    xml_node.get("interaction", &m_interaction);
    xml_node.get("lod_group", &m_lod_group);
    
    m_soccer_ball = false;
    xml_node.get("soccer_ball", &m_soccer_ball);
    
    std::string type;
    xml_node.get("type",    &type );

    m_type = type;


    if (xml_node.getName() == "particle-emitter")
    {
        m_type = "particle-emitter";
        m_presentation = new TrackObjectPresentationParticles(xml_node);
    }
    else if (type == "sfx-emitter")
    {
        // FIXME: at this time sound emitters are just disabled in multiplayer
        //        otherwise the sounds would be constantly heard
        if (race_manager->getNumLocalPlayers() < 2)
            m_presentation = new TrackObjectPresentationSound(xml_node);
    }
    else if (type == "action-trigger")
    {
        m_presentation = new TrackObjectPresentationActionTrigger(xml_node);
    }
    else if (type == "billboard")
    {
        m_presentation = new TrackObjectPresentationBillboard(xml_node);
    }
    else if (type=="cutscene_camera")
    {
        m_presentation = new TrackObjectPresentationEmpty(xml_node);
    }
    else
    {
        if (lod_node != NULL)
        {
            m_type = "lod";
            m_presentation = new TrackObjectPresentationLOD(xml_node, lod_node);
        }
        else
        {
            m_type = "mesh";
            m_presentation = new TrackObjectPresentationMesh(xml_node, 
                                                             m_enabled);
        }
        
        if (m_interaction != "ghost" && m_interaction != "none")
        {
            m_rigid_body = PhysicalObject::fromXML(type == "movable",
                                                   xml_node,
                                                   this);
        }
    }
    
    
    if (type == "animation" || xml_node.hasChildNamed("curve"))
    {
        m_animator = new ThreeDAnimation(xml_node, this);
    }
    
    reset();
}   // TrackObject

// ----------------------------------------------------------------------------

/** Destructor. Removes the node from the scene graph, and also
 *  drops the textures of the mesh. Sound buffers are also freed.
 */
TrackObject::~TrackObject()
{
    delete m_presentation;
    delete m_animator;
    delete m_rigid_body;
}   // ~TrackObject

// ----------------------------------------------------------------------------
/** Initialises an object before a race starts.
 */
void TrackObject::reset()
{
    if (m_presentation != NULL) m_presentation->reset();
    
    if (m_animator != NULL) m_animator->reset();
}   // reset
// ----------------------------------------------------------------------------
/** Enables or disables this object. This affects the visibility, i.e. 
 *  disabled objects will not be displayed anymore.
 *  \param mode Enable (true) or disable (false) this object.
 */
void TrackObject::setEnable(bool mode)
{
    m_enabled = mode;
    if (m_presentation != NULL) m_presentation->setEnable(m_enabled);
}   // setEnable
// ----------------------------------------------------------------------------
void TrackObject::update(float dt)
{
    if (m_presentation != NULL) m_presentation->update(dt);
    
    if (m_rigid_body != NULL) m_rigid_body->update(dt);
    
    if (m_animator != NULL) m_animator->update(dt);
}   // update


// ----------------------------------------------------------------------------

void TrackObject::move(const core::vector3df& xyz, const core::vector3df& hpr,
                       const core::vector3df& scale, bool updateRigidBody)
{
    if (m_presentation != NULL) m_presentation->move(xyz, hpr, scale);
    if (updateRigidBody && m_rigid_body != NULL) m_rigid_body->move(xyz, hpr);
}

// ----------------------------------------------------------------------------

const core::vector3df& TrackObject::getPosition() const
{
    if (m_presentation != NULL)
        return m_presentation->getPosition();
    else
        return m_init_xyz;
}

// ----------------------------------------------------------------------------

const core::vector3df& TrackObject::getRotation() const
{
    if (m_presentation != NULL)
        return m_presentation->getRotation();
    else
        return m_init_xyz;
}

// ----------------------------------------------------------------------------

const core::vector3df& TrackObject::getScale() const
{
    if (m_presentation != NULL)
        return m_presentation->getScale();
    else
        return m_init_xyz;
}
