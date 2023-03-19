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

#include "challenges/story_mode_timer.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "font/bold_face.hpp"
#include "font/digit_face.hpp"
#include "font/font_manager.hpp"
#include "font/regular_face.hpp"
#include "graphics/2dutils.hpp"
#include "graphics/b3d_mesh_loader.hpp"
#include "graphics/camera.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/fixed_pipeline_renderer.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/graphics_restrictions.hpp"
#include "graphics/light.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/particle_kind_manager.hpp"
#include "graphics/per_camera_node.hpp"
#include "graphics/referee.hpp"
#include "graphics/render_target.hpp"
#include "graphics/rtts.hpp"
#include "graphics/shader.hpp"
#include "graphics/shader_based_renderer.hpp"
#include "graphics/shader_files_manager.hpp"
#include "graphics/shared_gpu_objects.hpp"
#include "graphics/sp_mesh_loader.hpp"
#include "graphics/sp/sp_base.hpp"
#include "graphics/sp/sp_dynamic_draw_call.hpp"
#include "graphics/sp/sp_mesh.hpp"
#include "graphics/sp/sp_mesh_node.hpp"
#include "graphics/sp/sp_shader_manager.hpp"
#include "graphics/sp/sp_texture_manager.hpp"
#include "graphics/stk_text_billboard.hpp"
#include "graphics/stk_tex_manager.hpp"
#include "graphics/sun.hpp"
#include "guiengine/dialog_queue.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/message_queue.hpp"
#include "guiengine/modaldialog.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/screen_keyboard.hpp"
#include "guiengine/skin.hpp"
#include "io/file_manager.hpp"
#include "items/item_manager.hpp"
#include "items/powerup_manager.hpp"
#include "items/attachment_manager.hpp"
#include "items/projectile_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/kart_properties_manager.hpp"
#include "main_loop.hpp"
#include "modes/world.hpp"
#include "network/network_config.hpp"
#include "network/stk_host.hpp"
#include "network/stk_peer.hpp"
#include "physics/physics.hpp"
#include "scriptengine/property_animator.hpp"
#include "states_screens/dialogs/confirm_resolution_dialog.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "states_screens/state_manager.hpp"
#include "tracks/track_manager.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"
#include "utils/file_utils.hpp"
#include "utils/log.hpp"
#include "utils/profiler.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"
#include "utils/vs.hpp"

#include <irrlicht.h>

#if !defined(SERVER_ONLY) && defined(ANDROID)
#include <SDL.h>
#if SDL_VERSION_ATLEAST(2, 0, 9)
#define ENABLE_SCREEN_ORIENTATION_HANDLING 1
#endif
#endif

#ifndef SERVER_ONLY
#include <ge_main.hpp>
#include <ge_vulkan_driver.hpp>
#include <ge_vulkan_texture_descriptor.hpp>
#endif

#ifdef ENABLE_RECORDER
#include <chrono>
#include <openglrecorder.h>
#endif

/* Build-time check that the Irrlicht we're building against works for us.
 * Should help prevent distros building against an incompatible library.
 */

#if (  IRRLICHT_VERSION_MAJOR < 1                   || \
       IRRLICHT_VERSION_MINOR < 7                   || \
      _IRR_MATERIAL_MAX_TEXTURES_ < 8               || \
      ( !defined(_IRR_COMPILE_WITH_OPENGL_) &&         \
        !defined(SERVER_ONLY)               &&         \
        !defined(_IRR_COMPILE_WITH_OGLES2_)       ) || \
      !defined(_IRR_COMPILE_WITH_B3D_LOADER_)             )
#error "Building against an incompatible Irrlicht. Distros, \
please use the included version."
#endif

using namespace irr;

/** singleton */
IrrDriver *irr_driver = NULL;

#ifndef SERVER_ONLY
GPUTimer* m_perf_query[Q_LAST];
static const char* m_perf_query_phase[Q_LAST] =
{
    "Shadows Cascade 0",
    "Shadows Cascade 1",
    "Shadows Cascade 2",
    "Shadows Cascade 3",
    "Solid Pass",
    "Env Map",
    "SunLight",
    "PointLights",
    "SSAO",
    "Light Scatter",
    "Glow",
    "Combine Diffuse Color",
    "Skybox",
    "Transparent",
    "Particles",
    "Depth of Field",
    "Godrays",
    "Bloom",
    "Tonemap",
    "Motion Blur",
    "Lightning",
    "MLAA",
    "GUI",
};
#endif

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
    m_logger_level = irr::ELL_WARNING;
    m_screen_orientation = -1;
    m_render_nw_debug = false;
    m_resolution_changing = RES_CHANGE_NONE;

    struct irr::SIrrlichtCreationParameters p;
#ifdef __SWITCH__
    // Switch doesn't like multiple window create/closes, so we hardcode it
    // Aforementioned chicken and egg problem isn't an issue because switch's SDL only supports two resolutions
    p.DriverType    = video::EDT_OPENGL;
    p.Bits          = 24U;
    p.WindowSize    = core::dimension2d<u32>(1280,720);
    p.ForceLegacyDevice = UserConfigParams::m_force_legacy_device;
#else
    p.DriverType    = video::EDT_NULL;
    p.Bits          = 16U;
    p.WindowSize    = core::dimension2d<u32>(640,480);
#endif
    p.Fullscreen    = false;
    p.SwapInterval  = 0;
    p.EventReceiver = NULL;
    p.FileSystem    = file_manager->getFileSystem();
    p.PrivateData   = NULL;

    m_device = createDeviceEx(p);

    m_request_screenshot = false;
    m_renderer            = NULL;
    m_wind                = new Wind();
    m_ssaoviz = false;
    m_shadowviz = false;
    m_boundingboxesviz = false;
    m_last_light_bucket_distance = 0;
    m_clear_color                = video::SColor(255, 100, 101, 140);
    m_skinning_joint             = 0;
    m_recording = false;
    m_sun_interposer = NULL;
    m_scene_complexity           = 0;

#ifndef SERVER_ONLY
    for (unsigned i = 0; i < Q_LAST; i++)
    {
        m_perf_query[i] = new GPUTimer(m_perf_query_phase[i]);
    }
#endif
}   // IrrDriver

// ----------------------------------------------------------------------------
/** Destructor - removes the irrlicht device.
 */
IrrDriver::~IrrDriver()
{
#ifdef ENABLE_RECORDER
    ogrDestroy();
#endif
    STKTexManager::getInstance()->kill();
    delete m_wind;
    delete m_renderer;
#ifndef SERVER_ONLY
    for (unsigned i = 0; i < Q_LAST; i++)
    {
        delete m_perf_query[i];
    }
#endif
    assert(m_device != NULL);
    m_device->drop();
    m_device = NULL;
    m_modes.clear();
}   // ~IrrDriver

// ----------------------------------------------------------------------------
const char* IrrDriver::getGPUQueryPhaseName(unsigned q)
{
#ifndef SERVER_ONLY
    assert(q < Q_LAST);
    return m_perf_query_phase[q];
#else
    return "";
#endif
}   // getGPUQueryPhaseName

// ----------------------------------------------------------------------------
/** Called before a race is started, after all cameras are set up.
 */
void IrrDriver::reset()
{
#ifndef SERVER_ONLY
    m_renderer->resetPostProcessing();
#endif
}   // reset

// ----------------------------------------------------------------------------
core::array<video::IRenderTarget> &IrrDriver::getMainSetup()
{
  return m_mrt;
}   // getMainSetup

// ----------------------------------------------------------------------------

#ifndef SERVER_ONLY

GPUTimer &IrrDriver::getGPUTimer(unsigned i)
{
    return *m_perf_query[i];
}   // getGPUTimer
#endif
// ----------------------------------------------------------------------------

#ifndef SERVER_ONLY
std::unique_ptr<RenderTarget> IrrDriver::createRenderTarget(const irr::core::dimension2du &dimension,
                                                            const std::string &name)
{
    return m_renderer->createRenderTarget(dimension, name);
}   // createRenderTarget
#endif   // ~SERVER_ONLY

// ----------------------------------------------------------------------------
/** If the position of the window should be remembered, store it in the config
 *  file.
 *  \post The user config file must still be saved!
 */
void IrrDriver::updateConfigIfRelevant()
{
#ifndef SERVER_ONLY
    if (!UserConfigParams::m_fullscreen &&
         UserConfigParams::m_remember_window_location)
    {
        int x = 0;
        int y = 0;
        
        bool success = m_device->getWindowPosition(&x, &y);
        
        if (!success)
        {
            Log::warn("irr_driver", "Could not retrieve window location");
            return;
        }
        
        Log::verbose("irr_driver", "Retrieved window location for config: "
                                   "%i %i", x, y);
                                   
        // If the windows position is saved, it must be a non-negative
        // number. So if the window is partly off screen, move it to the
        // corresponding edge.
        UserConfigParams::m_window_x = std::max(0, x);
        UserConfigParams::m_window_y = std::max(0, y);
    }
#endif   // !SERVER_ONLY
}   // updateConfigIfRelevant
core::recti IrrDriver::getSplitscreenWindow(int WindowNum) 
{
    const int playernum = RaceManager::get()->getNumLocalPlayers();
    const float playernum_sqrt = sqrtf((float)playernum);
    
    int rows = int(  UserConfigParams::split_screen_horizontally
                   ? ceil(playernum_sqrt)
                   : round(playernum_sqrt)                       );
    int cols = int(  UserConfigParams::split_screen_horizontally
                   ? round(playernum_sqrt)
                   : ceil(playernum_sqrt)                        );
    
    if (rows == 0){rows = 1;}
    if (cols == 0) {cols = 1;}
    //This could add a bit of overhang
    const int width_of_space =
        int(ceil(   (float)irr_driver->getActualScreenSize().Width
                  / (float)cols)                                  );
    const int height_of_space =
        int (ceil(  (float)irr_driver->getActualScreenSize().Height
                  / (float)rows)                                   );

    const int x_grid_Position = WindowNum % cols;
    const int y_grid_Position = int(floor((WindowNum) / cols));

//To prevent the viewport going over the right side, we use std::min to ensure the right corners are never larger than the total width
    return core::recti(
        x_grid_Position * width_of_space,
        y_grid_Position * height_of_space,
        (x_grid_Position * width_of_space) + width_of_space,
        (y_grid_Position * height_of_space) + height_of_space);
}
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
#ifndef MOBILE_STK
            // Mobile STK reports only 1 desktop (phone) resolution at native scale
            if ((h < MIN_SUPPORTED_HEIGHT || w < MIN_SUPPORTED_WIDTH) &&
                (!(h==600 && w==800 && UserConfigParams::m_artist_debug_mode) &&
                (!(h==720 && w==1280 && ALLOW_1280_X_720 == true))))
                continue;
#endif
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
#if !defined(SERVER_ONLY)
    std::string abs_shader_dir = file_manager->getFileSystem()
        ->getAbsolutePath(file_manager->getShadersDir().c_str()).c_str();
    GE::setShaderFolder(abs_shader_dir);
#endif
    SIrrlichtCreationParameters params;
    core::stringw display_msg;

    m_logger_level = irr::ELL_INFORMATION;
#ifndef __SWITCH__
    // If --no-graphics option was used, the null device can still be used.
    if (!GUIEngine::isNoGraphics())
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
        else if (!UserConfigParams::m_fullscreen &&
            (UserConfigParams::m_real_width > (int)ssize.Width ||
            UserConfigParams::m_real_height > (int)ssize.Height))
        {
            Log::warn("irr_driver", "The window size specified in "
                      "user config is larger than your screen!");
            UserConfigParams::m_real_width = (int)ssize.Width;
            UserConfigParams::m_real_height = (int)ssize.Height;
        }

        if (UserConfigParams::m_fullscreen)
        {
            if (modes->getVideoModeCount() > 0)
            {
                core::dimension2d<u32> res = core::dimension2du(
                                                    UserConfigParams::m_real_width,
                                                    UserConfigParams::m_real_height);
                res = modes->getVideoModeResolution(res, res);

                UserConfigParams::m_real_width = res.Width;
                UserConfigParams::m_real_height = res.Height;
            }
            else
            {
                Log::warn("irr_driver", "Cannot get information about "
                          "resolutions. Disable fullscreen.");
                UserConfigParams::m_fullscreen = false;
            }
        }

        if (UserConfigParams::m_real_width < 1 || UserConfigParams::m_real_height < 1)
        {
            Log::warn("irr_driver", "Invalid window size. "
                         "Try to use the default one.");
            UserConfigParams::m_real_width = MIN_SUPPORTED_WIDTH;
            UserConfigParams::m_real_height = MIN_SUPPORTED_HEIGHT;
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

begin:
#if !defined(SERVER_ONLY)
        GE::setVideoDriver(NULL);
#endif

        video::E_DRIVER_TYPE driver_created = video::EDT_NULL;
        if (std::string(UserConfigParams::m_render_driver) == "gl")
        {
#if defined(USE_GLES2)
            driver_created = video::EDT_OGLES2;
#else
            driver_created = video::EDT_OPENGL;
#endif
        }
        else if (std::string(UserConfigParams::m_render_driver) == "directx9")
        {
            driver_created = video::EDT_DIRECT3D9;
        }
        else if (std::string(UserConfigParams::m_render_driver) == "vulkan")
        {
            driver_created = video::EDT_VULKAN;
#ifndef SERVER_ONLY
            GE::getGEConfig()->m_texture_compression =
                UserConfigParams::m_texture_compression;
            GE::getGEConfig()->m_render_scale =
                UserConfigParams::m_scale_rtts_factor;
            GE::getGEConfig()->m_pbr =
                UserConfigParams::m_dynamic_lights;
#endif
        }
        else
        {
            Log::warn("IrrDriver", "Unknown driver %s, revert to gl",
                UserConfigParams::m_render_driver.c_str());
            UserConfigParams::m_render_driver.revertToDefaults();
#if defined(USE_GLES2)
            driver_created = video::EDT_OGLES2;
#else
            driver_created = video::EDT_OPENGL;
#endif
        }

#ifndef SERVER_ONLY
        GE::getGEConfig()->m_fullscreen_desktop =
            (driver_created == video::EDT_VULKAN &&
            UserConfigParams::m_vulkan_fullscreen_desktop) ||
            UserConfigParams::m_non_ge_fullscreen_desktop;
#endif
        if (UserConfigParams::m_swap_interval > 1)
            UserConfigParams::m_swap_interval = 1;

        // Try 32 and, upon failure, 24 then 16 bit per pixels
        for (int bits=32; bits>15; bits -=8)
        {
            if(UserConfigParams::logMisc())
                Log::verbose("irr_driver", "Trying to create device with "
                             "%i bits\n", bits);

            params.DriverType    = driver_created;
            params.PrivateData   = NULL;
            params.Stencilbuffer = false;
            params.Bits          = bits;
            params.EventReceiver = this;
            params.Fullscreen    = UserConfigParams::m_fullscreen;
            params.SwapInterval  = UserConfigParams::m_swap_interval;
            params.FileSystem    = file_manager->getFileSystem();
            params.WindowSize    =
                core::dimension2du(UserConfigParams::m_real_width,
                                   UserConfigParams::m_real_height);
            params.HandleSRGB    = false;
            params.ShadersPath   = (file_manager->getShadersDir() +
                                                           "irrlicht/").c_str();
            // Set window to remembered position
            if (  !UserConfigParams::m_fullscreen
                && UserConfigParams::m_remember_window_location
                && UserConfigParams::m_window_x >= 0
                && UserConfigParams::m_window_y >= 0            )
            {
                params.WindowPosition.X = UserConfigParams::m_window_x;
                params.WindowPosition.Y = UserConfigParams::m_window_y;
            }

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
            if (!m_device && driver_created == video::EDT_VULKAN)
            {
                display_msg = L"Vulkan unsupported";
                UserConfigParams::m_render_driver.revertToDefaults();
                goto begin;
            }

            if(m_device)
                break;
        }   // for bits=32, 24, 16


        // if still no device, try with a default screen size, maybe
        // size is the problem
        if(!m_device)
        {
            UserConfigParams::m_real_width  = MIN_SUPPORTED_WIDTH;
            UserConfigParams::m_real_height = MIN_SUPPORTED_HEIGHT;
            m_device = createDevice(driver_created,
                        core::dimension2du(UserConfigParams::m_real_width,
                                           UserConfigParams::m_real_height ),
                                    32, //bits per pixel
                                    UserConfigParams::m_fullscreen,
                                    false,  // stencil buffers
                                    0,  // vsync
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
#endif // __SWITCH__

    if(!m_device)
    {
        Log::fatal("irr_driver", "Couldn't initialise irrlicht device. Quitting.\n");
    }
    m_actual_screen_size = m_device->getVideoDriver()->getCurrentRenderTargetSize();
    UserConfigParams::m_width = m_actual_screen_size.Width;
    UserConfigParams::m_height = m_actual_screen_size.Height;
    UserConfigParams::m_real_width = (unsigned)((float)UserConfigParams::m_width / m_device->getNativeScaleX());
    UserConfigParams::m_real_height = (unsigned)((float)UserConfigParams::m_height / m_device->getNativeScaleY());

#ifndef SERVER_ONLY 

    GE::setVideoDriver(m_device->getVideoDriver());

    GE::GEVulkanDriver* vk = GE::getVKDriver();
    if (vk)
    {
        GE::GEVulkanTextureDescriptor* td = vk->getMeshTextureDescriptor();
        switch (UserConfigParams::m_anisotropic)
        {
        case 16:
            td->setSamplerUse(GE::GVS_3D_MESH_MIPMAP_16);
            break;
        case 4:
            td->setSamplerUse(GE::GVS_3D_MESH_MIPMAP_4);
            break;
        case 2:
            td->setSamplerUse(GE::GVS_3D_MESH_MIPMAP_2);
            break;
        default:
            Log::warn("irr_driver", "Unsupported anisotropic values, revert");
            UserConfigParams::m_anisotropic = 16;
            td->setSamplerUse(GE::GVS_3D_MESH_MIPMAP_16);
            break;
        }
    }
    // Assume sp is supported
    CentralVideoSettings::m_supports_sp = true;
    CVS->init();

    bool recreate_device = false;

    // Some drivers are able to create OpenGL 3.1 context, but shader-based
    // pipeline doesn't work for them. For example some radeon drivers
    // support only GLSL 1.3 and it causes STK to crash. We should force to use
    // fixed pipeline in this case.
    if (!GUIEngine::isNoGraphics() &&
        (GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_FORCE_LEGACY_DEVICE) ||
        (CVS->isGLSL() && !CentralVideoSettings::m_supports_sp)))
    {
        Log::warn("irr_driver", "Driver doesn't support shader-based pipeline. "
                                "Re-creating device to workaround the issue.");

        params.ForceLegacyDevice = true;
        recreate_device = true;
    }
#endif

    // Don't recreate on switch!
#if !defined(SERVER_ONLY) && !defined(__SWITCH__)
    if (!GUIEngine::isNoGraphics() && recreate_device)
    {
        m_device->closeDevice();
        m_device->clearSystemMessages();
        m_device->run();
        m_device->drop();
        GE::setVideoDriver(NULL);

        m_device = createDeviceEx(params);

        if(!m_device)
        {
            Log::fatal("irr_driver", "Couldn't initialise irrlicht device. Quitting.\n");
        }

        GE::setVideoDriver(m_device->getVideoDriver());
        CVS->init();
    }
#endif
    m_logger_level = irr::ELL_WARNING;

    m_scene_manager = m_device->getSceneManager();
    m_gui_env       = m_device->getGUIEnvironment();
    m_video_driver  = m_device->getVideoDriver();

    B3DMeshLoader* b3dl = new B3DMeshLoader(m_scene_manager);
    m_scene_manager->addExternalMeshLoader(b3dl);
    b3dl->drop();

    SPMeshLoader* spml = new SPMeshLoader(m_scene_manager);
    m_scene_manager->addExternalMeshLoader(spml);
    spml->drop();

#ifdef ENABLE_RECORDER
    ogrRegGeneralCallback(OGR_CBT_START_RECORDING,
        [] (void* user_data)
        {
            // Make sure reset progress bar each time
            MessageQueue::showProgressBar(-1, L"");
            MessageQueue::add
            (MessageQueue::MT_GENERIC, _("Video recording started."));
        }, NULL);
    ogrRegStringCallback(OGR_CBT_ERROR_RECORDING,
        [](const char* s, void* user_data)
        { Log::error("openglrecorder", "%s", s); }, NULL);
    ogrRegStringCallback(OGR_CBT_SAVED_RECORDING,
        [] (const char* s, void* user_data) { MessageQueue::add
        (MessageQueue::MT_GENERIC, _("Video saved in \"%s\".", s));
        }, NULL);
    ogrRegIntCallback(OGR_CBT_PROGRESS_RECORDING,
        [] (const int i, void* user_data)
        { MessageQueue::showProgressBar(i, _("Encoding progress:")); }, NULL);

    RecorderConfig cfg;
    cfg.m_triple_buffering = 1;
    cfg.m_record_audio = 1;
    cfg.m_width = m_actual_screen_size.Width;
    cfg.m_height = m_actual_screen_size.Height;
    int vf = UserConfigParams::m_video_format;
    cfg.m_video_format = (VideoFormat)vf;
    cfg.m_audio_format = OGR_AF_VORBIS;
    cfg.m_audio_bitrate = UserConfigParams::m_audio_bitrate;
    cfg.m_video_bitrate = UserConfigParams::m_video_bitrate;
    cfg.m_record_fps = UserConfigParams::m_record_fps;
    cfg.m_record_jpg_quality = UserConfigParams::m_recorder_jpg_quality;
    if (ogrInitConfig(&cfg) == 0)
    {
        Log::error("irr_driver",
            "RecorderConfig is invalid, use the default one.");
    }

    ogrRegReadPixelsFunction([]
        (int x, int y, int w, int h, unsigned int f, unsigned int t, void* d)
        { glReadPixels(x, y, w, h, f, t, d); });

#ifndef USE_GLES2
    ogrRegPBOFunctions([](int n, unsigned int* b) { glGenBuffers(n, b); },
        [](unsigned int t, unsigned int b) { glBindBuffer(t, b); },
        [](unsigned int t, ptrdiff_t s, const void* d, unsigned int u)
        { glBufferData(t, s, d, u); },
        [](int n, const unsigned int* b) { glDeleteBuffers(n, b); },
        [](unsigned int t, unsigned int a) { return glMapBuffer(t, a); },
        [](unsigned int t) { return glUnmapBuffer(t); });
#else
    ogrRegPBOFunctionsRange([](int n, unsigned int* b) { glGenBuffers(n, b); },
        [](unsigned int t, unsigned int b) { glBindBuffer(t, b); },
        [](unsigned int t, ptrdiff_t s, const void* d, unsigned int u)
        { glBufferData(t, s, d, u); },
        [](int n, const unsigned int* b) { glDeleteBuffers(n, b); },
        [](unsigned int t, ptrdiff_t o, ptrdiff_t l, unsigned int a) 
        { return glMapBufferRange(t, o, l, a); },
        [](unsigned int t) { return glUnmapBuffer(t); });
#endif

#endif

#ifndef SERVER_ONLY
    if (CVS->isGLSL())
    {
        m_renderer = new ShaderBasedRenderer();
        preloadShaders();
    }
    else
        m_renderer = new FixedPipelineRenderer();
#endif

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

#ifndef SERVER_ONLY
    if (CVS->isGLSL())
    {
        Log::info("irr_driver", "GLSL supported.");
    }

    // m_glsl might be reset in rtt if an error occurs.
    if (CVS->isGLSL())
    {
        m_mrt.clear();
        m_mrt.reallocate(2);
    }
    else
    {
        Log::warn("irr_driver", "Using the fixed pipeline (old GPU, or "
                                "shaders disabled in options)");
    }
#endif

    // Only change video driver settings if we are showing graphics
    if (!GUIEngine::isNoGraphics())
    {
        m_device->setWindowClass("SuperTuxKart");
        m_device->setWindowCaption(L"SuperTuxKart");
        m_device->getVideoDriver()
            ->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, true);
        m_device->getVideoDriver()
            ->setTextureCreationFlag(video::ETCF_OPTIMIZED_FOR_QUALITY, true);

        // Force creation of mipmaps even if the mipmaps flag in a b3d file
        // does not set the 'enable mipmap' flag.
        m_scene_manager->getParameters()
            ->setAttribute(scene::B3D_LOADER_IGNORE_MIPMAP_FLAG, true);
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

#ifndef SERVER_ONLY
    // set cursor visible by default (what's the default is not too clearly documented,
    // so let's decide ourselves...)
    if (!GUIEngine::isNoGraphics())
        m_device->getCursorControl()->setVisible(true);
#endif
    m_pointer_shown = true;
#ifdef ENABLE_SCREEN_ORIENTATION_HANDLING
    m_screen_orientation = (int)SDL_GetDisplayOrientation(0);
#endif
    if (GUIEngine::isNoGraphics())
        return;

    if (!display_msg.empty())
    {
        MessageDialog *dialog =
        new MessageDialog(display_msg, /*from queue*/ true);
        GUIEngine::DialogQueue::get()->pushDialog(dialog);
    }
}   // initDevice

// ----------------------------------------------------------------------------
void IrrDriver::setMaxTextureSize()
{
    const unsigned max =
        (UserConfigParams::m_high_definition_textures & 0x01) == 0 ?
        UserConfigParams::m_max_texture_size : 2048;
    io::IAttributes &att = m_video_driver->getNonConstDriverAttributes();
    att.setAttribute("MAX_TEXTURE_SIZE", core::dimension2du(max, max));
}   // setMaxTextureSize

// ----------------------------------------------------------------------------
void IrrDriver::unsetMaxTextureSize()
{
    io::IAttributes &att = m_video_driver->getNonConstDriverAttributes();
    att.setAttribute("MAX_TEXTURE_SIZE", core::dimension2du(2048, 2048));
}   // unsetMaxTextureSize

// ----------------------------------------------------------------------------
void IrrDriver::cleanSunInterposer()
{
    delete m_sun_interposer;
    m_sun_interposer = NULL;
}   // cleanSunInterposer

// ----------------------------------------------------------------------------
void IrrDriver::createSunInterposer()
{
#ifndef SERVER_ONLY
    scene::IMesh * sphere = m_scene_manager->getGeometryCreator()
        ->createSphereMesh(1, 16, 16);
    Material* material = material_manager->getDefaultSPMaterial("solid");
    m_sun_interposer = new SP::SPDynamicDrawCall
        (scene::EPT_TRIANGLES, NULL/*shader*/, material);
    for (unsigned i = 0; i < sphere->getMeshBufferCount(); i++)
    {
        scene::IMeshBuffer* mb = sphere->getMeshBuffer(i);
        if (!mb)
        {
            continue;
        }
        assert(mb->getVertexType() == video::EVT_STANDARD);
        video::S3DVertex* v_ptr = (video::S3DVertex*)mb->getVertices();
        uint16_t* idx_ptr = mb->getIndices();
        for (unsigned j = 0; j < mb->getIndexCount(); j++)
        {
            // For sun interposer we only need position for glow shader
            video::S3DVertexSkinnedMesh sp;
            const unsigned v_idx = idx_ptr[j];
            sp.m_position = v_ptr[v_idx].Pos;
            m_sun_interposer->addSPMVertex(sp);
        }
    }
    m_sun_interposer->recalculateBoundingBox();
    m_sun_interposer->setPosition(Track::getCurrentTrack()
        ->getGodRaysPosition());
    m_sun_interposer->setScale(core::vector3df(20));
    sphere->drop();
#endif
}

//-----------------------------------------------------------------------------
void IrrDriver::getOpenGLData(std::string *vendor, std::string *renderer,
                              std::string *version)
{
#ifndef SERVER_ONLY
    if (GUIEngine::isNoGraphics())
        return;
        
    *vendor   = (char*)glGetString(GL_VENDOR  );
    *renderer = (char*)glGetString(GL_RENDERER);
    *version  = (char*)glGetString(GL_VERSION );
#endif
}   // getOpenGLData

//-----------------------------------------------------------------------------
void IrrDriver::showPointer()
{
#ifndef SERVER_ONLY
    if (GUIEngine::isNoGraphics())
        return;

    if (!m_pointer_shown)
    {
        m_pointer_shown = true;
        this->getDevice()->getCursorControl()->setVisible(true);
    }
#endif
}   // showPointer

//-----------------------------------------------------------------------------
void IrrDriver::hidePointer()
{
#ifndef SERVER_ONLY
    if (GUIEngine::isNoGraphics())
        return;

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
#endif
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
#ifndef SERVER_ONLY
    bool success = m_device->moveWindow(x, y);
    
    if (!success)
    {
        Log::warn("irr_driver", "Could not set window location\n");
        return false;
    }
#endif
    return true;
}
//-----------------------------------------------------------------------------

void IrrDriver::changeResolution(const int w, const int h,
                                 const bool fullscreen)
{
    // update user config values
    UserConfigParams::m_prev_real_width = UserConfigParams::m_real_width;
    UserConfigParams::m_prev_real_height = UserConfigParams::m_real_height;
    UserConfigParams::m_prev_fullscreen = UserConfigParams::m_fullscreen;

    UserConfigParams::m_real_width = w;
    UserConfigParams::m_real_height = h;
    UserConfigParams::m_fullscreen = fullscreen;

    // Setting this flag will trigger a call to applyResolutionSetting()
    // in the next update call. This avoids the problem that changeResolution
    // is actually called from the gui, i.e. the event loop, i.e. while the
    // old device is active - so we can't delete this device (which we must
    // do in applyResolutionSettings
    if (w < 1024 || h < 720)
        m_resolution_changing = RES_CHANGE_YES_WARN;
    else
        m_resolution_changing = RES_CHANGE_YES;
}   // changeResolution

//-----------------------------------------------------------------------------

void IrrDriver::applyResolutionSettings(bool recreate_device)
{
#ifndef SERVER_ONLY
    // show black before resolution switch so we don't see OpenGL's buffer
    // garbage during switch
    if (recreate_device)
    {
        m_video_driver->beginScene(true, true, video::SColor(255,100,101,140));
        GL32_draw2DRectangle( video::SColor(255, 0, 0, 0), core::rect<s32>(0, 0,
            s32(m_device->getNativeScaleX() * (float)UserConfigParams::m_prev_real_width),
            s32(m_device->getNativeScaleY() * (float)UserConfigParams::m_prev_real_height)) );
        m_video_driver->endScene();
    }
    track_manager->removeAllCachedData();
    delete attachment_manager;
    attachment_manager = NULL;
    ProjectileManager::get()->removeTextures();
    ItemManager::removeTextures();
    kart_properties_manager->unloadAllKarts();
    delete powerup_manager;
    powerup_manager = NULL;
    Referee::cleanup();
    ParticleKindManager::get()->cleanup();
    delete input_manager;
    input_manager = NULL;
    delete font_manager;
    font_manager = NULL;
    GUIEngine::clear();
    GUIEngine::cleanUp();

    if (recreate_device)
    {
        m_device->closeDevice();
        m_device->clearSystemMessages();
        m_device->run();
    }
    delete material_manager;
    material_manager = NULL;

    // ---- Reinit
    // FIXME: this load sequence is (mostly) duplicated from main.cpp!!
    // That's just error prone
    // (we're sure to update main.cpp at some point and forget this one...)
    STKTexManager::getInstance()->kill();
#ifdef ENABLE_RECORDER
    if (recreate_device)
    {
        ogrDestroy();
        m_recording = false;
    }
#endif
    // initDevice will drop the current device.
    if (recreate_device)
    {
        delete m_renderer;
        m_renderer = NULL;
        SharedGPUObjects::reset();

        SP::setMaxTextureSize();
        initDevice();
    }
#ifndef SERVER_ONLY
    for (unsigned i = 0; i < Q_LAST; i++)
    {
        m_perf_query[i]->reset();
    }
    if (!recreate_device)
    {
        SP::SPTextureManager::get()->stopThreads();
        SP::SPShaderManager::destroy();
        SP::SPTextureManager::destroy();
        ShaderBase::killShaders();
        ShaderFilesManager::getInstance()->removeAllShaderFiles();
        unsetMaxTextureSize();
        SP::setMaxTextureSize();
        m_renderer->createPostProcessing();
    }
    if (CVS->isGLSL())
        SP::loadShaders();
#endif

    font_manager = new FontManager(); // Fonts are loaded in GUIEngine::init

    input_manager = new InputManager();
    input_manager->setMode(InputManager::MENU);
    // Input manager set first so it recieves SDL joystick event
    // Re-init GUI engine
    GUIEngine::init(m_device, m_video_driver, StateManager::get());
    // If not recreate device we need to add the previous joystick manually
    if (!recreate_device)
        input_manager->addJoystick();

    setMaxTextureSize();
    //material_manager->reInit();
    material_manager = new MaterialManager();
    material_manager->loadMaterial();
    powerup_manager = new PowerupManager();
    attachment_manager = new AttachmentManager();

    GUIEngine::addLoadingIcon(
        irr_driver->getTexture(file_manager
                               ->getAsset(FileManager::GUI_ICON,"options_video.png"))
                             );

    file_manager->pushTextureSearchPath(file_manager->getAsset(FileManager::MODEL,""), "models");
    const std::string materials_file =
        file_manager->getAssetChecked(FileManager::MODEL, "materials.xml");
    if (materials_file != "")
    {
        material_manager->addSharedMaterial(materials_file);
    }

    powerup_manager->loadPowerupsModels();
    ItemManager::loadDefaultItemMeshes();
    ProjectileManager::get()->loadData();
    Referee::init();
    GUIEngine::addLoadingIcon(
        irr_driver->getTexture(file_manager->getAsset(FileManager::GUI_ICON,"gift.png")) );


    kart_properties_manager->loadAllKarts();
    kart_properties_manager->onDemandLoadKartTextures(
        { UserConfigParams::m_default_kart }, false/*unload_unused*/);

    attachment_manager->loadModels();
    file_manager->popTextureSearchPath();
    std::string banana = file_manager->getAsset(FileManager::GUI_ICON, "banana.png");
    GUIEngine::addLoadingIcon(irr_driver->getTexture(banana) );
    // No need to reload cached track data (track_manager->cleanAllCachedData
    // above) - this happens dynamically when the tracks are loaded.
    track_manager->updateScreenshotCache();
    GUIEngine::reshowCurrentScreen();
    MessageQueue::updatePosition();
    // Preload the explosion effects (explode.png)
    ParticleKindManager::get()->getParticles("explosion.xml");
#endif   // !SERVER_ONLY
}   // applyResolutionSettings

// ----------------------------------------------------------------------------

void IrrDriver::cancelResChange()
{
    UserConfigParams::m_real_width = UserConfigParams::m_prev_real_width;
    UserConfigParams::m_real_height = UserConfigParams::m_prev_real_height;
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
/** Sets the material flags in this mesh depending on the settings in
 *  material_manager.
 *  \param mesh  The mesh to change the settings in.
 */
void IrrDriver::setAllMaterialFlags(scene::IMesh *mesh) const
{
#ifndef SERVER_ONLY
    if (CVS->isGLSL())
    {
        return;
    }
#endif
    unsigned int n=mesh->getMeshBufferCount();
    for(unsigned int i=0; i<n; i++)
    {
        scene::IMeshBuffer *mb = mesh->getMeshBuffer(i);
        video::SMaterial &irr_material = mb->getMaterial();
        video::ITexture* t = irr_material.getTexture(0);
        if (t)
        {
            material_manager->setAllMaterialFlags(t, mb);
        }
        else
        {
            material_manager->setAllUntexturedMaterialFlags(mb);
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
scene::ISceneNode *IrrDriver::addSphere(float radius,
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
#ifndef SERVER_ONLY
    if (CVS->isGLSL())
    {
        SP::SPMesh* spm = SP::convertEVTStandard(mesh, &color);
        SP::SPMeshNode* spmn = new SP::SPMeshNode(spm,
            m_scene_manager->getRootSceneNode(), m_scene_manager, -1,
            "sphere");
        spmn->setMesh(spm);
        spm->drop();
        spmn->drop();
        return spmn;
    }
#endif

#ifndef SERVER_ONLY
    bool vk = (GE::getVKDriver() != NULL);
    if (vk)
        GE::getGEConfig()->m_convert_irrlicht_mesh = true;
#endif
    scene::IMeshSceneNode *node = m_scene_manager->addMeshSceneNode(mesh);
#ifndef SERVER_ONLY
    if (vk)
        GE::getGEConfig()->m_convert_irrlicht_mesh = false;
#endif

    mesh->drop();
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
scene::ISceneNode *IrrDriver::addMesh(scene::IMesh *mesh,
                                      const std::string& debug_name,
                                      scene::ISceneNode *parent,
                                      std::shared_ptr<GE::GERenderInfo> render_info)
{
#ifdef SERVER_ONLY
    return m_scene_manager->addMeshSceneNode(mesh, parent);
#else
    if (!CVS->isGLSL())
    {
        scene::ISceneNode* node = m_scene_manager->addMeshSceneNode(mesh, parent);
        node->resetFirstRenderInfo(render_info);
        return node;
    }

    if (!parent)
      parent = m_scene_manager->getRootSceneNode();

    scene::ISceneNode* node = NULL;
    SP::SPMesh* spm = dynamic_cast<SP::SPMesh*>(mesh);
    if (spm || mesh == NULL)
    {
        SP::SPMeshNode* spmn = new SP::SPMeshNode(spm, parent, m_scene_manager,
            -1, debug_name, core::vector3df(0, 0, 0), core::vector3df(0, 0, 0),
            core::vector3df(1.0f, 1.0f, 1.0f), render_info);
        spmn->setMesh(spm);
        spmn->setAnimationState(false);
        node = spmn;
    }
    else
    {
        Log::warn("IrrDriver", "Use only spm in glsl");
        return NULL;
    }
    node->drop();

    return node;
#endif
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
                                           const std::string& tex_name,
                                           scene::ISceneNode* parent)
{
    scene::IBillboardSceneNode* node;
    node = m_scene_manager->addBillboardSceneNode(parent, size);

    const bool full_path = tex_name.find('/') != std::string::npos;

    Material* m = material_manager->getMaterial(tex_name, full_path,
        /*make_permanent*/false, /*complain_if_not_found*/true,
        /*strip_path*/full_path, /*install*/false);

    video::ITexture* tex = m->getTexture(true/*srgb*/,
        m->getShaderName() == "additive" ||
        m->getShaderName() == "alphablend" ?
        true : false/*premul_alpha*/);

    assert(node->getMaterialCount() > 0);
    node->setMaterialTexture(0, tex);
    if (!(m->getShaderName() == "additive" ||
        m->getShaderName() == "alphablend"))
    {
        // Alpha test for billboard otherwise
        m->setShaderName("alphatest");
    }
    m->setMaterialProperties(&(node->getMaterial(0)), NULL);
    return node;
}   // addBillboard

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
    v1.Normal = core::vector3df(0, 0, 1);
    v1.TCoords = core::vector2d<f32>(1,1);
    v1.Color = video::SColor(255, 255, 255, 255);

    video::S3DVertex v2;
    v2.Pos    = core::vector3df(w_2,-h_2,0);
    v2.Normal = core::vector3df(0, 0, 1);
    v2.TCoords = core::vector2d<f32>(0,1);
    v2.Color = video::SColor(255, 255, 255, 255);

    video::S3DVertex v3;
    v3.Pos    = core::vector3df(w_2,h_2,0);
    v3.Normal = core::vector3df(0, 0, 1);
    v3.TCoords = core::vector2d<f32>(0,0);
    v3.Color = video::SColor(255, 255, 255, 255);

    video::S3DVertex v4;
    v4.Pos    = core::vector3df(-w_2,h_2,0);
    v4.Normal = core::vector3df(0, 0, 1);
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
    if (STKTexManager::getInstance()->removeTexture(t))
        return;
    m_video_driver->removeTexture(t);
}   // removeTexture

// ----------------------------------------------------------------------------
/** Adds an animated mesh to the scene.
 *  \param mesh The animated mesh to add.
 */
scene::IAnimatedMeshSceneNode *IrrDriver::addAnimatedMesh(scene::IAnimatedMesh *mesh,
    const std::string& debug_name, scene::ISceneNode* parent,
    std::shared_ptr<GE::GERenderInfo> render_info)
{
    scene::IAnimatedMeshSceneNode* node;
#ifndef SERVER_ONLY
    SP::SPMesh* spm = dynamic_cast<SP::SPMesh*>(mesh);
    if (CVS->isGLSL() && (spm || mesh == NULL))
    {
        if (!parent)
        {
            parent = m_scene_manager->getRootSceneNode();
        }
        SP::SPMeshNode* spmn = new SP::SPMeshNode(spm, parent, m_scene_manager,
            -1, debug_name, core::vector3df(0, 0, 0), core::vector3df(0, 0, 0),
            core::vector3df(1.0f, 1.0f, 1.0f), render_info);
        spmn->drop();
        spmn->setMesh(mesh);
        node = spmn;
    }
    else
#endif
    {
        node = m_scene_manager->addAnimatedMeshSceneNode(mesh, parent, -1,
            core::vector3df(0, 0, 0),
            core::vector3df(0, 0, 0),
            core::vector3df(1, 1, 1),
            /*addIfMeshIsZero*/true);
        node->resetFirstRenderInfo(render_info);
    }
    return node;

}   // addAnimatedMesh

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
    return m_scene_manager->addSkyBoxSceneNode(texture[0], texture[1],
                                               texture[2], texture[3],
                                               texture[4], texture[5]);
}   // addSkyBox

// ----------------------------------------------------------------------------
void IrrDriver::suppressSkyBox()
{
#ifndef SERVER_ONLY
    m_renderer->removeSkyBox();;
#endif
}   // suppressSkyBox

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
/** Loads a texture from a file and returns the texture object. This is just
 *  a convenient wrapper which loads the texture from a STK asset directory.
 *  It calls the file manager to get the full path, then calls the normal
 *  getTexture() function.s
 *  \param type The FileManager::AssetType of the texture.
 *  \param filename File name of the texture to load.
 */
video::ITexture *IrrDriver::getTexture(FileManager::AssetType type,
                                       const std::string &filename)
{
    const std::string path = file_manager->getAsset(type, filename);
    return getTexture(path);
}   // getTexture

// ----------------------------------------------------------------------------
/** Loads a texture from a file and returns the texture object.
 *  \param filename File name of the texture to load.
 */
video::ITexture *IrrDriver::getTexture(const std::string &filename)
{
    return STKTexManager::getInstance()->getTexture(filename);
}   // getTexture

// ----------------------------------------------------------------------------
/** Appends a pointer to each texture used in this mesh to the vector.
 *  \param mesh The mesh from which the textures are being determined.
 *  \param texture_list The list to which to attach the pointer to.
 */
void IrrDriver::grabAllTextures(const scene::IMesh *mesh)
{
#ifndef SERVER_ONLY
    if (CVS->isGLSL())
    {
        // SPM files has shared_ptr auto-delete texture 
        return;
    }
#endif
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
#ifndef SERVER_ONLY
    if (CVS->isGLSL())
    {
        // SPM files has shared_ptr auto-delete texture 
        return;
    }
#endif
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
void IrrDriver::onLoadWorld()
{
#ifndef SERVER_ONLY
    m_renderer->onLoadWorld();
#endif
}   // onLoadWorld

    // ----------------------------------------------------------------------------
void IrrDriver::onUnloadWorld()
{
#ifndef SERVER_ONLY
    m_renderer->onUnloadWorld();
#endif
}   // onUnloadWorld

// ----------------------------------------------------------------------------
/** Sets the ambient light.
 *  \param light The colour of the light to set.
 *  \param force_SH_computation If false, do not recompute spherical harmonics
 *  coefficient when spherical harmonics textures have been defined
 */
void IrrDriver::setAmbientLight(const video::SColorf &light, bool force_SH_computation)
{
#ifndef SERVER_ONLY
    video::SColorf color = light;
    color.r = powf(color.r, 1.0f / 2.2f);
    color.g = powf(color.g, 1.0f / 2.2f);
    color.b = powf(color.b, 1.0f / 2.2f);
    
    m_scene_manager->setAmbientLight(color);
    m_renderer->setAmbientLight(light, force_SH_computation);    
#endif
}   // setAmbientLight

// ----------------------------------------------------------------------------
video::SColorf IrrDriver::getAmbientLight() const
{
    return m_scene_manager->getAmbientLight();
}

// ----------------------------------------------------------------------------
/** Displays the FPS on the screen.
 */
void IrrDriver::displayFPS()
{
#ifndef SERVER_ONLY
    gui::ScalableFont* font = GUIEngine::getSmallFont();
    font->setScale(0.7f);
    core::rect<s32> position;

    const int fheight = font->getHeightPerLine();
    const int rwidth = irr_driver->getActualScreenSize().Width / 6;
    const int swidth = irr_driver->getActualScreenSize().Width / 3;

    if (UserConfigParams::m_artist_debug_mode)
        position = core::rect<s32>(rwidth - 20, 0, int(rwidth * 4.25), 2 * fheight + (fheight / 5));
    else
        position = core::rect<s32>(swidth, 0, swidth * 2, fheight + (fheight / 5));
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

    uint32_t ping = 0;
    if (STKHost::existHost())
        ping = STKHost::get()->getClientPingToServer();

    core::stringw fps_string;
    if (no_trust)
    {
        no_trust--;

        static video::SColor fpsColor = video::SColor(255, 255, 255, 255);
        fps_string = _("FPS: %d/%d/%d - %d KTris, Ping: %dms", "-", "-",
            "-", SP::sp_solid_poly_count / 1000, ping);

        font->setBlackBorder(true);
        font->setThinBorder(true);
        font->drawQuick(fps_string, position, fpsColor, false);
        font->setThinBorder(false);
        font->setBlackBorder(false);
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

    if (UserConfigParams::m_artist_debug_mode)
    {
        if (CVS->isGLSL())
        {
            fps_string = StringUtils::insertValues
                        (L"FPS: %d/%d/%d - PolyCount: %d Solid, %d Shadows - LightDist: %d\n"
                          "Complexity %d, Total skinning joints: %d, Ping: %dms",
                        min, fps, max, SP::sp_solid_poly_count,
                        SP::sp_shadow_poly_count, m_last_light_bucket_distance, irr_driver->getSceneComplexity(),
                        m_skinning_joint, ping);
        }
        else
        {
            fps_string = StringUtils::insertValues(L"FPS: %d/%d/%d - PolyCount: %d Solid, Ping: %dms", min, fps,
                max, m_video_driver->getPrimitiveCountDrawn(0), ping);
        }
    }
    else
    {
        if (CVS->isGLSL())
        {
            fps_string = _("FPS: %d/%d/%d - %d KTris, Ping: %dms", min, fps,
                max, SP::sp_solid_poly_count / 1000, ping);
        }
        else
        {
            fps_string = _("FPS: %d/%d/%d - %d KTris, Ping: %dms", min, fps,
                max, (int)roundf(kilotris), ping);
        }
    }

    static video::SColor fpsColor = video::SColor(255, 255, 255, 255);

    font->setBlackBorder(true);
    font->setThinBorder(true);
    font->drawQuick(fps_string.c_str(), position, fpsColor, false);
    font->setThinBorder(false);
    font->setBlackBorder(false);

#endif
}   // updateFPS

// ----------------------------------------------------------------------------
/** Displays the timer for Story Mode on the screen.
 *  This can't be done in race or overworld GUIs as
 *  the speedrun timer has to be displayed on all screens.
 */
void IrrDriver::displayStoryModeTimer()
{
#ifndef SERVER_ONLY
    if (story_mode_timer->getStoryModeTime() < 0)
        return;

    gui::ScalableFont* font = GUIEngine::getHighresDigitFont();

    core::stringw timer_string;
    timer_string = story_mode_timer->getTimerString().c_str();

    //The normal timer size ; to not write over it
    core::dimension2du area = font->getDimension(L"99:99.99");
    int regular_timer_width = area.Width;
    if (UserConfigParams::m_speedrun_mode)
        area = font->getDimension(L"99:99:99.999");
    else
        area = font->getDimension(L"99:99:99");

    int screen_width = irr_driver->getActualScreenSize().Width;
    int screen_height = irr_driver->getActualScreenSize().Height;
    int speedrun_string_width = area.Width;
    int dist_from_right = speedrun_string_width + regular_timer_width + screen_width*4/100;

    core::rect<s32> position(screen_width - dist_from_right, screen_height*2/100,
                             screen_width                  , screen_height*6/100);

    font->setColoredBorder(irr::video::SColor(255, 0, 32, 80));

    if ( (UserConfigParams::m_speedrun_mode && story_mode_timer->speedrunIsFinished()) ||
         (!UserConfigParams::m_speedrun_mode && PlayerManager::getCurrentPlayer()->isFinished()) )
        font->draw(timer_string.c_str(), position, video::SColor(255, 0, 255, 0), false, false, NULL, true);
    else
        font->draw(timer_string.c_str(), position, video::SColor(255, 220, 255, 0), false, false, NULL, true);

    font->disableColoredBorder();
#endif
} // displayStoryModeTimer

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

    std::string track_name = RaceManager::get()->getTrackName();
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
void IrrDriver::handleWindowResize()
{
    bool dialog_exists = GUIEngine::ModalDialog::isADialogActive() ||
            GUIEngine::ScreenKeyboard::isActive();

    // This will allow main menu auto resize if missed a resize event
    core::dimension2du current_screen_size =
        m_video_driver->getCurrentRenderTargetSize();
    GUIEngine::Screen* screen = GUIEngine::getCurrentScreen();
    if (screen && screen->isResizable())
    {
        current_screen_size.Width = screen->getWidth();
        current_screen_size.Height = screen->getHeight();
    }

    bool screen_orientation_changed = false;
    int new_orientation = -1;
#ifdef ENABLE_SCREEN_ORIENTATION_HANDLING
    new_orientation = (int)SDL_GetDisplayOrientation(0);
    screen_orientation_changed = m_screen_orientation != new_orientation;
#endif
    const core::dimension2du& new_size = m_video_driver->getCurrentRenderTargetSize();
    if (m_actual_screen_size != new_size ||
        current_screen_size != new_size ||
        screen_orientation_changed)
    {
        // Don't update when dialog is opened or minimized
        if (dialog_exists || new_size.getArea() == 0)
            return;

        m_screen_orientation = new_orientation;
        m_actual_screen_size = new_size;
        UserConfigParams::m_width = m_actual_screen_size.Width;
        UserConfigParams::m_height = m_actual_screen_size.Height;
        UserConfigParams::m_real_width = (unsigned)((float)m_actual_screen_size.Width / m_device->getNativeScaleX());
        UserConfigParams::m_real_height = (unsigned)((float)m_actual_screen_size.Height / m_device->getNativeScaleY());
        resizeWindow();
    }
    // In case reset by opening options in game
    if (!dialog_exists &&
        StateManager::get()->getGameState() == GUIEngine::GAME &&
        !m_device->isResizable())
        m_device->setResizable(true);
}   // handleWindowResize

// ----------------------------------------------------------------------------
/** Update, called once per frame.
 *  \param dt Time since last update
 *  \param is_loading True if the rendering is called during loading of world,
 *         in which case world, physics etc must not be accessed/
 */
void IrrDriver::update(float dt, bool is_loading)
{
    bool show_dialog_yes = m_resolution_changing == RES_CHANGE_YES;
    bool show_dialog_warn = m_resolution_changing == RES_CHANGE_YES_WARN;
    // If the resolution should be switched, do it now. This will delete the
    // old device and create a new one.
    if (m_resolution_changing!=RES_CHANGE_NONE)
    {
        applyResolutionSettings(m_resolution_changing != RES_CHANGE_SAME);
        m_resolution_changing = RES_CHANGE_NONE;
    }

    handleWindowResize();

    if (show_dialog_yes)
        new ConfirmResolutionDialog(false);
    else if (show_dialog_warn)
        new ConfirmResolutionDialog(true);

    m_wind->update();

    PropertyAnimator::get()->update(dt);
#ifndef SERVER_ONLY
    if (CVS->isGLSL())
    {
        SP::SPTextureManager::get()->checkForGLCommand();
    }
#endif
    World *world = World::getWorld();

    if (world)
    {
#ifndef SERVER_ONLY
        m_renderer->render(dt, is_loading);

        GUIEngine::Screen* current_screen = GUIEngine::getCurrentScreen();
        if (current_screen != NULL && current_screen->needs3D())
        {
            GUIEngine::render(dt, is_loading);
        }

        if (!is_loading && Physics::get())
        {
            IrrDebugDrawer* debug_drawer = Physics::get()->getDebugDrawer();
            if (debug_drawer != NULL && debug_drawer->debugEnabled())
            {
                debug_drawer->beginNextFrame();
            }
        }
#endif
    }
    else
    {
#ifndef SERVER_ONLY
        m_video_driver->beginScene(/*backBuffer clear*/ true, /*zBuffer*/ true,
                                   video::SColor(255,100,101,140));

        GUIEngine::render(dt, is_loading);
        if (m_render_nw_debug && !is_loading)
        {
            renderNetworkDebug();
        }
        m_video_driver->endScene();
#endif
    }

    if (m_request_screenshot) doScreenShot();

    // Enable this next print statement to get render information printed
    // E.g. number of triangles rendered, culled etc. The stats is only
    // printed while the race is running and not while the in-game menu
    // is shown. This way the output can be studied by just opening the
    // menu.
    //if(World::getWorld() && World::getWorld()->isRacePhase())
    //    printRenderStats();
#ifdef ENABLE_RECORDER
    if (m_recording)
    {
        PROFILER_PUSH_CPU_MARKER("- Recording", 0x0, 0x50, 0x40);
        ogrCapture();
        PROFILER_POP_CPU_MARKER();
    }
#endif
}   // update

// ----------------------------------------------------------------------------
void IrrDriver::renderNetworkDebug()
{
#ifndef SERVER_ONLY
    if (!NetworkConfig::get()->isNetworking() ||
        NetworkConfig::get()->isServer() || !STKHost::existHost())
        return;

    auto peer = STKHost::get()->getServerPeerForClient();
    if (!peer)
        return;

    const core::dimension2d<u32>& screen_size =
        m_video_driver->getScreenSize();
    core::rect<s32>background_rect(
        (int)(0.02f * screen_size.Width),
        (int)(0.3f * screen_size.Height),
        (int)(0.98f * screen_size.Width),
        (int)(0.6f * screen_size.Height));
    video::SColor color(0x80, 0xFF, 0xFF, 0xFF);
    GL32_draw2DRectangle(color, background_rect);
    uint64_t r, d, h, m, s, f;
    r = STKHost::get()->getNetworkTimer();
    d = r / 86400000;
    r = r % 86400000;
    h = r / 3600000;
    r = r % 3600000;
    m = r / 60000;
    r = r % 60000;
    s = r / 1000;
    f = r % 1000;
    char str[128];
    sprintf(str, "%d day(s), %02d:%02d:%02d.%03d",
        (int)d, (int)h, (int)m, (int)s, (int)f);

    gui::IGUIFont* font = GUIEngine::getFont();
    unsigned height = font->getHeightPerLine() + 2;
    background_rect.UpperLeftCorner.X += 5;
    static video::SColor black = video::SColor(255, 0, 0, 0);
    font->drawQuick(StringUtils::insertValues(
        L"Server time: %s      Server state frequency: %d",
        str, NetworkConfig::get()->getStateFrequency()),
        background_rect, black, false);

    background_rect.UpperLeftCorner.Y += height;
    font->drawQuick(StringUtils::insertValues(
        L"Upload speed (KBps): %f      Download speed (KBps): %f",
        (float)STKHost::get()->getUploadSpeed() / 1024.0f,
        (float)STKHost::get()->getDownloadSpeed() / 1024.0f,
        NetworkConfig::get()->getStateFrequency()), background_rect, black,
        false);

    background_rect.UpperLeftCorner.Y += height;
    font->drawQuick(StringUtils::insertValues(
        L"Packet loss: %d      Packet loss variance: %d",
        peer->getENetPeer()->packetLoss,
        peer->getENetPeer()->packetLossVariance,
        NetworkConfig::get()->getStateFrequency()), background_rect, black,
        false);
#endif
}   // renderNetworkDebug

// ----------------------------------------------------------------------------
void IrrDriver::setRecording(bool val)
{
#ifdef ENABLE_RECORDER
    if (!CVS->isARBPixelBufferObjectUsable())
    {
        Log::warn("irr_driver", "PBO extension missing, can't record video.");
        return;
    }
    if (val == (ogrCapturing() == 1))
        return;
    m_recording = val;
    if (val == true)
    {
        std::string track_name = World::getWorld() != NULL ?
            RaceManager::get()->getTrackName() : "menu";
        time_t rawtime;
        time(&rawtime);
        tm* timeInfo = localtime(&rawtime);
        char time_buffer[256];
        sprintf(time_buffer, "%i.%02i.%02i_%02i.%02i.%02i",
            timeInfo->tm_year + 1900, timeInfo->tm_mon + 1,
            timeInfo->tm_mday, timeInfo->tm_hour,
            timeInfo->tm_min, timeInfo->tm_sec);
        std::string portable_path = FileUtils::getPortableWritingPath(
            file_manager->getScreenshotDir());
        ogrSetSavedName(
            (portable_path + track_name + "_" + time_buffer).c_str());
        ogrPrepareCapture();
    }
    else
    {
        ogrStopCapture();
    }
#else
    Log::error("Recorder", "Recording unavailable, STK was compiled without "
               "recording support.  Please re-compile STK with libopenglrecorder "
               "to enable recording.  If you got SuperTuxKart from your distribution's "
               "repositories, please use the official binaries, or contact your "
               "distributions's package mantainers.");
#endif
}   // setRecording

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
        if (event.LogEvent.Level >= m_logger_level)
        {
            switch (event.LogEvent.Level)
            {
            case ELL_DEBUG:
                Log::debug("[IrrDriver Logger]", "%s", event.LogEvent.Text);
                break;
            case ELL_INFORMATION:
                Log::info("[IrrDriver Logger]", "%s", event.LogEvent.Text);
                break;
            case ELL_WARNING:
                Log::warn("[IrrDriver Logger]", "%s", event.LogEvent.Text);
                break;
            case ELL_ERROR:
                Log::error("[IrrDriver Logger]", "%s", event.LogEvent.Text);
                break;
            default:
                break;
            }
        }
        return true;
    }
    default:
        return false;
    }   // switch

    return false;
}   // OnEvent

// ----------------------------------------------------------------------------
scene::ISceneNode *IrrDriver::addLight(const core::vector3df &pos,
                                       float energy, float radius,
                                       float r, float g, float b,
                                       bool sun_, scene::ISceneNode* parent)
{
#ifndef SERVER_ONLY
    if (CVS->isGLSL())
    {
        if (parent == NULL) parent = m_scene_manager->getRootSceneNode();
        LightNode *light = NULL;

        if (!sun_)
            light = new LightNode(m_scene_manager, parent, energy, radius,
                                  r, g, b);
        else
            light = new SunNode(m_scene_manager, parent, r, g, b);

        light->setPosition(pos);
        light->updateAbsolutePosition();

        m_lights.push_back(light);

        if (sun_)
        {
            m_renderer->addSunLight(pos);
        }
        return light;
    }
    else
    {
        scene::ILightSceneNode* light = m_scene_manager
               ->addLightSceneNode(m_scene_manager->getRootSceneNode(),
                                   pos, video::SColorf(r, g, b, 1.0f));
        light->setRadius(radius);
        return light;
    }
#else
    return NULL;
#endif
}   // addLight

// ----------------------------------------------------------------------------

void IrrDriver::clearLights()
{
    for (unsigned int i = 0; i < m_lights.size(); i++)
    {
        m_lights[i]->drop();
    }

    m_lights.clear();
}   // clearLights

// ----------------------------------------------------------------------------
GLuint IrrDriver::getRenderTargetTexture(TypeRTT which)
{
    return m_renderer->getRenderTargetTexture(which);
}   // getRenderTargetTexture

// ----------------------------------------------------------------------------
GLuint IrrDriver::getDepthStencilTexture()
{
    return m_renderer->getDepthStencilTexture();
}   // getDepthStencilTexture

// ----------------------------------------------------------------------------
void IrrDriver::resetDebugModes()
{
    m_ssaoviz = false;
    m_shadowviz = false;
    m_lightviz = false;
    m_boundingboxesviz = false;
#ifndef SERVER_ONLY
    SP::sp_debug_view = false;
#endif
}

// ----------------------------------------------------------------------------
void IrrDriver::resizeWindow()
{
#ifndef SERVER_ONLY
    // Reload fonts
    font_manager->getFont<RegularFace>()->init();
    font_manager->getFont<BoldFace>()->init();
    font_manager->getFont<DigitFace>()->init();
    // Reload GUIEngine
    GUIEngine::reloadForNewSize();
    if (World::getWorld())
    {
        for(unsigned int i=0; i<Camera::getNumCameras(); i++)
            Camera::getCamera(i)->setupCamera();
        ShaderBasedRenderer* sbr = dynamic_cast<ShaderBasedRenderer*>(m_renderer);
        if (sbr)
        {
            delete sbr->getRTTs();
            // This will recreate the RTTs
            sbr->onLoadWorld();
        }
        STKTextBillboard::updateAllTextBillboards();
        World::getWorld()->getRaceGUI()->recreateGUI();
    }

#ifdef ENABLE_RECORDER
    ogrDestroy();
    RecorderConfig cfg;
    cfg.m_triple_buffering = 1;
    cfg.m_record_audio = 1;
    cfg.m_width = m_actual_screen_size.Width;
    cfg.m_height = m_actual_screen_size.Height;
    int vf = UserConfigParams::m_video_format;
    cfg.m_video_format = (VideoFormat)vf;
    cfg.m_audio_format = OGR_AF_VORBIS;
    cfg.m_audio_bitrate = UserConfigParams::m_audio_bitrate;
    cfg.m_video_bitrate = UserConfigParams::m_video_bitrate;
    cfg.m_record_fps = UserConfigParams::m_record_fps;
    cfg.m_record_jpg_quality = UserConfigParams::m_recorder_jpg_quality;
    if (ogrInitConfig(&cfg) == 0)
    {
        Log::error("irr_driver",
            "RecorderConfig is invalid, use the default one.");
    }
#endif
#endif
}

// ----------------------------------------------------------------------------
const core::dimension2d<u32>& IrrDriver::getFrameSize() const
{
    return m_video_driver->getCurrentRenderTargetSize();
}

// ----------------------------------------------------------------------------
unsigned int IrrDriver::getRealTime()
{
    return m_device->getTimer()->getRealTime();
}

// ----------------------------------------------------------------------------
u32 IrrDriver::getDefaultFramebuffer() const
{
    return m_video_driver->getDefaultFramebuffer();
}
