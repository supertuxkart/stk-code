//  SuperTuxKart - a fun racing game with go-kart
//
//  Copyright (C) 2013-2013 Joerg Henrichs, Marianne Gagnon
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
#include "graphics/irr_driver.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/mesh_tools.hpp"
#include "graphics/particle_emitter.hpp"
#include "graphics/particle_kind_manager.hpp"
#include "graphics/stkmeshscenenode.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "input/device_manager.hpp"
#include "input/input_device.hpp"
#include "input/input_manager.hpp"
#include "items/item_manager.hpp"
#include "modes/world.hpp"
#include "states_screens/dialogs/race_paused_dialog.hpp"
#include "states_screens/dialogs/tutorial_message_dialog.hpp"
#include "tracks/model_definition_loader.hpp"
#include "tracks/track.hpp"
#include "tracks/track_object_manager.hpp"
#include "scriptengine/script_engine.hpp"
#include <ISceneManager.h>
#include <IMeshSceneNode.h>
#include <ICameraSceneNode.h>
#include <IBillboardSceneNode.h>
#include <IParticleSystemSceneNode.h>
#include <ILightSceneNode.h>
#include <IMeshManipulator.h>

// ----------------------------------------------------------------------------

TrackObjectPresentation::TrackObjectPresentation(const XMLNode& xml_node)
{
    m_init_xyz   = core::vector3df(0,0,0);
    m_init_hpr   = core::vector3df(0,0,0);
    m_init_scale = core::vector3df(1,1,1);


    if (!xml_node.get("xyz",     &m_init_xyz  ))
    {
        // support for old deprecated syntax
        xml_node.getXYZ(&m_init_xyz);
    }

    xml_node.get("hpr",     &m_init_hpr  );
    xml_node.get("scale",   &m_init_scale);
}


// ----------------------------------------------------------------------------

const core::vector3df& TrackObjectPresentationSceneNode::getPosition() const
{
    if (m_node == NULL) return m_init_xyz;
    return m_node->getPosition();
}

const core::vector3df TrackObjectPresentationSceneNode::getAbsolutePosition() const
{
    if (m_node == NULL) return m_init_xyz;
    m_node->updateAbsolutePosition();
    return m_node->getAbsolutePosition();
}

 
const core::vector3df& TrackObjectPresentationSceneNode::getRotation() const
{
    if (m_node == NULL) return m_init_hpr;
    return m_node->getRotation();
}

const core::vector3df& TrackObjectPresentationSceneNode::getScale() const
{
    if (m_node == NULL) return m_init_scale;
    return m_node->getScale();
}


void TrackObjectPresentationSceneNode::move(const core::vector3df& xyz, const core::vector3df& hpr,
    const core::vector3df& scale)
{
    if (m_node == NULL) return;

    if (m_node->getParent() != NULL)
    {
        scene::ISceneNode* parent = m_node->getParent();
        m_node->setPosition((xyz - parent->getAbsolutePosition()) / parent->getScale());
    }
    else
    {
        m_node->setPosition(xyz);
    }
    m_node->setRotation(hpr);
    m_node->setScale(scale);
    m_node->updateAbsolutePosition();
}

void TrackObjectPresentationSceneNode::setEnable(bool enabled)
{
    if (m_node != NULL)
        m_node->setVisible(enabled);
}

void TrackObjectPresentationSceneNode::reset()
{
    if (m_node == NULL) return;

    m_node->setPosition(m_init_xyz);
    m_node->setRotation(m_init_hpr);
    m_node->setScale(m_init_scale);
}

// ----------------------------------------------------------------------------

TrackObjectPresentationEmpty::TrackObjectPresentationEmpty(const XMLNode& xml_node) :
    TrackObjectPresentationSceneNode(xml_node)
{
    m_node = irr_driver->getSceneManager()->addEmptySceneNode();
    m_node->setPosition(m_init_xyz);
    m_node->setRotation(m_init_hpr);
    m_node->setScale(m_init_scale);
}

TrackObjectPresentationEmpty::~TrackObjectPresentationEmpty()
{
    irr_driver->removeNode(m_node);
}

// ----------------------------------------------------------------------------

TrackObjectPresentationLibraryNode::TrackObjectPresentationLibraryNode(
    const XMLNode& xml_node,
    ModelDefinitionLoader& model_def_loader) :
TrackObjectPresentationSceneNode(xml_node)
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
        std::string lib_node_path = lib_path + "node.xml";
        libroot = file_manager->createXMLTree(lib_node_path);
        if (libroot == NULL)
        {
            Log::error("TrackObjectPresentationLibraryNode", "Cannot find library '%s'", lib_node_path.c_str());
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
        create_lod_definitions = false; // LOD definitions are already created, don't create them again
    }

    m_node->setPosition(m_init_xyz);
    m_node->setRotation(m_init_hpr);
    m_node->setScale(m_init_scale);
    m_node->updateAbsolutePosition();

    assert(libroot != NULL);
    World::getWorld()->getTrack()->loadObjects(libroot, lib_path, model_def_loader,
        create_lod_definitions, m_node);
}

TrackObjectPresentationLibraryNode::~TrackObjectPresentationLibraryNode()
{
    irr_driver->removeNode(m_node);
}

// ----------------------------------------------------------------------------

TrackObjectPresentationLOD::TrackObjectPresentationLOD(const XMLNode& xml_node,
    scene::ISceneNode* parent, ModelDefinitionLoader& model_def_loader) :
    TrackObjectPresentationSceneNode(xml_node)
{
    m_node = model_def_loader.instanciateAsLOD(&xml_node, parent);
    if (m_node == NULL) throw std::runtime_error("Cannot load LOD node");
    m_node->setPosition(m_init_xyz);
    m_node->setRotation(m_init_hpr);
    m_node->setScale(m_init_scale);
}

TrackObjectPresentationLOD::~TrackObjectPresentationLOD()
{
    if (m_node)
        irr_driver->removeNode(m_node);
}

// ----------------------------------------------------------------------------

TrackObjectPresentationMesh::TrackObjectPresentationMesh(const XMLNode& xml_node,
    bool enabled, scene::ISceneNode* parent) :
    TrackObjectPresentationSceneNode(xml_node)
{
    m_is_looped  = false;
    m_mesh       = NULL;
    m_node       = NULL;

    xml_node.get("looped",  &m_is_looped );
    std::string model_name;
    xml_node.get("model",   &model_name  );

    m_model_file = model_name;
    m_is_in_skybox = false;
    std::string render_pass;
    xml_node.get("renderpass", &render_pass);

    bool skeletal_animation = true; // for backwards compatibility, if unspecified assume there is
    xml_node.get("skeletal-animation", &skeletal_animation);

    if (render_pass == "skybox")
    {
        m_is_in_skybox = true;
    }

    //std::string full_path =
    //    World::getWorld()->getTrack()->getTrackFile(model_name);

    bool animated = skeletal_animation && (UserConfigParams::m_graphical_effects ||
                     World::getWorld()->getIdent() == IDENT_CUTSCENE);
    bool displacing = false;
    xml_node.get("displacing", &displacing);
    animated &= !displacing;

    if (animated)
    {
        m_mesh = irr_driver->getAnimatedMesh(model_name);
    }
    else
    {
        m_mesh = irr_driver->getMesh(model_name);
    }

    if (!m_mesh)
    {
        throw std::runtime_error("Model '" + model_name + "' cannot be found");
    }

    if (!animated)
        m_mesh = MeshTools::createMeshWithTangents(m_mesh, &MeshTools::isNormalMap);

    init(&xml_node, parent, enabled);
}

TrackObjectPresentationMesh::TrackObjectPresentationMesh(
    scene::IAnimatedMesh* model, const core::vector3df& xyz,
    const core::vector3df& hpr, const core::vector3df& scale) :
    TrackObjectPresentationSceneNode(xyz, hpr, scale)
{
    m_is_looped = false;
    m_mesh = NULL;
    m_node = NULL;

    m_mesh = model;
    init(NULL, NULL, true);
}

TrackObjectPresentationMesh::TrackObjectPresentationMesh(
        const std::string& model_file, const core::vector3df& xyz,
        const core::vector3df& hpr, const core::vector3df& scale) :
        TrackObjectPresentationSceneNode(xyz, hpr, scale)
{
    m_is_looped  = false;
    m_mesh       = NULL;
    m_node       = NULL;

    bool animated = (UserConfigParams::m_graphical_effects ||
             World::getWorld()->getIdent() == IDENT_CUTSCENE);

    m_model_file = model_file;

    if (file_manager->fileExists(model_file))
    {
        if (animated)
        {
            m_mesh = irr_driver->getAnimatedMesh(model_file);
        }
        else
        {
            m_mesh = irr_driver->getMesh(model_file);
        }
    }

    if (!m_mesh)
    {
        throw std::runtime_error("Model '" + model_file + "' cannot be found");
    }

    init(NULL, NULL, true);
}

void TrackObjectPresentationMesh::init(const XMLNode* xml_node, scene::ISceneNode* parent, bool enabled)
{
    bool skeletal_animation = true; // for backwards compatibility, if unspecified assume there is
    if(xml_node)
        xml_node->get("skeletal-animation", &skeletal_animation);

    bool animated = skeletal_animation && (UserConfigParams::m_graphical_effects ||
             World::getWorld()->getIdent() == IDENT_CUTSCENE);
    bool displacing = false;
    if(xml_node)
        xml_node->get("displacing", &displacing);
    animated &= !displacing;

    m_mesh->grab();
    irr_driver->grabAllTextures(m_mesh);

    if (m_is_in_skybox)
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
            irr_driver->addAnimatedMesh((scene::IAnimatedMesh*)m_mesh, m_model_file, parent);
        m_node = node;

        m_frame_start = node->getStartFrame();
        if (xml_node)
            xml_node->get("frame-start", &m_frame_start);

        m_frame_end = node->getEndFrame();
        if (xml_node)
            xml_node->get("frame-end", &m_frame_end);
    }
    else
    {
        bool displacing = false;
        if (xml_node)
            xml_node->get("displacing", &displacing);

        m_node = irr_driver->addMesh(m_mesh, m_model_file, parent);

        STKMeshSceneNode* stkmesh = dynamic_cast<STKMeshSceneNode*>(m_node);
        if (displacing && stkmesh != NULL)
            stkmesh->setIsDisplacement(displacing);

        m_frame_start = 0;
        m_frame_end = 0;

        if (World::getWorld() != NULL && World::getWorld()->getTrack() != NULL && xml_node != NULL)
            World::getWorld()->getTrack()->handleAnimatedTextures(m_node, *xml_node);
    }
//#ifdef DEBUG
//    std::string debug_name = model_name+" (track-object)";
//    m_node->setName(debug_name.c_str());
//#endif

    if(!enabled)
        m_node->setVisible(false);

    m_node->setPosition(m_init_xyz);
    m_node->setRotation(m_init_hpr);
    m_node->setScale(m_init_scale);
}

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
}

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
}

int TrackObjectPresentationMesh::getCurrentFrame()
{
    if (m_node->getType() == scene::ESNT_ANIMATED_MESH)
    {
        scene::IAnimatedMeshSceneNode *a_node =
            (scene::IAnimatedMeshSceneNode*)m_node;

        return (int)a_node->getFrameNr();
    }
    return -1; //Not a skeletal animation
}

void TrackObjectPresentationMesh::setCurrentFrame(int frame)
{
    if (m_node->getType() == scene::ESNT_ANIMATED_MESH)
    {
        scene::IAnimatedMeshSceneNode *a_node =
            (scene::IAnimatedMeshSceneNode*)m_node;

        a_node->setCurrentFrame((f32)frame);
    }
}

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
}
// ----------------------------------------------------------------------------


TrackObjectPresentationSound::TrackObjectPresentationSound(const XMLNode& xml_node,
                                                           scene::ISceneNode* parent) 
                            : TrackObjectPresentation(xml_node)
{
    // TODO: respect 'parent' if any

    m_sound = NULL;
    m_xyz = m_init_xyz;

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
    if (m_sound != NULL)
    {
        // muting when too far is implemented manually since not supported by OpenAL
        // so need to call this every frame to update the muting state if listener
        // moved
        m_sound->setPosition(m_xyz);
    }
}   // update

// ----------------------------------------------------------------------------
void TrackObjectPresentationSound::onTriggerItemApproached(Item* who)
{
    if (m_sound != NULL && m_sound->getStatus() != SFXBase::SFX_PLAYING)
    {
        m_sound->play();
    }
}   // onTriggerItemApproached

// ----------------------------------------------------------------------------
void TrackObjectPresentationSound::triggerSound(bool loop)
{
    if (m_sound != NULL)
    {
        m_sound->setLoop(loop);
        m_sound->play();
    }
}   // triggerSound

// ----------------------------------------------------------------------------
void TrackObjectPresentationSound::stopSound()
{
    if (m_sound != NULL) m_sound->stop();
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
                                        const core::vector3df& scale)
{
    m_xyz = xyz;
    if (m_sound != NULL) m_sound->setPosition(xyz);
}   // move

// ----------------------------------------------------------------------------

TrackObjectPresentationBillboard::TrackObjectPresentationBillboard(const XMLNode& xml_node, 
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
        Log::warn("TrackObjectPresentation", "Billboard texture '%s' not found", texture_name.c_str());
    }
    m_node = irr_driver->addBillboard(core::dimension2df(width, height), texture, parent);
    Material *stk_material = material_manager->getMaterial(texture_name);
    stk_material->setMaterialProperties(&(m_node->getMaterial(0)), NULL);

    m_node->setPosition(m_init_xyz);
}

// ----------------------------------------------------------------------------
void TrackObjectPresentationBillboard::update(float dt)
{
    if (m_fade_out_when_close)
    {
        scene::ICameraSceneNode* curr_cam = irr_driver->getSceneManager()->getActiveCamera();
        const float dist =  m_node->getAbsolutePosition().getDistanceFrom( curr_cam->getPosition() );

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
            int a = (int)(255*(dist - m_fade_out_start) / (m_fade_out_end - m_fade_out_start));
            node->setColor(video::SColor(a, 255, 255, 255));
        }
    }
}

// ----------------------------------------------------------------------------
TrackObjectPresentationBillboard::~TrackObjectPresentationBillboard()
{
    if (m_node)
        irr_driver->removeNode(m_node);
}

// ----------------------------------------------------------------------------

TrackObjectPresentationParticles::TrackObjectPresentationParticles(const XMLNode& xml_node, scene::ISceneNode* parent) :
    TrackObjectPresentationSceneNode(xml_node)
{
    m_emitter = NULL;
    m_lod_emitter_node = NULL;

    std::string path;
    xml_node.get("kind", &path);
    
    int clip_distance = -1;
    xml_node.get("clip_distance", &clip_distance);

    xml_node.get("conditions", &m_trigger_condition);

    try
    {
        ParticleKind* kind = ParticleKindManager::get()->getParticles( path.c_str() );
        if (kind == NULL)
        {
            throw std::runtime_error(path + " could not be loaded");
        }
        ParticleEmitter* emitter = new ParticleEmitter(kind, m_init_xyz, parent);


        if (clip_distance > 0)
        {
            scene::ISceneManager* sm = irr_driver->getSceneManager();
            scene::ISceneNode* sroot = sm->getRootSceneNode();
            LODNode* lod = new LODNode("particles", parent == NULL ? sroot : parent, sm);
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

        if (m_trigger_condition.size() > 0)
        {
            m_emitter->setCreationRateAbsolute(0.0f);
        }
    }
    catch (std::runtime_error& e)
    {
        Log::warn ("Track", "Could not load particles '%s'; cause :\n    %s", path.c_str(), e.what());
    }
}

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
}

// ----------------------------------------------------------------------------
void TrackObjectPresentationParticles::update(float dt)
{
    if (m_emitter != NULL)
    {
        m_emitter->update(dt);
    }
}

// ----------------------------------------------------------------------------
void TrackObjectPresentationParticles::triggerParticles()
{
    if (m_emitter != NULL)
    {
        m_emitter->setCreationRateAbsolute(1.0f);
        m_emitter->setParticleType(m_emitter->getParticlesInfo());
    }
}

// ----------------------------------------------------------------------------

TrackObjectPresentationLight::TrackObjectPresentationLight(const XMLNode& xml_node, scene::ISceneNode* parent) :
    TrackObjectPresentationSceneNode(xml_node)
{
    xml_node.get("color", &m_color);
    const video::SColorf colorf(m_color);

    m_energy = 1.0f;
    xml_node.get("energy", &m_energy);

    m_distance = 20.f * m_energy;
    xml_node.get("distance", &m_distance);

    if (irr_driver->isGLSL())
    {
        m_node = irr_driver->addLight(m_init_xyz, m_energy, m_distance, colorf.r, colorf.g, colorf.b, false, parent);
    }
    else
    {
        m_node = NULL; // lights require shaders to work
        //scene::ILightSceneNode* node = irr_driver->getSceneManager()->addLightSceneNode(NULL, m_init_xyz, m_color, m_distance);
        //node->setLightType(video::ELT_POINT);
        //node->enableCastShadow(true);
        //m_node = node;
    }
}

// ----------------------------------------------------------------------------
TrackObjectPresentationLight::~TrackObjectPresentationLight()
{
}

// ----------------------------------------------------------------------------

TrackObjectPresentationActionTrigger::TrackObjectPresentationActionTrigger(const XMLNode& xml_node) :
    TrackObjectPresentation(xml_node)
{
    float trigger_distance = 1.0f;
    xml_node.get("distance", &trigger_distance);

    xml_node.get("action", &m_action);

    m_action_active = true;

    if (m_action.size() == 0)
        Log::warn("TrackObject", "Action-trigger has no action defined.");

    ItemManager::get()->newItem(m_init_xyz, trigger_distance, this);
}

// ----------------------------------------------------------------------------

TrackObjectPresentationActionTrigger::TrackObjectPresentationActionTrigger
(const core::vector3df& xyz,std::string script_name, float distance)
:TrackObjectPresentation(xyz)
{
    m_init_xyz = xyz;
    m_init_hpr = core::vector3df(0, 0, 0);
    m_init_scale = core::vector3df(1, 1, 1);
    float trigger_distance = distance;
    m_action = script_name;
    m_action_active = true;


    ItemManager::get()->newItem(m_init_xyz, trigger_distance, this);
}

void TrackObjectPresentationActionTrigger::onTriggerItemApproached(Item* who)
{
    if (!m_action_active) return;

    // TODO: replace all of these hardcoded actions with scripting

    if (m_action == "garage")
    {
        m_action_active = false;

        new RacePausedDialog(0.8f, 0.6f);
        //dynamic_cast<OverWorld*>(World::getWorld())->scheduleSelectKart();
    }
    //action trigger near big doors in the overword to notify players that they'll open once they finish all the challenges
    else if (m_action == "big_door")
    {
        m_action_active = false;

        Track* m_track = World::getWorld()->getTrack();
        unsigned int unlocked_challenges = m_track->getNumOfCompletedChallenges();
        std::vector<OverworldChallenge> m_challenges = m_track->getChallengeList();

        // allow ONE unsolved challenge : the last one
        if (unlocked_challenges < m_challenges.size() - 1)
        {
            new TutorialMessageDialog(_("Complete all challenges to unlock the big door!"), true);
        }
    }
    else if (m_action == "tutorial_drive")
    {
        //if (World::getWorld()->getPhase() == World::RACE_PHASE)
        {
            m_action_active = false;
            //World::getWorld()->getRaceGUI()->clearAllMessages();

            InputDevice* device = input_manager->getDeviceManager()->getLatestUsedDevice();
            DeviceConfig* config = device->getConfiguration();
            irr::core::stringw accel = config->getBindingAsString(PA_ACCEL);
            irr::core::stringw left = config->getBindingAsString(PA_STEER_LEFT);
            irr::core::stringw right = config->getBindingAsString(PA_STEER_RIGHT);

            new TutorialMessageDialog(_("Accelerate with <%s> and steer with <%s> and <%s>", accel, left, right),
                false);
        }
    }
    else if (m_action == "tutorial_bananas")
    {
        m_action_active = false;

        new TutorialMessageDialog(_("Avoid bananas!"), true);
    }
    else if (m_action == "tutorial_giftboxes")
    {
        m_action_active = false;
        InputDevice* device = input_manager->getDeviceManager()->getLatestUsedDevice();
        DeviceConfig* config = device->getConfiguration();
        irr::core::stringw fire = config->getBindingAsString(PA_FIRE);

        new TutorialMessageDialog(_("Collect gift boxes, and fire the weapon with <%s> to blow away these boxes!", fire),
            true);
    }
    else if (m_action == "tutorial_backgiftboxes")
    {
        m_action_active = false;
        InputDevice* device = input_manager->getDeviceManager()->getLatestUsedDevice();
        DeviceConfig* config = device->getConfiguration();
        irr::core::stringw fire = config->getBindingAsString(PA_FIRE);

        new TutorialMessageDialog(_("Press <B> to look behind, to fire the weapon with <%s> while pressing <B> to to fire behind!", fire),
            true);
    }
    else if (m_action == "tutorial_nitro_collect")
    {
        m_action_active = false;

        new TutorialMessageDialog(_("Collect nitro bottles (we will use them after the curve)"),
            true);
    }
    else if (m_action == "tutorial_nitro_use")
    {
        m_action_active = false;
        InputDevice* device = input_manager->getDeviceManager()->getLatestUsedDevice();
        DeviceConfig* config = device->getConfiguration();
        irr::core::stringw nitro = config->getBindingAsString(PA_NITRO);

        new TutorialMessageDialog(_("Use the nitro you collected by pressing <%s>!", nitro),
            true);
    }
    else if (m_action == "tutorial_rescue")
    {
        m_action_active = false;
        InputDevice* device = input_manager->getDeviceManager()->getLatestUsedDevice();
        DeviceConfig* config = device->getConfiguration();
        irr::core::stringw rescue = config->getBindingAsString(PA_RESCUE);

        new TutorialMessageDialog(_("Oops! When you're in trouble, press <%s> to be rescued", rescue),
            false);
    }
    else if (m_action == "tutorial_skidding")
    {
        m_action_active = false;
        //World::getWorld()->getRaceGUI()->clearAllMessages();

        InputDevice* device = input_manager->getDeviceManager()->getLatestUsedDevice();
        DeviceConfig* config = device->getConfiguration();
        irr::core::stringw skid = config->getBindingAsString(PA_DRIFT);


        new TutorialMessageDialog(_("Accelerate and press the <%s> key while turning to skid. Skidding for a short while can help you turn faster to take sharp turns.", skid),
            true);
    }
    else if (m_action == "tutorial_skidding2")
    {
        m_action_active = false;
        World::getWorld()->getRaceGUI()->clearAllMessages();

        new TutorialMessageDialog(_("Note that if you manage to skid for several seconds, you will receive a bonus speedup as a reward!"),
            true);
    }
    else if (m_action == "tutorial_endmessage")
    {
        m_action_active = false;
        World::getWorld()->getRaceGUI()->clearAllMessages();

        new TutorialMessageDialog(_("You are now ready to race. Good luck!"),
            true);
    }
    else if (m_action == "tutorial_exit")
    {
        World::getWorld()->scheduleExitRace();
        return;
    }
    else
    {	
        //TODO move all above functions into scripts and remove the ifs
        Scripting::ScriptEngine* m_script_engine = World::getWorld()->getScriptEngine();
        m_action_active = false;
        m_script_engine->runScript(m_action);
        
        /*
        Catch exception -> script not found
        fprintf(stderr, "[TrackObject] WARNING: unknown action <%s>\n",
                m_action.c_str());
        
         */
    }
}



