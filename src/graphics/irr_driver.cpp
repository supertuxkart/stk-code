//  $Id: irr_driver.cpp 694 2006-08-29 07:42:36Z hiker $
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

#include "graphics/irr_driver.hpp"
#include "gui/engine.hpp"

#include "user_config.hpp"
#include "io/file_manager.hpp"

IrrDriver *irr_driver = NULL;

IrrDriver::IrrDriver()
{
    file_manager->dropFileSystem();
    // Try different drivers: start with opengl, then DirectX
    for(int driver_type=0; driver_type<3; driver_type++)
    {
        video::E_DRIVER_TYPE type = driver_type==0 
                                  ? video::EDT_OPENGL 
                                  : (driver_type==1 
                                     ? video::EDT_DIRECT3D9 
                                     : video::EDT_DIRECT3D8);
        // Try 32 and 16 bit per pixels
        for(int bits=32; bits>15; bits -=16) 
        {
            m_device = createDevice(type,
                                   core::dimension2d<s32>(user_config->m_width,
                                                          user_config->m_height ),
                                    bits, //bits per pixel
                                    user_config->m_fullscreen,
                                    false,  // stencil buffers
                                    false,  // vsync
                                    this    // event receiver
                                    );
            if(m_device) break;
        }   // for bits=24, 16
        if(m_device) break;
    }   // for edt_types
    if(!m_device)
    {
        fprintf(stderr, "Couldn't initialise irrlicht device. Quitting.\n");
        exit(-1);
    }
    // Stores the new file system pointer.
    file_manager->setDevice(m_device);
    m_device->setWindowCaption(L"SuperTuxKart");
    m_scene_manager = m_device->getSceneManager();
    m_gui_env       = m_device->getGUIEnvironment();
    const std::string &font = file_manager->getFontFile("DomesticManners.xml");
    m_race_font     = m_gui_env->getFont(font.c_str());
}   // IrrDriver

// ----------------------------------------------------------------------------
IrrDriver::~IrrDriver()
{
    m_device->drop();
}   // ~IrrDriver


// ----------------------------------------------------------------------------
/** Loads an animated mesh and returns a pointer to it.
 *  \param filename File to load.
 */
scene::IAnimatedMesh *IrrDriver::getAnimatedMesh(const std::string &filename)
{
    return m_scene_manager->getMesh(filename.c_str());
}   // getAnimatedMesh

// ----------------------------------------------------------------------------
/** Loads a non-animated mesh and returns a pointer to it.
 *  \param filename  File to load.
 */
scene::IMesh *IrrDriver::getMesh(const std::string &filename)
{
    scene::IAnimatedMesh *m = m_scene_manager->getMesh(filename.c_str());
    if(!m) return NULL;
    return m->getMesh(0);
}   // getMesh

// ----------------------------------------------------------------------------
/** Converts the mesh into a water scene node.
 *  \param mesh The mesh which is converted into a water scene node.
 *  \param wave_height Height of the water waves.
 *  \param wave_speed Speed of the water waves.
 *  \param wave_length Lenght of a water wave.
 */
scene::ISceneNode* IrrDriver::addWaterNode(scene::IMesh *mesh,
                                           float wave_height,
                                           float wave_speed,
                                           float wave_length)
{
    return m_scene_manager->addWaterSurfaceSceneNode(mesh,
                                                     wave_height, wave_speed, 
                                                     wave_length);
}   // addWaterNode

// ----------------------------------------------------------------------------
/** Adds a mesh that will be optimised using an oct tree.
 *  \param mesh Mesh to add.
 */
scene::ISceneNode *IrrDriver::addOctTree(scene::IMesh *mesh)
{
    return m_scene_manager->addOctTreeSceneNode(mesh);
}   // addOctTree

// ----------------------------------------------------------------------------
/** Adds a particle scene node.
 */
scene::IParticleSystemSceneNode *IrrDriver::addParticleNode(bool default_emitter)
{
    return m_scene_manager->addParticleSystemSceneNode(default_emitter);
}   // addParticleNode

// ----------------------------------------------------------------------------
/** Adds a static mesh to scene. This should be used for smaller objects,
 *  since the node is not optimised.
 *  \param mesh The mesh to add.
 */
scene::ISceneNode *IrrDriver::addMesh(scene::IMesh *mesh)
{
    return m_scene_manager->addMeshSceneNode(mesh);
}   // addMesh

// ----------------------------------------------------------------------------
/** Creates a quad mesh buffer and adds it to the scene graph.
 */
scene::IMesh *IrrDriver::createQuadMesh(const video::SColor &color)
{
    scene::SMeshBuffer *buffer = new scene::SMeshBuffer();
    video::S3DVertex v;
    v.Pos = core::vector3df(0,0,0);

    // Add the vertices
    // ----------------
    buffer->Vertices.push_back(v);
    buffer->Vertices.push_back(v);
    buffer->Vertices.push_back(v);
    buffer->Vertices.push_back(v);

    // Define the indices for the triangles
    // ------------------------------------
    buffer->Indices.push_back(0);
    buffer->Indices.push_back(1);
    buffer->Indices.push_back(2);

    buffer->Indices.push_back(0);
    buffer->Indices.push_back(2);
    buffer->Indices.push_back(3);

    // Set the normals
    // ---------------
    core::vector3df n(1/sqrt(2.0f), 1/sqrt(2.0f), 0);
    buffer->Vertices[0].Normal = n;
    buffer->Vertices[1].Normal = n;
    buffer->Vertices[2].Normal = n;
    buffer->Vertices[3].Normal = n;
    video::SMaterial m;
    m.AmbientColor    = color;
    m.DiffuseColor    = color;
    m.EmissiveColor   = color;
    m.BackfaceCulling = false;
    buffer->Material  = m;
    SMesh *mesh       = new SMesh();
    mesh->addMeshBuffer(buffer);
    mesh->recalculateBoundingBox();
    buffer->drop();
    return mesh;
}   // createQuadMesh

// ----------------------------------------------------------------------------
/** Removes a scene node from the scene.
 *  \param node The scene node to remove.
 */
void IrrDriver::removeNode(scene::ISceneNode *node)
{
    node->remove();
}   // removeMesh

// ----------------------------------------------------------------------------
/** Removes a mesh from the mesh cache, freeing the memory.
 *  \param mesh The mesh to remove.
 */
void IrrDriver::removeMesh(scene::IMesh *mesh)
{
    m_scene_manager->getMeshCache()->removeMesh(mesh);
}   // removeMesh

// ----------------------------------------------------------------------------
/** Adds an animated mesh to the scene.
 *  \param mesh The animated mesh to add.
 */
scene::ISceneNode *IrrDriver::addAnimatedMesh(scene::IAnimatedMesh *mesh)
{
    return m_scene_manager->addAnimatedMeshSceneNode(mesh);
}   // addAnimatedMesh

// ----------------------------------------------------------------------------
/** Adds a sky dome. Documentation from irrlicht:
 *  A skydome is a large (half-) sphere with a panoramic texture on the inside
 *  and is drawn around the camera position.
 *  \param texture: Texture for the dome.
 *  \param horiRes: Number of vertices of a horizontal layer of the sphere.
 *  \param vertRes: Number of vertices of a vertical layer of the sphere.
 *  \param texturePercentage: How much of the height of the texture is used. 
 *         Should be between 0 and 1.
 *  \param spherePercentage: How much of the sphere is drawn. Value should be 
 *          between 0 and 2, where 1 is an exact half-sphere and 2 is a full 
 *          sphere.
 */
scene::ISceneNode *IrrDriver::addSkyDome(const std::string &texture_name, 
                                         int hori_res, int vert_res, 
                                         float texture_percent, 
                                         float sphere_percent)
{
    ITexture *texture = getTexture(texture_name);
    return m_scene_manager->addSkyDomeSceneNode(texture, hori_res, vert_res,
                                                texture_percent, 
                                                sphere_percent);
}   // addSkyDome

// ----------------------------------------------------------------------------
/** Adds a skybox using. Irrlicht documentation:
 *  A skybox is a big cube with 6 textures on it and is drawn around the camera 
 *  position.
 *  \param top: Texture for the top plane of the box.
 *  \param bottom: Texture for the bottom plane of the box.
 *  \param left: Texture for the left plane of the box.
 *  \param right: Texture for the right plane of the box.
 *  \param front: Texture for the front plane of the box.
 *  \param back: Texture for the back plane of the box.
 */
scene::ISceneNode *IrrDriver::addSkyBox(const std::vector<std::string> &texture_names)
{
    std::vector<video::ITexture*> t;
    for(unsigned int i=0; i<texture_names.size(); i++)
    {
        t.push_back(getTexture(texture_names[i]));
    }
    return m_scene_manager->addSkyBoxSceneNode(t[0], t[1], t[2], t[3], t[4], t[5]);
}   // addSkyBox

// ----------------------------------------------------------------------------
/** Adds a camera to the scene.
 */
scene::ICameraSceneNode *IrrDriver::addCamera()
 {
     return m_scene_manager->addCameraSceneNode();
 }   // addCamera

// ----------------------------------------------------------------------------
/** Loads a texture from a file and returns the texture object.
 *  \param filename File name of the texture to load.
 */
video::ITexture *IrrDriver::getTexture(const std::string &filename)
{
    return m_scene_manager->getVideoDriver()->getTexture(filename.c_str());
}   // getTexture

// ----------------------------------------------------------------------------
/** Sets the ambient light.
 *  \param light The colour of the light to set. 
 */
void IrrDriver::setAmbientLight(const video::SColor &light)
{
    m_scene_manager->setAmbientLight(light);
}   // setAmbientLight

// ----------------------------------------------------------------------------
/** Update, called once per frame.
 *  \param dt Time since last update
 */
void IrrDriver::update(float dt)
{
    m_device->getVideoDriver()->beginScene(true, true, video::SColor(255,100,101,140));
    m_scene_manager->drawAll();

    GUIEngine::render();
    
    m_device->getVideoDriver()->endScene();
}   // update

// ----------------------------------------------------------------------------
// Irrlicht Event handler.
bool IrrDriver::OnEvent(const irr::SEvent &event)
{
    switch (event.EventType) 
    {
    case   irr::EET_KEY_INPUT_EVENT: {
        printf("key input event\n");
               break;
           }  // keyboard
      case irr::EET_GUI_EVENT:         {
               return false;           }
      case irr::EET_MOUSE_INPUT_EVENT: {
               return false;           }
      case irr::EET_LOG_TEXT_EVENT:
          {
              // Ignore 'normal' messages
              if(event.LogEvent.Level>0)
              {
                  printf("Level %d: %s\n", 
                         event.LogEvent.Level,event.LogEvent.Text);
              }
              return true;
           }
      default:
          printf("Event: %d -> ",event.EventType);
          return false;
    }   // switch
    return false;
}   // OnEvent

// ----------------------------------------------------------------------------
