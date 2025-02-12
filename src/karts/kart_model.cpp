//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008-2015 Joerg Henrichs
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

#include "karts/kart_model.hpp"

#include <IMeshSceneNode.h>
#include <SMesh.h>
#include <ISceneManager.h>

#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/b3d_mesh_loader.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/lod_node.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/mesh_tools.hpp"
#include "graphics/moving_texture.hpp"
#include "graphics/sp/sp_mesh.hpp"
#include "graphics/sp/sp_mesh_buffer.hpp"
#include "graphics/sp/sp_mesh_node.hpp"
#include "guiengine/engine.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/ghost_kart.hpp"
#include "karts/kart_properties.hpp"
#include "physics/btKart.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"
#include "utils/log.hpp"

#include "IMeshManipulator.h"
#include <algorithm>
#include <ge_animation.hpp>
#include <ge_render_info.hpp>
#include <ge_spm.hpp>

#define SKELETON_DEBUG 0

float KartModel::UNDEFINED = -99.9f;

// ------------------------------------------------------------
// SpeedWeightedObject implementation

SpeedWeightedObject::Properties::Properties()
{
    m_strength_factor = -1.0f;
    m_speed_factor    = 0.0f;
    m_moving_texture  = NULL;
}   // SpeedWeightedObject::Properties::Properties

// ----------------------------------------------------------------------------
SpeedWeightedObject::Properties::~Properties()
{
    delete m_moving_texture;
}   // SpeedWeightedObject::Properties::~Properties

// ----------------------------------------------------------------------------
SpeedWeightedObject::Properties& SpeedWeightedObject::Properties::
           operator=(const SpeedWeightedObject::Properties& other)
{
    m_strength_factor = other.m_strength_factor;
    m_speed_factor = other.m_speed_factor;
    m_moving_texture = NULL;
    if (other.m_moving_texture)
        m_moving_texture = new MovingTexture(*other.m_moving_texture);
    return *this;
}   // SpeedWeightedObject::Properties::Properties& operator=

// ----------------------------------------------------------------------------
void SpeedWeightedObject::Properties::loadFromXMLNode(const XMLNode* xml_node)
{
    xml_node->get("strength-factor", &m_strength_factor);
    xml_node->get("speed-factor",    &m_speed_factor);
}   // SpeedWeightedObject::Properties::loadFromXMLNode

// ============================================================================
/** Default constructor which initialises all variables with defaults.
 *  Note that the KartModel is copied, so make sure to update makeCopy
 *  if any more variables are added to this object.
 *  ATM there are two pointers:
 *  - to the scene node (which is otherwise handled by kart/movable and set
 *    later anyway)
 *  - to the mesh. Sharing mesh is supported in irrlicht, so that's
 *    no problem.
 *  There are two different type of instances of this class:
 *  One is the 'master instances' which is part of the kart_properties.
 *  These instances have m_is_master = true, will cause an assertion
 *  crash if attachModel is called or the destructor notices any
 *  wheels being defined.
 *  The other types are copies of one of the master objects: these are
 *  used when actually displaying the karts (e.g. in race). They must
 *  be copied since otherwise (if the same kart is used more than once)
 *  shared variables in KartModel (esp. animation status) will cause
 *  incorrect animations. The mesh is shared (between the master instance
 *  and all of its copies).
 *  Technically the scene node and mesh should be grab'ed on copy,
 *  and dropped when the copy is deleted. But since the master copy
 *  in the kart_properties_manager is always kept, there is no risk of
 *  a mesh being deleted to early.
 */
KartModel::KartModel(bool is_master)
{
    m_version    = 0;
    m_is_master  = is_master;
    m_kart       = NULL;
    m_mesh       = NULL;
    m_hat_location = NULL;

    for(unsigned int i=0; i<4; i++)
    {
        m_wheel_graphics_position[i] = Vec3(UNDEFINED);
        m_wheel_graphics_radius[i]   = 0.0f;   // for kart without separate wheels
        m_wheel_model[i]             = NULL;
        m_wheel_node[i]              = NULL;

        // default value for kart suspensions. move to config file later
        // if we find each kart needs custom values
        m_min_suspension[i] = -0.07f;
        m_max_suspension[i] = 0.20f;
        m_dampen_suspension_amplitude[i] = 2.5f;
        m_default_physics_suspension[i] = 0.25f;
    }
    m_wheel_filename[0] = "";
    m_wheel_filename[1] = "";
    m_wheel_filename[2] = "";
    m_wheel_filename[3] = "";
    m_speed_weighted_objects.clear();
    m_headlight_objects.clear();
    m_animated_node     = NULL;
    for(unsigned int i=AF_BEGIN; i<=AF_END; i++)
        m_animation_frame[i]=-1;
    m_animation_speed   = 25;
    m_current_animation = AF_DEFAULT;
    m_support_colorization = false;
    m_kart_properties = NULL;
}   // KartModel

// ----------------------------------------------------------------------------
/** This function loads the information about the kart from a xml file. It
 *  does not actually load the models (see load()).
 *  \param node  XML object of configuration file.
 */
void KartModel::loadInfo(const XMLNode &node)
{
    node.get("model-file", &m_model_filename);
    if(const XMLNode *animation_node=node.getNode("animations"))
    {
        animation_node->get("left",           &m_animation_frame[AF_LEFT]      );
        animation_node->get("straight",       &m_animation_frame[AF_STRAIGHT]  );
        animation_node->get("right",          &m_animation_frame[AF_RIGHT]     );
        animation_node->get("start-winning",  &m_animation_frame[AF_WIN_START] );
        animation_node->get("start-winning-loop",
                                              &m_animation_frame[AF_WIN_LOOP_START] );
        animation_node->get("end-winning",    &m_animation_frame[AF_WIN_END]   );
        animation_node->get("end-winning-straight", &m_animation_frame[AF_WIN_END_STRAIGHT]  );
        animation_node->get("start-losing",   &m_animation_frame[AF_LOSE_START]);
        animation_node->get("start-losing-loop",
                                             &m_animation_frame[AF_LOSE_LOOP_START]);
        animation_node->get("end-losing",     &m_animation_frame[AF_LOSE_END]  );
        animation_node->get("end-losing-straight", &m_animation_frame[AF_LOSE_END_STRAIGHT]  );
        animation_node->get("start-explosion",&m_animation_frame[AF_LOSE_START]);
        animation_node->get("end-explosion",  &m_animation_frame[AF_LOSE_END]  );
        animation_node->get("start-jump",     &m_animation_frame[AF_JUMP_START]);
        animation_node->get("start-jump-loop",&m_animation_frame[AF_JUMP_LOOP] );
        animation_node->get("end-jump",       &m_animation_frame[AF_JUMP_END]  );
        animation_node->get("selection-start", &m_animation_frame[AF_SELECTION_START]);
        animation_node->get("selection-end",   &m_animation_frame[AF_SELECTION_END]  );
        animation_node->get("backpedal-left", &m_animation_frame[AF_BACK_LEFT]);
        animation_node->get("backpedal",      &m_animation_frame[AF_BACK_STRAIGHT]);
        animation_node->get("backpedal-right",&m_animation_frame[AF_BACK_RIGHT]);
        animation_node->get("speed",          &m_animation_speed               );
    }

    if(const XMLNode *wheels_node=node.getNode("wheels"))
    {
        loadWheelInfo(*wheels_node, "front-right", 0);
        loadWheelInfo(*wheels_node, "front-left",  1);
        loadWheelInfo(*wheels_node, "rear-right",  2);
        loadWheelInfo(*wheels_node, "rear-left",   3);
    }

    m_nitro_emitter_position[0] = Vec3 (0,0.1f,0);
    m_nitro_emitter_position[1] = Vec3 (0,0.1f,0);
    m_has_nitro_emitter = false;

    if(const XMLNode *nitroEmitter_node=node.getNode("nitro-emitter"))
    {
        loadNitroEmitterInfo(*nitroEmitter_node, "nitro-emitter-a", 0);
        loadNitroEmitterInfo(*nitroEmitter_node, "nitro-emitter-b", 1);
        m_has_nitro_emitter = true;
    }

    node.get("version", &m_version);
    if (m_version > 2)
    {
        if (const XMLNode *speed_weighted_objects_node = node.getNode("speed-weighted-objects"))
        {
            unsigned speed_weighted_objects_size = speed_weighted_objects_node->getNumNodes();
            m_speed_weighted_objects.resize(speed_weighted_objects_size);
            for (unsigned int i = 0 ;i < speed_weighted_objects_size; i++)
            {
                loadSpeedWeightedInfo(speed_weighted_objects_node->getNode(i), i);
            }
        }
        if (const XMLNode* headlights_node = node.getNode("headlights"))
        {
            loadHeadlights(*headlights_node);
        }
        if (const XMLNode* hat_node = node.getNode("hat"))
        {
            core::vector3df position, rotation, scale;
            hat_node->get("position", &position);
            hat_node->get("rotation", &rotation);
            hat_node->get("scale", &scale);
            core::matrix4 lm, sm, rm;
            lm.setTranslation(position);
            sm.setScale(scale);
            rm.setRotationDegrees(rotation);
            m_hat_location = new core::matrix4(lm * rm * sm);
            hat_node->get("bone", &m_hat_bone);
        }
    }

    if (const XMLNode* exhaust = node.getNode("exhaust"))
    {
        exhaust->get("file", &m_exhaust_xml);
    }
}   // loadInfo

// ----------------------------------------------------------------------------
/** Destructor.
 */
KartModel::~KartModel()
{
    if (m_animated_node)
    {
        m_animated_node->setAnimationEndCallback(NULL);
        m_animated_node->drop();
    }

    for(unsigned int i=0; i<4; i++)
    {
        if(m_wheel_node[i])
        {
            // Master KartModels should never have a wheel attached.
            assert(!m_is_master);
            m_wheel_node[i]->drop();
        }
        if(m_is_master && m_wheel_model[i])
        {
            irr_driver->dropAllTextures(m_wheel_model[i]);
            irr_driver->removeMeshFromCache(m_wheel_model[i]);
        }
    }

    for(size_t i=0; i<m_speed_weighted_objects.size(); i++)
    {
        if(m_speed_weighted_objects[i].m_node)
        {
            // Master KartModels should never have a speed weighted object attached.
            assert(!m_is_master);
            m_speed_weighted_objects[i].m_node->drop();
        }
        if (m_is_master && m_speed_weighted_objects[i].m_model)
        {
            m_speed_weighted_objects[i].m_model->drop();
            irr_driver->dropAllTextures(m_speed_weighted_objects[i].m_model);
            if (m_speed_weighted_objects[i].m_model->getReferenceCount() == 1)
            {
                irr_driver->removeMeshFromCache(m_speed_weighted_objects[i].m_model);
            }
        }
    }

    for (size_t i = 0; i < m_headlight_objects.size(); i++)
    {
        HeadlightObject& obj = m_headlight_objects[i];
        if (obj.getLightNode())
        {
            // Master KartModels should never have a headlight attached.
            assert(!m_is_master);
            obj.getLightNode()->drop();
        }
        if (m_is_master && obj.getModel())
        {
            obj.getModel()->drop();
            irr_driver->dropAllTextures(obj.getModel());
            if (obj.getModel()->getReferenceCount() == 1)
            {
                irr_driver->removeMeshFromCache(obj.getModel());
            }
        }
    }

    if (m_is_master && m_mesh)
    {
        m_mesh->drop();
        // If there is only one copy left, it's the copy in irrlicht's
        // mesh cache, so it can be removed.
        if (m_mesh && m_mesh->getReferenceCount() == 1)
        {
            irr_driver->dropAllTextures(m_mesh);
            irr_driver->removeMeshFromCache(m_mesh);
        }
    }

    delete m_hat_location;
#ifdef DEBUG
#if SKELETON_DEBUG
    irr_driver->clearDebugMeshes();
#endif
#endif

}  // ~KartModel

// ----------------------------------------------------------------------------
/** This function returns a copy of this object. The memory is allocated
 *  here, but needs to be managed (esp. freed) by the calling function.
 *  It is also marked not to be a master copy, so attachModel can be called
 *  for this instance.
 */
KartModel* KartModel::makeCopy(std::shared_ptr<GE::GERenderInfo> ri)
{
    // Make sure that we are copying from a master objects, and
    // that there is indeed no animated node defined here ...
    // just in case.
    assert(m_is_master);
    assert(!m_render_info);
    assert(!m_animated_node);
    assert(m_kart_properties);
    KartModel *km               = new KartModel(/*is master*/ false);
    km->m_kart_width            = m_kart_width;
    km->m_kart_length           = m_kart_length;
    km->m_kart_height           = m_kart_height;
    km->m_kart_highest_point    = m_kart_highest_point;
    km->m_kart_lowest_point     = m_kart_lowest_point;
    km->m_mesh                  = m_mesh;
    km->m_model_filename        = m_model_filename;
    km->m_animation_speed       = m_animation_speed;
    km->m_current_animation     = AF_DEFAULT;
    km->m_animated_node         = NULL;
    km->m_hat_name              = m_hat_name;
    km->m_hat_bone              = m_hat_bone;
    km->m_support_colorization  = m_support_colorization;
    km->m_render_info           = ri;
    km->m_inverse_bone_matrices = m_inverse_bone_matrices;
    km->m_version               = m_version;
    km->m_exhaust_xml           = m_exhaust_xml;

    km->m_nitro_emitter_position[0] = m_nitro_emitter_position[0];
    km->m_nitro_emitter_position[1] = m_nitro_emitter_position[1];
    km->m_has_nitro_emitter = m_has_nitro_emitter;
    if (m_hat_location)
    {
        km->m_hat_location = new core::matrix4();
        *(km->m_hat_location) = *m_hat_location;
    }

    for(unsigned int i=0; i<4; i++)
    {
        // Master should not have any wheel nodes.
        assert(!m_wheel_node[i]);
        km->m_wheel_model[i]                = m_wheel_model[i];
        km->m_wheel_filename[i]             = m_wheel_filename[i];
        km->m_wheel_graphics_position[i]    = m_wheel_graphics_position[i];
        km->m_wheel_graphics_radius[i]      = m_wheel_graphics_radius[i];
        km->m_min_suspension[i]             = m_min_suspension[i];
        km->m_max_suspension[i]             = m_max_suspension[i];
        km->m_dampen_suspension_amplitude[i]= m_dampen_suspension_amplitude[i];
    }

    km->m_speed_weighted_objects.resize(m_speed_weighted_objects.size());
    for(size_t i=0; i<m_speed_weighted_objects.size(); i++)
    {
        // Master should not have any speed weighted nodes.
        assert(!m_speed_weighted_objects[i].m_node);
        km->m_speed_weighted_objects[i] = m_speed_weighted_objects[i];
    }

    km->m_headlight_objects.resize(m_headlight_objects.size());
    for (size_t i = 0; i<m_headlight_objects.size(); i++)
    {
        // Master should not have any headlight nodes.
        assert(!m_headlight_objects[i].getLightNode());
        km->m_headlight_objects[i] = m_headlight_objects[i];
    }

    for(unsigned int i=AF_BEGIN; i<=AF_END; i++)
        km->m_animation_frame[i] = m_animation_frame[i];

    km->m_kart_properties = m_kart_properties;
    return km;
}   // makeCopy

// ----------------------------------------------------------------------------

/** Attach the kart model and wheels to the scene node.
 *  \return the node with the model attached
 */
scene::ISceneNode* KartModel::attachModel(bool animated_models, bool human_player)
{
    assert(!m_is_master);

    scene::ISceneNode* node = NULL;

    if (animated_models)
    {
        m_animated_node = irr_driver->addAnimatedMesh(m_mesh, "kartmesh",
               NULL/*parent*/, getRenderInfo());
        node = m_animated_node;
#ifdef DEBUG
        std::string debug_name = m_model_filename+" (animated-kart-model)";
        node->setName(debug_name.c_str());
#endif
        m_animated_node->setLoopMode(false);
        m_animated_node->grab();
    }
    else
    {
        // If no animations are shown, make sure to pick the frame
        // with a straight ahead animation (if exist).
        int straight_frame = m_animation_frame[AF_STRAIGHT]>=0
                           ? m_animation_frame[AF_STRAIGHT]
                           : 0;

        scene::IMesh* main_frame = m_mesh;
        main_frame = m_mesh->getMesh(straight_frame);
        main_frame->setHardwareMappingHint(scene::EHM_STATIC);
        std::string debug_name;

#ifdef DEBUG
        debug_name = m_model_filename + " (kart-model)";
#endif

        node = irr_driver->addMesh(main_frame, debug_name,
               NULL /*parent*/, getRenderInfo());

#ifdef DEBUG
        node->setName(debug_name.c_str());
#endif

    }

    // Attach the wheels
    for (unsigned int i = 0; i < 4; i++)
    {
        if(!m_wheel_model[i]) continue;
        m_wheel_node[i] = irr_driver->addMesh(m_wheel_model[i], "wheel",
                          node, getRenderInfo());
        Vec3 wheel_min = m_wheel_model[i]->getMin();
        Vec3 wheel_max = m_wheel_model[i]->getMax();
        m_wheel_graphics_radius[i] = 0.5f*(wheel_max.getY() - wheel_min.getY());

        m_wheel_node[i]->grab();
#ifdef DEBUG
        std::string debug_name = m_wheel_filename[i]+" (wheel)";
        m_wheel_node[i]->setName(debug_name.c_str());
#endif
        m_wheel_node[i]->setPosition(m_wheel_graphics_position[i].toIrrVector());
    }

    // Attach the speed weighted objects + set the animation state
    for (unsigned int i = 0; i < m_speed_weighted_objects.size() ;i++)
    {
        SpeedWeightedObject& obj = m_speed_weighted_objects[i];
        obj.m_node = NULL;
        if (obj.m_model)
        {
            const bool bone_attachment =
                m_animated_node && !obj.m_bone_name.empty();
            scene::ISceneNode* parent = bone_attachment ?
                m_animated_node->getJointNode(obj.m_bone_name.c_str()) : node;
            scene::ISceneNode* swo = NULL;
            if (animated_models)
            {
                // Only need to keep track of animated node for speed setting
                obj.m_node = irr_driver->addAnimatedMesh(obj.m_model, 
                    "speedweighted", parent, getRenderInfo());
                swo = obj.m_node;
                obj.m_node->grab();
                obj.m_node->setFrameLoop(0, obj.m_model->getFrameCount() - 1);
            }
            else
            {
                swo = irr_driver->addMesh(obj.m_model->getMesh(0),
                    "speedweighted", parent, getRenderInfo());
            }
#ifdef DEBUG
            std::string debug_name = obj.m_name + " (speed-weighted)";
            swo->setName(debug_name.c_str());
#endif
            configNode(swo, obj.m_location, bone_attachment ?
                getInverseBoneMatrix(obj.m_bone_name) : core::matrix4());
        }
    }

    const float each_energy = 0.5f / m_headlight_objects.size();
    const float each_radius = 5.0f / m_headlight_objects.size();
    for (unsigned int i = 0; i < m_headlight_objects.size(); i++)
    {
        HeadlightObject& obj = m_headlight_objects[i];
        Track* track = Track::getCurrentTrack();
        if (obj.getModel() && !(track == NULL || track->getIsDuringDay()))
        {
            const bool bone_attachment =
                m_animated_node && !obj.getBoneName().empty();
            scene::ISceneNode* parent = bone_attachment ?
                m_animated_node->getJointNode(obj.getBoneName().c_str()) : node;
            scene::ISceneNode* headlight_model =
                irr_driver->addMesh(obj.getModel(), "kart_headlight",
                parent, getRenderInfo());
#ifndef SERVER_ONLY
            if (human_player && CVS->isGLSL() && CVS->isDeferredEnabled())
            {
                obj.setLight(headlight_model, each_energy, each_radius);
            }
#endif
            configNode(headlight_model, obj.getLocation(), bone_attachment ?
                getInverseBoneMatrix(obj.getBoneName()) : core::matrix4());
        }
    }

    if (m_hat_location && !m_hat_name.empty())
    {
        file_manager->pushTextureSearchPath
            (file_manager->getAsset(FileManager::MODEL,""), "models");
        const bool bone_attachment = m_animated_node && !m_hat_bone.empty();
        scene::ISceneNode* parent = bone_attachment ?
            m_animated_node->getJointNode(m_hat_bone.c_str()) : node;
        scene::IMesh* hat_mesh = irr_driver->getAnimatedMesh
            (file_manager->getAsset(FileManager::MODEL, m_hat_name));
        scene::ISceneNode* node = irr_driver->addMesh(hat_mesh, "hat", parent,
            getRenderInfo());
        configNode(node, *m_hat_location, bone_attachment ?
                getInverseBoneMatrix(m_hat_bone) : core::matrix4());
        file_manager->popTextureSearchPath();
    }

    if (animated_models)
    {
        LODNode* lod_node = new LODNode("kart",
            irr_driver->getSceneManager()->getRootSceneNode(),
            irr_driver->getSceneManager());
        lod_node->add(human_player ? 10000: 100, node, true);
        return lod_node;
    }

    return node;
}   // attachModel

// ----------------------------------------------------------------------------
/** Add a light node emitted from the center mass the headlight.
 */
void HeadlightObject::setLight(scene::ISceneNode* parent,
                                          float energy, float radius)
{
    m_node = irr_driver->addLight(core::vector3df(0.0f, 0.0f, 0.0f),
        energy, radius, m_headlight_color.getRed() / 255.f,
        m_headlight_color.getGreen() / 255.f,
        m_headlight_color.getBlue() / 255.f, false/*sun*/, parent);
    m_node->grab();
}   // setLight

// ----------------------------------------------------------------------------
/** Loads the 3d model and all wheels.
 */
bool KartModel::loadModels(const KartProperties &kart_properties)
{
    assert(m_is_master);
    m_kart_properties = &kart_properties;
    std::string  full_path = kart_properties.getKartDir()+m_model_filename;
    // For b3d loader only
    if (m_animation_frame[AF_STRAIGHT] > -1)
    {
        B3DMeshLoader::m_straight_frame = m_animation_frame[AF_STRAIGHT];
    }
    m_mesh                 = irr_driver->getAnimatedMesh(full_path);
    B3DMeshLoader::m_straight_frame = 0;
    if(!m_mesh)
    {
        Log::error("Kart_Model", "Problems loading mesh '%s' - kart '%s' will"
                   "not be available.",
                   full_path.c_str(), kart_properties.getIdent().c_str());
        return false;
    }

#ifndef SERVER_ONLY
#endif
    m_mesh->grab();
    irr_driver->grabAllTextures(m_mesh);

    Vec3 kart_min = m_mesh->getMin();
    Vec3 kart_max = m_mesh->getMax();
#ifndef SERVER_ONLY
    // Test if kart model support colorization
    for (u32 i = 0; i < m_mesh->getMeshBufferCount(); i++)
    {
        SP::SPMeshBuffer* spmb =
            dynamic_cast<SP::SPMeshBuffer*>(m_mesh->getMeshBuffer(i));
        if (!spmb)
        {
            m_support_colorization = m_support_colorization ||
                m_mesh->getMeshBuffer(i)->getMaterial().isColorizable();
            continue;
        }
        // Pre-upload gl meshes and textures for kart screen
        spmb->uploadGLMesh();
        std::vector<Material*> mbs = spmb->getAllSTKMaterials();
        for (Material* m : mbs)
        {
            m_support_colorization =
                m_support_colorization || m->isColorizable();
        }
    }
#endif

#undef MOVE_KART_MESHES
#ifdef MOVE_KART_MESHES
    // Kart models are not exactly centered. The following code would
    // transform the mesh so that they are properly centered, but it
    // would also mean all location relative to the original kart's
    // center (wheel position, emitter, hat) would need to be modified.
    scene::IMeshManipulator *mani =
        irr_driver->getVideoDriver()->getMeshManipulator();
    Vec3 offset_from_center = -0.5f*(kart_max+kart_min);
    offset_from_center.setY(-kart_min.getY());
    offset_from_center.setY(0);

    core::matrix4 translate(core::matrix4::EM4CONST_IDENTITY);
    translate.setTranslation(offset_from_center.toIrrVector());
    mani->transform(m_mesh, translate);
    MeshTools::minMax3D(m_mesh->getMesh(m_animation_frame[AF_STRAIGHT]),
                        &kart_min, &kart_max);
#endif
    m_kart_highest_point = kart_max.getY();
    m_kart_lowest_point  = kart_min.getY();
    initInverseBoneMatrices();

    if (GUIEngine::isNoGraphics())
        m_mesh->freeMeshVertexBuffer();

    // Load the speed weighted object models. We need to do that now because it can affect the dimensions of the kart
    for(size_t i=0 ; i < m_speed_weighted_objects.size() ; i++)
    {
        SpeedWeightedObject&    obj = m_speed_weighted_objects[i];
        std::string full_name =
            kart_properties.getKartDir()+obj.m_name;
        obj.m_model = irr_driver->getAnimatedMesh(full_name);
        // Grab all textures. This is done for the master only, so
        // the destructor will only free the textures if a master
        // copy is freed.
#ifndef SERVER_ONLY
        if (CVS->isGLSL())
        {
            for (u32 j = 0; j < obj.m_model->getMeshBufferCount(); j++)
            {
                SP::SPMeshBuffer* mb = static_cast<SP::SPMeshBuffer*>
                    (obj.m_model->getMeshBuffer(j));
                // Pre-upload gl meshes and textures for kart screen
                mb->uploadGLMesh();
                if (obj.m_properties.m_moving_texture)
                {
                    for (unsigned k = 0; k < mb->getAllSTKMaterials().size();
                        k++)
                    {
                        mb->enableTextureMatrix(k);
                    }
                }
            }
        }
#endif
        obj.m_model->grab();
        irr_driver->grabAllTextures(obj.m_model);

        // Update min/max, speed weight can be scaled
        scene::IMesh* mesh = obj.m_model->getMesh(0);
        Vec3 obj_min = mesh->getMin();
        Vec3 obj_max = mesh->getMax();
        core::vector3df transformed_min, transformed_max;
        obj.m_location.transformVect(transformed_min, obj_min.toIrrVector());
        obj.m_location.transformVect(transformed_max, obj_max.toIrrVector());
        kart_min.min(transformed_min);
        kart_max.max(transformed_max);
        if (GUIEngine::isNoGraphics())
            mesh->freeMeshVertexBuffer();
    }

    for (unsigned int i = 0; i < m_headlight_objects.size(); i++)
    {
        HeadlightObject& obj = m_headlight_objects[i];
        std::string full_name = kart_properties.getKartDir() + obj.getFilename();
        scene::IMesh* mesh = irr_driver->getMesh(full_name);
        if (!mesh)
            continue;
        obj.setModel(mesh);
#ifndef SERVER_ONLY
        SP::uploadSPM(obj.getModel());
#endif
        obj.getModel()->grab();
        irr_driver->grabAllTextures(obj.getModel());
        if (GUIEngine::isNoGraphics())
            obj.getModel()->freeMeshVertexBuffer();
    }

    Vec3 size     = kart_max-kart_min;
    m_kart_width  = size.getX();
    m_kart_height = size.getY();
    m_kart_length = size.getZ();

    // TODO: Client and server get slightly different sizes, which
    // affets the physics (size determines inertia, which is used
    // in steering). So by rounding to three decimals we get
    // more consistent physics results.
    m_kart_width  = int(m_kart_width  * 1000) / 1000.0f;
    m_kart_height = int(m_kart_height * 1000) / 1000.0f;
    m_kart_length = int(m_kart_length * 1000) / 1000.0f;

    // Now set default some default parameters (if not defined) that
    // depend on the size of the kart model (wheel position, center
    // of gravity shift)
    for(unsigned int i=0; i<4; i++)
    {
        if(m_wheel_graphics_position[i].getX()==UNDEFINED)
        {
            m_wheel_graphics_position[i].setX( ( i==1||i==3)
                                               ? -0.5f*m_kart_width
                                               :  0.5f*m_kart_width  );
            m_wheel_graphics_position[i].setY(0);
            m_wheel_graphics_position[i].setZ( (i<2) ?  0.5f*m_kart_length
                                                     : -0.5f*m_kart_length);
        }
    }

    // Load the wheel models. This can't be done early, since the default
    // values for the graphical position must be defined, which in turn
    // depend on the size of the model.
    for(unsigned int i=0; i<4; i++)
    {
        // For kart models without wheels.
        if(m_wheel_filename[i]=="") continue;
        std::string full_wheel =
            kart_properties.getKartDir()+m_wheel_filename[i];
        m_wheel_model[i] = irr_driver->getMesh(full_wheel);
#ifndef SERVER_ONLY
        SP::uploadSPM(m_wheel_model[i]);
#endif
        // Grab all textures. This is done for the master only, so
        // the destructor will only free the textures if a master
        // copy is freed.
        irr_driver->grabAllTextures(m_wheel_model[i]);
        if (GUIEngine::isNoGraphics())
            m_wheel_model[i]->freeMeshVertexBuffer();
    }   // for i<4

    return true;
}   // loadModels

// ----------------------------------------------------------------------------
/** Loads a single nitro emitter node. Currently this the position of the nitro
 *  emitter relative to the kart.
 *  \param emitter_name Name of the nitro emitter, e.g. emitter-a.
 *  \param index Index of this emitter in the global m_emitter* fields.
 */
void KartModel::loadNitroEmitterInfo(const XMLNode &node,
                              const std::string &emitter_name, int index)
{
    const XMLNode *emitter_node = node.getNode(emitter_name);
    if(!emitter_node)
    {
        // Only print the warning if a model filename is given. Otherwise the
        // stk_config file is read (which has no model information).
        if(m_model_filename!="")
        {
            Log::error("Kart_Model", "Missing nitro emitter information for model"
                       "'%s'.", m_model_filename.c_str());
            Log::error("Kart_Model", "This can be ignored, but the nitro particles will not work");
        }
        return;
    }
    emitter_node->get("position", &m_nitro_emitter_position[index]);
}   // loadNitroEmitterInfo

// ----------------------------------------------------------------------------
/** Loads a single speed weighted node. */
void KartModel::loadSpeedWeightedInfo(const XMLNode* speed_weighted_node,
                                      int index)
{
    SpeedWeightedObject obj;
    if (speed_weighted_node->getName() == "object")
    {
        core::vector3df position, rotation, scale;
        speed_weighted_node->get("position", &position);
        speed_weighted_node->get("rotation", &rotation);
        speed_weighted_node->get("scale", &scale);
        core::matrix4 lm, sm, rm;
        lm.setTranslation(position);
        sm.setScale(scale);
        rm.setRotationDegrees(rotation);
        obj.m_location = lm * rm * sm;
        speed_weighted_node->get("bone", &obj.m_bone_name);
        speed_weighted_node->get("model", &obj.m_name);
        obj.m_properties.loadFromXMLNode(speed_weighted_node);
    }
    else
    {
        Log::warn("KartModel",
            "Unknown XML node in the speed weighted objects section");
    }
    if (!obj.m_name.empty())
    {
        m_speed_weighted_objects[index] = obj;
        float dx = 0.0f;
        float dy = 0.0f;
        float dt = 0.0f;
        bool step = false;
        speed_weighted_node->get("texture-speed-x", &dx);
        speed_weighted_node->get("texture-speed-y", &dy);
        speed_weighted_node->get("texture-speed-dt", &dt);
        speed_weighted_node->get("animated-by-step", &step);
        if (dx != 0.0f || dy != 0.0f)
        {
            m_speed_weighted_objects[index].m_properties.m_moving_texture =
                new MovingTexture(dx, dy, dt, step);
        }
    }
}   // loadSpeedWeightedInfo

// ----------------------------------------------------------------------------
/** Loads a single wheel node. Currently this is the name of the wheel model
 *  and the position of the wheel relative to the kart.
 *  \param wheel_name Name of the wheel, e.g. wheel-rear-left.
 *  \param index Index of this wheel in the global m_wheel* fields.
 */
void KartModel::loadWheelInfo(const XMLNode &node,
                              const std::string &wheel_name, int index)
{
    const XMLNode *wheel_node = node.getNode(wheel_name);
    // Ignore in case of karts with missing wheels (e.g. Sara)
    if(!wheel_node) return;

    wheel_node->get("model",            &m_wheel_filename[index]         );
    wheel_node->get("position",         &m_wheel_graphics_position[index]);
    wheel_node->get("min-suspension",   &m_min_suspension[index]         );
    wheel_node->get("max-suspension",   &m_max_suspension[index]         );
}   // loadWheelInfo

// ----------------------------------------------------------------------------

void KartModel::loadHeadlights(const XMLNode &node)
{
    int children = node.getNumNodes();
    for (int i = 0; i < children; i++)
    {
        const XMLNode* child = node.getNode(i);
        if (child->getName() == "object")
        {
            core::vector3df position, rotation, scale;
            child->get("position", &position);
            child->get("rotation", &rotation);
            child->get("scale", &scale);
            core::matrix4 lm, sm, rm;
            lm.setTranslation(position);
            sm.setScale(scale);
            rm.setRotationDegrees(rotation);
            core::matrix4 location = lm * rm * sm;
            std::string bone_name;
            child->get("bone", &bone_name);
            std::string model;
            child->get("model", &model);
            video::SColor headlight_color(-1);
            child->get("color", &headlight_color);
            m_headlight_objects.push_back(HeadlightObject(model, location,
                bone_name, headlight_color));
        }
        else
        {
            Log::warn("KartModel", "Unknown XML node in the headlights section");
        }
    }
}   // loadHeadlights

// ----------------------------------------------------------------------------
/** Resets the kart model. It stops animation from being played and resets
 *  the wheels to the correct position (i.e. no suspension).
 */
void KartModel::reset()
{
    for (unsigned int i = 0; i < 4; i++)
    {
        if (m_wheel_node[i])
        {
            core::vector3df rotation(btScalar(rand() % 360), 0, 0);
            m_wheel_node[i]->setRotation(rotation);
        }
    }
    update(0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

    // Stop any animations currently being played.
    setAnimation(KartModel::AF_DEFAULT);

    // Don't force any LOD. Non-animated karts are not LOD nodes.
    LODNode *lod = dynamic_cast<LODNode*>(m_kart->getNode());
    if (lod)
        lod->forceLevelOfDetail(-1);

    toggleHeadlights(true);
}   // reset

// ----------------------------------------------------------------------------
/** Called when the kart finished the race. It will force the highest LOD
 *  for the kart, since otherwise the end camera can be far away (due to
 *  zooming) and show non-animated karts.
 */
void KartModel::finishedRace()
{
    // Force the animated model, independent of actual camera distance.
    LODNode *lod = dynamic_cast<LODNode*>(m_kart->getNode());
    if (lod)
        lod->forceLevelOfDetail(0);
}   // finishedRace

// ----------------------------------------------------------------------------
/** Enables- or disables the end animation.
 *  \param type The type of animation to play.
 */
void KartModel::setAnimation(AnimationFrameType type, bool play_non_loop)
{
    // if animations disabled, give up
    if (m_animated_node == NULL) return;

    bool transition = false;
    if (m_current_animation == AF_JUMP_START && type == AF_DEFAULT)
    {
        // For seamless transition back to AF_DEFAULT
        transition = true;
    }

    m_current_animation = type;
    if ((type == AF_WIN_START || type == AF_LOSE_START) &&
        m_animation_frame[type] > -1 && play_non_loop)
    {
        // Special handling for soccer goal animation
        class SmoothTransition : public IAnimationEndCallBack
        {
            KartModel* m_kart_model;
            bool m_transition;
        public:
            SmoothTransition(KartModel* km, bool transition) :
                m_kart_model(km), m_transition(transition) {}
            virtual void OnAnimationEnd(IAnimatedMeshSceneNode* node)
            {
                if (m_transition)
                    m_kart_model->m_animated_node->setTransitionTime(0.2f);
                m_kart_model->setAnimation(AF_DEFAULT);
            }
        };
        AnimationFrameType end = (AnimationFrameType)(type + 2);
        if (m_animation_frame [end] == -1)
            end = (AnimationFrameType)((int)end - 1);
        AnimationFrameType to_straight = (AnimationFrameType)(type + 3);
        bool has_to_straight = m_animation_frame[to_straight] > -1;
        if (has_to_straight)
            end = to_straight;
        m_animated_node->setAnimationSpeed(m_animation_speed);
        m_animated_node->setFrameLoop(m_animation_frame[type],
            m_animation_frame[end]);
        m_animated_node->setLoopMode(false);
        SmoothTransition* st = new SmoothTransition(this, !has_to_straight);
        m_animated_node->setAnimationEndCallback(st);
        st->drop();
    }
    else if (m_current_animation==AF_DEFAULT)
    {
        m_animated_node->setLoopMode(false);
        // setTransitionTime before setFrameLoop so the node will save the last
        // frame
        if (transition)
            m_animated_node->setTransitionTime(0.2f);
        const bool support_backpedal =
            m_animation_frame[AF_BACK_STRAIGHT] > -1 &&
            m_animation_frame[AF_BACK_LEFT] > -1 &&
            m_animation_frame[AF_BACK_RIGHT] > -1;
        if (support_backpedal)
        {
            int start_frame = std::min(m_animation_frame[AF_LEFT],
                m_animation_frame[AF_RIGHT]);
            int end_frame = std::max(m_animation_frame[AF_BACK_LEFT],
                m_animation_frame[AF_BACK_RIGHT]);
            m_animated_node->setFrameLoop(start_frame, end_frame);
        }
        else
        {
            if(m_animation_frame[AF_LEFT] <= m_animation_frame[AF_RIGHT])
                m_animated_node->setFrameLoop(m_animation_frame[AF_LEFT],
                                              m_animation_frame[AF_RIGHT] );
            else
                m_animated_node->setFrameLoop(m_animation_frame[AF_RIGHT],
                                              m_animation_frame[AF_LEFT] );
        }
        m_animated_node->setAnimationEndCallback(NULL);
        m_animated_node->setAnimationSpeed(0);
        m_animated_node->setCurrentFrame(m_animation_frame[AF_STRAIGHT]);
    }
    else if(m_animation_frame[type]>-1)
    {
        // 'type' is the start frame of the animation, type + 1 the frame
        // to begin the loop with, type + 2 to end the frame with
        AnimationFrameType end = (AnimationFrameType)(type+2);
        if(m_animation_frame[end]==-1)
            end = (AnimationFrameType)((int)end-1);
        m_animated_node->setAnimationSpeed(m_animation_speed);
        m_animated_node->setFrameLoop(m_animation_frame[type],
                                      m_animation_frame[end]    );
        // Loop mode must be set to false so that we get a callback when
        // the first iteration is finished.
        m_animated_node->setLoopMode(false);
        m_animated_node->setAnimationEndCallback(this);
    }
    else
    {
        // Special animation not found, revert to default
        m_current_animation = AF_DEFAULT;
        m_animated_node->setAnimationEndCallback(NULL);
    }
}   // setAnimation

// ----------------------------------------------------------------------------
/** Called from irrlicht when a non-looped animation ends. This is used to
 *  implement an introductory frame sequence before the actual loop can
 *  start: first a non-looped version from the first frame to the last
 *  frame is being played. When this is finished, this function is called,
 *  which then enables the actual loop.
 *  \param node The node for which the animation ended. Should always be
 *         m_animated_node
 */
void KartModel::OnAnimationEnd(scene::IAnimatedMeshSceneNode *node)
{
    // It should only be called for the animated node of this
    // kart_model
    assert(node==m_animated_node);

    // 'type' is the start frame of the animation, type + 1 the frame
    // to begin the loop with, type + 2 to end the frame with
    AnimationFrameType start = (AnimationFrameType)(m_current_animation+1);
    // If there is no loop-start defined (i.e. no 'introductory' sequence)
    // use the normal start frame.
    if(m_animation_frame[start]==-1)
        start = m_current_animation;
    AnimationFrameType end   = (AnimationFrameType)(m_current_animation+2);

    // Switch to loop mode if the current animation has a loop defined
    // (else just disable the callback, and the last frame will be shown).
    if(m_animation_frame[end]>-1)
    {
        m_animated_node->setAnimationSpeed(m_animation_speed);
        m_animated_node->setFrameLoop(m_animation_frame[start],
                                      m_animation_frame[end]   );
        m_animated_node->setLoopMode(true);
    }
    m_animated_node->setAnimationEndCallback(NULL);
}   // OnAnimationEnd

// ----------------------------------------------------------------------------
void KartModel::setDefaultSuspension()
{
    GhostKart* gk = dynamic_cast<GhostKart*>(m_kart);
    if (gk)
    {
        for (int i = 0; i < 4; i++)
            m_default_physics_suspension[i] = gk->getSuspensionLength(0, i);
        return;
    }

    for(int i=0; i<m_kart->getVehicle()->getNumWheels(); i++)
    {
        const btWheelInfo &wi = m_kart->getVehicle()->getWheelInfo(i);
        m_default_physics_suspension[i] = wi.m_raycastInfo.m_suspensionLength;
    }
}   // setDefaultSuspension

// ----------------------------------------------------------------------------
/** Rotates and turns the wheels appropriately, and adjust for suspension
 *  updates the speed-weighted objects' animations. 
 *
 *  \param dt time since last frame
 *  \param distance How far the wheels have rotated since last time.
 *  \param steer The actual steer settings.
 *  \param suspension Suspension height for all four wheels.
 *  \param speed The speed of the kart in meters/sec, used for the
 *         speed-weighted objects' animations
 *  \param current_lean_angle How much the kart is leaning (positive meaning
 *         left side down)
 *  \param gt_replay_index The index to get replay data, used by ghost kart
 */
void KartModel::update(float dt, float distance, float steer, float speed,
                       float current_lean_angle, int gt_replay_index)
{
    core::vector3df wheel_steer(0, steer*30.0f, 0);

    for(unsigned int i=0; i<4; i++)
    {
        if (!m_kart || !m_wheel_node[i]) continue;
#ifdef DEBUG
        if (UserConfigParams::m_physics_debug &&
            !m_kart->isGhostKart())
        {
            const btWheelInfo &wi = m_kart->getVehicle()->getWheelInfo(i);
            // Make wheels that are not touching the ground invisible
            m_wheel_node[i]->setVisible(wi.m_raycastInfo.m_isInContact);
        }
#endif
        core::vector3df pos =  m_wheel_graphics_position[i].toIrrVector();

        float suspension_length = m_default_physics_suspension[i];
        GhostKart* gk = dynamic_cast<GhostKart*>(m_kart);
        // Prevent using suspension length uninitialized
        if ( !gk || gt_replay_index != -1)
        {
            if (gk)
            {
                suspension_length = gk->getSuspensionLength(gt_replay_index, i);
            }
            else
            {
                suspension_length = m_kart->getVehicle()->getWheelInfo(i).
                                    m_raycastInfo.m_suspensionLength;
            }
        }

        // Check documentation of Kart::updateGraphics for the following line
        pos.Y +=   m_default_physics_suspension[i]
                 - suspension_length
                 - m_kart_lowest_point;

        m_wheel_node[i]->setPosition(pos);

        // Now calculate the new rotation: (old + change) mod 360
        float new_rotation = m_wheel_node[i]->getRotation().X
                           + distance / m_wheel_graphics_radius[i] * RAD_TO_DEGREE;
        new_rotation = fmodf(new_rotation, 360);
        core::vector3df wheel_rotation(new_rotation, 0, 0);
        // Only apply steer to first 2 wheels.
        if (i < 2)
            wheel_rotation += wheel_steer;
        m_wheel_node[i]->setRotation(wheel_rotation);

        // Undo lean angle applied by parent
        core::matrix4 parent_m;
        parent_m.setInverseTranslation(core::vector3df
            (0, fabsf(tanf(current_lean_angle)) * m_kart_width * 0.5f, 0));
        parent_m.setInverseRotationRadians(core::vector3df
            (0, 0, -current_lean_angle));

        core::matrix4 final_m = parent_m * m_wheel_node[i]->getRelativeTransformation();
        m_wheel_node[i]->setPosition(final_m.getTranslation());
    } // for (i < 4)

    // Update the speed-weighted objects' animations
    if (m_kart != NULL)
    {
        for (size_t i = 0; i < m_speed_weighted_objects.size(); i++)
        {
            SpeedWeightedObject&    obj = m_speed_weighted_objects[i];

            if (obj.m_node == NULL)
            {
                continue;
            }
            // Animation strength
            const float strength_factor = obj.m_properties.m_strength_factor;
            if (strength_factor >= 0.0f)
            {
                float strength = speed * strength_factor;
                btClamp<float>(strength, 0.0f, 1.0f);
                obj.m_node->setAnimationStrength(strength);
            }

            // Animation speed
            const float speed_factor = obj.m_properties.m_speed_factor;
            if (speed_factor >= 0.0f)
            {
                float anim_speed = speed * speed_factor;
                obj.m_node->setAnimationSpeed(anim_speed);
            }

            if (obj.m_properties.m_moving_texture)
            {
                obj.m_properties.m_moving_texture->update(speed * dt);
                SP::SPMeshNode* spmn = dynamic_cast<SP::SPMeshNode*>(obj.m_node);
                if (spmn)
                {
                    for (unsigned i = 0; i < spmn->getSPM()->getMeshBufferCount(); i++)
                    {
                        auto& ret = spmn->getTextureMatrix(i);
                        ret[0] = obj.m_properties.m_moving_texture->getCurrentX();
                        ret[1] = obj.m_properties.m_moving_texture->getCurrentY();
                    }
                }
                else
                {
                    for (unsigned int i = 0;
                        i < obj.m_node->getMaterialCount(); i++)
                    {
                        video::SMaterial &irrMaterial =
                            obj.m_node->getMaterial(i);
                        for (unsigned int j = 0; j < video::MATERIAL_MAX_TEXTURES;
                            j++)
                        {
                            video::ITexture* t = irrMaterial.getTexture(j);
                            if (!t) continue;
                            core::matrix4 *m =
                                &irrMaterial.getTextureMatrix(j);
                            m->setTextureTranslate(
                                obj.m_properties.m_moving_texture->getCurrentX(),
                                obj.m_properties.m_moving_texture->getCurrentY());
                        }   // for j<MATERIAL_MAX_TEXTURES
                    }   // for i<getMaterialCount
                }

            }
        }
    }

    // If animations are disabled, stop here
    if (m_animated_node == NULL) return;

    // Check if the end animation is being played, if so, don't
    // play steering animation.
    if(m_current_animation!=AF_DEFAULT) return;

    if(m_animation_frame[AF_LEFT]<0) return;   // no animations defined

    // Update animation if necessary
    // -----------------------------
    const bool back = m_animation_frame[AF_BACK_STRAIGHT] > -1 && speed < 0.0f;
    float frame;
    if(steer>0.0f && back) frame = m_animation_frame[AF_BACK_STRAIGHT]
                                 - ( ( m_animation_frame[AF_BACK_STRAIGHT]
                                   -m_animation_frame[AF_BACK_RIGHT]  )*steer);
    else if(steer<0.0f && back) frame = m_animation_frame[AF_BACK_STRAIGHT]
                              + ( (m_animation_frame[AF_BACK_STRAIGHT]
                                   -m_animation_frame[AF_BACK_LEFT]   )*steer);
    else if(steer>0.0f) frame = m_animation_frame[AF_STRAIGHT]
                              - ( ( m_animation_frame[AF_STRAIGHT]
                                        -m_animation_frame[AF_RIGHT]  )*steer);
    else if(steer<0.0f) frame = m_animation_frame[AF_STRAIGHT]
                              + ( (m_animation_frame[AF_STRAIGHT]
                                        -m_animation_frame[AF_LEFT]   )*steer);
    else                frame = (float)(back ?
                                m_animation_frame[AF_BACK_STRAIGHT] :
                                m_animation_frame[AF_STRAIGHT]);
    m_animated_node->setCurrentFrame(frame);
}   // update

//-----------------------------------------------------------------------------
/** Called when a kart is rescued to reset all visual wheels to their default
 *  position to avoid that some wheels look too far away from the kart (which
 *  is not that visible while a kart is driving).
 */
void KartModel::resetVisualWheelPosition()
{
    for(unsigned int i=0; i<4; i++)
    {
        m_kart->getVehicle()->getWheelInfo(i).m_raycastInfo.m_suspensionLength =
            m_default_physics_suspension[i];
    }   // for i < 4
}   // resetVisualSuspension

//-----------------------------------------------------------------------------
std::shared_ptr<GE::GERenderInfo> KartModel::getRenderInfo()
{
    return m_support_colorization ||
        (m_render_info && m_render_info->isTransparent()) ?
        m_render_info : NULL;
}   // getRenderInfo

//-----------------------------------------------------------------------------
void KartModel::toggleHeadlights(bool on)
{
    for (unsigned int i = 0; i < m_headlight_objects.size(); i++)
    {
        HeadlightObject& obj = m_headlight_objects[i];
        if (obj.getLightNode())
        {
            obj.getLightNode()->setVisible(on);
        }
    }
}   // toggleHeadlights

//-----------------------------------------------------------------------------
/** Called when a kart is load. this will load all the inverse bone matrices
 *  for each bone in straight frame. The location, rotation and scale in
 *  kart.xml for attachments (speedweighted objects, headlight, hat...) are in
 *  object space, so if you use a inverse bone matrix * that matrix, it will be
 *  relative to the bone, and you can use the result to set parent.
 */
void KartModel::initInverseBoneMatrices()
{
    if (m_version < 3)
    {
        // Only need for >= 3 version of kart
        return;
    }
    float striaght_frame = (float)m_animation_frame[AF_STRAIGHT];
    if (m_animation_frame[AF_STRAIGHT] == -1)
    {
        Log::warn("KartModel", "%s has no striaght frame defined.",
            m_model_filename.c_str());
        striaght_frame = 0.0f;
    }
    std::vector<GE::Armature> armatures;

#ifndef SERVER_ONLY
    using namespace SP;
    SPMesh* spm = dynamic_cast<SPMesh*>(m_mesh);
    GE::GESPM* ge_spm = dynamic_cast<GE::GESPM*>(m_mesh);
    if (spm)
        armatures = spm->getArmatures();
    else if (ge_spm)
        armatures = ge_spm->getArmatures();
#endif

    if (!armatures.empty())
    {
        for (GE::Armature& arm : armatures)
        {
            arm.getInterpolatedMatrices(striaght_frame);
            for (auto& p : arm.m_world_matrices)
            {
                p.second = false;
            }
            for (unsigned i = 0; i < arm.m_joint_names.size(); i++)
            {
                core::matrix4 m;
                arm.getWorldMatrix(arm.m_interpolated_matrices, i)
                    .getInverse(m);
                m_inverse_bone_matrices[arm.m_joint_names[i]] = m;
            }
        }
    }
    else
    {
        // Due to irrlicht mesh doesn't expose bone name, we have to create a
        // dummy aniamted node
        // All bone matrices are configured in straight frame (as in exporting)
        scene::IAnimatedMeshSceneNode* node = irr_driver->getSceneManager()
            ->addAnimatedMeshSceneNode(m_mesh);
        const unsigned total_joint = node->getJointCount();
        for (unsigned i = 0; i < total_joint; i++)
        {
            node->setCurrentFrame(striaght_frame);
            node->OnAnimate(0);
            scene::IBoneSceneNode* bone = node->getJointNode(i);
            if (!bone)
                continue;
            bone->updateAbsolutePosition();
            node->setCurrentFrame(striaght_frame);
            node->OnAnimate(0);
            bone->updateAbsolutePosition();
            core::matrix4 inv;
            bone->getAbsoluteTransformation().getInverse(inv);
            const std::string bone_name = bone->getName();
            auto ret = m_inverse_bone_matrices.find(bone_name);
            if (ret != m_inverse_bone_matrices.end())
            {
                Log::warn("KartModel", "%s has duplicated bone, name: %s,"
                    " attachment may not work correctly.",
                    m_model_filename.c_str(), bone_name.c_str());
            }
            m_inverse_bone_matrices[bone_name] = inv;
        }
        node->remove();
    }
}   // initInverseBoneMatrices

//-----------------------------------------------------------------------------
const core::matrix4& KartModel::getInverseBoneMatrix
                                           (const std::string& bone_name) const
{
    // Remove after GESPM animation is done
    static core::matrix4 unused;
    assert(m_version >= 3);
    auto ret = m_inverse_bone_matrices.find(bone_name);
    if (ret == m_inverse_bone_matrices.end())
        return unused;
    return ret->second;
}   // getInverseBoneMatrix
