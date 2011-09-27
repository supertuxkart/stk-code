//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011 Joerg Henrichs 
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

#include "graphics/irr_driver.hpp"
#include "graphics/mesh_tools.hpp"
#include "karts/kart.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "utils/constants.hpp"
#include "utils/string_utils.hpp"

int                   Referee::m_st_first_start_frame  = 1;
int                   Referee::m_st_last_start_frame   = 1;
int                   Referee::m_st_first_rescue_frame = 1;
int                   Referee::m_st_last_rescue_frame  = 1;
int                   Referee::m_st_traffic_buffer     = -1;
Vec3                  Referee::m_st_start_offset       = Vec3(-2, 2, 2);
Vec3                  Referee::m_st_start_rotation     = Vec3(0, 180, 0);
Vec3                  Referee::m_st_scale              = Vec3(1, 1, 1);
scene::IAnimatedMesh *Referee::m_st_referee_mesh       = NULL;
video::ITexture      *Referee::m_st_traffic_lights[3]  = {NULL, NULL, NULL};

// ----------------------------------------------------------------------------
/** Loads the static mesh.
 */
void Referee::init()
{
    assert(!m_st_referee_mesh);
    const std::string filename=file_manager->getModelFile("referee.xml");
    if(filename=="")
    {
        printf("Can't find referee.xml, aborting.\n");
        exit(-1);
    }
    XMLNode *node = file_manager->createXMLTree(filename);
    if(!node)
    {
        printf("Can't read XML file referee.xml, aborting.\n");
        exit(-1);
    }
    if(node->getName()!="referee")
    {
        printf("The file referee.xml does not contain a referee"
               "node, aborting.\n");
    }
    std::string model_filename;
    node->get("model", &model_filename);

    m_st_referee_mesh = irr_driver->getAnimatedMesh(
                     file_manager->getModelFile(model_filename) );
    if(!m_st_referee_mesh)
    {
        printf("Can't find referee model '%s', aborting.\n", 
               model_filename.c_str());
    }

    // Translate the mesh so that the x/z middle point
    // and for y the lowest point are at 0,0,0:
    Vec3 min,max;
    MeshTools::minMax3D(m_st_referee_mesh, &min, &max);
    Vec3 offset_from_center = -0.5f*(max+min);
    offset_from_center.setY(0);
    scene::IMeshManipulator *mani = 
        irr_driver->getVideoDriver()->getMeshManipulator();

    core::matrix4 translate(core::matrix4::EM4CONST_IDENTITY);
    translate.setTranslation(offset_from_center.toIrrVector());
    mani->transform(m_st_referee_mesh, translate);
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

    std::vector<std::string> colors;
    node->get("colors", &colors);

    if(colors.size()>3)
        printf("Too many colors for referee defined, "
               "only first three will be used.\n");
    if(colors.size()<3)
    {
        printf("Not enough colors for referee defined, "
               "only first three will be used, aborting.\n");
        exit(-1);
    }
    for(unsigned int i=0; i<3; i++)
    {
        std::string full_path = file_manager->getTextureFile(colors[i]);
        if(full_path.size()==0)
        {
            printf("Can't find texture '%s' for referee, aborting.\n",
                   colors[i].c_str());
            exit(-1);
        }
        m_st_traffic_lights[i] = irr_driver->getTexture(full_path);
    }


    for(unsigned int i=0; i<m_st_referee_mesh->getMeshBufferCount(); i++)
    {
        scene::IMeshBuffer *mb = m_st_referee_mesh->getMeshBuffer(i);
        video::SMaterial &irrMaterial = mb->getMaterial();
        video::ITexture* t=irrMaterial.getTexture(0);
        std::string name=StringUtils::getBasename(t->getName()
                                                  .getInternalName().c_str());
        if(name==colors[0] || name==colors[1] ||name==colors[2] )
        {
            m_st_traffic_buffer = i;
            break;
        }
        else
        {
            irrMaterial.MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF;
        }
        
    }

}   // init

// ----------------------------------------------------------------------------
/** Frees the static mesh.
 */
void Referee::cleanup()
{
    irr_driver->removeMeshFromCache(m_st_referee_mesh);
}   // cleanup

// ----------------------------------------------------------------------------
/** Creates an instance of the referee, using the static values to initialise
 *  it.
 *  \param is_start_referee True when this is the start referee, i.e. it must
 *         have the traffic light attached.
 */
Referee::Referee(bool is_start_referee)
{
    assert(m_st_referee_mesh);
    // First add a NULL mesh, then set the material to be read only
    // (this appears to be the only way to get read only materials).
    // This way we only need to adjust the materials in the original
    // mesh. ATM it doesn't make any difference, but if we ever should
    // decide to use more than one referee model at startup we only
    // have to change the textures once, and all models will be in synch.
    if(is_start_referee)
    {
        m_scene_node = irr_driver->addAnimatedMesh(NULL);
        m_scene_node->setReadOnlyMaterials(true);
        m_scene_node->setMesh(m_st_referee_mesh);
        m_scene_node->grab();
        m_scene_node->setRotation(m_st_start_rotation.toIrrVector());
        m_scene_node->setScale(m_st_scale.toIrrVector());
        m_scene_node->setFrameLoop(m_st_first_start_frame, m_st_last_start_frame);
    }
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
}   // removeFromSceneGraph

// ----------------------------------------------------------------------------
/** Selects one of the states 'ready', 'set', or 'go' to be displayed by
 *  the referee.
 *  \param rsg 0=ready, 1=set, 2=go.
 */
void Referee::selectReadySetGo(int rsg)
{
    if(m_st_traffic_buffer<0) return;
    video::SMaterial &m = m_scene_node->getMesh()->getMeshBuffer(m_st_traffic_buffer)->getMaterial();
    m.setTexture(0, m_st_traffic_lights[rsg]);
    
    // disable lighting, we need to see the traffic light even if facing away
    // from the sun
    m.AmbientColor  = video::SColor(255, 255, 255, 255);
    m.DiffuseColor  = video::SColor(255, 255, 255, 255);
    m.EmissiveColor = video::SColor(255, 255, 255, 255);
    m.SpecularColor = video::SColor(255, 255, 255, 255);
}   // selectReadySetGo

