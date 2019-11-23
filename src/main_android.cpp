//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2016-2017 SuperTuxKart-Team
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

#ifdef MOBILE_STK

#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"

#ifdef ANDROID
#include "../../../lib/irrlicht/source/Irrlicht/CIrrDeviceAndroid.h"
#endif

extern int main(int argc, char *argv[]);

#ifdef ANDROID
struct android_app* global_android_app = NULL;
ANativeActivity* global_android_activity = NULL;

extern "C"
{
void set_global_android_activity(ANativeActivity* activity)
{
    global_android_activity = activity;
}
}
#endif

void override_default_params_for_mobile()
{
    // It has an effect only on the first run, when config file is created.
    // So that we can still modify these params in STK options and user's
    // choice will be then remembered.
    
    // Set smaller texture size to avoid high RAM usage
    UserConfigParams::m_max_texture_size = 256;
    UserConfigParams::m_high_definition_textures = false;
    
    // Disable advanced lighting by default to make the game playable
    UserConfigParams::m_dynamic_lights = false;

    // Enable multitouch race GUI
    UserConfigParams::m_multitouch_draw_gui = true;

#ifdef IOS_STK
    // Default 30 fps for battery saving
    UserConfigParams::m_swap_interval = 2;
#endif

#ifdef ANDROID
    // Set multitouch device scale depending on actual screen size
    int32_t screen_size = AConfiguration_getScreenSize(global_android_app->config);
    
    switch (screen_size)
    {
    case ACONFIGURATION_SCREENSIZE_SMALL:
    case ACONFIGURATION_SCREENSIZE_NORMAL:
        UserConfigParams::m_multitouch_scale.setDefaultValue(1.3f);
        UserConfigParams::m_multitouch_sensitivity_x.setDefaultValue(0.1f);
        UserConfigParams::m_font_size = 5.0f;
        break;
    case ACONFIGURATION_SCREENSIZE_LARGE:
        UserConfigParams::m_multitouch_scale.setDefaultValue(1.2f);
        UserConfigParams::m_multitouch_sensitivity_x.setDefaultValue(0.15f);
        UserConfigParams::m_font_size = 5.0f;
        break;
    case ACONFIGURATION_SCREENSIZE_XLARGE:
        UserConfigParams::m_multitouch_scale.setDefaultValue(1.1f);
        UserConfigParams::m_multitouch_sensitivity_x.setDefaultValue(0.2f);
        UserConfigParams::m_font_size = 4.0f;
        break;
    default:
        break;
    }
#endif

    // Enable screen keyboard
    UserConfigParams::m_screen_keyboard = 1;
    
    // It shouldn't matter, but STK is always run in fullscreen on android
    UserConfigParams::m_fullscreen = true;
    
    // Make sure that user can play every track even if there are installed
    // only few tracks and it's impossible to finish overworld challenges
    UserConfigParams::m_unlock_everything = 1;
    
    // Create default user istead of showing login screen to make life easier
    UserConfigParams::m_enforce_current_player = true;
}

#ifdef IOS_STK
void getConfigForDevice(const char* dev)
{
    // Check browser.geekbench.com/ios-benchmarks metal benchmark
    // https://gist.github.com/adamawolf/3048717 for device name mapping
    std::string device = dev;
    if (device.find("iPhone") != std::string::npos)
    {
        // Normal configuration default
        UserConfigParams::m_multitouch_scale.setDefaultValue(1.3f);
        UserConfigParams::m_multitouch_sensitivity_x.setDefaultValue(0.1f);
        UserConfigParams::m_font_size = 5.0f;
        device.erase(0, 6);
        auto versions = StringUtils::splitToUInt(device, ',');
        if (versions.size() == 2)
        {
            // A9 GPU
            if (versions[0] >= 8)
            {
                UserConfigParams::m_dynamic_lights = true;
                UserConfigParams::m_high_definition_textures = 1;
            }
            if (versions[0] < 7 || // iPhone 5s
                (versions[0] == 7 && versions[1] == 2) || // iPhone 6
                (versions[0] == 8 && versions[1] == 1) || // iPhone 6S
                (versions[0] == 8 && versions[1] == 4) || // iPhone SE
                (versions[0] == 9 && versions[1] == 1) || // iPhone 7
                (versions[0] == 9 && versions[1] == 3) || // iPhone 7
                (versions[0] == 10 && versions[1] == 1) || // iPhone 8
                (versions[0] == 10 && versions[1] == 4) // iPhone 8
                )
            {
                // Those phones have small screen
                UserConfigParams::m_multitouch_scale.setDefaultValue(1.45f);
            }
        }
    }
    else if (device.find("iPad") != std::string::npos)
    {
        // Normal configuration default
        UserConfigParams::m_multitouch_scale.setDefaultValue(0.95f);
        UserConfigParams::m_multitouch_sensitivity_x.setDefaultValue(0.2f);
        UserConfigParams::m_font_size = 3.0f;
        device.erase(0, 4);
        auto versions = StringUtils::splitToUInt(device, ',');
        if (versions.size() == 2)
        {
            if (versions[0] >= 7)
            {
                UserConfigParams::m_dynamic_lights = true;
                UserConfigParams::m_high_definition_textures = 1;
            }
        }
    }
    else if (device.find("iPod") != std::string::npos)
    {
        // All iPod touch has small screen
        UserConfigParams::m_font_size = 5.0f;
        UserConfigParams::m_multitouch_scale.setDefaultValue(1.45f);
        UserConfigParams::m_multitouch_sensitivity_x.setDefaultValue(0.1f);
        device.erase(0, 4);
        auto versions = StringUtils::splitToUInt(device, ',');
        if (versions.size() == 2)
        {
            // iPod Touch 7th Generation (A10)
            if (versions[0] >= 9)
            {
                UserConfigParams::m_dynamic_lights = true;
                UserConfigParams::m_high_definition_textures = 1;
            }
        }
    }
}

#endif

#ifdef ANDROID
void android_main(struct android_app* app) 
{
    Log::info("AndroidMain", "Loading application...");
        
    global_android_app = app;
    global_android_activity = app->activity;
    
    // Initialize global Android window state variables
    CIrrDeviceAndroid::onCreate();
    
    app_dummy();
    override_default_params_for_mobile();

    main(0, {});

    Log::info("AndroidMain", "Closing STK...");
    
    // TODO: Irrlicht device is properly waiting for destroy event, but
    // some global variables are not initialized/cleared in functions and thus 
    // its state is remembered when the window is restored. We will use exit
    // call to make sure that all variables are cleared until a proper fix will 
    // be done.
    fflush(NULL);
    _exit(0);
}
#endif

#endif
