//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Joerg Henrichs
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
#include "font/font_manager.hpp"
#include "graphics/callbacks.hpp"
#include "graphics/camera.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/2dutils.hpp"
#include "graphics/graphics_restrictions.hpp"
#include "graphics/light.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/particle_kind_manager.hpp"
#include "graphics/per_camera_node.hpp"
#include "graphics/post_processing.hpp"
#include "graphics/referee.hpp"
#include "graphics/render_info.hpp"
#include "graphics/shaders.hpp"
#include "graphics/shadow_matrices.hpp"
#include "graphics/stk_animated_mesh.hpp"
#include "graphics/stk_billboard.hpp"
#include "graphics/stk_mesh_scene_node.hpp"
#include "graphics/stk_scene_manager.hpp"
#include "graphics/sun.hpp"
#include "graphics/rtts.hpp"
#include "graphics/texture_manager.hpp"
#include "graphics/water.hpp"
#include "graphics/wind.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/message_queue.hpp"
#include "guiengine/modaldialog.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/screen.hpp"
#include "io/file_manager.hpp"
#include "items/item_manager.hpp"
#include "items/powerup_manager.hpp"
#include "items/attachment_manager.hpp"
#include "items/projectile_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/kart_properties_manager.hpp"
#include "main_loop.hpp"
#include "modes/profile_world.hpp"
#include "modes/world.hpp"
#include "physics/physics.hpp"
#include "scriptengine/property_animator.hpp"
#include "states_screens/dialogs/confirm_resolution_dialog.hpp"
#include "states_screens/state_manager.hpp"
#include "tracks/track_manager.hpp"
#include "utils/constants.hpp"
#include "utils/log.hpp"
#include "utils/profiler.hpp"
#include "utils/vs.hpp"

#include <irrlicht.h>
#include "../lib/irrlicht/source/Irrlicht/CSkinnedMesh.h"

/* Build-time check that the Irrlicht we're building against works for us.
 * Should help prevent distros building against an incompatible library.
 */

#if (IRRLICHT_VERSION_MAJOR < 1 || IRRLICHT_VERSION_MINOR < 7 || \
    _IRR_MATERIAL_MAX_TEXTURES_ < 8 || \
    (!defined(_IRR_COMPILE_WITH_OPENGL_) && \
     !defined(_IRR_COMPILE_WITH_OGLES2_)) || \
    !defined(_IRR_COMPILE_WITH_B3D_LOADER_))
#error "Building against an incompatible Irrlicht. Distros, \
please use the included version."
#endif

using namespace irr;

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#if defined(__linux__) && !defined(ANDROID)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

/** singleton */
IrrDriver *irr_driver = NULL;


GPUTimer          m_perf_query[Q_LAST];

const int MIN_SUPPORTED_HEIGHT = 768;
const int MIN_SUPPORTED_WIDTH  = 1024;
const bool ALLOW_1280_X_720    = true;

// ----------------------------------------------------------------------------
/** The constructor creates the irrlicht device. It first creates a NULL
 *  device. This is necessary to handle the Chicken/egg problem with irrlicht:
 *  access to the file system is given from the device, but we can't create the
 *  device before reading the user_config file (for resolution, fullscreen).
 *  So we create a dummy device here to begin with, which is then later (once
 *  the real device exists) changed in initDevice().
 */
IrrDriver::IrrDriver()
{
    m_shadow_matrices     = NULL;
    m_resolution_changing = RES_CHANGE_NONE;
    m_phase               = SOLID_NORMAL_AND_DEPTH_PASS;
    m_device              = createDevice(video::EDT_NULL,
                                         irr::core::dimension2d<u32>(640, 480),
                                         /*bits*/16U, /**fullscreen*/ false,
                                         /*stencilBuffer*/ false,
                                         /*vsync*/false,
                                         /*event receiver*/ NULL,
                                         file_manager->getFileSystem());
    m_request_screenshot = false;
    m_rtts                = NULL;
    m_post_processing     = NULL;
    m_wind                = new Wind();
    m_skybox              = NULL;
    m_spherical_harmonics = NULL;

    m_mipviz = m_wireframe = m_normals = m_ssaoviz = false;
    m_lightviz = m_shadowviz = m_distortviz = m_rsm = m_rh = m_gi = false;
    m_boundingboxesviz = false;
    m_last_light_bucket_distance = 0;
    memset(m_object_count, 0, sizeof(m_object_count));
    memset(m_poly_count, 0, sizeof(m_poly_count));
}   // IrrDriver

// ----------------------------------------------------------------------------
/** Destructor - removes the irrlicht device.
 */
IrrDriver::~IrrDriver()
{
    // Note that we can not simply delete m_post_processing here:
    // m_post_processing uses a material that has a reference to
    // m_post_processing (for a callback). So when the material is
    // removed it will try to drop the ref count of its callback object,
    // which is m_post_processing, and which was already deleted. So
    // instead we just decrease the ref count here. When the material
    // is deleted, it will trigger the actual deletion of
    // PostProcessing when decreasing the refcount of its callback object.
    if(m_post_processing)
    {
        // check if we createad the OpenGL device by calling initDevice()
        m_post_processing->drop();
    }
    assert(m_device != NULL);

    m_device->drop();
    m_device = NULL;
    m_modes.clear();

    delete m_shadow_matrices;
    m_shadow_matrices = NULL;
    if (CVS->isGLSL())
    {
        Shaders::destroy();
    }
    delete m_wind;
    delete m_spherical_harmonics;
}   // ~IrrDriver

// ----------------------------------------------------------------------------
/** Called before a race is started, after all cameras are set up.
 */
void IrrDriver::reset()
{
    if (CVS->isGLSL()) m_post_processing->reset();
}   // reset

void IrrDriver::setPhase(STKRenderingPass p)
{
    m_phase = p;
}

STKRenderingPass IrrDriver::getPhase() const
{
  return m_phase;
}

void IrrDriver::IncreaseObjectCount()
{
    m_object_count[m_phase]++;
}

void IrrDriver::IncreasePolyCount(unsigned Polys)
{
    m_poly_count[m_phase] += Polys;
}

core::array<video::IRenderTarget> &IrrDriver::getMainSetup()
{
  return m_mrt;
}

GPUTimer &IrrDriver::getGPUTimer(unsigned i)
{
    return m_perf_query[i];
}

// ----------------------------------------------------------------------------
void IrrDriver::computeMatrixesAndCameras(scene::ICameraSceneNode *const camnode,
                                          size_t width, size_t height)
{
    float w = width * UserConfigParams::m_scale_rtts_factor;
    float h = height * UserConfigParams::m_scale_rtts_factor;
    m_current_screen_size = core::vector2df(w, h);
    m_shadow_matrices->computeMatrixesAndCameras(camnode, int(w), int(h));
}   // computeMatrixesAndCameras

// ----------------------------------------------------------------------------

#if defined(__linux__) && !defined(ANDROID)
/*
Returns the parent window of "window" (i.e. the ancestor of window
that is a direct child of the root, or window itself if it is a direct child).
If window is the root window, returns window.
*/
Window get_toplevel_parent(Display* display, Window window)
{
     Window parent;
     Window root;
     Window * children;
     unsigned int num_children;

     while (true)
     {
         if (0 == XQueryTree(display, window, &root,
                   &parent, &children, &num_children))
         {
             Log::fatal("irr_driver", "XQueryTree error\n");
         }
         if (children) { //must test for null
             XFree(children);
         }
         if (window == root || parent == root) {
             return window;
         }
         else {
             window = parent;
         }
     }
}
#endif

// ----------------------------------------------------------------------------
/** If the position of the window should be remembered, store it in the config
 *  file.
 *  \post The user config file must still be saved!
 */
void IrrDriver::updateConfigIfRelevant()
{
        if (!UserConfigParams::m_fullscreen &&
             UserConfigParams::m_remember_window_location)
    {
#ifdef WIN32
        const video::SExposedVideoData& videoData = m_device->getVideoDriver()
                                                     ->getExposedVideoData();
        // this should work even if using DirectX in theory because the HWnd is
        // always third pointer in the struct, no matter which union is used
        HWND window = (HWND)videoData.OpenGLWin32.HWnd;
        WINDOWPLACEMENT placement;
        placement.length = sizeof(WINDOWPLACEMENT);
        if (GetWindowPlacement(window, &placement))
        {
            int x = (int)placement.rcNormalPosition.left;
            int y = (int)placement.rcNormalPosition.top;
            // If the windows position is saved, it must be a non-negative
            // number. So if the window is partly off screen, move it to the
            // corresponding edge.
            if(x<0) x = 0;
            if(y<0) y = 0;
            Log::verbose("irr_driver",
                       "Retrieved window location for config : %i %i\n", x, y);

            if (UserConfigParams::m_window_x != x || UserConfigParams::m_window_y != y)
            {
                UserConfigParams::m_window_x = x;
                UserConfigParams::m_window_y = y;
            }
        }
        else
        {
            Log::warn("irr_driver", "Could not retrieve window location\n");
        }
#elif defined(__linux__) && !defined(ANDROID)
        const video::SExposedVideoData& videoData =
            m_device->getVideoDriver()->getExposedVideoData();
        Display* display = (Display*)videoData.OpenGLLinux.X11Display;
        XWindowAttributes xwa;
        XGetWindowAttributes(display, get_toplevel_parent(display,
                                       videoData.OpenGLLinux.X11Window), &xwa);
        int wx = xwa.x;
        int wy = xwa.y;
        Log::verbose("irr_driver",
                     "Retrieved window location for config : %i %i\n", wx, wy);


        if (UserConfigParams::m_window_x != wx || UserConfigParams::m_window_y != wy)
        {
            UserConfigParams::m_window_x = wx;
            UserConfigParams::m_window_y = wy;
        }
#endif
    }
}   // updateConfigIfRelevant

// ----------------------------------------------------------------------------
/** Gets a list of supported video modes from the irrlicht device. This data
 *  is stored in m_modes.
 */
void IrrDriver::createListOfVideoModes()
{
    // Note that this is actually reported by valgrind as a leak, but it is
    // a leak in irrlicht: this list is dynamically created the first time
    // it is used, but then not cleaned on exit.
    video::IVideoModeList* modes = m_device->getVideoModeList();
    const int count = modes->getVideoModeCount();

    for(int i=0; i<count; i++)
    {
        // only consider 32-bit resolutions for now
        if (modes->getVideoModeDepth(i) >= 24)
        {
            const int w = modes->getVideoModeResolution(i).Width;
            const int h = modes->getVideoModeResolution(i).Height;
            if ((h < MIN_SUPPORTED_HEIGHT || w < MIN_SUPPORTED_WIDTH) &&
                (!(h==600 && w==800 && UserConfigParams::m_artist_debug_mode) &&
                (!(h==720 && w==1280 && ALLOW_1280_X_720 == true))))
                continue;

            VideoMode mode(w, h);
            m_modes.push_back( mode );
        }   // if depth >=24
    }   // for i < video modes count
}   // createListOfVideoModes

// ----------------------------------------------------------------------------
/** This creates the actualy OpenGL device. This is called
 */
void IrrDriver::initDevice()
{
    SIrrlichtCreationParameters params;

    // If --no-graphics option was used, the null device can still be used.
    if (!ProfileWorld::isNoGraphics())
    {
        // This code is only executed once. No need to reload the video
        // modes every time the resolution changes.
        if(m_modes.size()==0)
        {
            createListOfVideoModes();
            // The debug name is only set if irrlicht is compiled in debug
            // mode. So we use this to print a warning to the user.
            if(m_device->getDebugName())
            {
                Log::warn("irr_driver",
                          "!!!!! Performance warning: Irrlicht compiled with "
                          "debug mode.!!!!!\n");
                Log::warn("irr_driver",
                          "!!!!! This can have a significant performance "
                          "impact         !!!!!\n");
            }

        } // end if firstTime

        video::IVideoModeList* modes = m_device->getVideoModeList();
        const core::dimension2d<u32> ssize = modes->getDesktopResolution();

        if (ssize.Width < 1 || ssize.Height < 1)
        {
            Log::warn("irr_driver", "Unknown desktop resolution.");
        }
        else if (UserConfigParams::m_width > (int)ssize.Width ||
                 UserConfigParams::m_height > (int)ssize.Height)
        {
            Log::warn("irr_driver", "The window size specified in "
                      "user config is larger than your screen!");
            UserConfigParams::m_width = (int)ssize.Width;
            UserConfigParams::m_height = (int)ssize.Height;
        }

        if (UserConfigParams::m_fullscreen)
        {
            if (modes->getVideoModeCount() > 0)
            {
                core::dimension2d<u32> res = core::dimension2du(
                                                    UserConfigParams::m_width,
                                                    UserConfigParams::m_height);
                res = modes->getVideoModeResolution(res, res);

                UserConfigParams::m_width = res.Width;
                UserConfigParams::m_height = res.Height;
            }
            else
            {
                Log::warn("irr_driver", "Cannot get information about "
                          "resolutions. Disable fullscreen.");
                UserConfigParams::m_fullscreen = false;
            }
        }

        if (UserConfigParams::m_width < 1 || UserConfigParams::m_height < 1)
        {
            Log::warn("irr_driver", "Invalid window size. "
                         "Try to use the default one.");
            UserConfigParams::m_width = MIN_SUPPORTED_WIDTH;
            UserConfigParams::m_height = MIN_SUPPORTED_HEIGHT;
        }

        m_device->closeDevice();
        m_video_driver  = NULL;
        m_gui_env       = NULL;
        m_scene_manager = NULL;
        // In some circumstances it would happen that a WM_QUIT message
        // (apparently sent for this NULL device) is later received by
        // the actual window, causing it to immediately quit.
        // Following advise on the irrlicht forums I added the following
        // two calles - the first one didn't make a difference (but
        // certainly can't hurt), but the second one apparenlty solved
        // the problem for now.
        m_device->clearSystemMessages();
        m_device->run();
        m_device->drop();
        m_device  = NULL;

        params.ForceLegacyDevice = (UserConfigParams::m_force_legacy_device ||
            UserConfigParams::m_gamepad_visualisation);

        // Try 32 and, upon failure, 24 then 16 bit per pixels
        for (int bits=32; bits>15; bits -=8)
        {
            if(UserConfigParams::logMisc())
                Log::verbose("irr_driver", "Trying to create device with "
                             "%i bits\n", bits);

#if defined(USE_GLES2)
            params.DriverType    = video::EDT_OGLES2;
#else
            params.DriverType    = video::EDT_OPENGL;
#endif
            params.Stencilbuffer = false;
            params.Bits          = bits;
            params.EventReceiver = this;
            params.Fullscreen    = UserConfigParams::m_fullscreen;
            params.Vsync         = UserConfigParams::m_vsync;
            params.FileSystem    = file_manager->getFileSystem();
            params.WindowSize    =
                core::dimension2du(UserConfigParams::m_width,
                                   UserConfigParams::m_height);
            params.HandleSRGB    = true;
            params.ShadersPath   = (file_manager->getShadersDir() +
                                                           "irrlicht/").c_str();

            /*
            switch ((int)UserConfigParams::m_antialiasing)
            {
            case 0:
                break;
            case 1:
                params.AntiAlias = 2;
                break;
            case 2:
                params.AntiAlias = 4;
                break;
            case 3:
                params.AntiAlias = 8;
                break;
            default:
                Log::error("irr_driver",
                           "[IrrDriver] WARNING: Invalid value for "
                           "anti-alias setting : %i\n",
                           (int)UserConfigParams::m_antialiasing);
            }
            */
            m_device = createDeviceEx(params);

            if(m_device)
                break;
        }   // for bits=32, 24, 16


        // if still no device, try with a default screen size, maybe
        // size is the problem
        if(!m_device)
        {
            UserConfigParams::m_width  = MIN_SUPPORTED_WIDTH;
            UserConfigParams::m_height = MIN_SUPPORTED_HEIGHT;
#if defined(USE_GLES2)
            m_device = createDevice(video::EDT_OGLES2,
#else
            m_device = createDevice(video::EDT_OPENGL,
#endif
                        core::dimension2du(UserConfigParams::m_width,
                                           UserConfigParams::m_height ),
                                    32, //bits per pixel
                                    UserConfigParams::m_fullscreen,
                                    false,  // stencil buffers
                                    false,  // vsync
                                    this,   // event receiver
                                    file_manager->getFileSystem()
                                    );
            if (m_device)
            {
                Log::verbose("irr_driver", "An invalid resolution was set in "
                             "the config file, reverting to saner values\n");
            }
        }
    }

    if(!m_device)
    {
        Log::fatal("irr_driver", "Couldn't initialise irrlicht device. Quitting.\n");
    }

    CVS->init();

    bool recreate_device = false;

    // Some drivers are able to create OpenGL 3.1 context, but shader-based
    // pipeline doesn't work for them. For example some radeon drivers
    // support only GLSL 1.3 and it causes STK to crash. We should force to use
    // fixed pipeline in this case.
    if (!ProfileWorld::isNoGraphics() &&
        GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_FORCE_LEGACY_DEVICE))
    {
        Log::warn("irr_driver", "Driver doesn't support shader-based pipeline. "
                                "Re-creating device to workaround the issue.");

        params.ForceLegacyDevice = true;
        recreate_device = true;
    }

    // This is the ugly hack for intel driver on linux, which doesn't
    // use sRGB-capable visual, even if we request it. This causes
    // the screen to be darker than expected. It affects mesa 10.6 and newer.
    // Though we are able to force to use the proper format on mesa side by
    // setting WithAlphaChannel parameter.
    else if (CVS->needsSRGBCapableVisualWorkaround())
    {
        Log::warn("irr_driver", "Created visual is not sRGB-capable. "
                                "Re-creating device to workaround the issue.");

        params.WithAlphaChannel = true;
        recreate_device = true;
    }

    if (!ProfileWorld::isNoGraphics() && recreate_device)
    {
        m_device->closeDevice();
        m_device->clearSystemMessages();
        m_device->run();
        m_device->drop();

        m_device = createDeviceEx(params);

        if(!m_device)
        {
            Log::fatal("irr_driver", "Couldn't initialise irrlicht device. Quitting.\n");
        }

        CVS->init();
    }

    m_scene_manager = m_device->getSceneManager();
    m_gui_env       = m_device->getGUIEnvironment();
    m_video_driver  = m_device->getVideoDriver();
    m_sync = 0;

    m_actual_screen_size = m_video_driver->getCurrentRenderTargetSize();

    m_spherical_harmonics = new SphericalHarmonics(m_scene_manager->getAmbientLight().toSColor());

    if (UserConfigParams::m_shadows_resolution != 0 &&
        (UserConfigParams::m_shadows_resolution < 512 ||
         UserConfigParams::m_shadows_resolution > 2048))
    {
        Log::warn("irr_driver",
               "Invalid value for UserConfigParams::m_shadows_resolution : %i",
            (int)UserConfigParams::m_shadows_resolution);
        UserConfigParams::m_shadows_resolution = 0;
    }

    // This remaps the window, so it has to be done before the clear to avoid flicker
    m_device->setResizable(false);

    // Immediate clear to black for a nicer user loading experience
    m_video_driver->beginScene(/*backBuffer clear*/true, /* Z */ false);
    m_video_driver->endScene();

    if (CVS->isGLSL())
    {
        Log::info("irr_driver", "GLSL supported.");
    }

/*    if (!supportGeometryShader())
    {
        // these options require geometry shaders
        UserConfigParams::m_shadows = 0;
        UserConfigParams::m_gi = false;
    }*/

    // m_glsl might be reset in rtt if an error occurs.
    if (CVS->isGLSL())
    {
        Shaders::init();

        m_mrt.clear();
        m_mrt.reallocate(2);

    }
    else
    {
        Log::warn("irr_driver", "Using the fixed pipeline (old GPU, or "
                                "shaders disabled in options)");
    }

    // Only change video driver settings if we are showing graphics
    if (!ProfileWorld::isNoGraphics())
    {
#if defined(__linux__) && !defined(ANDROID)
        // Set class hints on Linux, used by Window Managers.
        const video::SExposedVideoData& videoData = m_video_driver
                                                ->getExposedVideoData();
        XClassHint* classhint = XAllocClassHint();
        classhint->res_name = (char*)"SuperTuxKart";
        classhint->res_class = (char*)"SuperTuxKart";
        XSetClassHint((Display*)videoData.OpenGLLinux.X11Display,
                           videoData.OpenGLLinux.X11Window,
                           classhint);
        XFree(classhint);
#endif
        m_device->setWindowCaption(L"SuperTuxKart");
        m_device->getVideoDriver()
            ->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, true);
        m_device->getVideoDriver()
            ->setTextureCreationFlag(video::ETCF_OPTIMIZED_FOR_QUALITY, true);

        // Force creation of mipmaps even if the mipmaps flag in a b3d file
        // does not set the 'enable mipmap' flag.
        m_scene_manager->getParameters()
            ->setAttribute(scene::B3D_LOADER_IGNORE_MIPMAP_FLAG, true);

        // Set window to remembered position
        if (  !UserConfigParams::m_fullscreen
            && UserConfigParams::m_remember_window_location
            && UserConfigParams::m_window_x >= 0
            && UserConfigParams::m_window_y >= 0            )
        {
            moveWindow(UserConfigParams::m_window_x,
                       UserConfigParams::m_window_y);
        } // If reinstating window location
    } // If showing graphics

    // Initialize material2D
    video::SMaterial& material2D = m_video_driver->getMaterial2D();
    material2D.setFlag(video::EMF_ANTI_ALIASING, true);
    for (unsigned int n=0; n<video::MATERIAL_MAX_TEXTURES; n++)
    {
        material2D.TextureLayer[n].BilinearFilter = false;
        material2D.TextureLayer[n].TrilinearFilter = true;
        material2D.TextureLayer[n].TextureWrapU = video::ETC_CLAMP_TO_EDGE;
        material2D.TextureLayer[n].TextureWrapV = video::ETC_CLAMP_TO_EDGE;

        //material2D.TextureLayer[n].LODBias = 16;
        material2D.UseMipMaps = true;
    }
    material2D.AntiAliasing=video::EAAM_FULL_BASIC;
    //m_video_driver->enableMaterial2D();

    // Initialize post-processing if supported
    m_post_processing = new PostProcessing(m_video_driver);

    // set cursor visible by default (what's the default is not too clearly documented,
    // so let's decide ourselves...)
    m_device->getCursorControl()->setVisible(true);
    m_pointer_shown = true;
    m_shadow_matrices = new ShadowMatrices();
}   // initDevice

// ----------------------------------------------------------------------------
void IrrDriver::setMaxTextureSize()
{
    if( (UserConfigParams::m_high_definition_textures & 0x01) == 0)
    {
        io::IAttributes &att = m_video_driver->getNonConstDriverAttributes();
        att.setAttribute("MAX_TEXTURE_SIZE", core::dimension2du(
                                        UserConfigParams::m_max_texture_size,
                                        UserConfigParams::m_max_texture_size));
    }
}   // setMaxTextureSize

// ----------------------------------------------------------------------------
void IrrDriver::cleanSunInterposer()
{
    delete m_sun_interposer;
}   // cleanSunInterposer

// ----------------------------------------------------------------------------
void IrrDriver::createSunInterposer()
{
    scene::IMesh * sphere = m_scene_manager->getGeometryCreator()
                                           ->createSphereMesh(1, 16, 16);
    for (unsigned i = 0; i < sphere->getMeshBufferCount(); ++i)
    {
        scene::IMeshBuffer *mb = sphere->getMeshBuffer(i);
        if (!mb)
            continue;
        mb->getMaterial().setTexture(0,
                        getUnicolorTexture(video::SColor(255, 255, 255, 255)));
        mb->getMaterial().setTexture(1,
                                getUnicolorTexture(video::SColor(0, 0, 0, 0)));
        mb->getMaterial().setTexture(7,
                                getUnicolorTexture(video::SColor(0, 0, 0, 0)));
    }
    m_sun_interposer = new STKMeshSceneNode(sphere,
                                            m_scene_manager->getRootSceneNode(),
                                            NULL, -1, "sun_interposer");

    m_sun_interposer->grab();
    m_sun_interposer->setParent(NULL);
    m_sun_interposer->setScale(core::vector3df(20));

    m_sun_interposer->getMaterial(0).Lighting = false;
    m_sun_interposer->getMaterial(0).ColorMask = video::ECP_NONE;
    m_sun_interposer->getMaterial(0).ZWriteEnable = false;
    m_sun_interposer->getMaterial(0).MaterialType = Shaders::getShader(ES_OBJECTPASS);

    sphere->drop();
}

//-----------------------------------------------------------------------------
void IrrDriver::getOpenGLData(std::string *vendor, std::string *renderer,
                              std::string *version)
{
    *vendor   = (char*)glGetString(GL_VENDOR  );
    *renderer = (char*)glGetString(GL_RENDERER);
    *version  = (char*)glGetString(GL_VERSION );
}   // getOpenGLData

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
    // always visible in artist debug mode, to be able to use the context menu
    if (UserConfigParams::m_artist_debug_mode)
    {
        this->getDevice()->getCursorControl()->setVisible(true);
        return;
    }

    if (m_pointer_shown)
    {
        m_pointer_shown = false;
        this->getDevice()->getCursorControl()->setVisible(false);
    }
}   // hidePointer

//-----------------------------------------------------------------------------

core::position2di IrrDriver::getMouseLocation()
{
    return this->getDevice()->getCursorControl()->getPosition();
}

//-----------------------------------------------------------------------------
/** Moves the STK main window to coordinates (x,y)
 *  \return true on success, false on failure
 *          (always true on Linux at the moment)
 */
bool IrrDriver::moveWindow(int x, int y)
{
#ifdef WIN32
    const video::SExposedVideoData& videoData =
                    m_video_driver->getExposedVideoData();
    // this should work even if using DirectX in theory,
    // because the HWnd is always third pointer in the struct,
    // no matter which union is used
    HWND window = (HWND)videoData.OpenGLWin32.HWnd;
    if (SetWindowPos(window, HWND_TOP, x, y, -1, -1,
                     SWP_NOOWNERZORDER | SWP_NOSIZE))
    {
        // everything OK
        return true;
    }
    else
    {
        Log::warn("irr_driver", "Could not set window location\n");
        return false;
    }
#elif defined(__linux__) && !defined(ANDROID)
    const video::SExposedVideoData& videoData = m_video_driver->getExposedVideoData();

    Display* display = (Display*)videoData.OpenGLLinux.X11Display;
    int screen = DefaultScreen(display);
    int screen_w = DisplayWidth(display, screen);
    int screen_h = DisplayHeight(display, screen);

    if (x + UserConfigParams::m_width > screen_w)
    {
        x = screen_w - UserConfigParams::m_width;
    }

    if (y + UserConfigParams::m_height > screen_h)
    {
        y = screen_h - UserConfigParams::m_height;
    }

    // TODO: Actually handle possible failure
    XMoveWindow(display, videoData.OpenGLLinux.X11Window, x, y);
#endif
    return true;
}
//-----------------------------------------------------------------------------

void IrrDriver::changeResolution(const int w, const int h,
                                 const bool fullscreen)
{
    // update user config values
    UserConfigParams::m_prev_width = UserConfigParams::m_width;
    UserConfigParams::m_prev_height = UserConfigParams::m_height;
    UserConfigParams::m_prev_fullscreen = UserConfigParams::m_fullscreen;

    UserConfigParams::m_width = w;
    UserConfigParams::m_height = h;
    UserConfigParams::m_fullscreen = fullscreen;

    // Setting this flag will trigger a call to applyResolutionSetting()
    // in the next update call. This avoids the problem that changeResolution
    // is actually called from the gui, i.e. the event loop, i.e. while the
    // old device is active - so we can't delete this device (which we must
    // do in applyResolutionSettings
    m_resolution_changing = RES_CHANGE_YES;
}   // changeResolution

//-----------------------------------------------------------------------------

void IrrDriver::applyResolutionSettings()
{
    // show black before resolution switch so we don't see OpenGL's buffer
    // garbage during switch
    m_video_driver->beginScene(true, true, video::SColor(255,100,101,140));
    GL32_draw2DRectangle( video::SColor(255, 0, 0, 0),
                            core::rect<s32>(0, 0,
                                            UserConfigParams::m_prev_width,
                                            UserConfigParams::m_prev_height) );
    m_video_driver->endScene();
    track_manager->removeAllCachedData();
    attachment_manager->removeTextures();
    projectile_manager->removeTextures();
    ItemManager::removeTextures();
    kart_properties_manager->unloadAllKarts();
    powerup_manager->unloadPowerups();
    Referee::cleanup();
    ParticleKindManager::get()->cleanup();
    delete input_manager;
    delete font_manager;
    font_manager = NULL;
    GUIEngine::clear();
    GUIEngine::cleanUp();

    m_device->closeDevice();
    m_device->clearSystemMessages();
    m_device->run();

    delete material_manager;
    material_manager = NULL;

    // ---- Reinit
    // FIXME: this load sequence is (mostly) duplicated from main.cpp!!
    // That's just error prone
    // (we're sure to update main.cpp at some point and forget this one...)
    ShaderBase::updateShaders();
    VAOManager::getInstance()->kill();
    SolidPassCmd::getInstance()->kill();
    ShadowPassCmd::getInstance()->kill();
    RSMPassCmd::getInstance()->kill();
    GlowPassCmd::getInstance()->kill();
    resetTextureTable();
    // initDevice will drop the current device.
    if (CVS->isGLSL())
    {
        Shaders::destroy();
    }
    initDevice();

    font_manager = new FontManager();
    font_manager->loadFonts();
    // Re-init GUI engine
    GUIEngine::init(m_device, m_video_driver, StateManager::get());

    //material_manager->reInit();
    material_manager = new MaterialManager();
    material_manager->loadMaterial();
    input_manager = new InputManager ();
    input_manager->setMode(InputManager::MENU);

    GUIEngine::addLoadingIcon(
        irr_driver->getTexture(file_manager
                               ->getAsset(FileManager::GUI,"options_video.png"))
                             );

    file_manager->pushTextureSearchPath(file_manager->getAsset(FileManager::MODEL,""));
    const std::string materials_file =
        file_manager->getAssetChecked(FileManager::MODEL, "materials.xml");
    if (materials_file != "")
    {
        material_manager->addSharedMaterial(materials_file);
    }

    powerup_manager->loadAllPowerups ();
    ItemManager::loadDefaultItemMeshes();
    projectile_manager->loadData();
    Referee::init();
    GUIEngine::addLoadingIcon(
        irr_driver->getTexture(file_manager->getAsset(FileManager::GUI,"gift.png")) );

    file_manager->popTextureSearchPath();

    kart_properties_manager->loadAllKarts();

    attachment_manager->loadModels();
    std::string banana = file_manager->getAsset(FileManager::GUI, "banana.png");
    GUIEngine::addLoadingIcon(irr_driver->getTexture(banana) );
    // No need to reload cached track data (track_manager->cleanAllCachedData
    // above) - this happens dynamically when the tracks are loaded.
    GUIEngine::reshowCurrentScreen();
    MessageQueue::updatePosition();
}   // applyResolutionSettings

// ----------------------------------------------------------------------------

void IrrDriver::cancelResChange()
{
    UserConfigParams::m_width = UserConfigParams::m_prev_width;
    UserConfigParams::m_height = UserConfigParams::m_prev_height;
    UserConfigParams::m_fullscreen = UserConfigParams::m_prev_fullscreen;

    // This will trigger calling applyResolutionSettings in update(). This is
    // necessary to avoid that the old screen is deleted, while it is
    // still active (i.e. sending out events which triggered the change
    // of resolution
    // Setting this flag will trigger a call to applyResolutionSetting()
    // in the next update call. This avoids the problem that changeResolution
    // is actually called from the gui, i.e. the event loop, i.e. while the
    // old device is active - so we can't delete this device (which we must
    // do in applyResolutionSettings)
    m_resolution_changing=RES_CHANGE_CANCEL;

}   // cancelResChange

// ----------------------------------------------------------------------------
/** Prints statistics about rendering, e.g. number of drawn and culled
 *  triangles etc. Note that printing this information will also slow
 *  down STK.
 */
void IrrDriver::printRenderStats()
{
    io::IAttributes * attr = m_scene_manager->getParameters();
    Log::verbose("irr_driver",
           "[%ls], FPS:%3d Tri:%.03fm Cull %d/%d nodes (%d,%d,%d)\n",
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
    scene::IAnimatedMesh *m  = NULL;

    if (StringUtils::getExtension(filename) == "b3dz")
    {
        // compressed file
        io::IFileSystem* file_system = getDevice()->getFileSystem();
        if (!file_system->addFileArchive(filename.c_str(),
                                         /*ignoreCase*/false,
                                         /*ignorePath*/true, io::EFAT_ZIP))
        {
            Log::error("irr_driver",
                       "getMesh: Failed to open zip file <%s>\n",
                       filename.c_str());
            return NULL;
        }

        // Get the recently added archive
        io::IFileArchive* zip_archive =
        file_system->getFileArchive(file_system->getFileArchiveCount()-1);
        io::IReadFile* content = zip_archive->createAndOpenFile(0);
        m = m_scene_manager->getMesh(content);
        content->drop();

        file_system->removeFileArchive(file_system->getFileArchiveCount()-1);
    }
    else
    {
        m = m_scene_manager->getMesh(filename.c_str());
    }

    if(!m) return NULL;

    setAllMaterialFlags(m);

    return m;
}   // getAnimatedMesh

// ----------------------------------------------------------------------------

/** Loads a non-animated mesh and returns a pointer to it.
 *  \param filename  File to load.
 */
scene::IMesh *IrrDriver::getMesh(const std::string &filename)
{
    scene::IAnimatedMesh* am = getAnimatedMesh(filename);
    if (am == NULL)
    {
        Log::error("irr_driver", "Cannot load mesh <%s>\n",
                   filename.c_str());
        return NULL;
    }
    return am->getMesh(0);
}   // getMesh

// ----------------------------------------------------------------------------
/** Create a skinned mesh which has copied all meshbuffers and joints of the
 *  original mesh. Note, that this will not copy any other information like
 *   joints data.
 *  \param mesh Original mesh
 *  \return Newly created skinned mesh. You should call drop() when you don't
 *          need it anymore.
 */
scene::IAnimatedMesh *IrrDriver::copyAnimatedMesh(scene::IAnimatedMesh *orig)
{
    using namespace scene;
    CSkinnedMesh *mesh = dynamic_cast<CSkinnedMesh*>(orig);
    if (!mesh)
    {
        Log::error("copyAnimatedMesh", "Given mesh was not a skinned mesh.");
        return NULL;
    }

    scene::IAnimatedMesh* out = mesh->clone();
    return out;
}   // copyAnimatedMesh

// ----------------------------------------------------------------------------
/** Sets the material flags in this mesh depending on the settings in
 *  material_manager.
 *  \param mesh  The mesh to change the settings in.
 */
void IrrDriver::setAllMaterialFlags(scene::IMesh *mesh) const
{
    unsigned int n=mesh->getMeshBufferCount();
    for(unsigned int i=0; i<n; i++)
    {
        scene::IMeshBuffer *mb = mesh->getMeshBuffer(i);
        video::SMaterial &irr_material = mb->getMaterial();

        // special case : for splatting, the main material is on layer 1.
        // it was done this way to provide a fallback for computers
        // where shaders are not supported
        video::ITexture* t2 = irr_material.getTexture(1);
        bool is_splatting = false;
        if (t2)
        {
            Material* mat = material_manager->getMaterialFor(t2, mb);
            if (mat != NULL && mat->getShaderType() == Material::SHADERTYPE_SPLATTING)
            {
                material_manager->setAllMaterialFlags(t2, mb);
                is_splatting = true;
            }
        }

        if (!is_splatting)
        {
            video::ITexture* t = irr_material.getTexture(0);
            if (t)
            {
                material_manager->setAllMaterialFlags(t, mb);
            }
            else
            {
                material_manager->setAllUntexturedMaterialFlags(mb);
            }
        }

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
                                           scene::IMesh **welded,
                                           float wave_height,
                                           float wave_speed,
                                           float wave_length)
{
    mesh->setMaterialFlag(video::EMF_GOURAUD_SHADING, true);
    scene::IMesh* welded_mesh = m_scene_manager->getMeshManipulator()
                                               ->createMeshWelded(mesh);
    scene::ISceneNode* out = NULL;

    // TODO: using cand's new WaterNode would be better, but it does not
    // support our material flags (like transparency, etc.)
    //if (!m_glsl)
    //{
        out = m_scene_manager->addWaterSurfaceSceneNode(welded_mesh,
                                                     wave_height, wave_speed,
                                                     wave_length);
    //} else
    //{
    //    out = new WaterNode(m_scene_manager, welded_mesh, wave_height, wave_speed,
    //                        wave_length);
    //}

    out->getMaterial(0).setFlag(video::EMF_GOURAUD_SHADING, true);
    welded_mesh->drop();  // The scene node keeps a reference

    *welded = welded_mesh;

    return out;
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
/** Adds a sphere with a given radius and color.
 *  \param radius The radius of the sphere.
 *  \param color The color to use (default (0,0,0,0)
 */
scene::IMeshSceneNode *IrrDriver::addSphere(float radius,
                                            const video::SColor &color)
{
    scene::IMesh *mesh = m_scene_manager->getGeometryCreator()
                       ->createSphereMesh(radius);

    mesh->setMaterialFlag(video::EMF_COLOR_MATERIAL, true);
    video::SMaterial &m = mesh->getMeshBuffer(0)->getMaterial();
    m.AmbientColor    = color;
    m.DiffuseColor    = color;
    m.EmissiveColor   = color;
    m.BackfaceCulling = false;
    m.MaterialType    = video::EMT_SOLID;
    //m.setTexture(0, getUnicolorTexture(video::SColor(128, 255, 105, 180)));
    m.setTexture(0, getUnicolorTexture(color));
    m.setTexture(1, getUnicolorTexture(video::SColor(0, 0, 0, 0)));
    m.setTexture(7, getUnicolorTexture(video::SColor(0, 0, 0, 0)));

    if (CVS->isGLSL())
    {
        STKMeshSceneNode *node =
            new STKMeshSceneNode(mesh,
                                m_scene_manager->getRootSceneNode(),
                                NULL, -1, "sphere");
        return node;
    }

    scene::IMeshSceneNode *node = m_scene_manager->addMeshSceneNode(mesh);
    return node;
}   // addSphere

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
                                          const std::string& debug_name,
                                          scene::ISceneNode *parent,
                                          RenderInfo* render_info,
                                          bool all_parts_colorized)
{
    if (!CVS->isGLSL())
        return m_scene_manager->addMeshSceneNode(mesh, parent);

    if (!parent)
      parent = m_scene_manager->getRootSceneNode();

    scene::IMeshSceneNode* node = new STKMeshSceneNode(mesh, parent,
                                                       m_scene_manager, -1,
                                                       debug_name,
                                                       core::vector3df(0, 0, 0),
                                                       core::vector3df(0, 0, 0),
                                                       core::vector3df(1.0f, 1.0f, 1.0f),
                                                       true, render_info,
                                                       all_parts_colorized);
    node->drop();

    return node;
}   // addMesh

// ----------------------------------------------------------------------------

PerCameraNode *IrrDriver::addPerCameraNode(scene::ISceneNode* node,
                                           scene::ICameraSceneNode* camera,
                                           scene::ISceneNode *parent)
{
    return new PerCameraNode((parent ? parent
                                     : m_scene_manager->getRootSceneNode()),
                             m_scene_manager, -1, camera, node);
}   // addNode


// ----------------------------------------------------------------------------
/** Adds a billboard node to scene.
 */
scene::ISceneNode *IrrDriver::addBillboard(const core::dimension2d< f32 > size,
                                           video::ITexture *texture,
                                           scene::ISceneNode* parent,
                                           bool alphaTesting)
{
    scene::IBillboardSceneNode* node;
    if (CVS->isGLSL())
    {
        if (!parent)
            parent = m_scene_manager->getRootSceneNode();

        node = new STKBillboard(parent, m_scene_manager, -1,
                                vector3df(0., 0., 0.), size);
        node->drop();
    }
    else
        node = m_scene_manager->addBillboardSceneNode(parent, size);
    assert(node->getMaterialCount() > 0);
    node->setMaterialTexture(0, texture);
    if(alphaTesting)
        node->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF);
    return node;
}   // addMesh

// ----------------------------------------------------------------------------
/** Creates a quad mesh with a given material.
 *  \param material The material to use (NULL if no material).
 *  \param create_one_quad If true creates one quad in the mesh.
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
    scene::SMesh *mesh   = new scene::SMesh();
    mesh->addMeshBuffer(buffer);
    mesh->recalculateBoundingBox();
    buffer->drop();
    return mesh;
}   // createQuadMesh

// ----------------------------------------------------------------------------
/** Creates a quad mesh buffer with a given width and height (z coordinate is
 *  0).
 *  \param material The material to use for this quad.
 *  \param w Width of the quad.
 *  \param h Height of the quad.
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
    v1.Color = video::SColor(255, 255, 255, 255);

    video::S3DVertex v2;
    v2.Pos    = core::vector3df(w_2,-h_2,0);
    v2.Normal = core::vector3df(0, 0, -1.0f);
    v2.TCoords = core::vector2d<f32>(0,1);
    v1.Color = video::SColor(255, 255, 255, 255);

    video::S3DVertex v3;
    v3.Pos    = core::vector3df(w_2,h_2,0);
    v3.Normal = core::vector3df(0, 0, -1.0f);
    v3.TCoords = core::vector2d<f32>(0,0);
    v3.Color = video::SColor(255, 255, 255, 255);

    video::S3DVertex v4;
    v4.Pos    = core::vector3df(-w_2,h_2,0);
    v4.Normal = core::vector3df(0, 0, -1.0f);
    v4.TCoords = core::vector2d<f32>(1,0);
    v4.Color = video::SColor(255, 255, 255, 255);

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
    scene::SMesh *mesh = new scene::SMesh();
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
}   // removeNode

// ----------------------------------------------------------------------------
/** Removes a mesh from the mesh cache, freeing the memory.
 *  \param mesh The mesh to remove.
 */
void IrrDriver::removeMeshFromCache(scene::IMesh *mesh)
{
    m_scene_manager->getMeshCache()->removeMesh(mesh);
}   // removeMeshFromCache

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
scene::IAnimatedMeshSceneNode *IrrDriver::addAnimatedMesh(scene::IAnimatedMesh *mesh,
    const std::string& debug_name, scene::ISceneNode* parent,
    RenderInfo* render_info, bool all_parts_colorized)
{
    if (!CVS->isGLSL())
    {
        return m_scene_manager->addAnimatedMeshSceneNode(mesh, parent, -1,
            core::vector3df(0, 0, 0),
            core::vector3df(0, 0, 0),
            core::vector3df(1, 1, 1),
            /*addIfMeshIsZero*/true);
    }

    if (!parent)
        parent = m_scene_manager->getRootSceneNode();
    scene::IAnimatedMeshSceneNode* node =
        new STKAnimatedMesh(mesh, parent, m_scene_manager, -1, debug_name,
        core::vector3df(0, 0, 0), core::vector3df(0, 0, 0),
        core::vector3df(1, 1, 1), render_info, all_parts_colorized);
    node->drop();
    return node;
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
scene::ISceneNode *IrrDriver::addSkyDome(video::ITexture *texture,
                                         int hori_res, int vert_res,
                                         float texture_percent,
                                         float sphere_percent)
{
    Log::error("skybox", "Using deprecated SkyDome");
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
scene::ISceneNode *IrrDriver::addSkyBox(const std::vector<video::ITexture*> &texture,
    const std::vector<video::ITexture*> &spherical_harmonics_textures)
{
    assert(texture.size() == 6);

    if (CVS->isGLSL())
    {
        m_skybox = new Skybox(texture);
    }

    if(spherical_harmonics_textures.size() == 6)
    {
        m_spherical_harmonics->setTextures(spherical_harmonics_textures);
    }

    return m_scene_manager->addSkyBoxSceneNode(texture[0], texture[1],
                                               texture[2], texture[3],
                                               texture[4], texture[5]);
}   // addSkyBox

void IrrDriver::suppressSkyBox()
{
    delete m_skybox;
    m_skybox = NULL;
}

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
/** Sets an error message to be displayed when a texture is not found. This
 *  error message is shown before the "Texture '%s' not found" message. It can
 *  be used to supply additional details like what kart is currently being
 *  loaded.
 *  \param error Error message, potentially with a '%' which will be replaced
 *               with detail.
 *  \param detail String to replace a '%' in the error message.
 */
void IrrDriver::setTextureErrorMessage(const std::string &error,
                                       const std::string &detail)
{
    if(detail=="")
        m_texture_error_message = error;
    else
        m_texture_error_message = StringUtils::insertValues(error, detail);
}   // setTextureErrorMessage

// ----------------------------------------------------------------------------
/** Disables the texture error message again.
 */
void IrrDriver::unsetTextureErrorMessage()
{
    m_texture_error_message = "";
}   // unsetTextureErrorMessage

// ----------------------------------------------------------------------------
/** Retrieve all textures in the specified directory, generate a smaller
*   version for each of them and save them in the cache. Smaller textures are
*   generated only if they do not already exist or if their original version
*   is newer than the cached one.
*   \param dir Directory from where textures will be retrieved.
*              Must end with '/'.
*   \return Directory where smaller textures were cached.
*/
std::string IrrDriver::generateSmallerTextures(const std::string& dir)
{
    std::set<std::string> files;
    file_manager->listFiles(files, dir, true);

    std::set<std::string>::const_iterator it;
    for (it = files.begin(); it != files.end(); ++it)
    {
        std::string ext = StringUtils::toLowerCase(StringUtils::getExtension(*it));
        if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "bmp")
        {
            getSmallerTexture(*it);
        }
    } // for it in files

    return file_manager->getTextureCacheLocation(dir);
} // generateSmallerTextures

// ----------------------------------------------------------------------------
/** Return the filename for the cached smaller version of the texture. Also,
*   generate the smaller version of the texture if it does not already
*   exist or if the original version is newer than the cached one.
*   \param filename File name of the original texture.
*   \return File name of the cached texture.
*/
std::string IrrDriver::getSmallerTexture(const std::string& filename)
{
    // Retrieve the filename of the cached texture
    std::string cached_file = file_manager->getTextureCacheLocation(filename);

    // If the cached texture does not exist, we generate it.
    if (!file_manager->fileExists(cached_file) ||
        file_manager->fileIsNewer(filename, cached_file))
    {
        video::IVideoDriver* video_driver = irr_driver->getVideoDriver();
        video::IImage* img =
            video_driver->createImageFromFile(filename.c_str());

        if (img != NULL)
        {
            core::dimension2d<u32> dim = img->getDimension();
            core::dimension2d<u32> new_dim; // Dimension of the cached texture
            const unsigned scale_factor = 2;
            // Resize the texture only if it can be done properly
            if (dim.Width < scale_factor || dim.Height < scale_factor)
                new_dim = dim;
            else
                new_dim = dim / scale_factor;

            video::IImage* scaled =
                video_driver->createImage(img->getColorFormat(), new_dim);
            img->copyToScaling(scaled);

            video_driver->writeImageToFile(scaled, cached_file.c_str());
        } // if img != NULL
    } // if !file_manager->fileExists(cached_file)
    return cached_file;
} // getSmallerTexture

// ----------------------------------------------------------------------------
/** Loads a texture from a file and returns the texture object. This is just
 *  a convenient wrapper which loads the texture from a STK asset directory.
 *  It calls the file manager to get the full path, then calls the normal
 *  getTexture() function.s
 *  \param type The FileManager::AssetType of the texture.
 *  \param filename File name of the texture to load.
 *  \param is_premul If the alpha values needd to be multiplied for
 *         all pixels.
 *  \param is_prediv If the alpha value needs to be divided into
 *         each pixel.
 */
video::ITexture *IrrDriver::getTexture(FileManager::AssetType type,
                                       const std::string &filename,
                                       bool is_premul,
                                       bool is_prediv,
                                       bool complain_if_not_found)
{
    const std::string path = file_manager->getAsset(type, filename);
    return getTexture(path, is_premul, is_prediv, complain_if_not_found);
}   // getTexture

// ----------------------------------------------------------------------------
/** Loads a texture from a file and returns the texture object.
 *  \param filename File name of the texture to load.
 *  \param is_premul If the alpha values needd to be multiplied for
 *         all pixels.
 *  \param is_prediv If the alpha value needs to be divided into
 *         each pixel.
 */
video::ITexture *IrrDriver::getTexture(const std::string &filename,
                                       bool is_premul,
                                       bool is_prediv,
                                       bool complain_if_not_found)
{
    video::ITexture* out;
    if(!is_premul && !is_prediv)
    {
        if (!complain_if_not_found) m_device->getLogger()->setLogLevel(ELL_NONE);
        out = m_video_driver->getTexture(filename.c_str());
        if (!complain_if_not_found) m_device->getLogger()->setLogLevel(ELL_WARNING);
    }
    else
    {
        // FIXME: can't we just do this externally, and just use the
        // modified textures??
        video::IImage* img =
            m_video_driver->createImageFromFile(filename.c_str());
        // PNGs are non premul, but some are used for premul tasks, so convert
        // http://home.comcast.net/~tom_forsyth/blog.wiki.html#[[Premultiplied%20alpha]]
        // FIXME check param, not name
        if(img && is_premul &&
            StringUtils::hasSuffix(filename.c_str(), ".png") &&
            (img->getColorFormat() == video::ECF_A8R8G8B8) &&
            img->lock())
        {
            core::dimension2d<u32> dim = img->getDimension();
            for(unsigned int x = 0; x < dim.Width; x++)
            {
                for(unsigned int y = 0; y < dim.Height; y++)
                {
                    video::SColor col = img->getPixel(x, y);
                    unsigned int alpha = col.getAlpha();
                    unsigned int red   = alpha * col.getRed()   / 255;
                    unsigned int blue  = alpha * col.getBlue()  / 255;
                    unsigned int green = alpha * col.getGreen() / 255;
                    col.set(alpha, red, green, blue);
                    img->setPixel(x, y, col, false);
                }   // for y
            }   // for x
            img->unlock();
        }   // if png and ColorFOrmat and lock
        // Other formats can be premul, but the tasks can be non premul
        // So divide to get the separate RGBA (only possible if alpha!=0)
        else if(img && is_prediv &&
            (img->getColorFormat() == video::ECF_A8R8G8B8) &&
            img->lock())
        {
            core::dimension2d<u32> dim = img->getDimension();
            for(unsigned int  x = 0; x < dim.Width; x++)
            {
                for(unsigned int y = 0; y < dim.Height; y++)
                {
                    video::SColor col = img->getPixel(x, y);
                    unsigned int alpha = col.getAlpha();
                    // Avoid divide by zero
                    if (alpha) {
                        unsigned int red   = 255 * col.getRed() / alpha ;
                        unsigned int blue  = 255 * col.getBlue() / alpha;
                        unsigned int green = 255 * col.getGreen() / alpha;
                        col.set(alpha, red, green, blue);
                        img->setPixel(x, y, col, false);
                    }
                }   // for y
            }   // for x
            img->unlock();
        }   // if premul && color format && lock
        out = m_video_driver->addTexture(filename.c_str(), img, NULL);
    }   // if is_premul or is_prediv


    if (complain_if_not_found && out == NULL)
    {

        if(m_texture_error_message.size()>0)
        {
            Log::error("irr_driver", m_texture_error_message.c_str());
        }
        Log::error("irr_driver", "Texture '%s' not found.", filename.c_str());
    }

    m_texturesFileName[out] = filename;

    return out;
}   // getTexture

// ----------------------------------------------------------------------------
/** Clear the texture-filename reminder.
*/
void IrrDriver::clearTexturesFileName()
{
    m_texturesFileName.clear();
} // clearTexturesFileName

// ----------------------------------------------------------------------------
/** Get the texture file name using a texture pointer.
*   \param tex Pointer on the texture for which we want to find the file name.
*   \return Filename of the texture if found, or an empty string otherwise.
*/
std::string IrrDriver::getTextureName(video::ITexture* tex)
{
    std::map<video::ITexture*, std::string>::iterator it;
    it = m_texturesFileName.find(tex);
    if (it != m_texturesFileName.end())
        return it->second;
    else
        return "";
} // getTextureName

// ----------------------------------------------------------------------------
/** Appends a pointer to each texture used in this mesh to the vector.
 *  \param mesh The mesh from which the textures are being determined.
 *  \param texture_list The list to which to attach the pointer to.
 */
void IrrDriver::grabAllTextures(const scene::IMesh *mesh)
{
    const unsigned int n = mesh->getMeshBufferCount();

    for(unsigned int i=0; i<n; i++)
    {
        scene::IMeshBuffer *b = mesh->getMeshBuffer(i);
        video::SMaterial   &m = b->getMaterial();
        for(unsigned int j=0; j<video::MATERIAL_MAX_TEXTURES; j++)
        {
            video::ITexture *t = m.getTexture(j);
            if(t)
                t->grab();
        }   // for j < MATERIAL_MAX_TEXTURE
    }   // for i <getMeshBufferCount
}   // grabAllTextures

// ----------------------------------------------------------------------------
/** Appends a pointer to each texture used in this mesh to the vector.
 *  \param mesh The mesh from which the textures are being determined.
 *  \param texture_list The list to which to attach the pointer to.
 */
void IrrDriver::dropAllTextures(const scene::IMesh *mesh)
{
    const unsigned int n = mesh->getMeshBufferCount();

    for(unsigned int i=0; i<n; i++)
    {
        scene::IMeshBuffer *b = mesh->getMeshBuffer(i);
        video::SMaterial   &m = b->getMaterial();
        for(unsigned int j=0; j<video::MATERIAL_MAX_TEXTURES; j++)
        {
            video::ITexture *t = m.getTexture(j);
            if(t)
            {
                t->drop();
                if(t->getReferenceCount()==1)
                    removeTexture(t);
            }   // if t
        }   // for j < MATERIAL_MAX_TEXTURE
    }   // for i <getMeshBufferCount
}   // dropAllTextures

// ----------------------------------------------------------------------------
video::ITexture* IrrDriver::applyMask(video::ITexture* texture,
                                      const std::string& mask_path)
{
    video::IImage* img =
        m_video_driver->createImage(texture, core::position2d<s32>(0,0),
                                    texture->getSize());

    video::IImage* mask =
        m_video_driver->createImageFromFile(mask_path.c_str());

    if (img == NULL || mask == NULL) return NULL;

    if (img->lock() && mask->lock())
    {
        core::dimension2d<u32> dim = img->getDimension();
        for (unsigned int x = 0; x < dim.Width; x++)
        {
            for (unsigned int y = 0; y < dim.Height; y++)
            {
                video::SColor col = img->getPixel(x, y);
                video::SColor alpha = mask->getPixel(x, y);
                col.setAlpha( alpha.getRed() );
                img->setPixel(x, y, col, false);
            }   // for y
        }   // for x

        mask->unlock();
        img->unlock();
    }
    else
    {
        return NULL;
    }

    std::string base =
        StringUtils::getBasename(texture->getName().getPath().c_str());
    video::ITexture *t = m_video_driver->addTexture(base.c_str(),img, NULL);
    img->drop();
    mask->drop();
    return t;
}   // applyMask
// ----------------------------------------------------------------------------
void IrrDriver::setRTT(RTT* rtt)
{
    m_shadow_matrices->resetShadowCamNodes();
    m_rtts = rtt;
}
// ----------------------------------------------------------------------------
void IrrDriver::onLoadWorld()
{
    if (CVS->isGLSL())
    {
        const core::recti &viewport = Camera::getCamera(0)->getViewport();
        size_t width = viewport.LowerRightCorner.X - viewport.UpperLeftCorner.X;
        size_t height = viewport.LowerRightCorner.Y - viewport.UpperLeftCorner.Y;
        m_rtts = new RTT(width, height);
    }
}
// ----------------------------------------------------------------------------
void IrrDriver::onUnloadWorld()
{
    delete m_rtts;
    m_rtts = NULL;

    suppressSkyBox();
}
// ----------------------------------------------------------------------------
/** Sets the ambient light.
 *  \param light The colour of the light to set.
 */
void IrrDriver::setAmbientLight(const video::SColorf &light)
{
    m_scene_manager->setAmbientLight(light);
    m_spherical_harmonics->setAmbientLight(light.toSColor());
}   // setAmbientLight

video::SColorf IrrDriver::getAmbientLight() const
{
    return m_scene_manager->getAmbientLight();
}

// ----------------------------------------------------------------------------
/** Displays the FPS on the screen.
 */
void IrrDriver::displayFPS()
{
    gui::IGUIFont* font = GUIEngine::getSmallFont();
    core::rect<s32> position;

    if (UserConfigParams::m_artist_debug_mode)
        position = core::rect<s32>(75, 0, 1100, 40);
    else
        position = core::rect<s32>(75, 0, 900, 40);
    GL32_draw2DRectangle(video::SColor(150, 96, 74, 196), position, NULL);
    // We will let pass some time to let things settle before trusting FPS counter
    // even if we also ignore fps = 1, which tends to happen in first checks
    const int NO_TRUST_COUNT = 200;
    static int no_trust = NO_TRUST_COUNT;

    // Min and max info tracking, per mode, so user can check game vs menus
    bool current_state     = StateManager::get()->getGameState()
                               == GUIEngine::GAME;
    static bool prev_state = false;
    static int min         = 999; // Absurd values for start will print first time
    static int max         = 0;   // but no big issue, maybe even "invisible"
    static float low       = 1000000.0f; // These two are for polycount stats
    static float high      = 0.0f;       // just like FPS, but in KTris

    // Reset limits if state changes
    if (prev_state != current_state)
    {
        min = 999;
        max = 0;
        low = 1000000.0f;
        high = 0.0f;
        no_trust = NO_TRUST_COUNT;
        prev_state = current_state;
    }

    if (no_trust)
    {
        no_trust--;

        static video::SColor fpsColor = video::SColor(255, 0, 0, 0);
        font->draw( L"FPS: ...", core::rect< s32 >(100,0,400,50), fpsColor,
                    false );

        return;
    }

    // Ask for current frames per second and last number of triangles
    // processed (trimed to thousands)
    const int fps         = m_video_driver->getFPS();
    const float kilotris  = m_video_driver->getPrimitiveCountDrawn(0)
                                * (1.f / 1000.f);

    if (min > fps && fps > 1) min = fps; // Start moments sometimes give useless 1
    if (max < fps) max = fps;
    if (low > kilotris) low = kilotris;
    if (high < kilotris) high = kilotris;

    core::stringw fpsString;

    if (UserConfigParams::m_artist_debug_mode)
    {
        fpsString = _("FPS: %d/%d/%d  - PolyCount: %d Solid, "
                      "%d Shadows - LightDist : %d",
                    min, fps, max, m_poly_count[SOLID_NORMAL_AND_DEPTH_PASS],
                    m_poly_count[SHADOW_PASS], m_last_light_bucket_distance);
        m_poly_count[SOLID_NORMAL_AND_DEPTH_PASS] = 0;
        m_poly_count[SHADOW_PASS] = 0;
        m_object_count[SOLID_NORMAL_AND_DEPTH_PASS] = 0;
        m_object_count[SOLID_NORMAL_AND_DEPTH_PASS] = 0;
        m_object_count[TRANSPARENT_PASS] = 0;
    }
    else
        fpsString = _("FPS: %d/%d/%d - %d KTris", min, fps, max, (int)roundf(kilotris));

    static video::SColor fpsColor = video::SColor(255, 0, 0, 0);

    font->draw( fpsString.c_str(), position, fpsColor, false );
}   // updateFPS

// ----------------------------------------------------------------------------
#ifdef DEBUG
void IrrDriver::drawDebugMeshes()
{
    for (unsigned int n=0; n<m_debug_meshes.size(); n++)
    {
        scene::IMesh* mesh = m_debug_meshes[n]->getMesh();
        scene::ISkinnedMesh* smesh = static_cast<scene::ISkinnedMesh*>(mesh);
        const core::array< scene::ISkinnedMesh::SJoint * >& joints =
            smesh->getAllJoints();

        for (unsigned int j=0; j<joints.size(); j++)
        {
            drawJoint( false, true, joints[j], smesh, j);
        }
    }

    video::SColor color(255,255,255,255);
    video::SMaterial material;
    material.Thickness = 2;
    material.AmbientColor = color;
    material.DiffuseColor = color;
    material.EmissiveColor= color;
    material.BackfaceCulling = false;
    material.setFlag(video::EMF_LIGHTING, false);
    getVideoDriver()->setMaterial(material);
    getVideoDriver()->setTransform(video::ETS_WORLD, core::IdentityMatrix);

    for (unsigned int n=0; n<m_debug_meshes.size(); n++)
    {
        scene::IMesh* mesh = m_debug_meshes[n]->getMesh();


        scene::ISkinnedMesh* smesh = static_cast<scene::ISkinnedMesh*>(mesh);
        const core::array< scene::ISkinnedMesh::SJoint * >& joints =
            smesh->getAllJoints();

        for (unsigned int j=0; j<joints.size(); j++)
        {
            scene::IMesh* mesh = m_debug_meshes[n]->getMesh();
            scene::ISkinnedMesh* smesh = static_cast<scene::ISkinnedMesh*>(mesh);

            drawJoint(true, false, joints[j], smesh, j);
        }
    }
}   // drawDebugMeshes

// ----------------------------------------------------------------------------
/** Draws a joing for debugging skeletons.
 *  \param drawline If true draw a line to the parent.
 *  \param drawname If true draw the name of the joint.
 *  \param joint The joint to draw.
 *  \param mesh The mesh whose skeleton is drawn (only used to get
 *         all joints to find the parent).
 *  \param id Index, which (%4) determines the color to use.
 */
void IrrDriver::drawJoint(bool drawline, bool drawname,
                          scene::ISkinnedMesh::SJoint* joint,
                          scene::ISkinnedMesh* mesh, int id)
{
    scene::ISkinnedMesh::SJoint* parent = NULL;
    const core::array< scene::ISkinnedMesh::SJoint * >& joints
        = mesh->getAllJoints();
    for (unsigned int j=0; j<joints.size(); j++)
    {
        if (joints[j]->Children.linear_search(joint) != -1)
        {
            parent = joints[j];
            break;
        }
    }

    core::vector3df jointpos = joint->GlobalMatrix.getTranslation();

    video::SColor color(255, 255,255,255);
    if (parent == NULL) color = video::SColor(255,0,255,0);

    switch (id % 4)
    {
        case 0:
            color = video::SColor(255,255,0,255);
            break;
        case 1:
            color = video::SColor(255,255,0,0);
            break;
        case 2:
            color = video::SColor(255,0,0,255);
            break;
        case 3:
            color = video::SColor(255,0,255,255);
            break;
    }


    if (parent)
    {
        core::vector3df parentpos = parent->GlobalMatrix.getTranslation();

        jointpos = joint->GlobalMatrix.getTranslation();

        if (drawline)
        {
            irr_driver->getVideoDriver()->draw3DLine(jointpos,
                                                     parentpos,
                                                     color);
        }

    }
    else
    {
        /*
        if (drawline)
        {
            irr_driver->getVideoDriver()->draw3DLine(jointpos,
                                                     core::vector3df(0,0,0),
                                                     color);
        }
         */
    }

    if (joint->Children.size() == 0)
    {
        switch ((id + 1) % 4)
        {
            case 0:
                color = video::SColor(255,255,0,255);
                break;
            case 1:
                color = video::SColor(255,255,0,0);
                break;
            case 2:
                color = video::SColor(255,0,0,255);
                break;
            case 3:
                color = video::SColor(255,0,255,255);
                break;
        }

        // This code doesn't quite work. 0.25 is used so that the bone is not
        // way too long (not sure why I need to manually size it down)
        // and the rotation of the bone is often rather off
        core::vector3df v(0.0f, 0.25f, 0.0f);
        //joint->GlobalMatrix.rotateVect(v);
        joint->LocalMatrix.rotateVect(v);
        v *= joint->LocalMatrix.getScale();
        irr_driver->getVideoDriver()->draw3DLine(jointpos,
                                                 jointpos + v,
                                                 color);
    }

    switch ((id + 1) % 4)
    {
        case 0:
            color = video::SColor(255,255,0,255);
            break;
        case 1:
            color = video::SColor(255,255,0,0);
            break;
        case 2:
            color = video::SColor(255,0,0,255);
            break;
        case 3:
            color = video::SColor(255,0,255,255);
            break;
    }

    if (drawname)
    {
        irr_driver->getVideoDriver()->setTransform(video::ETS_WORLD,
                                                   core::IdentityMatrix);

        core::vector2di textpos =
            irr_driver->getSceneManager()->getSceneCollisionManager()
            ->getScreenCoordinatesFrom3DPosition(jointpos);

        GUIEngine::getSmallFont()->draw( stringw(joint->Name.c_str()),
                                         core::rect<s32>(textpos,
                                               core::dimension2d<s32>(500,50)),
                                         color, false, false );
    }
}
#endif

// ----------------------------------------------------------------------------
/** Requess a screenshot from irrlicht, and save it in a file.
 */
void IrrDriver::doScreenShot()
{
    m_request_screenshot = false;

    video::IImage* image = m_video_driver->createScreenShot();
    if(!image)
    {
        Log::error("irr_driver", "Could not create screen shot.");
        return;
    }

    // Screenshot was successful.
    time_t rawtime;
    time ( &rawtime );
    tm* timeInfo = localtime( &rawtime );
    char time_buffer[256];
    sprintf(time_buffer, "%i.%02i.%02i_%02i.%02i.%02i",
            timeInfo->tm_year + 1900, timeInfo->tm_mon+1,
            timeInfo->tm_mday, timeInfo->tm_hour,
            timeInfo->tm_min, timeInfo->tm_sec);

    std::string track_name = race_manager->getTrackName();
    if (World::getWorld() == NULL) track_name = "menu";
    std::string path = file_manager->getScreenshotDir()+track_name+"-"
                     + time_buffer+".png";

    if (irr_driver->getVideoDriver()->writeImageToFile(image, path.c_str(), 0))
    {
        RaceGUIBase* base = World::getWorld()
                          ? World::getWorld()->getRaceGUI()
                          : NULL;
        if (base)
        {
            base->addMessage(
                      core::stringw(("Screenshot saved to\n" + path).c_str()),
                      NULL, 2.0f, video::SColor(255,255,255,255), true, false);
        }   // if base
    }
    else
    {
        RaceGUIBase* base = World::getWorld()->getRaceGUI();
        if (base)
        {
            base->addMessage(
                core::stringw(("FAILED saving screenshot to\n" + path +
                              "\n:(").c_str()),
                NULL, 2.0f, video::SColor(255,255,255,255),
                true, false);
        }   // if base
    }   // if failed writing screenshot file
    image->drop();
}   // doScreenShot

// ----------------------------------------------------------------------------
/** Update, called once per frame.
 *  \param dt Time since last update
 */
void IrrDriver::update(float dt)
{
    // User aborted (e.g. closed window)
    // =================================
    if (!m_device->run())
    {
        // Don't bother cleaning up GUI, has no use and may result in crashes
        //GUIEngine::cleanUp();
        //GUIEngine::deallocate();
        main_loop->abort();
        return;
    }

    // If we quit via the menu the m_device->run() does not return true.
    // To avoid any other calls, we return here.
    if(main_loop->isAborted())
        return;

    // If the resolution should be switched, do it now. This will delete the
    // old device and create a new one.
    if (m_resolution_changing!=RES_CHANGE_NONE)
    {
        applyResolutionSettings();
        if(m_resolution_changing==RES_CHANGE_YES)
        new ConfirmResolutionDialog();
        m_resolution_changing = RES_CHANGE_NONE;
    }

    m_wind->update();

    PropertyAnimator::get()->update(dt);

    World *world = World::getWorld();

    if (world)
    {
        if (CVS->isGLSL())
            renderGLSL(dt);
        else
            renderFixed(dt);

        GUIEngine::Screen* current_screen = GUIEngine::getCurrentScreen();
        if (current_screen != NULL && current_screen->needs3D())
        {
            GUIEngine::render(dt);
        }

        if (world->getPhysics() != NULL)
        {
            IrrDebugDrawer* debug_drawer = world->getPhysics()->getDebugDrawer();
            if (debug_drawer != NULL && debug_drawer->debugEnabled())
            {
                debug_drawer->beginNextFrame();
            }
        }
    }
    else
    {
        m_video_driver->beginScene(/*backBuffer clear*/ true, /*zBuffer*/ true,
                                   video::SColor(255,100,101,140));

        GUIEngine::render(dt);

        m_video_driver->endScene();
    }

    if (m_request_screenshot) doScreenShot();

    // Enable this next print statement to get render information printed
    // E.g. number of triangles rendered, culled etc. The stats is only
    // printed while the race is running and not while the in-game menu
    // is shown. This way the output can be studied by just opening the
    // menu.
    //if(World::getWorld() && World::getWorld()->isRacePhase())
    //    printRenderStats();
}   // update

// ----------------------------------------------------------------------------

void IrrDriver::requestScreenshot()
{
    RaceGUIBase* base = World::getWorld()
                         ? World::getWorld()->getRaceGUI()
                         : NULL;
    if (base)
    {
        base->clearAllMessages();
    }

    m_request_screenshot = true;
}

// ----------------------------------------------------------------------------
/** This is not really used to process events, it's only used to shut down
 *  irrLicht's chatty logging until the event handler is ready to take
 *  the task.
 */
bool IrrDriver::OnEvent(const irr::SEvent &event)
{
    //TODO: ideally we wouldn't use this object to STFU irrlicht's chatty
    //      debugging, we'd just create the EventHandler earlier so it
    //      can act upon it
    switch (event.EventType)
    {
    case irr::EET_LOG_TEXT_EVENT:
    {
        // Ignore 'normal' messages
        if (event.LogEvent.Level > 1)
        {
            Log::warn("[IrrDriver Temp Logger]", "Level %d: %s\n",
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

bool IrrDriver::supportsSplatting()
{
    return CVS->isGLSL();
}

// ----------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark RTT
#endif

// ----------------------------------------------------------------------------
/**
 * THIS IS THE OLD OPENGL 1 RTT PROVIDER, USE THE SHADER-BASED
 * RTT FOR NEW DEVELOPMENT
 * Begins a rendering to a texture.
 *  \param dimension The size of the texture.
 *  \param name Name of the texture.
 *  \param persistent_texture Whether the created RTT texture should persist in
 *                            memory after the RTTProvider is deleted
 */
IrrDriver::RTTProvider::RTTProvider(const core::dimension2du &dimension,
                                    const std::string &name, bool persistent_texture)
{
    m_persistent_texture = persistent_texture;
    m_video_driver = irr_driver->getVideoDriver();
    m_render_target_texture =
        m_video_driver->addRenderTargetTexture(dimension,
                                               name.c_str(),
                                               video::ECF_A8R8G8B8);
    if (m_render_target_texture != NULL)
    {
        m_video_driver->setRenderTarget(m_render_target_texture);
    }

    m_rtt_main_node = NULL;
    m_camera        = NULL;
    m_light         = NULL;
}   // RTTProvider

// ----------------------------------------------------------------------------
IrrDriver::RTTProvider::~RTTProvider()
{
    tearDownRTTScene();

    if (!m_persistent_texture)
        irr_driver->removeTexture(m_render_target_texture);
}   // ~RTTProvider

// ----------------------------------------------------------------------------
/** Sets up a given vector of meshes for render-to-texture. Ideal to embed a 3D
 *  object inside the GUI. If there are multiple meshes, the first mesh is
 *  considered to be the root, and all following meshes will have their
 *  locations relative to the location of the first mesh.
 */
void IrrDriver::RTTProvider::setupRTTScene(PtrVector<scene::IMesh, REF>& mesh,
                                           AlignedArray<Vec3>& mesh_location,
                                           AlignedArray<Vec3>& mesh_scale,
                                           const std::vector<int>& model_frames)
{
    if (model_frames[0] == -1)
    {
        scene::ISceneNode* node =
            irr_driver->getSceneManager()->addMeshSceneNode(mesh.get(0), NULL);
        node->setPosition( mesh_location[0].toIrrVector() );
        node->setScale( mesh_scale[0].toIrrVector() );
        node->setMaterialFlag(video::EMF_FOG_ENABLE, false);
        m_rtt_main_node = node;
    }
    else
    {
        scene::IAnimatedMeshSceneNode* node =
            irr_driver->getSceneManager()->addAnimatedMeshSceneNode(
                                    (scene::IAnimatedMesh*)mesh.get(0), NULL );
        node->setPosition( mesh_location[0].toIrrVector() );
        node->setFrameLoop(model_frames[0], model_frames[0]);
        node->setAnimationSpeed(0);
        node->setScale( mesh_scale[0].toIrrVector() );
        node->setMaterialFlag(video::EMF_FOG_ENABLE, false);

        m_rtt_main_node = node;
    }

    assert(m_rtt_main_node != NULL);
    assert(mesh.size() == mesh_location.size());
    assert(mesh.size() == model_frames.size());

    const int mesh_amount = mesh.size();
    for (int n=1; n<mesh_amount; n++)
    {
        if (model_frames[n] == -1)
        {
            scene::ISceneNode* node =
                irr_driver->getSceneManager()->addMeshSceneNode(mesh.get(n),
                                                                m_rtt_main_node);
            node->setPosition( mesh_location[n].toIrrVector() );
            node->updateAbsolutePosition();
            node->setScale( mesh_scale[n].toIrrVector() );
        }
        else
        {
            scene::IAnimatedMeshSceneNode* node =
                irr_driver->getSceneManager()
                ->addAnimatedMeshSceneNode((scene::IAnimatedMesh*)mesh.get(n),
                                           m_rtt_main_node                   );
            node->setPosition( mesh_location[n].toIrrVector() );
            node->setFrameLoop(model_frames[n], model_frames[n]);
            node->setAnimationSpeed(0);
            node->updateAbsolutePosition();
            node->setScale( mesh_scale[n].toIrrVector() );
            //Log::info("RTTProvider::setupRTTScene", "Set frame %d", model_frames[n]);
        }
    }

    irr_driver->getSceneManager()->setAmbientLight(video::SColor(255, 35, 35, 35) );

    const core::vector3df &spot_pos = core::vector3df(0, 30, 40);
    m_light = irr_driver->getSceneManager()
        ->addLightSceneNode(NULL, spot_pos, video::SColorf(1.0f,1.0f,1.0f),
                            1600 /* radius */);
    m_light->setLightType(video::ELT_SPOT);
    m_light->setRotation((core::vector3df(0, 10, 0) - spot_pos).getHorizontalAngle());
    m_light->updateAbsolutePosition();

    m_rtt_main_node->setMaterialFlag(video::EMF_GOURAUD_SHADING , true);
    m_rtt_main_node->setMaterialFlag(video::EMF_LIGHTING, true);

    const int materials = m_rtt_main_node->getMaterialCount();
    for (int n=0; n<materials; n++)
    {
        m_rtt_main_node->getMaterial(n).setFlag(video::EMF_LIGHTING, true);

         // set size of specular highlights
        m_rtt_main_node->getMaterial(n).Shininess = 100.0f;
        m_rtt_main_node->getMaterial(n).SpecularColor.set(255,50,50,50);
        m_rtt_main_node->getMaterial(n).DiffuseColor.set(255,150,150,150);

        m_rtt_main_node->getMaterial(n).setFlag(video::EMF_GOURAUD_SHADING ,
                                                true);
    }

    m_camera =  irr_driver->getSceneManager()->addCameraSceneNode();

    m_camera->setPosition( core::vector3df(0.0, 20.0f, 70.0f) );
    if (CVS->isGLSL())
        m_camera->setUpVector( core::vector3df(0.0, 1.0, 0.0) );
    else
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
 *  \return the texture that was rendered to, or NULL if RTT does not work on
 *          this computer
 */
video::ITexture* IrrDriver::RTTProvider::renderToTexture(float angle,
                                                         bool is_2d_render)
{
    // m_render_target_texture will be NULL if RTT doesn't work on this computer
    if (m_render_target_texture == NULL) return NULL;

    // Rendering a 2d only model (using direct opengl rendering)
    // does not work if setRenderTarget is called here again.
    // And rendering 3d only works if it is called here :(
    if(!is_2d_render)
        m_video_driver->setRenderTarget(m_render_target_texture);

    if (angle != -1 && m_rtt_main_node != NULL)
        m_rtt_main_node->setRotation( core::vector3df(0, angle, 0) );

    video::SOverrideMaterial &overridemat = m_video_driver->getOverrideMaterial();
    overridemat.EnablePasses = scene::ESNRP_SOLID;
    overridemat.EnableFlags = video::EMF_MATERIAL_TYPE;
    overridemat.Material.MaterialType = video::EMT_SOLID;

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

    overridemat.EnablePasses = 0;

    m_video_driver->setRenderTarget(0, false, false);
    return m_render_target_texture;
}

// ----------------------------------------------------------------------------

void IrrDriver::applyObjectPassShader(scene::ISceneNode * const node, bool rimlit)
{
    if (!CVS->isGLSL())
        return;

    // Don't override sky
    if (node->getType() == scene::ESNT_SKY_DOME ||
        node->getType() == scene::ESNT_SKY_BOX)
        return;

    const u32 mcount = node->getMaterialCount();
    u32 i;
    const video::E_MATERIAL_TYPE ref =
        Shaders::getShader(rimlit ? ES_OBJECTPASS_RIMLIT : ES_OBJECTPASS_REF);
    const video::E_MATERIAL_TYPE pass =
        Shaders::getShader(rimlit ? ES_OBJECTPASS_RIMLIT : ES_OBJECTPASS);

    const video::E_MATERIAL_TYPE origref = Shaders::getShader(ES_OBJECTPASS_REF);
    const video::E_MATERIAL_TYPE origpass = Shaders::getShader(ES_OBJECTPASS);

    bool viamb = false;
    scene::IMesh *mesh = NULL;
    if (node->getType() == scene::ESNT_ANIMATED_MESH)
    {
        viamb = ((scene::IAnimatedMeshSceneNode *) node)->isReadOnlyMaterials();
        mesh = ((scene::IAnimatedMeshSceneNode *) node)->getMesh();
    }
    else if (node->getType() == scene::ESNT_MESH)
    {
        viamb = ((scene::IMeshSceneNode *) node)->isReadOnlyMaterials();
        mesh = ((scene::IMeshSceneNode *) node)->getMesh();
    }
    //else if (node->getType() == scene::ESNT_WATER_SURFACE)
    //{
    //    viamb = (dynamic_cast<scene::IMeshSceneNode*>(node))->isReadOnlyMaterials();
    //    mesh = (dynamic_cast<scene::IMeshSceneNode*>(node))->getMesh();
    //}

    for (i = 0; i < mcount; i++)
    {
        video::SMaterial &nodemat = node->getMaterial(i);
        video::SMaterial &mbmat = mesh ? mesh->getMeshBuffer(i)->getMaterial()
                                       : nodemat;
        video::SMaterial *mat = &nodemat;

        if (viamb)
            mat = &mbmat;

        if (mat->MaterialType == video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF ||
            mat->MaterialType == origref)
            mat->MaterialType = ref;
        else if (mat->MaterialType == video::EMT_SOLID ||
                 mat->MaterialType == origpass ||
                 (mat->MaterialType >= video::EMT_LIGHTMAP &&
                 mat->MaterialType <= video::EMT_LIGHTMAP_LIGHTING_M4))
            mat->MaterialType = pass;
    }


    core::list<scene::ISceneNode*> kids = node->getChildren();
    scene::ISceneNodeList::Iterator it = kids.begin();
    for (; it != kids.end(); ++it)
    {
        applyObjectPassShader(*it, rimlit);
    }
}

// ----------------------------------------------------------------------------

void IrrDriver::applyObjectPassShader()
{
    if (!CVS->isGLSL())
        return;

    applyObjectPassShader(m_scene_manager->getRootSceneNode());
}

// ----------------------------------------------------------------------------

scene::ISceneNode *IrrDriver::addLight(const core::vector3df &pos,
                                       float energy, float radius,
                                       float r, float g, float b,
                                       bool sun, scene::ISceneNode* parent)
{
    if (CVS->isGLSL())
    {
        if (parent == NULL) parent = m_scene_manager->getRootSceneNode();
        LightNode *light = NULL;

        if (!sun)
            light = new LightNode(m_scene_manager, parent, energy, radius,
                                  r, g, b);
        else
            light = new SunNode(m_scene_manager, parent, r, g, b);

        light->grab();

        light->setPosition(pos);
        light->updateAbsolutePosition();

        m_lights.push_back(light);

        if (sun)
        {
            //m_sun_interposer->setPosition(pos);
            //m_sun_interposer->updateAbsolutePosition();
            m_shadow_matrices->addLight(pos);

            ((WaterShaderProvider *) Shaders::getCallback(ES_WATER) )
                                                         ->setSunPosition(pos);
        }

        return light;
    }
    else
    {
        return m_scene_manager
               ->addLightSceneNode(m_scene_manager->getRootSceneNode(),
                                   pos, video::SColorf(1.0f, r, g, b));
    }
}   // addLight

// ----------------------------------------------------------------------------

void IrrDriver::clearLights()
{
    u32 i;
    const u32 max = (int)m_lights.size();
    for (i = 0; i < max; i++)
    {
        m_lights[i]->drop();
    }

    m_lights.clear();
}   // clearLights

// ----------------------------------------------------------------------------

GLuint IrrDriver::getRenderTargetTexture(TypeRTT which)
{
    return m_rtts->getRenderTarget(which);
}   // getRenderTargetTexture

// ----------------------------------------------------------------------------

FrameBuffer& IrrDriver::getFBO(TypeFBO which)
{
    return m_rtts->getFBO(which);
}   // getFBO

// ----------------------------------------------------------------------------

GLuint IrrDriver::getDepthStencilTexture()
{
    return m_rtts->getDepthStencilTexture();
}   // getDepthStencilTexture

