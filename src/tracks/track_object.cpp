//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 Joerg Henrichs, Marianne Gagnon
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
#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/lod_node.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/sp/sp_mesh_buffer.hpp"
#include "graphics/sp/sp_mesh_node.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "input/device_manager.hpp"
#include "items/item_manager.hpp"
#include "network/network_config.hpp"
#include "physics/physical_object.hpp"
#include "race/race_manager.hpp"
#include "scriptengine/script_engine.hpp"
#include "tracks/track.hpp"
#include "tracks/model_definition_loader.hpp"
#include "utils/string_utils.hpp"

#include <IAnimatedMeshSceneNode.h>
#include <ISceneManager.h>
#include <ge_render_info.hpp>

/** A track object: any additional object on the track. This object implements
 *  a graphics-only representation, i.e. there is no physical representation.
 *  Derived classes can implement a physical representation (see
 *  physics/physical_object) or animations.
 * \param xml_node The xml node from which the initial data is taken. This is
 *                 for now: initial position, initial rotation, name of the
 *                 model, enable/disable status, timer information.
 * \param lod_node Lod node (defaults to NULL).
 */
TrackObject::TrackObject(const XMLNode &xml_node, scene::ISceneNode* parent,
                         ModelDefinitionLoader& model_def_loader,
                         TrackObject* parent_library)
{
    init(xml_node, parent, model_def_loader, parent_library);
}   // TrackObject

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
    m_init_xyz        = xyz;
    m_init_hpr        = hpr;
    m_init_scale      = scale;
    m_enabled         = true;
    m_presentation    = NULL;
    m_animator        = NULL;
    m_parent_library  = NULL;
    m_interaction     = interaction;
    m_presentation    = presentation;
    m_is_driveable    = false;
    m_soccer_ball     = false;
    m_initially_visible = false;
    m_type            = "";

    if (m_interaction != "ghost" && m_interaction != "none" &&
        physics_settings )
    {
        m_physical_object = std::make_shared<PhysicalObject>
            (is_dynamic, *physics_settings, this);
    }

    reset();
}   // TrackObject

// ----------------------------------------------------------------------------
/** Initialises the track object based on the specified XML data.
 *  \param xml_node The XML data.
 *  \param parent The parent scene node.
 *  \param model_def_loader Used to load level-of-detail nodes.
 */
void TrackObject::init(const XMLNode &xml_node, scene::ISceneNode* parent,
                       ModelDefinitionLoader& model_def_loader,
                       TrackObject* parent_library)
{
    m_init_xyz   = core::vector3df(0,0,0);
    m_init_hpr   = core::vector3df(0,0,0);
    m_init_scale = core::vector3df(1,1,1);
    m_enabled    = true;
    m_initially_visible = false;
    m_presentation = NULL;
    m_animator = NULL;
    m_parent_library = parent_library;

    xml_node.get("id",      &m_id        );
    xml_node.get("model",   &m_name      );
    xml_node.get("xyz",     &m_init_xyz  );
    xml_node.get("hpr",     &m_init_hpr  );
    xml_node.get("scale",   &m_init_scale);
    xml_node.get("enabled", &m_enabled   );

    m_interaction = "static";
    xml_node.get("interaction", &m_interaction);
    xml_node.get("lod_group", &m_lod_group);

    m_is_driveable = false;
    xml_node.get("driveable", &m_is_driveable);

    bool lod_instance = false;
    xml_node.get("lod_instance", &lod_instance);

    m_soccer_ball = false;
    xml_node.get("soccer_ball", &m_soccer_ball);
    
    std::string type;
    xml_node.get("type",    &type );

    m_type = type;

    m_initially_visible = true;
    xml_node.get("if", &m_visibility_condition);
    if (m_visibility_condition == "false")
    {
        m_initially_visible = false;
    }
    if (!m_initially_visible)
        setEnabled(false);

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
    else if (xml_node.getName() == "library")
    {
        xml_node.get("name", &m_name);
        m_presentation = new TrackObjectPresentationLibraryNode(this, xml_node, model_def_loader);
        if (parent_library != NULL)
        {
            Track::getCurrentTrack()->addMetaLibrary(parent_library, this);
        }
    }
    else if (type == "sfx-emitter")
    {
        // FIXME: at this time sound emitters are just disabled in multiplayer
        //        otherwise the sounds would be constantly heard, for networking
        //        the index of item needs to be same so we create and disable it
        //        in TrackObjectPresentationSound constructor
        m_presentation = new TrackObjectPresentationSound(xml_node, parent,
            RaceManager::get()->getNumLocalPlayers() > 1);
    }
    else if (type == "action-trigger")
    {
        std::string action;
        xml_node.get("action", &action);
        m_name = action; //adds action as name so that it can be found by using getName()
        m_presentation = new TrackObjectPresentationActionTrigger(xml_node, parent_library);
    }
    else if (type == "billboard")
    {
        m_presentation = new TrackObjectPresentationBillboard(xml_node, parent);
    }
    else if (type=="cutscene_camera")
    {
        m_presentation = new TrackObjectPresentationEmpty(xml_node);
    }
    else
    {
        // Colorization settings
        std::string model_name;
        xml_node.get("model", &model_name);
#ifndef SERVER_ONLY
        scene::IMesh* mesh = NULL;
        if (model_name.size() > 0)
        {
            mesh = irr_driver->getMesh(model_name);
        }
        else
        {
            std::string group_name = "";
            xml_node.get("lod_group", &group_name);
            // Try to get the first mesh from lod groups
            mesh = model_def_loader.getFirstMeshFor(group_name);
        }

        // Use the first material in mesh to determine hue
        Material* colorized = NULL;
        if (mesh != NULL)
        {
            for (u32 j = 0; j < mesh->getMeshBufferCount(); j++)
            {
                scene::IMeshBuffer* buf = mesh->getMeshBuffer(j);
                SP::SPMeshBuffer* mb = dynamic_cast<SP::SPMeshBuffer*>(buf);
                if (!mb)
                {
                    Material* m = material_manager->getMaterialFor(buf
                        ->getMaterial().getTexture(0), buf);
                    if (m->isColorizable() && m->hasRandomHue())
                    {
                        colorized = m;
                        break;
                    }
                    continue;
                }
                std::vector<Material*> mbs = mb->getAllSTKMaterials();
                for (Material* m : mbs)
                {
                    if (m->isColorizable() && m->hasRandomHue())
                    {
                        colorized = m;
                        break;
                    }
                }
                if (colorized != NULL)
                {
                    break;
                }
            }
        }

        // If at least one material is colorizable, add RenderInfo for it
        if (colorized != NULL)
        {
            const float hue = colorized->getRandomHue();
            if (hue > 0.0f)
            {
                m_render_info = std::make_shared<GE::GERenderInfo>(hue);
            }
        }

#endif
        scene::ISceneNode *glownode = NULL;
        bool is_movable = false;
        if (lod_instance)
        {
            m_type = "lod";
            TrackObjectPresentationLOD* lod_node =
                new TrackObjectPresentationLOD(xml_node, parent, model_def_loader, m_render_info);
            m_presentation = lod_node;

            LODNode* node = (LODNode*)lod_node->getNode();
            if (type == "movable" && parent != NULL)
            {
                // HACK: unparent movables from their parent library object if any,
                // because bullet provides absolute transforms, not transforms relative
                // to the parent object
                node->updateAbsolutePosition();
                core::matrix4 absTransform = node->getAbsoluteTransformation();
                node->setParent(irr_driver->getSceneManager()->getRootSceneNode());
                node->setPosition(absTransform.getTranslation());
                node->setRotation(absTransform.getRotationDegrees());
                node->setScale(absTransform.getScale());
            }

            glownode = node->getAllNodes()[0];
        }
        else
        {
            m_type = "mesh";
            m_presentation = new TrackObjectPresentationMesh(xml_node,
                                                             m_enabled,
                                                             parent,
                                                             m_render_info);
            scene::ISceneNode* node = ((TrackObjectPresentationMesh *)m_presentation)->getNode();
            if (type == "movable" && parent != NULL)
            {
                // HACK: unparent movables from their parent library object if any,
                // because bullet provides absolute transforms, not transforms relative
                // to the parent object
                node->updateAbsolutePosition();
                core::matrix4 absTransform = node->getAbsoluteTransformation();
                node->setParent(irr_driver->getSceneManager()->getRootSceneNode());
                node->setPosition(absTransform.getTranslation());
                // Doesn't seem necessary to set rotation here, TODO: not sure why
                //node->setRotation(absTransform.getRotationDegrees());
                node->setScale(absTransform.getScale());
                is_movable = true;
            }

            glownode = node;
        }

        std::string render_pass;
        xml_node.get("renderpass", &render_pass);

        if (m_interaction != "ghost" && m_interaction != "none" &&
            render_pass != "skybox"                                     )
        {
            m_physical_object = PhysicalObject::fromXML(type == "movable",
                                                   xml_node,
                                                   this);
        }

        if (parent_library != NULL)
        {
            if (is_movable)
                parent_library->addMovableChild(this);
            else
                parent_library->addChild(this);
        }

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
    }


    if (type == "animation" || xml_node.hasChildNamed("curve"))
    {
        try
        {
            m_animator = new ThreeDAnimation(xml_node, this);
        }
        catch (std::exception& e)
        {
#ifndef SERVER_ONLY
            Log::debug("TrackObject", e.what());
#endif
        }
    }

    reset();

    if (!m_initially_visible)
        setEnabled(false);
    if (parent_library != NULL && !parent_library->isEnabled())
        setEnabled(false);
}   // TrackObject

// ----------------------------------------------------------------------------

void TrackObject::onWorldReady()
{
    if (m_visibility_condition == "false")
    {
        m_initially_visible = false;
    }
    else if (m_visibility_condition.size() > 0)
    {
        unsigned char result = -1;
        Scripting::ScriptEngine* script_engine = 
                                        Scripting::ScriptEngine::getInstance();

        std::ostringstream fn_signature;
        std::vector<std::string> arguments;
        if (m_visibility_condition.find("(") != std::string::npos && 
            m_visibility_condition.find(")") != std::string::npos)
        {
            // There are arguments to pass to the function
            // TODO: For the moment we only support string arguments
            // TODO: this parsing could be improved
            unsigned first = (unsigned)m_visibility_condition.find("(");
            unsigned last = (unsigned)m_visibility_condition.find_last_of(")");
            std::string fn_name = m_visibility_condition.substr(0, first);
            std::string str_arguments = m_visibility_condition.substr(first + 1, last - first - 1);
            arguments = StringUtils::split(str_arguments, ',');

            fn_signature << "bool " << fn_name << "(";

            for (unsigned int i = 0; i < arguments.size(); i++)
            {
                if (i > 0)
                    fn_signature << ",";
                fn_signature << "string";
            }

            fn_signature << ",Track::TrackObject@)";
        }
        else
        {
            fn_signature << "bool " << m_visibility_condition << "(Track::TrackObject@)";
        }

        TrackObject* self = this;
        script_engine->runFunction(true, fn_signature.str(),
            [&](asIScriptContext* ctx) 
            {
                for (unsigned int i = 0; i < arguments.size(); i++)
                {
                    ctx->SetArgObject(i, &arguments[i]);
                }
                ctx->SetArgObject((int)arguments.size(), self);
            },
            [&](asIScriptContext* ctx) { result = ctx->GetReturnByte(); });

        if (result == 0)
            m_initially_visible = false;
    }
    if (!m_initially_visible)
        setEnabled(false);
}

// ----------------------------------------------------------------------------

/** Destructor. Removes the node from the scene graph, and also
 *  drops the textures of the mesh. Sound buffers are also freed.
 */
TrackObject::~TrackObject()
{
    delete m_presentation;
    delete m_animator;
}   // ~TrackObject

// ----------------------------------------------------------------------------
/** Initialises an object before a race starts.
 */
void TrackObject::reset()
{
    if (m_presentation   ) m_presentation->reset();
    if (m_animator       ) m_animator->reset();
    if (m_physical_object) m_physical_object->reset();
}   // reset

// ----------------------------------------------------------------------------
/** Enables or disables this object. This affects the visibility, i.e.
 *  disabled objects will not be displayed anymore.
 *  \param mode Enable (true) or disable (false) this object.
 */
void TrackObject::setEnabled(bool enabled)
{
    m_enabled = enabled;

    if (m_presentation != NULL)
        m_presentation->setEnable(m_enabled);

    if (getType() == "mesh")
    {
        if (m_physical_object)
        {
            if (enabled)
                m_physical_object->addBody();
            else
                m_physical_object->removeBody();
        }
    }

    for (unsigned int i = 0; i < m_movable_children.size(); i++)
    {
        m_movable_children[i]->setEnabled(enabled);
    }
}   // setEnable

// ----------------------------------------------------------------------------

void TrackObject::resetEnabled()
{
    m_enabled = m_initially_visible;

    if (m_presentation != NULL)
        m_presentation->setEnable(m_initially_visible);

    if (getType() == "mesh")
    {
        if (m_physical_object)
        {
            if (m_initially_visible)
                m_physical_object->addBody();
            else
                m_physical_object->removeBody();
        }
    }

    for (unsigned int i = 0; i < m_movable_children.size(); i++)
    {
        m_movable_children[i]->resetEnabled();
    }
}   // resetEnabled

// ----------------------------------------------------------------------------
/** This updates all only graphical elements. It is only called once per
 *  rendered frame, not once per time step.
 *  float dt Time since last rame.
 */
void TrackObject::updateGraphics(float dt)
{
    if (m_presentation) m_presentation->updateGraphics(dt);
    if (m_physical_object) m_physical_object->updateGraphics(dt);
    if (m_animator) m_animator->updateWithWorldTicks(false/*has_physics*/);
}   // update

// ----------------------------------------------------------------------------
/** This updates once per physics time step.
 *  float dt Time since last rame.
 */
void TrackObject::update(float dt)
{
    if (m_presentation) m_presentation->update(dt);
    if (m_physical_object) m_physical_object->update(dt);
    if (m_animator) m_animator->updateWithWorldTicks(true/*has_physics*/);
}   // update


// ----------------------------------------------------------------------------
/** This reset all physical object moved by 3d animation back to current ticks
 */
void TrackObject::resetAfterRewind()
{
    if (!m_animator || !m_physical_object)
        return;
    m_animator->updateWithWorldTicks(true/*has_physics*/);
    btTransform new_trans;
    m_physical_object->getMotionState()->getWorldTransform(new_trans);
    m_physical_object->getBody()->setCenterOfMassTransform(new_trans);
    m_physical_object->getBody()->saveKinematicState(stk_config->ticks2Time(1));
}   // resetAfterRewind

// ----------------------------------------------------------------------------
/** Does a raycast against the track object. The object must have a physical
 *  object.
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
bool TrackObject::castRay(const btVector3 &from, 
                          const btVector3 &to, btVector3 *hit_point,
                          const Material **material, btVector3 *normal,
                          bool interpolate_normal) const
{
    if(!m_physical_object)
    {
        Log::warn("TrackObject", "Can't raycast on non-physical object.");
        return false;
    }
    return m_physical_object->castRay(from, to, hit_point, material, normal,
                                      interpolate_normal);
}   // castRay

// ----------------------------------------------------------------------------

void TrackObject::move(const core::vector3df& xyz, const core::vector3df& hpr,
                       const core::vector3df& scale, bool update_rigid_body,
                       bool isAbsoluteCoord)
{
    if (m_presentation != NULL)
        m_presentation->move(xyz, hpr, scale, isAbsoluteCoord);

    if (update_rigid_body && m_physical_object)
    {
        movePhysicalBodyToGraphicalNode(xyz, hpr);
    }
}   // move

// ----------------------------------------------------------------------------

void TrackObject::movePhysicalBodyToGraphicalNode(const core::vector3df& xyz,
                                                  const core::vector3df& hpr)
{
    // If we set a bullet position from an irrlicht position, we need to
    // get the absolute transform from the presentation object (as set in
    // the line before), since xyz etc here are only relative to a
    // potential parent scene node.
    TrackObjectPresentationSceneNode *tops =
        dynamic_cast<TrackObjectPresentationSceneNode*>(m_presentation);
    if (tops)
    {
        const core::matrix4 &m = tops->getNode()
            ->getAbsoluteTransformation();
        m_physical_object->move(m.getTranslation(), m.getRotationDegrees());
    }
    else
    {
        m_physical_object->move(xyz, hpr);
    }
}   // movePhysicalBodyToGraphicalNode

// ----------------------------------------------------------------------------
const core::vector3df& TrackObject::getPosition() const
{
    if (m_presentation != NULL)
        return m_presentation->getPosition();
    else
        return m_init_xyz;
}   // getPosition

// ----------------------------------------------------------------------------

const core::vector3df TrackObject::getAbsoluteCenterPosition() const
{
    if (m_presentation != NULL)
        return m_presentation->getAbsoluteCenterPosition();
    else
        return m_init_xyz;
}   // getAbsolutePosition

// ----------------------------------------------------------------------------

const core::vector3df TrackObject::getAbsolutePosition() const
{
    if (m_presentation != NULL)
        return m_presentation->getAbsolutePosition();
    else
        return m_init_xyz;
}   // getAbsolutePosition

// ----------------------------------------------------------------------------

const core::vector3df& TrackObject::getRotation() const
{
    if (m_presentation != NULL)
        return m_presentation->getRotation();
    else
        return m_init_xyz;
}  // getRotation

// ----------------------------------------------------------------------------

const core::vector3df& TrackObject::getScale() const
{
    if (m_presentation != NULL)
        return m_presentation->getScale();
    else
        return m_init_scale;
}   // getScale

// ----------------------------------------------------------------------------

void TrackObject::addMovableChild(TrackObject* child)
{
    if (!m_enabled)
        child->setEnabled(false);
    m_movable_children.push_back(child);
}

// ----------------------------------------------------------------------------

void TrackObject::addChild(TrackObject* child)
{
    if (!m_enabled)
        child->setEnabled(false);
    m_children.push_back(child);
}

// ----------------------------------------------------------------------------

// scripting function
void TrackObject::moveTo(const Scripting::SimpleVec3* pos, bool isAbsoluteCoord)
{
    move(core::vector3df(pos->getX(), pos->getY(), pos->getZ()),
        core::vector3df(0.0f, 0.0f, 0.0f), // TODO: preserve rotation
        core::vector3df(1.0f, 1.0f, 1.0f), // TODO: preserve scale
        true, // updateRigidBody
        isAbsoluteCoord);
}

// ----------------------------------------------------------------------------
scene::IAnimatedMeshSceneNode* TrackObject::getMesh()
{
    if (getPresentation<TrackObjectPresentationLOD>())
    {
        LODNode* ln = dynamic_cast<LODNode*>
            (getPresentation<TrackObjectPresentationLOD>()->getNode());
        if (ln && !ln->getAllNodes().empty())
        {
            scene::IAnimatedMeshSceneNode* an =
                dynamic_cast<scene::IAnimatedMeshSceneNode*>
                (ln->getFirstNode());
            if (an)
            {
                return an;
            }
        }
    }
    else if (getPresentation<TrackObjectPresentationMesh>())
    {
        scene::IAnimatedMeshSceneNode* an =
            dynamic_cast<scene::IAnimatedMeshSceneNode*>
            (getPresentation<TrackObjectPresentationMesh>()->getNode());
        if (an)
        {
            return an;
        }
    }
    Log::debug("TrackObject", "No animated mesh");
    return NULL;
}   // getMesh

// ----------------------------------------------------------------------------
/* This function will join this (if true) static track object to main track
 * model, the geometry creator in irrlicht will draw its physical shape to
 * triangle mesh, so it can be combined to main track mesh.
 * \return True if this track object is joinable and can be removed if needeed
 */
bool TrackObject::joinToMainTrack()
{
    // If no physical object or there is animator, skip it
    // Also no joining if will affect kart (like moveable, flatten...)
    if (!isEnabled() || !m_physical_object || hasAnimatorRecursively() ||
        m_physical_object->isDynamic() || m_physical_object->isCrashReset() ||
        m_physical_object->isExplodeKartObject() ||
        m_physical_object->isFlattenKartObject())
        return false;

    // Scripting exploding barrel is assumed to be joinable in networking
    // as it doesn't support it
    if (!NetworkConfig::get()->isNetworking() &&
        (!m_physical_object->getOnKartCollisionFunction().empty() ||
        !m_physical_object->getOnItemCollisionFunction().empty()))
        return false;

    // Skip driveable non-exact shape object
    // Notice driveable object should always has exact shape specified in
    // blender
    if (m_is_driveable && !m_physical_object->hasTriangleMesh())
        return false;

    m_physical_object->joinToMainTrack();
    // This will remove the separated body
    m_physical_object.reset();
    return true;
}   // joinToMainTrack

// ----------------------------------------------------------------------------
TrackObject* TrackObject::cloneToChild()
{
    // Only clone object that is enabled and has a physical object
    // Soccer ball is made disabled by soccer world to hide initially
    if ((isEnabled() || m_soccer_ball) && m_physical_object)
    {
        TrackObject* to_clone = new TrackObject(*this);
        // We handle visibility condition in main process already
        to_clone->m_visibility_condition.clear();
        to_clone->m_presentation = NULL;
        to_clone->m_render_info.reset();
        if (m_animator)
            to_clone->m_animator = m_animator->clone(to_clone);
        to_clone->m_parent_library = NULL;
        to_clone->m_movable_children.clear();
        to_clone->m_children.clear();
        to_clone->m_physical_object = m_physical_object->clone(to_clone);
        // All track objects need to be initially enabled in init
        to_clone->m_enabled = true;
        return to_clone;
    }
    return NULL;
}   // joinToMainTrack
