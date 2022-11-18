//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011-2015 Joerg Henrichs
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

#include "graphics/referee.hpp"
#include "config/stk_config.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/light.hpp"
#include "graphics/material.hpp"
#include "graphics/mesh_tools.hpp"
#include "graphics/sp/sp_mesh_buffer.hpp"
#include "graphics/sp/sp_mesh_node.hpp"
#include "karts/abstract_kart.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "modes/world.hpp"
#include "utils/constants.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"

#include <ISceneManager.h>
#include <ITexture.h>

int                   Referee::m_st_first_start_frame  = 1;
int                   Referee::m_st_last_start_frame   = 1;
int                   Referee::m_st_first_rescue_frame = 1;
int                   Referee::m_st_last_rescue_frame  = 1;
int                   Referee::m_st_traffic_buffer     = -1;
Vec3                  Referee::m_st_start_offset       = Vec3(-2, 2, 2);
Vec3                  Referee::m_st_start_rotation     = Vec3(0, 180, 0);
Vec3                  Referee::m_st_scale              = Vec3(1, 1, 1);
float                 Referee::m_height                = 0.0f;
scene::IAnimatedMesh *Referee::m_st_referee_mesh       = NULL;

// ----------------------------------------------------------------------------
/** Loads the static mesh.
 */
void Referee::init()
{
    assert(!m_st_referee_mesh);
    const std::string filename=file_manager->getAssetChecked(FileManager::MODEL,
                                                             "referee.xml", true);
    XMLNode *node = file_manager->createXMLTree(filename);
    if(!node)
    {
        Log::fatal("referee", "Can't read XML file referee.xml, aborting.");
    }
    if(node->getName()!="referee")
    {
        Log::fatal("referee", "The file referee.xml does not contain a referee"
               "node, aborting.");
    }
    std::string model_filename;
    node->get("model", &model_filename);

    m_st_referee_mesh = irr_driver->getAnimatedMesh(
                                 file_manager->getAsset(FileManager::MODEL,
                                                        model_filename)      );
    if(!m_st_referee_mesh)
    {
        Log::fatal("referee", "Can't find referee model '%s', aborting.",
               model_filename.c_str());
    }

    node->get("first-rescue-frame", &m_st_first_rescue_frame);
    node->get("last-rescue-frame",  &m_st_last_rescue_frame );
    node->get("first-start-frame",  &m_st_first_start_frame );
    node->get("last-start-frame",   &m_st_last_start_frame  );
    node->get("start-offset",       &m_st_start_offset      );
    node->get("scale",              &m_st_scale             );
    node->get("start-rotation",     &m_st_start_rotation    );

    float angle_to_kart = atan2(m_st_start_offset.getX(),
                                m_st_start_offset.getZ())
                        * RAD_TO_DEGREE;
    m_st_start_rotation.setY(m_st_start_rotation.getY()+angle_to_kart);

    for(unsigned int i=0; i<m_st_referee_mesh->getMeshBufferCount(); i++)
    {
        if (m_st_traffic_buffer != -1)
        {
            break;
        }
        scene::IMeshBuffer *mb = m_st_referee_mesh->getMeshBuffer(i);
        SP::SPMeshBuffer* spmb = dynamic_cast<SP::SPMeshBuffer*>(mb);
        if (spmb)
        {
            auto ret = spmb->getAllSTKMaterials();
            for (unsigned j = 0; j < ret.size(); j++)
            {
                std::string name =
                    StringUtils::getBasename(ret[j]->getSamplerPath(0));
                if (name == "traffic_light.png")
                {
                    m_st_traffic_buffer = i;
                    spmb->enableTextureMatrix(j);
                    break;
                }
            }
            continue;
        }
        video::SMaterial &irrMaterial = mb->getMaterial();
        video::ITexture* t=irrMaterial.getTexture(0);
        if(!t) continue;

        std::string name=StringUtils::getBasename(t->getName()
                                                  .getInternalName().c_str());
        if (name == "traffic_light.png")
        {
            m_st_traffic_buffer = i;
            break;
        }
        else
        {
            irrMaterial.MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF;
        }

    }

    delete node;
}   // init

// ----------------------------------------------------------------------------
/** Frees the static mesh.
 */
void Referee::cleanup()
{
    irr_driver->removeMeshFromCache(m_st_referee_mesh);
    m_st_referee_mesh = NULL;
    m_st_traffic_buffer = -1;
}   // cleanup

// ----------------------------------------------------------------------------
/** Creates an instance of the referee, using the static values to initialise
 *  it. This is the constructor used when a start referee is needed.
 */
Referee::Referee()
{
    assert(m_st_referee_mesh);
    // First add a NULL mesh, then set the material to be read only
    // (this appears to be the only way to get read only materials).
    // This way we only need to adjust the materials in the original
    // mesh. ATM it doesn't make any difference, but if we ever should
    // decide to use more than one referee model at startup we only
    // have to change the textures once, and all models will be in synch.
    //
    // Disabled due to texture matrix not working in legacy video drivers
    m_scene_node = irr_driver->addAnimatedMesh(NULL, "referee");
    //m_scene_node->setReadOnlyMaterials(true);
    m_scene_node->setMesh(m_st_referee_mesh);
    m_scene_node->grab();
    m_scene_node->setRotation(m_st_start_rotation.toIrrVector());
    m_scene_node->setScale(m_st_scale.toIrrVector());
    m_scene_node->setFrameLoop(m_st_first_start_frame,
                               m_st_last_start_frame);
#ifndef SERVER_ONLY
    if (CVS->isGLSL() && CVS->isDeferredEnabled())
    {
        m_light = irr_driver->addLight(core::vector3df(0.0f, 0.0f, 0.6f), 0.7f, 2.0f,
            0.7f /* r */, 0.0 /* g */, 0.0f /* b */, false /* sun */, m_scene_node);
    }
    else
#endif
    {
        m_light = NULL;
    }
}   // Referee

// ----------------------------------------------------------------------------
/** Creates an instance of the referee, using the static values to initialise
 *  it. This is the constructor used when a rescue referee is needed.
 *  \param kart The kart which the referee should rescue.
 */
Referee::Referee(const AbstractKart &kart)
{
    assert(m_st_referee_mesh);
    // First add a NULL mesh, then set the material to be read only
    // (this appears to be the only way to get read only materials).
    // This way we only need to adjust the materials in the original
    // mesh. ATM it doesn't make any difference, but if we ever should
    // decide to use more than one referee model at startup we only
    // have to change the textures once, and all models will be in synch.
    m_scene_node = irr_driver->addAnimatedMesh(NULL, "referee");
    //m_scene_node->setReadOnlyMaterials(true);
    m_scene_node->setMesh(m_st_referee_mesh);
    m_scene_node->grab();
    m_scene_node->setScale(m_st_scale.toIrrVector());
    m_scene_node->setPosition(core::vector3df(0, kart.getKartHeight() + 0.4f, 0));

}   // Referee

// ----------------------------------------------------------------------------
Referee::~Referee()
{
    if(m_scene_node->getParent())
        irr_driver->removeNode(m_scene_node);
    m_scene_node->drop();
}   // ~Referee

// ----------------------------------------------------------------------------
/** Make sure that this referee is attached to the scene graph. This is used
 *  for the start referee, which is removed from scene graph once the ready-
 *  set-go phase is over (it is kept in case of a restart of the race).
 */
void Referee::attachToSceneNode()
{
    if(!m_scene_node->getParent())
        m_scene_node->setParent(irr_driver->getSceneManager()
                                          ->getRootSceneNode());

    if (m_light != NULL)
        m_light->setVisible(true);
}   // attachToSceneNode

// ----------------------------------------------------------------------------
/** Removes the referee's scene node from the scene graph, but still keeps
 *  the scene node in memory. This is used for the start referee, so that
 *  it is quickly available in case of a restart.
 */
void Referee::removeFromSceneGraph()
{
    if(isAttached())
        irr_driver->removeNode(m_scene_node);
    if (m_light != NULL)
        m_light->setVisible(false);
}   // removeFromSceneGraph

// ----------------------------------------------------------------------------
/** Selects one of the states 'ready', 'set', or 'go' to be displayed by
 *  the referee.
 *  \param rsg 0=ready, 1=set, 2=go.
 */
void Referee::selectReadySetGo(int rsg)
{
    if (m_st_traffic_buffer < 0)
        return;

    SP::SPMeshNode* spmn = dynamic_cast<SP::SPMeshNode*>(m_scene_node);
    if (spmn)
    {
        spmn->setTextureMatrix(m_st_traffic_buffer, {{ 0.0f, rsg * 0.333f }});
    }
    else
    {
        video::SMaterial &m = m_scene_node->getMaterial(m_st_traffic_buffer);
        core::matrix4* matrix = &m.getTextureMatrix(0);
        matrix->setTextureTranslate(0.0f, rsg*0.333f);
        // disable lighting, we need to see the traffic light even if facing away
        // from the sun
        m.AmbientColor  = video::SColor(255, 255, 255, 255);
        m.DiffuseColor  = video::SColor(255, 255, 255, 255);
        m.EmissiveColor = video::SColor(255, 255, 255, 255);
        m.SpecularColor = video::SColor(255, 255, 255, 255);
    }

    if (m_light != NULL)
    {
        if (rsg == 0)
        {
            ((LightNode*)m_light)->setColor(0.6f, 0.0f, 0.0f);
        }
        else if (rsg == 1)
        {
            ((LightNode*)m_light)->setColor(0.7f, 0.23f, 0.0f);
        }
        else if (rsg == 2)
        {
            ((LightNode*)m_light)->setColor(0.0f, 0.6f, 0.0f);
        }
    }
}   // selectReadySetGo

// ----------------------------------------------------------------------------
/** Set the referee animation frame with created ticks of \ref RescueAnimation,
 *  so that it's synchronized with world ticks, and can be rewound easily.
 */
void Referee::setAnimationFrameWithCreatedTicks(int created_ticks)
{
    float dur = stk_config->ticks2Time(
        World::getWorld()->getTicksSinceStart() - created_ticks);
    dur *= 25.0f;
    float ref_dur = (float)(m_st_last_rescue_frame - m_st_first_rescue_frame);
    float frame = std::fmod(dur, ref_dur);
    frame += (float)m_st_first_rescue_frame;
    m_scene_node->setCurrentFrame(frame);
}   // setAnimationFrameWithCreatedTicks

// ----------------------------------------------------------------------------
/** Moves the referee to the specified position. */
void Referee::setPosition(const Vec3 &xyz)
{
    m_scene_node->setPosition(xyz.toIrrVector());
}   // setPosition

// ----------------------------------------------------------------------------
/** Sets the rotation of the scene node (in degrees).
 *  \param hpr Rotation in degrees. */
void Referee::setRotation(const Vec3 &hpr)
{
    m_scene_node->setRotation(hpr.toIrrVector());
}   // setRotation

// ----------------------------------------------------------------------------
/** Returns true if this referee is attached to the scene graph. */
bool Referee::isAttached() const
{
    return m_scene_node->getParent() != NULL;
}   // isAttached
