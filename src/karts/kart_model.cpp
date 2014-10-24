//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008-2013 Joerg Henrichs
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
#include <ISceneManager.h>

#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/lod_node.hpp"
#include "graphics/mesh_tools.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/kart_properties.hpp"
#include "physics/btKart.hpp"
#include "utils/constants.hpp"
#include "utils/log.hpp"

#include "IMeshManipulator.h"

#define SKELETON_DEBUG 0

float KartModel::UNDEFINED = -99.9f;

// ------------------------------------------------------------
// SpeedWeightedObject implementation

#define SPEED_WEIGHTED_OBJECT_PROPERTY_UNDEFINED -99.f

SpeedWeightedObject::Properties::Properties()
{
    m_strength_factor = m_speed_factor = m_texture_speed.X = m_texture_speed.Y = SPEED_WEIGHTED_OBJECT_PROPERTY_UNDEFINED;
}

void SpeedWeightedObject::Properties::loadFromXMLNode(const XMLNode* xml_node)
{
    xml_node->get("strength-factor", &m_strength_factor);
    xml_node->get("speed-factor",    &m_speed_factor);
    xml_node->get("texture-speed-x", &m_texture_speed.X);
    xml_node->get("texture-speed-y", &m_texture_speed.Y);
}

void SpeedWeightedObject::Properties::checkAllSet()
{
#define CHECK_NEG(  a,strA) if(a<=SPEED_WEIGHTED_OBJECT_PROPERTY_UNDEFINED) {                   \
            Log::fatal("SpeedWeightedObject", "Missing default value for '%s'.",    \
                        strA);              \
        }
    CHECK_NEG(m_strength_factor, "speed-weighted strength-factor"    );
    CHECK_NEG(m_speed_factor,    "speed-weighted speed-factor"    );
    CHECK_NEG(m_texture_speed.X,  "speed-weighted texture speed X"    );
    CHECK_NEG(m_texture_speed.Y,  "speed-weighted texture speed Y"    );
#undef CHECK_NEG
}

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
    m_is_master  = is_master;
    m_kart       = NULL;
    m_mesh       = NULL;
    m_hat_name   = "";
    m_hat_node   = NULL;
    m_hat_offset = core::vector3df(0,0,0);

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
    }
    m_wheel_filename[0] = "";
    m_wheel_filename[1] = "";
    m_wheel_filename[2] = "";
    m_wheel_filename[3] = "";
    m_speed_weighted_objects.clear();
    m_animated_node     = NULL;
    m_mesh              = NULL;
    for(unsigned int i=AF_BEGIN; i<=AF_END; i++)
        m_animation_frame[i]=-1;
    m_animation_speed   = 25;
    m_current_animation = AF_DEFAULT;
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
        animation_node->get("start-losing",   &m_animation_frame[AF_LOSE_START]);
        animation_node->get("start-losing-loop",
                                             &m_animation_frame[AF_LOSE_LOOP_START]);
        animation_node->get("end-losing",     &m_animation_frame[AF_LOSE_END]  );
        animation_node->get("start-explosion",&m_animation_frame[AF_LOSE_START]);
        animation_node->get("end-explosion",  &m_animation_frame[AF_LOSE_END]  );
        animation_node->get("start-jump",     &m_animation_frame[AF_JUMP_START]);
        animation_node->get("start-jump-loop",&m_animation_frame[AF_JUMP_LOOP] );
        animation_node->get("end-jump",       &m_animation_frame[AF_JUMP_END]  );
        animation_node->get("start-speed-weighted",     &m_animation_frame[AF_SPEED_WEIGHTED_START]     );
        animation_node->get("end-speed-weighted",       &m_animation_frame[AF_SPEED_WEIGHTED_END]       );
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

    if(const XMLNode *speed_weighted_objects_node=node.getNode("speed-weighted-objects"))
    {
        SpeedWeightedObject::Properties   fallback_properties;
        fallback_properties.loadFromXMLNode(speed_weighted_objects_node);

        for(unsigned int i=0 ; i < speed_weighted_objects_node->getNumNodes() ; i++)
        {
            loadSpeedWeightedInfo(speed_weighted_objects_node->getNode(i), fallback_properties);
        }
    }

    if(const XMLNode *hat_node=node.getNode("hat"))
    {
        hat_node->get("offset", &m_hat_offset);
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
        if(m_is_master && m_speed_weighted_objects[i].m_model)
        {
            irr_driver->dropAllTextures(m_speed_weighted_objects[i].m_model);
            irr_driver->removeMeshFromCache(m_speed_weighted_objects[i].m_model);
        }
    }

    if(m_is_master && m_mesh)
    {
        m_mesh->drop();
        // If there is only one copy left, it's the copy in irrlicht's
        // mesh cache, so it can be remove.
        if(m_mesh && m_mesh->getReferenceCount()==1)
        {
            irr_driver->dropAllTextures(m_mesh);
            irr_driver->removeMeshFromCache(m_mesh);
        }
    }

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
KartModel* KartModel::makeCopy()
{
    // Make sure that we are copying from a master objects, and
    // that there is indeed no animated node defined here ...
    // just in case.
    assert(m_is_master);
    assert(!m_animated_node);
    KartModel *km           = new KartModel(/*is master*/ false);
    km->m_kart_width        = m_kart_width;
    km->m_kart_length       = m_kart_length;
    km->m_kart_height       = m_kart_height;
    km->m_kart_highest_point= m_kart_highest_point;
    km->m_kart_lowest_point = m_kart_lowest_point;
    km->m_mesh              = m_mesh;
    km->m_model_filename    = m_model_filename;
    km->m_animation_speed   = m_animation_speed;
    km->m_current_animation = AF_DEFAULT;
    km->m_animated_node     = NULL;
    km->m_hat_offset        = m_hat_offset;
    km->m_hat_name          = m_hat_name;
    
    km->m_nitro_emitter_position[0] = m_nitro_emitter_position[0];
    km->m_nitro_emitter_position[1] = m_nitro_emitter_position[1];
    km->m_has_nitro_emitter = m_has_nitro_emitter;
    
    for(unsigned int i=0; i<4; i++)
    {
        km->m_wheel_model[i]             = m_wheel_model[i];
        // Master should not have any wheel nodes.
        assert(!m_wheel_node[i]);
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

    for(unsigned int i=AF_BEGIN; i<=AF_END; i++)
        km->m_animation_frame[i] = m_animation_frame[i];

    return km;
}   // makeCopy

// ----------------------------------------------------------------------------

/** Attach the kart model and wheels to the scene node.
 *  \return the node with the model attached
 */
scene::ISceneNode* KartModel::attachModel(bool animated_models, bool always_animated)
{
    assert(!m_is_master);

    scene::ISceneNode* node;

    if (animated_models)
    {
        LODNode* lod_node = new LODNode("kart",
                                        irr_driver->getSceneManager()->getRootSceneNode(),
                                        irr_driver->getSceneManager()                    );


        node = irr_driver->addAnimatedMesh(m_mesh, "kartmesh");
        // as animated mesh are not cheap to render use frustum box culling
        if (irr_driver->isGLSL())
            node->setAutomaticCulling(scene::EAC_OFF);
        else
            node->setAutomaticCulling(scene::EAC_FRUSTUM_BOX);

        if (always_animated)
        {
            // give a huge LOD distance for the player's kart. the reason is that it should
            // use its animations for the shadow pass too, where the camera can be quite far
            lod_node->add(10000, node, true);
            scene::ISceneNode* static_model = attachModel(false, false);
            lod_node->add(10001, static_model, true);
            m_animated_node = static_cast<scene::IAnimatedMeshSceneNode*>(node);
        }
        else
        {
            lod_node->add(20, node, true);
            scene::ISceneNode* static_model = attachModel(false, false);
            lod_node->add(100, static_model, true);
            m_animated_node = static_cast<scene::IAnimatedMeshSceneNode*>(node);
        }

        attachHat();

#ifdef DEBUG
        std::string debug_name = m_model_filename+" (animated-kart-model)";
        node->setName(debug_name.c_str());
#if SKELETON_DEBUG
        irr_driver->addDebugMesh(m_animated_node);
#endif
#endif
        m_animated_node->setLoopMode(false);
        m_animated_node->grab();
        node = lod_node;

        // Become the owner of the wheels
        for(unsigned int i=0; i<4; i++)
        {
            if (!m_wheel_model[i] || !m_wheel_node[i]) continue;
            m_wheel_node[i]->setParent(lod_node);
        }

        // Become the owner of the speed weighted objects
        for(size_t i=0; i<m_speed_weighted_objects.size(); i++)
        {
            if(!m_speed_weighted_objects[i].m_node) continue;
            m_speed_weighted_objects[i].m_node->setParent(lod_node);
        }

        // Enable rim lighting for the kart
        irr_driver->applyObjectPassShader(lod_node, true);
        std::vector<scene::ISceneNode*> &lodnodes = lod_node->getAllNodes();
        const u32 max = (u32)lodnodes.size();
        for (u32 i = 0; i < max; i++)
        {
            irr_driver->applyObjectPassShader(lodnodes[i], true);
        }
    }
    else
    {
        // If no animations are shown, make sure to pick the frame
        // with a straight ahead animation (if exist).
        int straight_frame = m_animation_frame[AF_STRAIGHT]>=0
                           ? m_animation_frame[AF_STRAIGHT]
                           : 0;

        scene::IMesh* main_frame = m_mesh->getMesh(straight_frame);
        main_frame->setHardwareMappingHint(scene::EHM_STATIC);

        std::string debug_name;

#ifdef DEBUG
       debug_name = m_model_filename + " (kart-model)";
#endif

        node = irr_driver->addMesh(main_frame, debug_name);

#ifdef DEBUG
        node->setName(debug_name.c_str());
#endif


        // Attach the wheels
        for(unsigned int i=0; i<4; i++)
        {
            if(!m_wheel_model[i]) continue;
            m_wheel_node[i] = irr_driver->addMesh(m_wheel_model[i], "wheel", node);
            Vec3 wheel_min, wheel_max;
            MeshTools::minMax3D(m_wheel_model[i], &wheel_min, &wheel_max);
            m_wheel_graphics_radius[i] = 0.5f*(wheel_max.getY() - wheel_min.getY());

            m_wheel_node[i]->grab();
            ((scene::IMeshSceneNode *) m_wheel_node[i])->setReadOnlyMaterials(true);
    #ifdef DEBUG
            std::string debug_name = m_wheel_filename[i]+" (wheel)";
            m_wheel_node[i]->setName(debug_name.c_str());
    #endif
            m_wheel_node[i]->setPosition(m_wheel_graphics_position[i].toIrrVector());
        }

        // Attach the speed weighted objects + set the animation state
        for(size_t i=0 ; i < m_speed_weighted_objects.size() ; i++)
        {
            SpeedWeightedObject&    obj = m_speed_weighted_objects[i];
            obj.m_node = NULL;
            if(obj.m_model)
            {
                obj.m_node = irr_driver->addAnimatedMesh(obj.m_model, "speedweighted", node);
                obj.m_node->grab();

                obj.m_node->setFrameLoop(m_animation_frame[AF_SPEED_WEIGHTED_START], m_animation_frame[AF_SPEED_WEIGHTED_END]);

        #ifdef DEBUG
                std::string debug_name = obj.m_name+" (speed-weighted)";
                obj.m_node->setName(debug_name.c_str());
        #endif
                obj.m_node->setPosition(obj.m_position.toIrrVector());
            }
        }
    }
    return node;
}   // attachModel


// ----------------------------------------------------------------------------
/** Loads the 3d model and all wheels.
 */
bool KartModel::loadModels(const KartProperties &kart_properties)
{
    assert(m_is_master);
    std::string  full_path = kart_properties.getKartDir()+m_model_filename;
    m_mesh                 = irr_driver->getAnimatedMesh(full_path);
    if(!m_mesh)
    {
        Log::error("Kart_Model", "Problems loading mesh '%s' - kart '%s' will"
                   "not be available.",
                   full_path.c_str(), kart_properties.getIdent().c_str());
        return false;
    }
    m_mesh->grab();
    irr_driver->grabAllTextures(m_mesh);

    Vec3 kart_min, kart_max;
    MeshTools::minMax3D(m_mesh->getMesh(m_animation_frame[AF_STRAIGHT]),
                        &kart_min, &kart_max);

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
        irr_driver->grabAllTextures(obj.m_model);

        // Update min/max
        Vec3 obj_min, obj_max;
        MeshTools::minMax3D(obj.m_model, &obj_min, &obj_max);
        obj_min += obj.m_position;
        obj_max += obj.m_position;
        kart_min.min(obj_min);
        kart_max.max(obj_max);
    }

    Vec3 size     = kart_max-kart_min;
    m_kart_width  = size.getX();
    m_kart_height = size.getY();
    m_kart_length = size.getZ();

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

    float y_off = kart_properties.getGraphicalYOffset();
    if(y_off!=0)
    {
        for (unsigned int i = 0; i < 4; i++)
            m_wheel_graphics_position[i].setY(
                                  m_wheel_graphics_position[i].getY() - y_off);
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
        // Grab all textures. This is done for the master only, so
        // the destructor will only free the textures if a master
        // copy is freed.
        irr_driver->grabAllTextures(m_wheel_model[i]);
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

/** Loads a single speed weighted node. */
void KartModel::loadSpeedWeightedInfo(const XMLNode* speed_weighted_node, const SpeedWeightedObject::Properties& fallback_properties)
{
    SpeedWeightedObject obj;
    obj.m_properties    = fallback_properties;
    obj.m_properties.loadFromXMLNode(speed_weighted_node);
    
    speed_weighted_node->get("position", &obj.m_position);
    speed_weighted_node->get("model",    &obj.m_name);

    if(!obj.m_name.empty())
        m_speed_weighted_objects.push_back(obj);
}

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
    if(!wheel_node)
    {
        // Only print the warning if a model filename is given. Otherwise the
        // stk_config file is read (which has no model information).
        if(m_model_filename!="")
        {
            Log::error("Kart_Model", "Missing wheel information '%s' for model "
                       "'%s'.", wheel_name.c_str(), m_model_filename.c_str());
            Log::error("Kart_Model", "This can be ignored, but the wheels will "
                       "not rotate.");
        }
        return;
    }
    wheel_node->get("model",            &m_wheel_filename[index]         );
    wheel_node->get("position",         &m_wheel_graphics_position[index]);
    wheel_node->get("min-suspension",   &m_min_suspension[index]         );
    wheel_node->get("max-suspension",   &m_max_suspension[index]         );
}   // loadWheelInfo

// ----------------------------------------------------------------------------
/** Resets the kart model. It stops animation from being played and resets
 *  the wheels to the correct position (i.e. no suspension).
 */
void KartModel::reset()
{
    update(0.0f, 0.0f, 0.0f, 0.0f);

    // Stop any animations currently being played.
    setAnimation(KartModel::AF_DEFAULT);
    // Don't force any LOD
    ((LODNode*)m_kart->getNode())->forceLevelOfDetail(-1);
}   // reset

// ----------------------------------------------------------------------------
/** Called when the kart finished the race. It will force the highest LOD
 *  for the kart, since otherwise the end camera can be far away (due to
 *  zooming) and show non-animated karts.
 */
void KartModel::finishedRace()
{
    // Force the animated model, independent of actual camera distance.
    ((LODNode*)m_kart->getNode())->forceLevelOfDetail(0);
}   // finishedRace

// ----------------------------------------------------------------------------
/** Enables- or disables the end animation.
 *  \param type The type of animation to play.
 */
void KartModel::setAnimation(AnimationFrameType type)
{
    // if animations disabled, give up
    if (m_animated_node == NULL) return;

    m_current_animation = type;
    if(m_current_animation==AF_DEFAULT)
    {
        m_animated_node->setLoopMode(false);
        if(m_animation_frame[AF_LEFT] <= m_animation_frame[AF_RIGHT])
            m_animated_node->setFrameLoop(m_animation_frame[AF_LEFT],
                                          m_animation_frame[AF_RIGHT] );
        else
            m_animated_node->setFrameLoop(m_animation_frame[AF_RIGHT],
                                          m_animation_frame[AF_LEFT] );
        m_animated_node->setAnimationEndCallback(NULL);
        m_animated_node->setAnimationSpeed(0);
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

    // It should be a non-default type of animation, and should have
    // a non negative frame (i.e. the animation is indeed defined).
    if(m_current_animation==AF_DEFAULT ||
        m_animation_frame[m_current_animation]<=-1)
    {
        Log::debug("Kart_Model", "OnAnimationEnd for '%s': current %d frame %d",
               m_model_filename.c_str(),
               m_current_animation, m_animation_frame[m_current_animation]);
        assert(false);
    }

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
    for(int i=0; i<m_kart->getVehicle()->getNumWheels(); i++)
    {
        const btWheelInfo &wi = m_kart->getVehicle()->getWheelInfo(i);
        m_default_physics_suspension[i] = wi.m_raycastInfo.m_suspensionLength;
    }
}   // setDefaultSuspension

// ----------------------------------------------------------------------------
/** Rotates and turns the wheels appropriately, and adjust for suspension
    + updates the speed-weighted objects' animations.
 *  \param dt time since last frame
 *  \param rotation_dt How far the wheels have rotated since last time.
 *  \param steer The actual steer settings.
 *  \param suspension Suspension height for all four wheels.
 *  \param speed The speed of the kart in meters/sec, used for the 
 *         speed-weighted objects' animations
 */
void KartModel::update(float dt, float rotation_dt, float steer,  float speed)
{
   core::vector3df wheel_steer(0, steer*30.0f, 0);

    for(unsigned int i=0; i<4; i++)
    {
        if(!m_wheel_node[i]) continue;
        const btWheelInfo &wi = m_kart->getVehicle()->getWheelInfo(i);
#ifdef DEBUG
        if(UserConfigParams::m_physics_debug && m_kart)
        {
            // Make wheels that are not touching the ground invisible
            m_wheel_node[i]->setVisible(wi.m_raycastInfo.m_isInContact);
        }
#endif
        float rel_suspension = wi.m_raycastInfo.m_suspensionLength
                             - m_default_physics_suspension[i];
        // If the suspension is too compressed
        if(rel_suspension< m_min_suspension[i])
            rel_suspension = m_min_suspension[i];
        else if(rel_suspension > m_max_suspension[i])
            rel_suspension = m_max_suspension[i];

        core::vector3df pos =  m_wheel_graphics_position[i].toIrrVector();
        pos.Y -= rel_suspension;

        m_wheel_node[i]->setPosition(pos);

        // Now calculate the new rotation: (old + change) mod 360
        float new_rotation = m_wheel_node[i]->getRotation().X
                             + rotation_dt * RAD_TO_DEGREE;
        new_rotation = fmodf(new_rotation, 360);
        core::vector3df wheel_rotation(new_rotation, 0, 0);
        // Only apply steer to first 2 wheels.
        if (i < 2)
            wheel_rotation += wheel_steer;
        m_wheel_node[i]->setRotation(wheel_rotation);
    } // for (i < 4)

    // If animations are disabled, stop here
    if (m_animated_node == NULL) return;

    // Update the speed-weighted objects' animations
    for(size_t i=0 ; i < m_speed_weighted_objects.size() ; i++)
    {
        SpeedWeightedObject&    obj = m_speed_weighted_objects[i];

#define GET_VALUE(obj, value_name)   \
    obj.m_properties.value_name > SPEED_WEIGHTED_OBJECT_PROPERTY_UNDEFINED ? obj.m_properties.value_name : \
    m_kart->getKartProperties()->getSpeedWeightedObjectProperties().value_name

        // Animation strength
        float strength = 1.0f;
        const float strength_factor =   GET_VALUE(obj, m_strength_factor);
        if(strength_factor >= 0.0f)
        {
            strength = speed * strength_factor;
            btClamp<float>(strength, 0.0f, 1.0f);
        }
        
        // Animation speed
        const float speed_factor =   GET_VALUE(obj, m_speed_factor);
        if(speed_factor >= 0.0f)
        {
            float anim_speed = speed * speed_factor;
            obj.m_node->setAnimationSpeed(anim_speed);
        }

        // Texture animation
        core::vector2df tex_speed;
        tex_speed.X = GET_VALUE(obj, m_texture_speed.X);
        tex_speed.Y = GET_VALUE(obj, m_texture_speed.Y);
        if(tex_speed != core::vector2df(0.0f, 0.0f))
        {
            obj.m_texture_cur_offset += speed * tex_speed * dt;
            if(obj.m_texture_cur_offset.X > 1.0f) obj.m_texture_cur_offset.X = fmod(obj.m_texture_cur_offset.X, 1.0f);
            if(obj.m_texture_cur_offset.Y > 1.0f) obj.m_texture_cur_offset.Y = fmod(obj.m_texture_cur_offset.Y, 1.0f);
            
            for(unsigned int i=0; i<obj.m_node->getMaterialCount(); i++)
            {
                video::SMaterial &irrMaterial=obj.m_node->getMaterial(i);
                for(unsigned int j=0; j<video::MATERIAL_MAX_TEXTURES; j++)
                {
                    video::ITexture* t=irrMaterial.getTexture(j);
                    if(!t) continue;
                    core::matrix4 *m = &irrMaterial.getTextureMatrix(j);
                    m->setTextureTranslate(obj.m_texture_cur_offset.X, obj.m_texture_cur_offset.Y);
                }   // for j<MATERIAL_MAX_TEXTURES
            }   // for i<getMaterialCount
        }
#undef GET_VALUE
    }

    // Check if the end animation is being played, if so, don't
    // play steering animation.
    if(m_current_animation!=AF_DEFAULT) return;

    if(m_animation_frame[AF_LEFT]<0) return;   // no animations defined

    // Update animation if necessary
    // -----------------------------
    float frame;
    if(steer>0.0f)      frame = m_animation_frame[AF_STRAIGHT]
                              - ( ( m_animation_frame[AF_STRAIGHT]
                                        -m_animation_frame[AF_RIGHT]  )*steer);
    else if(steer<0.0f) frame = m_animation_frame[AF_STRAIGHT]
                              + ( (m_animation_frame[AF_STRAIGHT]
                                        -m_animation_frame[AF_LEFT]   )*steer);
    else                frame = (float)m_animation_frame[AF_STRAIGHT];

    m_animated_node->setCurrentFrame(frame);
}   // update
//-----------------------------------------------------------------------------
void KartModel::attachHat(){
    m_hat_node = NULL;
    if(m_hat_name.size()>0)
    {
        scene::IBoneSceneNode *bone = m_animated_node->getJointNode("Head");
        if(!bone)
            bone = m_animated_node->getJointNode("head");
        if(bone)
        {
            // Till we have all models fixed, accept Head and head as bone name
            scene::IMesh *hat_mesh =
                irr_driver->getAnimatedMesh(
                           file_manager->getAsset(FileManager::MODEL, m_hat_name));
            m_hat_node = irr_driver->addMesh(hat_mesh, "hat");
            bone->addChild(m_hat_node);
            m_animated_node->setCurrentFrame((float)m_animation_frame[AF_STRAIGHT]);
            m_animated_node->OnAnimate(0);
            bone->updateAbsolutePosition();
             // With the hat node attached to the head bone, we have to
            // reverse the transformation of the bone, so that the hat
            // is still properly placed. Esp. the hat offset needs
            // to be rotated.
            const core::matrix4 mat = bone->getAbsoluteTransformation();
            core::matrix4 inv;
            mat.getInverse(inv);
            core::vector3df rotated_offset;
            inv.rotateVect(rotated_offset, m_hat_offset);
            m_hat_node->setPosition(rotated_offset);
            m_hat_node->setScale(inv.getScale());
            m_hat_node->setRotation(inv.getRotationDegrees());
        }   // if bone
    }   // if(m_hat_name)
}
