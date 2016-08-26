//  SuperTuxKart - a fun racing game with go-kart
//
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

#include "tracks/track_object_presentation.hpp"

#include "audio/sfx_base.hpp"
#include "audio/sfx_buffer.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/user_config.hpp"
#include "graphics/camera.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/light.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/mesh_tools.hpp"
#include "graphics/particle_emitter.hpp"
#include "graphics/particle_kind_manager.hpp"
#include "graphics/stk_mesh_scene_node.hpp"
#include "graphics/render_info.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "input/device_manager.hpp"
#include "input/input_device.hpp"
#include "input/input_manager.hpp"
#include "items/item_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "modes/world.hpp"
#include "scriptengine/script_engine.hpp"
#include "states_screens/dialogs/tutorial_message_dialog.hpp"
#include "tracks/check_cylinder.hpp"
#include "tracks/check_manager.hpp"
#include "tracks/check_sphere.hpp"
#include "tracks/model_definition_loader.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "tracks/track_object_manager.hpp"

#include <IBillboardSceneNode.h>
#include <ICameraSceneNode.h>
#include <ILightSceneNode.h>
#include <IMeshManipulator.h>
#include <IMeshSceneNode.h>
#include <IParticleSystemSceneNode.h>
#include <ISceneManager.h>

// ----------------------------------------------------------------------------
TrackObjectPresentation::TrackObjectPresentation(const XMLNode& xml_node)
{
    m_init_xyz   = core::vector3df(0,0,0);
    m_init_hpr   = core::vector3df(0,0,0);
    m_init_scale = core::vector3df(1,1,1);

    if (!xml_node.get("xyz", &m_init_xyz  ))
    {
        // support for old deprecated syntax
        xml_node.getXYZ(&m_init_xyz);
    }

    xml_node.get("hpr", &m_init_hpr  );
    xml_node.get("scale",   &m_init_scale);
}   // TrackObjectPresentation

// ----------------------------------------------------------------------------
const core::vector3df& TrackObjectPresentationSceneNode::getPosition() const
{
    if (m_node == NULL) return m_init_xyz;
    return m_node->getPosition();
}   // getPosition

// ----------------------------------------------------------------------------
const core::vector3df TrackObjectPresentationSceneNode::getAbsolutePosition() const
{
    if (m_node == NULL) return m_init_xyz;
    m_node->updateAbsolutePosition();
    return m_node->getAbsolutePosition();
}   // getAbsolutePosition

// ----------------------------------------------------------------------------

const core::vector3df TrackObjectPresentationSceneNode::getAbsoluteCenterPosition() const
{
    if (m_node == NULL) return m_init_xyz;
    m_node->updateAbsolutePosition();
    core::aabbox3d<f32> bounds = m_node->getTransformedBoundingBox();
    return bounds.getCenter();
}

// ----------------------------------------------------------------------------
const core::vector3df& TrackObjectPresentationSceneNode::getRotation() const
{
    if (m_node == NULL) return m_init_hpr;
    return m_node->getRotation();
}   // getRotation

// ----------------------------------------------------------------------------
const core::vector3df& TrackObjectPresentationSceneNode::getScale() const
{
    if (m_node == NULL) return m_init_scale;
    return m_node->getScale();
}   // getScale

// ----------------------------------------------------------------------------
void TrackObjectPresentationSceneNode::move(const core::vector3df& xyz,
                                            const core::vector3df& hpr,
                                            const core::vector3df& scale,
                                            bool isAbsoluteCoord)
{
    if (m_node == NULL) return;

    if (m_node->getParent() != NULL && isAbsoluteCoord)
    {
        scene::ISceneNode* parent = m_node->getParent();
        m_node->setPosition((xyz - parent->getAbsolutePosition())
                            / parent->getScale());
    }
    else
    {
        m_node->setPosition(xyz);
    }
    m_node->setRotation(hpr);
    m_node->setScale(scale);
    m_node->updateAbsolutePosition();
}   // move

// ----------------------------------------------------------------------------
void TrackObjectPresentationSceneNode::setEnable(bool enabled)
{
    if (m_node != NULL && (!enabled || !m_force_always_hidden))
        m_node->setVisible(enabled);
}   // setEnable

// ----------------------------------------------------------------------------
void TrackObjectPresentationSceneNode::reset()
{
    if (m_node == NULL) return;

    m_node->setPosition(m_init_xyz);
    m_node->setRotation(m_init_hpr);
    m_node->setScale(m_init_scale);
}   // reset

// ----------------------------------------------------------------------------
TrackObjectPresentationEmpty::TrackObjectPresentationEmpty(const XMLNode& xml_node)
                            : TrackObjectPresentationSceneNode(xml_node)
{
    m_node = irr_driver->getSceneManager()->addEmptySceneNode();
    m_node->setPosition(m_init_xyz);
    m_node->setRotation(m_init_hpr);
    m_node->setScale(m_init_scale);
}   // TrackObjectPresentationEmpty

// ----------------------------------------------------------------------------
TrackObjectPresentationEmpty::~TrackObjectPresentationEmpty()
{
    irr_driver->removeNode(m_node);
}   // ~TrackObjectPresentationEmpty

// ----------------------------------------------------------------------------
TrackObjectPresentationLibraryNode::TrackObjectPresentationLibraryNode(
    TrackObject* parent,
    const XMLNode& xml_node,
    ModelDefinitionLoader& model_def_loader)
    : TrackObjectPresentationSceneNode(xml_node)
{
    std::string name;
    xml_node.get("name", &name);

    m_node = irr_driver->getSceneManager()->addEmptySceneNode();
#ifdef DEBUG
    m_node->setName(("libnode_" + name).c_str());
#endif

    XMLNode* libroot;
    std::string lib_path =
        file_manager->getAsset(FileManager::LIBRARY, name) + "/";

    bool create_lod_definitions = true;

    if (!model_def_loader.containsLibraryNode(name))
    {
        World* world = World::getWorld();
        Track* track = NULL;
        if (world != NULL)
            track = world->getTrack();
        std::string local_lib_node_path;
        std::string local_script_file_path;
        if (track != NULL)
        {
            local_lib_node_path = track->getTrackFile("library/" + name + "/node.xml");
            local_script_file_path = track->getTrackFile("library/" + name + "/scripting.as");
        }
        std::string lib_node_path = lib_path + "node.xml";
        std::string lib_script_file_path = lib_path + "scripting.as";

        if (local_lib_node_path.size() > 0 && file_manager->fileExists(local_lib_node_path))
        {
            lib_path = track->getTrackFile("library/" + name);
            libroot = file_manager->createXMLTree(local_lib_node_path);
            if (track != NULL)
                World::getWorld()->getScriptEngine()->loadScript(local_script_file_path, false);
        }
        else if (file_manager->fileExists(lib_node_path))
        {
            libroot = file_manager->createXMLTree(lib_node_path);
            if (track != NULL)
                World::getWorld()->getScriptEngine()->loadScript(lib_script_file_path, false);
        }
        else
        {
            Log::error("TrackObjectPresentationLibraryNode",
                "Cannot find library '%s'", lib_node_path.c_str());
            return;
        }

        if (libroot == NULL)
        {
            Log::error("TrackObjectPresentationLibraryNode",
                       "Cannot find library '%s'", lib_node_path.c_str());
            return;
        }

        file_manager->pushTextureSearchPath(lib_path + "/");
        file_manager->pushModelSearchPath(lib_path);
        material_manager->pushTempMaterial(lib_path + "/materials.xml");
        model_def_loader.addToLibrary(name, libroot);

        // Load LOD groups
        const XMLNode *lod_xml_node = libroot->getNode("lod");
        if (lod_xml_node != NULL)
        {
            for (unsigned int i = 0; i < lod_xml_node->getNumNodes(); i++)
            {
                const XMLNode* lod_group_xml = lod_xml_node->getNode(i);
                for (unsigned int j = 0; j < lod_group_xml->getNumNodes(); j++)
                {
                    model_def_loader.addModelDefinition(lod_group_xml->getNode(j));
                }
            }
        }
    }
    else
    {
        libroot = model_def_loader.getLibraryNodes()[name];
        assert(libroot != NULL);
        // LOD definitions are already created, don't create them again
        create_lod_definitions = false;
    }

    m_node->setPosition(m_init_xyz);
    m_node->setRotation(m_init_hpr);
    m_node->setScale(m_init_scale);
    m_node->updateAbsolutePosition();

    assert(libroot != NULL);
    World::getWorld()->getTrack()->loadObjects(libroot, lib_path, model_def_loader,
        create_lod_definitions, m_node, parent);
    m_parent = parent;
}   // TrackObjectPresentationLibraryNode

// ----------------------------------------------------------------------------
TrackObjectPresentationLibraryNode::~TrackObjectPresentationLibraryNode()
{
    irr_driver->removeNode(m_node);
}   // TrackObjectPresentationLibraryNode
// ----------------------------------------------------------------------------
void TrackObjectPresentationLibraryNode::move(const core::vector3df& xyz, const core::vector3df& hpr,
    const core::vector3df& scale, bool isAbsoluteCoord, bool moveChildrenPhysicalBodies)
{
    TrackObjectPresentationSceneNode::move(xyz, hpr, scale, isAbsoluteCoord);

    if (moveChildrenPhysicalBodies)
    {
        for (TrackObject* obj : m_parent->getChildren())
        {
            obj->reset();
            if (obj->getPhysicalObject() != NULL)
            {
                obj->movePhysicalBodyToGraphicalNode(obj->getAbsolutePosition(), obj->getRotation());
            }
        }
    }
}
// ----------------------------------------------------------------------------
TrackObjectPresentationLOD::TrackObjectPresentationLOD(const XMLNode& xml_node,
                                       scene::ISceneNode* parent,
                                       ModelDefinitionLoader& model_def_loader,
                                       RenderInfo* ri)
                          : TrackObjectPresentationSceneNode(xml_node)
{
    m_node = model_def_loader.instanciateAsLOD(&xml_node, parent, ri);
    if (m_node == NULL) throw std::runtime_error("Cannot load LOD node");
    m_node->setPosition(m_init_xyz);
    m_node->setRotation(m_init_hpr);
    m_node->setScale(m_init_scale);
}   // TrackObjectPresentationLOD

// ----------------------------------------------------------------------------
TrackObjectPresentationLOD::~TrackObjectPresentationLOD()
{
    if (m_node)
        irr_driver->removeNode(m_node);
}   // TrackObjectPresentationLOD

// ----------------------------------------------------------------------------
TrackObjectPresentationMesh::TrackObjectPresentationMesh(
                                                     const XMLNode& xml_node,
                                                     bool enabled,
                                                     scene::ISceneNode* parent,
                                                     RenderInfo* render_info)
                           : TrackObjectPresentationSceneNode(xml_node)
{
    m_is_looped  = false;
    m_mesh       = NULL;
    m_node       = NULL;

    xml_node.get("looped",  &m_is_looped );
    std::string model_name;
    xml_node.get("model",   &model_name  );

    m_render_info = render_info;
    m_model_file = model_name;
    m_is_in_skybox = false;
    std::string render_pass;
    xml_node.get("renderpass", &render_pass);

    // for backwards compatibility, if unspecified assume there is
    bool skeletal_animation = true;
    xml_node.get("skeletal-animation", &skeletal_animation);

    if (render_pass == "skybox")
    {
        m_is_in_skybox = true;
    }

    bool animated = skeletal_animation && (UserConfigParams::m_graphical_effects ||
                     World::getWorld()->getIdent() == IDENT_CUTSCENE);
    bool displacing = false;
    xml_node.get("displacing", &displacing);
    animated &= !displacing;

    if (animated)
        m_mesh = irr_driver->getAnimatedMesh(model_name);
    else
        m_mesh = irr_driver->getMesh(model_name);

    if (!m_mesh)
    {
        throw std::runtime_error("Model '" + model_name + "' cannot be found");
    }

    if (!animated)
    {
        m_mesh = MeshTools::createMeshWithTangents(m_mesh,
                                                   &MeshTools::isNormalMap);
    }
    init(&xml_node, parent, enabled);
}   // TrackObjectPresentationMesh

// ----------------------------------------------------------------------------
TrackObjectPresentationMesh::TrackObjectPresentationMesh(
                                                  scene::IAnimatedMesh* model,
                                                  const core::vector3df& xyz,
                                                  const core::vector3df& hpr,
                                                  const core::vector3df& scale)
                           : TrackObjectPresentationSceneNode(xyz, hpr, scale)
{
    m_is_looped    = false;
    m_is_in_skybox = false;
    m_mesh         = NULL;
    m_node         = NULL;
    m_mesh         = model;
    m_render_info  = NULL;
    init(NULL, NULL, true);
}   // TrackObjectPresentationMesh

// ----------------------------------------------------------------------------
TrackObjectPresentationMesh::TrackObjectPresentationMesh(
                                                 const std::string& model_file,
                                                 const core::vector3df& xyz,
                                                 const core::vector3df& hpr,
                                                 const core::vector3df& scale)
                           : TrackObjectPresentationSceneNode(xyz, hpr, scale)
{
    m_is_looped    = false;
    m_mesh         = NULL;
    m_node         = NULL;
    m_is_in_skybox = false;
    m_render_info  = NULL;
    bool animated  = (UserConfigParams::m_graphical_effects ||
                      World::getWorld()->getIdent() == IDENT_CUTSCENE);

    m_model_file = model_file;

    if (file_manager->fileExists(model_file))
    {
        if (animated)
            m_mesh = irr_driver->getAnimatedMesh(model_file);
        else
            m_mesh = irr_driver->getMesh(model_file);
    }

    if (!m_mesh)
    {
        throw std::runtime_error("Model '" + model_file + "' cannot be found");
    }

    init(NULL, NULL, true);
}   // TrackObjectPresentationMesh

// ----------------------------------------------------------------------------
void TrackObjectPresentationMesh::init(const XMLNode* xml_node,
                                       scene::ISceneNode* parent, bool enabled)
{
    // for backwards compatibility, if unspecified assume there is
    bool skeletal_animation = true;
    if(xml_node)
        xml_node->get("skeletal-animation", &skeletal_animation);

    bool animated = skeletal_animation && (UserConfigParams::m_graphical_effects ||
             World::getWorld()->getIdent() == IDENT_CUTSCENE);
    bool displacing = false;
    std::string interaction;
    if (xml_node)
    {
        xml_node->get("displacing", &displacing);
        xml_node->get("interaction", &interaction);
    }
    animated &= !displacing;

    m_mesh->grab();
    irr_driver->grabAllTextures(m_mesh);

    if (interaction == "physicsonly")
    {
        std::string type;
        xml_node->get("type", &type);
        if (type == "animation" || xml_node->hasChildNamed("curve"))
        {
            // Animated
            //m_node = irr_driver->getSceneManager()->addEmptySceneNode();
            m_node = irr_driver->addMesh(m_mesh, m_model_file, parent, m_render_info);
            enabled = false;
            m_force_always_hidden = true;
            m_frame_start = 0;
            m_frame_end = 0;
        }
        else
        {
            // Static
            m_node = irr_driver->addMesh(m_mesh, m_model_file, parent, m_render_info);
            enabled = false;
            m_force_always_hidden = true;
            m_frame_start = 0;
            m_frame_end = 0;

            if (World::getWorld() && World::getWorld()->getTrack() && xml_node)
                World::getWorld()->getTrack()->addPhysicsOnlyNode(m_node);
        }
    }
    else if (m_is_in_skybox)
    {
        // Tell the driver that this mesh is a part of the background
        scene::IMeshSceneNode * const node =
            irr_driver->getSceneManager()->addMeshSceneNode(m_mesh);
        node->grab();
        node->setParent(NULL);

        irr_driver->addBackgroundNode(node);

        m_node = node;
    }
    else if (animated)
    {
        scene::IAnimatedMeshSceneNode *node =
            irr_driver->addAnimatedMesh((scene::IAnimatedMesh*)m_mesh,
                                        m_model_file, parent, m_render_info);
        m_node = node;

        m_frame_start = node->getStartFrame();
        if (xml_node)
            xml_node->get("frame-start", &m_frame_start);

        m_frame_end = node->getEndFrame();
        if (xml_node)
            xml_node->get("frame-end", &m_frame_end);

        if (World::getWorld() && World::getWorld()->getTrack() && xml_node)
            World::getWorld()->getTrack()
            ->handleAnimatedTextures(m_node, *xml_node);
    }
    else
    {
        bool displacing = false;
        if (xml_node)
            xml_node->get("displacing", &displacing);

        m_node = irr_driver->addMesh(m_mesh, m_model_file, parent, m_render_info);

        STKMeshSceneNode* stkmesh = dynamic_cast<STKMeshSceneNode*>(m_node);
        if (displacing && stkmesh != NULL)
            stkmesh->setIsDisplacement(displacing);

        m_frame_start = 0;
        m_frame_end = 0;

        if (World::getWorld() && World::getWorld()->getTrack() && xml_node)
            World::getWorld()->getTrack()
                             ->handleAnimatedTextures(m_node, *xml_node);
    }

    if(!enabled)
        m_node->setVisible(false);

    m_node->setPosition(m_init_xyz);
    m_node->setRotation(m_init_hpr);
    m_node->setScale(m_init_scale);
}   // init

// ----------------------------------------------------------------------------
TrackObjectPresentationMesh::~TrackObjectPresentationMesh()
{
    if (m_node)
        irr_driver->removeNode(m_node);

    if(m_mesh)
    {
        irr_driver->dropAllTextures(m_mesh);
        m_mesh->drop();
        if(m_mesh->getReferenceCount()==1)
            irr_driver->removeMeshFromCache(m_mesh);
    }
}   // ~TrackObjectPresentationMesh

// ----------------------------------------------------------------------------
void TrackObjectPresentationMesh::reset()
{
    if (m_node->getType()==scene::ESNT_ANIMATED_MESH)
    {
        scene::IAnimatedMeshSceneNode *a_node =
            (scene::IAnimatedMeshSceneNode*)m_node;

        a_node->setPosition(m_init_xyz);
        a_node->setRotation(m_init_hpr);
        a_node->setScale(m_init_scale);
        a_node->setLoopMode(m_is_looped);
        a_node->setCurrentFrame((float)(a_node->getStartFrame()));

        // trick to reset the animation AND also the timer inside it
        a_node->OnAnimate(0);
        a_node->OnAnimate(0);

        // irrlicht's "setFrameLoop" is a misnomer, it just sets the first and
        // last frame, even if looping is disabled
        a_node->setFrameLoop(m_frame_start, m_frame_end);
    }
}   // reset

// ----------------------------------------------------------------------------
int TrackObjectPresentationMesh::getCurrentFrame()
{
    if (m_node->getType() == scene::ESNT_ANIMATED_MESH)
    {
        scene::IAnimatedMeshSceneNode *a_node =
            (scene::IAnimatedMeshSceneNode*)m_node;

        return (int)a_node->getFrameNr();
    }
    return -1; //Not a skeletal animation
}   // getCurrentFrame

// ----------------------------------------------------------------------------
void TrackObjectPresentationMesh::setCurrentFrame(int frame)
{
    if (m_node->getType() == scene::ESNT_ANIMATED_MESH)
    {
        scene::IAnimatedMeshSceneNode *a_node =
            (scene::IAnimatedMeshSceneNode*)m_node;

        a_node->setCurrentFrame((f32)frame);
    }
}   // setCurrentFrame

// ----------------------------------------------------------------------------
/** Set custom loops, as well as pause by scripts.
 *  \param start Start frame.
 *  \param end End frame.
 */
void TrackObjectPresentationMesh::setLoop(int start, int end)
{
    if (m_node->getType() == scene::ESNT_ANIMATED_MESH)
    {
        scene::IAnimatedMeshSceneNode *a_node =
            (scene::IAnimatedMeshSceneNode*)m_node;

        // irrlicht's "setFrameLoop" is a misnomer, it just sets the first and
        // last frame, even if looping is disabled
        a_node->setFrameLoop(start, end);
    }
}   // setLoop

// ----------------------------------------------------------------------------
TrackObjectPresentationSound::TrackObjectPresentationSound(
                                                     const XMLNode& xml_node,
                                                     scene::ISceneNode* parent)
                            : TrackObjectPresentation(xml_node)
{
    // TODO: respect 'parent' if any

    m_enabled = true;
    m_sound = NULL;
    m_xyz   = m_init_xyz;

    std::string sound;
    xml_node.get("sound", &sound);

    float rolloff = 0.5;
    xml_node.get("rolloff",  &rolloff );
    float volume = 1.0;
    xml_node.get("volume",   &volume );

    bool trigger_when_near = false;
    xml_node.get("play-when-near", &trigger_when_near);

    float trigger_distance = 1.0f;
    xml_node.get("distance", &trigger_distance);

    xml_node.get("conditions", &m_trigger_condition);

    float max_dist = 390.0f;
    xml_node.get("max_dist", &max_dist );

    // first try track dir, then global dir
    std::string soundfile = World::getWorld()->getTrack()->getTrackFile(sound);
    //std::string soundfile = file_manager->getAsset(FileManager::MODEL,sound);
    if (!file_manager->fileExists(soundfile))
    {
        soundfile = file_manager->getAsset(FileManager::SFX, sound);
    }

    SFXBuffer* buffer = new SFXBuffer(soundfile,
                                      true /* positional */,
                                      rolloff,
                                      max_dist,
                                      volume);
    buffer->load();

    m_sound = SFXManager::get()->createSoundSource(buffer, true, true);
    if (m_sound != NULL)
    {
        m_sound->setPosition(m_init_xyz);
        if (!trigger_when_near && m_trigger_condition.empty())
        {
            m_sound->setLoop(true);
            m_sound->play();
        }
    }
    else
        Log::error("TrackObject", "Sound emitter object could not be created.");

    if (trigger_when_near)
    {
        ItemManager::get()->newItem(m_init_xyz, trigger_distance, this);
    }
}   // TrackObjectPresentationSound

// ----------------------------------------------------------------------------
void TrackObjectPresentationSound::update(float dt)
{
    if (m_sound != NULL && m_enabled)
    {
        // muting when too far is implemented manually since not supported by
        // OpenAL so need to call this every frame to update the muting state
        // if listener moved
        m_sound->setPosition(m_xyz);
    }
}   // update

// ----------------------------------------------------------------------------
void TrackObjectPresentationSound::onTriggerItemApproached()
{
    if (m_sound != NULL && m_sound->getStatus() != SFXBase::SFX_PLAYING && m_enabled)
    {
        m_sound->play();
    }
}   // onTriggerItemApproached

// ----------------------------------------------------------------------------
void TrackObjectPresentationSound::triggerSound(bool loop)
{
    if (m_sound != NULL && m_enabled)
    {
        m_sound->setLoop(loop);
        m_sound->play();
    }
}   // triggerSound

// ----------------------------------------------------------------------------
void TrackObjectPresentationSound::stopSound()
{
    if (m_sound != NULL) 
        m_sound->stop();
}   // stopSound

// ----------------------------------------------------------------------------
TrackObjectPresentationSound::~TrackObjectPresentationSound()
{
    if (m_sound)
    {
        m_sound->deleteSFX();
    }
}   // ~TrackObjectPresentationSound

// ----------------------------------------------------------------------------
void TrackObjectPresentationSound::move(const core::vector3df& xyz, 
                                        const core::vector3df& hpr,
                                        const core::vector3df& scale,
                                        bool isAbsoluteCoord)
{
    m_xyz = xyz;
    if (m_sound != NULL && m_enabled)
        m_sound->setPosition(xyz);
}   // move

// ----------------------------------------------------------------------------

void TrackObjectPresentationSound::setEnable(bool enabled)
{
    if (enabled != m_enabled)
    {
        m_enabled = enabled;
        if (enabled)
            triggerSound(true);
        else
            stopSound();
    }
}

// ----------------------------------------------------------------------------
TrackObjectPresentationBillboard::TrackObjectPresentationBillboard(
                                                     const XMLNode& xml_node, 
                                                     scene::ISceneNode* parent)
                                : TrackObjectPresentationSceneNode(xml_node)
{
    std::string texture_name;
    float       width, height;

    m_fade_out_start = 50.0f;
    m_fade_out_end = 150.0f;

    xml_node.get("texture", &texture_name);
    xml_node.get("width",   &width       );
    xml_node.get("height",  &height      );

    m_fade_out_when_close = false;
    xml_node.get("fadeout", &m_fade_out_when_close);

    if (m_fade_out_when_close)
    {
        xml_node.get("start",  &m_fade_out_start);
        xml_node.get("end",    &m_fade_out_end  );
    }

    video::ITexture* texture =
        irr_driver->getTexture(file_manager->searchTexture(texture_name));
    if (texture == NULL)
    {
        Log::warn("TrackObjectPresentation", "Billboard texture '%s' not found",
                  texture_name.c_str());
    }
    m_node = irr_driver->addBillboard(core::dimension2df(width, height),
                                      texture, parent);
    Material *stk_material = material_manager->getMaterial(texture_name);
    stk_material->setMaterialProperties(&(m_node->getMaterial(0)), NULL);

    m_node->setPosition(m_init_xyz);
}   // TrackObjectPresentationBillboard

// ----------------------------------------------------------------------------
void TrackObjectPresentationBillboard::update(float dt)
{
    if (m_fade_out_when_close)
    {
        scene::ICameraSceneNode* curr_cam = irr_driver->getSceneManager()
                                                      ->getActiveCamera();
        const float dist =  m_node->getAbsolutePosition()
                           .getDistanceFrom( curr_cam->getPosition() );

        scene::IBillboardSceneNode* node = (scene::IBillboardSceneNode*)m_node;

        if (dist < m_fade_out_start)
        {
            node->setColor(video::SColor(0, 255, 255, 255));
        }
        else if (dist > m_fade_out_end)
        {
            node->setColor(video::SColor(255, 255, 255, 255));
        }
        else
        {
            int a = (int)(255*(dist - m_fade_out_start) 
                          / (m_fade_out_end - m_fade_out_start));
            node->setColor(video::SColor(a, 255, 255, 255));
        }
    }   // m_fade_out_when_close
}   // update

// ----------------------------------------------------------------------------
TrackObjectPresentationBillboard::~TrackObjectPresentationBillboard()
{
    if (m_node)
        irr_driver->removeNode(m_node);
}   // ~TrackObjectPresentationBillboard

// ----------------------------------------------------------------------------
TrackObjectPresentationParticles::TrackObjectPresentationParticles(
                                                     const XMLNode& xml_node, 
                                                     scene::ISceneNode* parent)
                                : TrackObjectPresentationSceneNode(xml_node)
{
    m_emitter = NULL;
    m_lod_emitter_node = NULL;

    std::string path;
    xml_node.get("kind", &path);
    
    int clip_distance = -1;
    xml_node.get("clip_distance", &clip_distance);
    xml_node.get("conditions",    &m_trigger_condition);

    bool auto_emit = true;
    xml_node.get("auto_emit", &auto_emit);

    m_delayed_stop = false;
    m_delayed_stop_time = 0.0;

    try
    {
        ParticleKind* kind = ParticleKindManager::get()->getParticles(path);
        if (kind == NULL)
        {
            throw std::runtime_error(path + " could not be loaded");
        }
        ParticleEmitter* emitter = new ParticleEmitter(kind, m_init_xyz, parent);


        if (clip_distance > 0)
        {
            scene::ISceneManager* sm = irr_driver->getSceneManager();
            scene::ISceneNode* sroot = sm->getRootSceneNode();
            LODNode* lod = new LODNode("particles", !parent ? sroot : parent, sm);
            lod->add(clip_distance, (scene::ISceneNode*)emitter->getNode(), true);
            m_node = lod;
            m_lod_emitter_node = lod;
            m_emitter = emitter;
        }
        else
        {
            m_node = emitter->getNode();
            m_emitter = emitter;
        }

        if (m_trigger_condition.size() > 0 || !auto_emit)
        {
            m_emitter->setCreationRateAbsolute(0.0f);
        }
    }
    catch (std::runtime_error& e)
    {
        Log::warn ("Track", "Could not load particles '%s'; cause :\n    %s",
                   path.c_str(), e.what());
    }
}   // TrackObjectPresentationParticles

// ----------------------------------------------------------------------------
TrackObjectPresentationParticles::~TrackObjectPresentationParticles()
{
    if (m_emitter)
    {
        if (m_lod_emitter_node != NULL)
        {
            irr_driver->removeNode(m_lod_emitter_node);
            m_emitter->unsetNode();
        }
        delete m_emitter; // this will also delete m_node
    }
}   // ~TrackObjectPresentationParticles

// ----------------------------------------------------------------------------
void TrackObjectPresentationParticles::update(float dt)
{
    if (m_emitter != NULL)
    {
        m_emitter->update(dt);
    }

    if (m_delayed_stop)
    {
        if (m_delayed_stop_time < 0.0f)
        {
            m_delayed_stop = false;
            stop();
        }
        m_delayed_stop_time -= dt;
    }
}   // update

// ----------------------------------------------------------------------------
void TrackObjectPresentationParticles::triggerParticles()
{
    if (m_emitter != NULL)
    {
        m_emitter->setCreationRateAbsolute(1.0f);
        m_emitter->setParticleType(m_emitter->getParticlesInfo());
    }
}   // triggerParticles
// ----------------------------------------------------------------------------
void TrackObjectPresentationParticles::stop()
{
    if (m_emitter != NULL)
    {
        m_emitter->setCreationRateAbsolute(0.0f);
        m_emitter->clearParticles();
    }
}
// ----------------------------------------------------------------------------
void TrackObjectPresentationParticles::stopIn(double delay)
{
    m_delayed_stop = true;
    m_delayed_stop_time = delay;
}
// ----------------------------------------------------------------------------
void TrackObjectPresentationParticles::setRate(float rate)
{
    if (m_emitter != NULL)
    {
        m_emitter->setCreationRateAbsolute(rate);
        m_emitter->setParticleType(m_emitter->getParticlesInfo());
    }
}
// ----------------------------------------------------------------------------
TrackObjectPresentationLight::TrackObjectPresentationLight(
                                                     const XMLNode& xml_node, 
                                                     scene::ISceneNode* parent)
                            : TrackObjectPresentationSceneNode(xml_node)
{
    xml_node.get("color", &m_color);
    const video::SColorf colorf(m_color);

    m_energy = 1.0f;
    xml_node.get("energy", &m_energy);

    m_distance = 20.f * m_energy;
    xml_node.get("distance", &m_distance);

    if (CVS->isGLSL())
    {
        m_node = irr_driver->addLight(m_init_xyz, m_energy, m_distance,
                                      colorf.r, colorf.g, colorf.b, false,
                                      parent);
    }
    else
    {
        m_node = NULL; // lights require shaders to work
    }
}   // TrackObjectPresentationLight

// ----------------------------------------------------------------------------
TrackObjectPresentationLight::~TrackObjectPresentationLight()
{
}   // ~TrackObjectPresentationLight
// ----------------------------------------------------------------------------
void TrackObjectPresentationLight::setEnergy(float energy)
{
    m_energy = energy;
    LightNode* lnode = dynamic_cast<LightNode*>(m_node);
    if (lnode != NULL)
    {
        lnode->setEnergy(energy);
    }
}
// ----------------------------------------------------------------------------
TrackObjectPresentationActionTrigger::TrackObjectPresentationActionTrigger(
                                                       const XMLNode& xml_node)
                                    :  TrackObjectPresentation(xml_node)
{
    float trigger_distance = 1.0f;
    xml_node.get("distance", &trigger_distance);
    xml_node.get("action",   &m_action        );

    std::string trigger_type;
    xml_node.get("trigger-type", &trigger_type);
    if (trigger_type == "point" || trigger_type.empty())
    {
        m_type = TRIGGER_TYPE_POINT;
    }
    else if (trigger_type == "cylinder")
    {
        m_type = TRIGGER_TYPE_CYLINDER;
    }
    else
    {
        assert(false);
    }

    m_action_active = true;

    if (m_action.size() == 0)
        Log::warn("TrackObject", "Action-trigger has no action defined.");

    if (m_type == TRIGGER_TYPE_POINT)
    {
        // TODO: rewrite as a sphere check structure?
        ItemManager::get()->newItem(m_init_xyz, trigger_distance, this);
        // CheckManager::get()->add(new CheckSphere(xml_node, 0 /* TODO what is this? */));
    }
    else if (m_type == TRIGGER_TYPE_CYLINDER)
    {
        CheckManager::get()->add(new CheckCylinder(xml_node, 0 /* TODO what is this? */, this));
    }
    else
    {
        assert(false);
    }
}   // TrackObjectPresentationActionTrigger

// ----------------------------------------------------------------------------
TrackObjectPresentationActionTrigger::TrackObjectPresentationActionTrigger(
                                                const core::vector3df& xyz,
                                                const std::string& script_name,
                                                float distance) 
                                    : TrackObjectPresentation(xyz)
{
    m_init_xyz             = xyz;
    m_init_hpr             = core::vector3df(0, 0, 0);
    m_init_scale           = core::vector3df(1, 1, 1);
    float trigger_distance = distance;
    m_action               = script_name;
    m_action_active        = true;
    m_type                 = TRIGGER_TYPE_POINT;
    ItemManager::get()->newItem(m_init_xyz, trigger_distance, this);
}   // TrackObjectPresentationActionTrigger

// ----------------------------------------------------------------------------
void TrackObjectPresentationActionTrigger::onTriggerItemApproached()
{
    if (!m_action_active) return;

    Scripting::ScriptEngine* script_engine =
        World::getWorld()->getScriptEngine();
    m_action_active = false; // TODO: allow auto re-activating?
    int idKart = 0;
    Camera* camera = Camera::getActiveCamera();
    if (camera != NULL && camera->getKart() != NULL)
        idKart = camera->getKart()->getWorldKartId();
    script_engine->runFunction(true, "void " + m_action + "(int)",
        [=](asIScriptContext* ctx) { ctx->SetArgDWord(0, idKart); });
}   // onTriggerItemApproached
