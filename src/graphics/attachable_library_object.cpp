//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 Joerg Henrichs, Marianne Gagnon
//  Copyright (C) 2026 Alayan
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

#include "graphics/attachable_library_object.hpp"

#include "animations/three_d_animation.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/sp/sp_mesh_buffer.hpp"
#include "graphics/sp/sp_mesh_node.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "utils/string_utils.hpp"

#include <IAnimatedMeshSceneNode.h>
#include <ISceneManager.h>
#include <ge_render_info.hpp>

/** An attachable library object: an object that's part of an attachable library
 * It is graphics-only, with no physics.
 * \param xml_node The xml node from which the initial data is taken. This is
 *                 for now: initial position, initial rotation, name of the
 *                 model, enable/disable status, timer information.
 */
AttachableLibraryObject::AttachableLibraryObject(const XMLNode &xml_node, scene::ISceneNode* parent)
{
    init(xml_node, parent);
}   // AttachableLibraryObject

/** Private constructor used when copying an existing AttachableLibraryObject */
AttachableLibraryObject::AttachableLibraryObject(const std::string& name, const std::string& id,
        core::vector3df xyz, core::vector3df hpr, core::vector3df scale,
        bool enabled, const std::string& type, scene::ISceneNode* parent,
        video::SColor color, float distance, float energy,
        const std::string& kind_path, int clip_distance,
        const std::string& trigger_condition, bool auto_emit,
        const std::string& model_path)
{
    m_name = name;
    m_id = id;
    m_init_xyz   = xyz;
    m_init_hpr   = hpr;
    m_init_scale = scale;
    m_enabled = enabled;
    m_type = type;

    if (m_type == "particle-emitter")
    {
        m_presentation = new TrackObjectPresentationParticles(parent,
            kind_path, clip_distance, trigger_condition, auto_emit,
            xyz, hpr, scale);
    }
    else if (m_type == "light")
    {
        m_presentation = new TrackObjectPresentationLight(parent,
            color, distance, energy, xyz, hpr, scale);
    }
    else if (m_type == "mesh")
    {
        m_presentation = new TrackObjectPresentationMesh(parent,
            model_path, xyz, hpr, scale, true /* no track */, m_name);
        // TODO some kind of function also called in init handling glow, etc.
    }

    // TODO : handle m_animator if it's useful
    m_animator = NULL;

    reset();
}   // AttachableLibraryObject

// ----------------------------------------------------------------------------
/** Initialises the track object based on the specified XML data.
 *  \param xml_node The XML data.
 *  \param parent The parent scene node.
 */
void AttachableLibraryObject::init(const XMLNode &xml_node, scene::ISceneNode* parent)
{
    m_init_xyz   = core::vector3df(0,0,0);
    m_init_hpr   = core::vector3df(0,0,0);
    m_init_scale = core::vector3df(1,1,1);
    m_enabled    = true;
    m_presentation = NULL;
    m_animator = NULL;

    xml_node.get("id",      &m_id        );
    xml_node.get("model",   &m_name      );
    xml_node.get("xyz",     &m_init_xyz  );
    xml_node.get("hpr",     &m_init_hpr  );
    xml_node.get("scale",   &m_init_scale);
    xml_node.get("enabled", &m_enabled   );

    std::string type;
    xml_node.get("type",    &type );

    m_type = type;

    if (xml_node.getName() == "particle-emitter")
    {
        m_type = "particle-emitter";
        m_presentation = new TrackObjectPresentationParticles(xml_node, parent);
    }
    else if (xml_node.getName() == "light")
    {
        m_type = "light";
        m_presentation = new TrackObjectPresentationLight(xml_node, parent);
    }
    else
    {
        m_type = "mesh";
        m_presentation = new TrackObjectPresentationMesh(xml_node, m_enabled,
            parent, m_render_info, true /* no track */);

// TODO : Add those back, they can be useful according to Sven
/*
        scene::ISceneNode* node = ((TrackObjectPresentationMesh *)m_presentation)->getNode();
        scene::ISceneNode *glownode = node;

        std::string render_pass;
        xml_node.get("renderpass", &render_pass);

        video::SColor glow;
        if (xml_node.get("glow", &glow) && glownode)
        {
            float r, g, b;
            r = glow.getRed() / 255.0f;
            g = glow.getGreen() / 255.0f;
            b = glow.getBlue() / 255.0f;
            SP::SPMeshNode* spmn = dynamic_cast<SP::SPMeshNode*>(glownode);
            if (spmn)
            {
                spmn->setGlowColor(video::SColorf(r, g, b));
            }
        }

        bool is_in_shadowpass = true;
        if (xml_node.get("shadow-pass", &is_in_shadowpass) && glownode)
        {
            SP::SPMeshNode* spmn = dynamic_cast<SP::SPMeshNode*>(glownode);
            if (spmn)
            {
                spmn->setInShadowPass(is_in_shadowpass);
            }
        }

        bool forcedbloom = false;
        if (xml_node.get("forcedbloom", &forcedbloom) && forcedbloom && glownode)
        {
            float power = 1;
            xml_node.get("bloompower", &power);
            btClamp(power, 0.5f, 10.0f);
            irr_driver->addForcedBloomNode(glownode, power);
        }
        */
    } // IMPORTANT

/* TODO : Support properly, as this might be useful. We skip for now.
    if (type == "animation" || xml_node.hasChildNamed("curve"))
    {
        try
        {
            m_animator = new ThreeDAnimation(xml_node, this);
        }
        catch (std::runtime_error& e)
        {
#ifndef SERVER_ONLY
            Log::debug("AttachableLibraryObject", e.what());
#endif
        }
    }
*/
    reset();
}   // init

// ----------------------------------------------------------------------------

/** Destructor. Removes the node from the scene graph, and also
 *  drops the textures of the mesh. Sound buffers are also freed.
 */
AttachableLibraryObject::~AttachableLibraryObject()
{
    delete m_presentation;
    delete m_animator;
}   // ~AttachableLibraryObject

// ----------------------------------------------------------------------------
/** Initialises an object before a race starts.
 */
void AttachableLibraryObject::reset()
{
    if (m_presentation   ) m_presentation->reset();
    if (m_animator       ) m_animator->reset();
}   // reset

// ----------------------------------------------------------------------------
/** This updates all only graphical elements. It is only called once per
 *  rendered frame, not once per time step.
 *  float dt Time since last rame.
 */
void AttachableLibraryObject::updateGraphics(float dt)
{
    if (m_presentation) m_presentation->updateGraphics(dt);
    if (m_animator) m_animator->updateWithWorldTicks(false/*has_physics*/);
}   // updateGraphics

AttachableLibraryObject* AttachableLibraryObject::clone(scene::ISceneNode* parent,
                                                    const std::string& lib_folder)
{
    // TODO : Add what's needed to create m_animator

    // Some parameters are only relevant for certain types.
    // We create default values that specific types will override.
    // Other types will ignore the irrelevant values
    video::SColor color;
    color.set(0);
    float distance = 0.0f;
    float energy = 0.0f;
    std::string kind_path = "";
    std::string trigger_condition = "";
    int clip_distance = -1;
    bool auto_emit = true;
    std::string model_path;
    if (m_type == "light")
    {
        TrackObjectPresentationLight* light_presentation =
            dynamic_cast<TrackObjectPresentationLight*>(m_presentation);
        color = light_presentation->getColor();
        distance = light_presentation->getDistance();
        energy = light_presentation->getEnergy();
    }
    else if (m_type == "particle-emitter")
    {
        TrackObjectPresentationParticles* particle_presentation =
            dynamic_cast<TrackObjectPresentationParticles*>(m_presentation);
        kind_path = particle_presentation->getKindPath();
        clip_distance = particle_presentation->getClipDistance();
        trigger_condition = particle_presentation->getTriggerCondition();
        auto_emit = particle_presentation->getAutoEmit();
    }
    else if (m_type == "mesh")
    {
        TrackObjectPresentationMesh* mesh_presentation =
            dynamic_cast<TrackObjectPresentationMesh*>(m_presentation);
        model_path = file_manager->getAsset(FileManager::LIBRARY, lib_folder);
        model_path += "/" + mesh_presentation->getModelFile();
    }

    AttachableLibraryObject *cloned_obj =
        new AttachableLibraryObject(m_name, m_id, m_init_xyz, m_init_hpr, m_init_scale,
            m_enabled, m_type, parent, color, distance, energy, kind_path, clip_distance,
            trigger_condition, auto_emit, model_path);

    return cloned_obj;
}   // clone