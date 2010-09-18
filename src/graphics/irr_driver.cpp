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

#include "config/user_config.hpp"
#include "graphics/camera.hpp"
#include "graphics/material_manager.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/modaldialog.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/screen.hpp"
#include "io/file_manager.hpp"
#include "items/item_manager.hpp"
#include "items/powerup_manager.hpp"
#include "items/attachment_manager.hpp"
#include "items/projectile_manager.hpp"
#include "karts/kart_properties_manager.hpp"
#include "main_loop.hpp"
#include "modes/world.hpp"
#include "states_screens/dialogs/confirm_resolution_dialog.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/constants.hpp"

using namespace irr::core;
using namespace irr::scene;
using namespace irr::video;

/** singleton */
IrrDriver *irr_driver = NULL;

const int MIN_SUPPORTED_HEIGHT = 480; //TODO: do some tests, 480 might be too small without a special menu
const int MIN_SUPPORTED_WIDTH  = 800;

// ----------------------------------------------------------------------------
IrrDriver::IrrDriver()
{
    m_res_switching = false;
    m_device = NULL;
    file_manager->dropFileSystem();
    initDevice();
}   // IrrDriver

// ----------------------------------------------------------------------------
IrrDriver::~IrrDriver()
{
    //std::cout << "^^^^^^^^ Dropping m_device ^^^^^^^^\n";
    assert(m_device != NULL);
    m_device->drop();
    m_device = NULL;
}   // ~IrrDriver
// ----------------------------------------------------------------------------

void IrrDriver::initDevice()
{
    static bool firstTime = true;

    // ---- the first time, get a list of available video modes
    if (firstTime)
    {
        //std::cout << "^^^^^^^^ Creating m_device (as NULL) ^^^^^^^^\n";
        m_device = createDevice(video::EDT_NULL);
        
        video::IVideoModeList* modes = m_device->getVideoModeList();
        const int count = modes->getVideoModeCount();
        //std::cout << "--------------\n  allowed modes  \n--------------\n";
        //std::cout << "Desktop depth : " << modes->getDesktopDepth()  << std::endl;
        //std::cout << "Desktop resolution : " << modes->getDesktopResolution().Width << "," << modes->getDesktopResolution().Height << std::endl;
        
        //std::cout << "Found " << count << " valid modes\n";
        for(int i=0; i<count; i++)
        {
            // only consider 32-bit resolutions for now
            if (modes->getVideoModeDepth(i) >= 24)
            {
                const int w = modes->getVideoModeResolution(i).Width;
                const int h = modes->getVideoModeResolution(i).Height;
                if (h < MIN_SUPPORTED_HEIGHT || w < MIN_SUPPORTED_WIDTH) continue;

                VideoMode mode;
                mode.width = w;
                mode.height = h;
                m_modes.push_back( mode );
            }
            
            //std::cout <<
            //"bits : " << modes->getVideoModeDepth(i) <<
            //" resolution=" << modes->getVideoModeResolution(i).Width <<
            //"x" << modes->getVideoModeResolution(i).Height << std::endl;
        }
        //std::cout << "^^^^^^^^ Closing m_device ^^^^^^^^\n";
        m_device->closeDevice();
        // In some circumstances it would happen that a WM_QUIT message
        // (apparently sent for this NULL device) is later received by
        // the actual window, causing it to immediately quit.
        // Following advise on the irrlicht forums I added the following
        // two calles - the first one didn't make a difference (but 
        // certainly can't hurt), but the second one apparenlty solved
        // the problem for now.
        m_device->clearSystemMessages();  
        m_device->run();

        firstTime = false;
    } // end if firstTime
    



    int numDrivers = 5;

    // Test if user has chosen a driver or if we should try all to find a woring
    // one.
    if( UserConfigParams::m_renderer != 0 )
    {
        numDrivers = 1;
    }

    // ---- open device
    // Try different drivers: start with opengl, then DirectX
    for(int driver_type=0; driver_type<numDrivers; driver_type++)
    {

        video::E_DRIVER_TYPE type;

        // Test if user has chosen a driver or if we should try all to find a
        // woring one.
        if( UserConfigParams::m_renderer != 0 )
        {
            // Get the correct type.
            type = getEngineDriverType( UserConfigParams::m_renderer );
        } else {
            // Get the correct type.
            type = getEngineDriverType( driver_type );
        }

        // Try 32 and, upon failure, 24 then 16 bit per pixels
        for(int bits=32; bits>15; bits -=8)
        {
            //std::cout << "^^^^^^^^ CREATING m_device ^^^^^^^^\n";
            m_device = createDevice(type,
                                    core::dimension2d<u32>(UserConfigParams::m_width,
                                                           UserConfigParams::m_height ),
                                    bits, //bits per pixel
                                    UserConfigParams::m_fullscreen,
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
    
    m_device->setResizable(false);
    
    // Stores the new file system pointer.
    file_manager->setDevice(m_device);
    m_device->setWindowCaption(L"SuperTuxKart");
    m_scene_manager = m_device->getSceneManager();
    
    // Force creation of mipmaps even if the mipmaps flag in a b3d file
    // does not set the 'enable mipmap' flag.
    m_scene_manager->getParameters()->setAttribute(scene::B3D_LOADER_IGNORE_MIPMAP_FLAG, true);
    m_device->getVideoDriver()->setTextureCreationFlag(
                                                    video::ETCF_CREATE_MIP_MAPS,
                                                    true);
    //m_device->getVideoDriver()->setTextureCreationFlag(
    //                                                   video::ETCF_OPTIMIZED_FOR_SPEED ,
    //                                                   true);
    //m_device->getVideoDriver()->setTextureCreationFlag(
    //                                                   video::ETCF_ALWAYS_16_BIT ,
    //                                                   true);
    m_gui_env       = m_device->getGUIEnvironment();
    m_video_driver  = m_device->getVideoDriver();
    //const std::string &font = file_manager->getFontFile("DomesticManners.xml");
    //m_race_font     = m_gui_env->getFont(font.c_str());
        

    video::SMaterial& material2D = m_video_driver->getMaterial2D();
    material2D.setFlag(video::EMF_ANTI_ALIASING, true);
    for (unsigned int n=0; n<MATERIAL_MAX_TEXTURES; n++)
    {
        material2D.TextureLayer[n].BilinearFilter = true;
        //material2D.TextureLayer[n].TextureWrap = ETC_CLAMP_TO_EDGE;
        material2D.TextureLayer[n].TextureWrapU = ETC_CLAMP_TO_EDGE;
        material2D.TextureLayer[n].TextureWrapV = ETC_CLAMP_TO_EDGE;
        
        material2D.TextureLayer[n].LODBias = 8;
    }
    material2D.AntiAliasing=video::EAAM_FULL_BASIC;
    //m_video_driver->enableMaterial2D();

    
    // set cursor visible by default (what's the default is not too clearly documented,
    // so let's decide ourselves...)
    m_device->getCursorControl()->setVisible(true);
    m_pointer_shown = true;
}


//-----------------------------------------------------------------------------
video::E_DRIVER_TYPE IrrDriver::getEngineDriverType( int index )
{
    video::E_DRIVER_TYPE type;
    std::string rendererName = "";

    // Choose the driver type.
    switch(index)
    {
        // TODO Change default renderer dependen on operating system?
        // Direct3D9 for Windows and OpenGL for Unix like systems?
        case 0:
            type = video::EDT_OPENGL;
            rendererName = "OpenGL";
            break;
        case 1:
            type = video::EDT_OPENGL;
            rendererName = "OpenGL";
            break;
        case 2:
            type = video::EDT_DIRECT3D9;
            rendererName = "Direct3D9";
            break;
        case 3:
            type = video::EDT_DIRECT3D8;
            rendererName = "Direct3D8";
            break;
        case 4:
            type = video::EDT_SOFTWARE;
            rendererName = "Software";
            break;
        case 5:
            type = video::EDT_BURNINGSVIDEO;
            rendererName = "Burning's Video Software";
            break;
        default:
            type = video::EDT_NULL;
            rendererName = "Null Device";
    }

    // Ouput which render will be tried.
    std::cout << "Trying " << rendererName << " rendering." << std::endl;

    return type;
}


//-----------------------------------------------------------------------------
void IrrDriver::showPointer()
{
    if (!m_pointer_shown)
    {
        m_pointer_shown = true;
        this->getDevice()->getCursorControl()->setVisible(true);
    }
}   // showPointer

//-----------------------------------------------------------------------------
void IrrDriver::hidePointer()
{
    if (m_pointer_shown)
    {
        m_pointer_shown = false;
        this->getDevice()->getCursorControl()->setVisible(false);
    }
}   // hidePointer

//-----------------------------------------------------------------------------

void IrrDriver::changeResolution(const int w, const int h, const bool fullscreen)
{
    // update user config values
    UserConfigParams::m_prev_width = UserConfigParams::m_width;
    UserConfigParams::m_prev_height = UserConfigParams::m_height;
    UserConfigParams::m_prev_fullscreen = UserConfigParams::m_fullscreen;
    
    UserConfigParams::m_width = w;
    UserConfigParams::m_height = h;
    UserConfigParams::m_fullscreen = fullscreen;

    doApplyResSettings();
    
    new ConfirmResolutionDialog();
}

//-----------------------------------------------------------------------------

void IrrDriver::doApplyResSettings()
{
    m_res_switching = true;
    
    // show black before resolution switch so we don't see OpenGL's buffer garbage during switch
    m_device->getVideoDriver()->beginScene(true, true, video::SColor(255,100,101,140));
    m_device->getVideoDriver()->draw2DRectangle( SColor(255, 0, 0, 0),
                                                core::rect<s32>(0, 0,
                                                                UserConfigParams::m_prev_width,
                                                                UserConfigParams::m_prev_height) );
    m_device->getVideoDriver()->endScene();

    // startScreen             -> removeTextures();
    attachment_manager      -> removeTextures();
    projectile_manager      -> removeTextures();
    item_manager            -> removeTextures();
    kart_properties_manager -> unloadAllKarts();
    powerup_manager         -> unloadPowerups();
    GUIEngine::clear();
    GUIEngine::cleanUp();
    
    //std::cout << "^^^^^^^^ Closing m_device ^^^^^^^^\n";
    m_device->closeDevice();
    //std::cout << "^^^^^^^^ Dropping m_device ^^^^^^^^\n";
    m_device->drop();
    m_device        = NULL;
    m_video_driver  = NULL;
    m_gui_env       = NULL;
    m_scene_manager = NULL;
    
    
    // ---- Reinit
    // FIXME: this load sequence is (mostly) duplicated from main.cpp!! That's just error prone
    // (we're sure to update main.cpp at some point and forget this one...)
    
    initDevice();
    
    // Re-init GUI engine
    GUIEngine::init(m_device, m_video_driver, StateManager::get());
        
    material_manager->reInit();
    GUIEngine::addLoadingIcon( irr_driver->getTexture(file_manager->getGUIDir() + "/options_video.png") );
    
    file_manager->pushTextureSearchPath(file_manager->getModelFile(""));
    const std::string materials_file = file_manager->getModelFile("materials.xml");
    if (materials_file != "")
    {
        material_manager->pushTempMaterial(materials_file);
    }
    
    powerup_manager         -> loadAllPowerups ();
    item_manager            -> loadDefaultItems();
    projectile_manager      -> loadData();
    GUIEngine::addLoadingIcon( irr_driver->getTexture(file_manager->getGUIDir() + "/gift.png") );
    
    if (materials_file != "")
    {
        material_manager->popTempMaterial();
    }
    
    file_manager->popTextureSearchPath();

    kart_properties_manager -> loadAllKarts();
    
    attachment_manager      -> loadModels();
    GUIEngine::addLoadingIcon( irr_driver->getTexture(file_manager->getGUIDir() + "/banana.png") );

    GUIEngine::reshowCurrentScreen();
    
}   // changeResolution

// ----------------------------------------------------------------------------

void IrrDriver::cancelResChange()
{
    UserConfigParams::m_width = UserConfigParams::m_prev_width;
    UserConfigParams::m_height = UserConfigParams::m_prev_height;
    UserConfigParams::m_fullscreen = UserConfigParams::m_prev_fullscreen;
    
    doApplyResSettings();
}   // cancelResChange

// ----------------------------------------------------------------------------
/** Prints statistics about rendering, e.g. number of drawn and culled 
 *  triangles etc. Note that printing this information will also slow
 *  down STK.
 */
void IrrDriver::printRenderStats()
{
    io::IAttributes * attr = m_scene_manager->getParameters();
    printf("[%ls], FPS:%3d Tri:%.03fm Cull %d/%d nodes (%d,%d,%d)\n",
	   m_video_driver->getName(),
	   m_video_driver->getFPS (),
	   (f32) m_video_driver->getPrimitiveCountDrawn( 0 ) * ( 1.f / 1000000.f ),
	   attr->getAttributeAsInt ( "culled" ),
	   attr->getAttributeAsInt ( "calls" ),
	   attr->getAttributeAsInt ( "drawn_solid" ),
	   attr->getAttributeAsInt ( "drawn_transparent" ),
	   attr->getAttributeAsInt ( "drawn_transparent_effect" )
	   );

}   // printRenderStats

// ----------------------------------------------------------------------------
/** Loads an animated mesh and returns a pointer to it.
 *  \param filename File to load.
 */
scene::IAnimatedMesh *IrrDriver::getAnimatedMesh(const std::string &filename)
{
    scene::IAnimatedMesh *m = m_scene_manager->getMesh(filename.c_str());
    if(m) setAllMaterialFlags(m);
    return m;
}   // getAnimatedMesh

// ----------------------------------------------------------------------------

/** Loads a non-animated mesh and returns a pointer to it.
 *  \param filename  File to load.
 */
scene::IMesh *IrrDriver::getMesh(const std::string &filename)
{
    scene::IAnimatedMesh *m = m_scene_manager->getMesh(filename.c_str());
    if(!m) return NULL;
    setAllMaterialFlags(m);
    
    return m->getMesh(0);
}   // getMesh

// ----------------------------------------------------------------------------
/** Sets the material flags in this mesh depending on the settings in
 *  material_manager.
 *  \param mesh  The mesh to change the settings in.
 */
void IrrDriver::setAllMaterialFlags(scene::IAnimatedMesh *mesh) const
{
    unsigned int n=mesh->getMeshBufferCount();
    for(unsigned int i=0; i<n; i++)
    {
        scene::IMeshBuffer *mb = mesh->getMeshBuffer(i);
        video::SMaterial &irr_material=mb->getMaterial();
        for(unsigned int j=0; j<video::MATERIAL_MAX_TEXTURES; j++)
        {
            video::ITexture* t=irr_material.getTexture(j);
            //if (!t) material_manager->setAllFlatMaterialFlags(mb);
            //else    material_manager->setAllMaterialFlags(t, mb);
            if(t) material_manager->setAllMaterialFlags(t, mb);
            
        }   // for j<MATERIAL_MAX_TEXTURES
        material_manager->setAllUntexturedMaterialFlags(mb);
    }  // for i<getMeshBufferCount()
}   // setAllMaterialFlags

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
scene::IMeshSceneNode *IrrDriver::addOctTree(scene::IMesh *mesh)
{
    return m_scene_manager->addOctreeSceneNode(mesh);
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
scene::IMeshSceneNode *IrrDriver::addMesh(scene::IMesh *mesh,
                                          scene::ISceneNode *parent)
{
    return m_scene_manager->addMeshSceneNode(mesh, parent);
}   // addMesh

// ----------------------------------------------------------------------------
/** Adds a billboard node to scene.
 */
scene::ISceneNode *IrrDriver::addBillboard(const core::dimension2d< f32 > size, video::ITexture *texture, scene::ISceneNode* parent)
{
    scene::IBillboardSceneNode* node = m_scene_manager->addBillboardSceneNode(parent, size);
    assert(node->getMaterialCount() > 0);
    node->setMaterialTexture(0, texture);
    return node;
}   // addMesh

// ----------------------------------------------------------------------------
/** Creates a quad mesh buffer and adds it to the scene graph. (FIXME: wrong docs? I don't think it does)
 */
scene::IMesh *IrrDriver::createQuadMesh(const video::SMaterial *material,
                                        bool create_one_quad)
{
    scene::SMeshBuffer *buffer = new scene::SMeshBuffer();
    if(create_one_quad)
    {
        video::S3DVertex v;
        v.Pos    = core::vector3df(0,0,0);
        v.Normal = core::vector3df(1/sqrt(2.0f), 1/sqrt(2.0f), 0);

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
    }
    if(material)
        buffer->Material = *material;
    SMesh *mesh       = new SMesh();
    mesh->addMeshBuffer(buffer);
    mesh->recalculateBoundingBox();
    buffer->drop();
    return mesh;
}   // createQuadMesh

/** Creates a quad mesh buffer
 */
scene::IMesh *IrrDriver::createTexturedQuadMesh(const video::SMaterial *material, 
                                                const double w, const double h)
{
    scene::SMeshBuffer *buffer = new scene::SMeshBuffer();
    
    const float w_2 = (float)w/2.0f;
    const float h_2 = (float)h/2.0f;

    video::S3DVertex v1;
    v1.Pos    = core::vector3df(-w_2,-h_2,0);
    v1.Normal = core::vector3df(0, 0, -1.0f);
    v1.TCoords = core::vector2d<f32>(1,1);
    
    video::S3DVertex v2;
    v2.Pos    = core::vector3df(w_2,-h_2,0);
    v2.Normal = core::vector3df(0, 0, -1.0f);
    v2.TCoords = core::vector2d<f32>(0,1);
    
    video::S3DVertex v3;
    v3.Pos    = core::vector3df(w_2,h_2,0);
    v3.Normal = core::vector3df(0, 0, -1.0f);
    v3.TCoords = core::vector2d<f32>(0,0);
    
    video::S3DVertex v4;
    v4.Pos    = core::vector3df(-w_2,h_2,0);
    v4.Normal = core::vector3df(0, 0, -1.0f);
    v4.TCoords = core::vector2d<f32>(1,0);
    
    
    // Add the vertices
    // ----------------
    buffer->Vertices.push_back(v1);
    buffer->Vertices.push_back(v2);
    buffer->Vertices.push_back(v3);
    buffer->Vertices.push_back(v4);
    
    // Define the indices for the triangles
    // ------------------------------------
    buffer->Indices.push_back(0);
    buffer->Indices.push_back(1);
    buffer->Indices.push_back(2);
    
    buffer->Indices.push_back(0);
    buffer->Indices.push_back(2);
    buffer->Indices.push_back(3);

    if (material) buffer->Material = *material;
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
/** Removes a texture from irrlicht's texture cache.
 *  \param t The texture to remove.
 */
void IrrDriver::removeTexture(video::ITexture *t)
{
    m_video_driver->removeTexture(t);
}   // removeTexture

// ----------------------------------------------------------------------------
/** Adds an animated mesh to the scene.
 *  \param mesh The animated mesh to add.
 */
scene::IAnimatedMeshSceneNode *IrrDriver::addAnimatedMesh(scene::IAnimatedMesh *mesh)
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
scene::ICameraSceneNode *IrrDriver::addCameraSceneNode()
 {
     return m_scene_manager->addCameraSceneNode();
 }   // addCameraSceneNode

// ----------------------------------------------------------------------------
/** Removes a camera. This can't be done with removeNode() since the camera
 *  can be marked as active, meaning a drop will not delete it. While this
 *  doesn't really cause a memory leak (the camera is removed the next time
 *  a camera is added), it's a bit cleaner and easier to check for memory
 *  leaks, since the scene root should now always be empty.
 */
void IrrDriver::removeCameraSceneNode(scene::ICameraSceneNode *camera)
{
    if(camera==m_scene_manager->getActiveCamera())
        m_scene_manager->setActiveCamera(NULL);    // basically causes a drop
    camera->remove();
}   // removeCameraSceneNode

// ----------------------------------------------------------------------------
/** Loads a texture from a file and returns the texture object.
 *  \param filename File name of the texture to load.
 */
video::ITexture *IrrDriver::getTexture(const std::string &filename)
{
    video::ITexture* out = m_scene_manager->getVideoDriver()->getTexture(filename.c_str());
    
#ifndef NDEBUG
    if (out == NULL)
    {
        printf("Texture '%s' not found.\n", filename.c_str());
        printf("Put a breakpoint at line %s:%i to debug!\n", __FILE__, __LINE__ );
    }
#endif
    
    return out;
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
/** Displays the FPS on the screen.
 */
void IrrDriver::displayFPS()
{
    // We will let pass some time to let things settle before trusting FPS counter
    // even if we also ignore fps = 1, which tends to happen in first checks
    const int NO_TRUST_COUNT = 25;
    static int no_trust = NO_TRUST_COUNT;

    if (no_trust)
    {
        no_trust--;
        return;
    }

    // Ask for current frames per second and last number of triangles processed (trimed to thousands)
    gui::IGUIFont* font = GUIEngine::getFont();
    const int fps       = m_device->getVideoDriver()->getFPS();
    const int kilotris  = (int)(m_device->getVideoDriver()->getPrimitiveCountDrawn(0)
                                * (1.f / 1000.f)                                     );

    // Min and max info tracking, per mode, so user can check game vs menus
    bool current_state     = StateManager::get()->getGameState() == GUIEngine::GAME;
    static bool prev_state = false;
    static int min         = 999; // Absurd values for start will print first time
    static int max         = 0;   // but no big issue, maybe even "invisible"

    // Reset limits if state changes
    if (prev_state != current_state)
    {
        min = 999;
        max = 0;
        no_trust = NO_TRUST_COUNT;
        prev_state = current_state;
    }

    if (min > fps && fps > 1) min = fps; // Start moments sometimes give useless 1
    if (max < fps) max = fps;

    static char buffer[32];
    sprintf(buffer, "FPS: %i/%i/%i - %i KTris", min, fps, max, kilotris);

    core::stringw fpsString = buffer;

    static video::SColor fpsColor = video::SColor(255, 255, 0, 0);
    font->draw( fpsString.c_str(), core::rect< s32 >(100,0,400,50), fpsColor, false );
}   // updateFPS

// ----------------------------------------------------------------------------
/** Update, called once per frame.
 *  \param dt Time since last update
 */
void IrrDriver::update(float dt)
{
    if (!m_device->run())
    {
        // FIXME: I have NO idea why, after performing resolution switch, the irrlicht device asks once to be deleted
        if (m_res_switching)    m_res_switching = false;
        else                    main_loop->abort();
    }
    else if (m_res_switching)
    {
        m_res_switching = false;
    }
    
    World *world = World::getWorld();
    
    const bool inRace = world!=NULL;
    
    // With bullet debug view we have to clear the back buffer, but
    // that's not necessary for non-debug
    bool back_buffer_clear = inRace && (world->getPhysics()->isDebug() || world->clearBackBuffer());
    
    if (world != NULL)
    {
        m_device->getVideoDriver()->beginScene(back_buffer_clear,
                                               true, world->getClearColor());
    }
    else
    {
        m_device->getVideoDriver()->beginScene(back_buffer_clear,
                                               true, video::SColor(255,100,101,140));
    }

    {   // Just to mark the begin/end scene block
        GUIEngine::GameState state = StateManager::get()->getGameState();
        if (state != GUIEngine::GAME)
        {
            // This code needs to go outside beginScene() / endScene() since
            // the model view widget will do off-screen rendering there
            const int updateAmount = GUIEngine::needsUpdate.size();
            for(int n=0; n<updateAmount; n++)
            {
                GUIEngine::needsUpdate[n].update(dt);
            }
        }

        if (inRace)
        {
            RaceGUIBase *rg = world->getRaceGUI();
            for(unsigned int i=0; i<world->getNumKarts(); i++)
            {
                Kart *kart=world->getKart(i);
                if(kart->getCamera()) 
                {
                    kart->getCamera()->activate();
                    m_scene_manager->drawAll();
                    // Note that drawAll must be called before rendering
                    // the bullet debug view, since otherwise the camera
                    // is not set up properly. This is only used for 
                    // the bullet debug view.
#ifdef DEBUG
                    World::getWorld()->getPhysics()->draw();
#endif
                }   // if kart->Camera
            }   // for i<world->getNumKarts()
            // To draw the race gui we set the viewport back to the full
            // screen. 
            m_video_driver->setViewPort(core::recti(0, 0,
                UserConfigParams::m_width,
                UserConfigParams::m_height));
            for(unsigned int i=0; i<world->getNumKarts(); i++)
            {
                Kart *kart = world->getKart(i);
                if(kart->getCamera())
                    rg->renderPlayerView(kart);
            }  // for i<getNumKarts
        }
        else
        {
            // render 3D stuff in cutscenes too
            m_scene_manager->drawAll();
        }
        
        // The render and displayFPS calls interfere with bullet debug
        // rendering, so they can not be called.
        //FIXME   if(!inRace || !UserConfigParams::m_bullet_debug)
        {
            // Either render the gui, or the global elements of the race gui.
            GUIEngine::render(dt);
        }

    }   // just to mark the begin/end scene block
    m_device->getVideoDriver()->endScene();
    // Enable this next print statement to get render information printed
    // E.g. number of triangles rendered, culled etc. The stats is only
    // printed while the race is running and not while the in-game menu
    // is shown. This way the output can be studied by just opening the
    // menu.
    //if(World::getWorld() && World::getWorld()->isRacePhase()) 
    //    printRenderStats();
}   // update

// ----------------------------------------------------------------------------
// Irrlicht Event handler.
bool IrrDriver::OnEvent(const irr::SEvent &event)
{
    //TODO: ideally we wouldn't use this object to STFU irrlicht's chatty debugging, we'd
    //      just create the EventHandler earlier so it can act upon it
    switch (event.EventType)
    {
        case irr::EET_LOG_TEXT_EVENT:
        {
            // Ignore 'normal' messages
            if (event.LogEvent.Level>0)
            {
                printf("Level %d: %s\n",
                       event.LogEvent.Level,event.LogEvent.Text);
            }
            return true;
        }
        default:
            return false;
    }   // switch
    
    return false;
}   // OnEvent

// ----------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark RTT
#endif

// ----------------------------------------------------------------------------
/** Begins a rendering to a texture.
 *  \param dimension The size of the texture. 
 *  \param name Name of the texture.
 *  \param dont_do_set_render_target This is a fix for the problem that 2d
 *         rendering doesn't work if setRenderTarget is called here, but 3d
 *         rendering doesn't work if it's not called here :(
 */
IrrDriver::RTTProvider::RTTProvider(const core::dimension2du &dimension, 
                                    const std::string &name)
{
    m_video_driver = irr_driver->getVideoDriver();
    
    m_render_target_texture = m_video_driver->addRenderTargetTexture(dimension, 
                                                                     name.c_str(),
                                                                     video::ECF_A8R8G8B8);
    m_video_driver->setRenderTarget(m_render_target_texture);
    
    m_rtt_main_node = NULL;
    m_camera = NULL;
    m_light = NULL;
}   // RTTProvider

// ----------------------------------------------------------------------------
IrrDriver::RTTProvider::~RTTProvider()
{
    tearDownRTTScene();
}   // ~RTTProvider

// ----------------------------------------------------------------------------
/** Sets up a given vector of meshes for render-to-texture. Ideal to embed a 3D
 *  object inside the GUI. If there are multiple meshes, the first mesh is considered
 *  to be the root, and all following meshes will have their locations relative to
 * the location of the first mesh.
 */
void IrrDriver::RTTProvider::setupRTTScene(ptr_vector<scene::IMesh, REF>& mesh, 
                                           std::vector<Vec3>& mesh_location,
                                           const std::vector<int>& model_frames)
{         
    if (model_frames[0] == -1)
    {
        scene::ISceneNode* node = irr_driver->getSceneManager()->addMeshSceneNode(mesh.get(0), NULL);
        node->setPosition( mesh_location[0].toIrrVector() );
        m_rtt_main_node = node;
    }
    else
    {
        scene::IAnimatedMeshSceneNode* node = irr_driver->getSceneManager()->addAnimatedMeshSceneNode((IAnimatedMesh*)mesh.get(0), NULL);
        node->setPosition( mesh_location[0].toIrrVector() );
        node->setFrameLoop(model_frames[0], model_frames[0]);
        node->setAnimationSpeed(0);
        
        m_rtt_main_node = node;
        //std::cout << "(((( set frame " << model_frames[0] << " ))))\n";
    }
    
    assert(m_rtt_main_node != NULL);
    assert(mesh.size() == (int)mesh_location.size());
    assert(mesh.size() == (int)model_frames.size());

    const int mesh_amount = mesh.size();
    for (int n=1; n<mesh_amount; n++)
    {
        if (model_frames[n] == -1)
        {
            scene::ISceneNode* node = irr_driver->getSceneManager()->addMeshSceneNode(mesh.get(n), m_rtt_main_node);
            node->setPosition( mesh_location[n].toIrrVector() );
            node->updateAbsolutePosition();
        }
        else
        {
            scene::IAnimatedMeshSceneNode* node = irr_driver->getSceneManager()->addAnimatedMeshSceneNode((IAnimatedMesh*)mesh.get(n), m_rtt_main_node);
            node->setPosition( mesh_location[n].toIrrVector() );
            node->setFrameLoop(model_frames[n], model_frames[n]);
            node->setAnimationSpeed(0);
            node->updateAbsolutePosition();
            //std::cout << "(((( set frame " << model_frames[n] << " ))))\n";
        }
    }
    
    m_rtt_main_node->setScale( core::vector3df(35.0f, 35.0f, 35.0f) );
    
    //vector3d< f32 > modelsize = mesh->getBoundingBox().getExtent();
    //std::cout << "box size " << modelsize.X*50.0 << ", " << modelsize.Y*50.0 << ", " << modelsize.Z*50.0 << std::endl;
    
    irr_driver->getSceneManager()->setAmbientLight(video::SColor(255, 120, 120, 120));
    
    const core::vector3df &sun_pos = core::vector3df( 0, 200, 100.0f );
    m_light = irr_driver->getSceneManager()->addLightSceneNode(NULL, sun_pos, video::SColorf(1.0f,1.0f,1.0f), 10000.0f /* radius */);
    m_light->getLightData().DiffuseColor = irr::video::SColorf(0.5f, 0.5f, 0.5f, 0.5f);
    m_light->getLightData().SpecularColor = irr::video::SColorf(1.0f, 1.0f, 1.0f, 1.0f);
    
    m_rtt_main_node->setMaterialFlag(EMF_GOURAUD_SHADING , true);
    m_rtt_main_node->setMaterialFlag(EMF_LIGHTING, true);
    
    const int materials = m_rtt_main_node->getMaterialCount();
    for (int n=0; n<materials; n++)
    {
        m_rtt_main_node->getMaterial(n).setFlag(EMF_LIGHTING, true);
        
        m_rtt_main_node->getMaterial(n).Shininess = 100.0f; // set size of specular highlights
        m_rtt_main_node->getMaterial(n).SpecularColor.set(255,50,50,50); 
        m_rtt_main_node->getMaterial(n).DiffuseColor.set(255,150,150,150);
        
        m_rtt_main_node->getMaterial(n).setFlag(EMF_GOURAUD_SHADING , true);
    }
    
    m_camera =  irr_driver->getSceneManager()->addCameraSceneNode();
    
    m_camera->setPosition( core::vector3df(0.0, 20.0f, 70.0f) );
    m_camera->setUpVector( core::vector3df(0.0, 1.0, 0.0) );
    m_camera->setTarget( core::vector3df(0, 10, 0.0f) );
    m_camera->setFOV( DEGREE_TO_RAD*50.0f );
    m_camera->updateAbsolutePosition();
    
    // Detach the note from the scene so we can render it independently
    m_rtt_main_node->setVisible(false);
    m_light->setVisible(false);
}   // setupRTTScene

// ----------------------------------------------------------------------------
void IrrDriver::RTTProvider::tearDownRTTScene()
{
    //if (m_rtt_main_node != NULL) m_rtt_main_node->drop();
    if (m_rtt_main_node != NULL) m_rtt_main_node->remove();
    if (m_light != NULL) m_light->remove();
    if (m_camera != NULL) m_camera->remove();
    
    m_rtt_main_node = NULL;
    m_camera = NULL;
    m_light = NULL;
}   // tearDownRTTScene

// ----------------------------------------------------------------------------
/**
 * Performs the actual render-to-texture
 *  \param target The texture to render the meshes to.
 *  \param angle (Optional) heading for all meshes.
 */
ITexture* IrrDriver::RTTProvider::renderToTexture(float angle,
                                                  bool is_2d_render)
{
    // Rendering a 2d only model (using direct opengl rendering)
    // does not work if setRenderTarget is called here again.
    // And rendering 3d only works if it is called here :(
    if(!is_2d_render)
        m_video_driver->setRenderTarget(m_render_target_texture);  
    
    if (angle != -1 && m_rtt_main_node != NULL) m_rtt_main_node->setRotation( core::vector3df(0, angle, 0) );
    
    if (m_rtt_main_node == NULL)
    {
        irr_driver->getSceneManager()->drawAll();
    }
    else
    {
        m_rtt_main_node->setVisible(true);
        m_light->setVisible(true);
        irr_driver->getSceneManager()->drawAll();
        m_rtt_main_node->setVisible(false);
        m_light->setVisible(false);
    }
    
    m_video_driver->setRenderTarget(0, false, false);
    return m_render_target_texture;
}
