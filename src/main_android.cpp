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

#ifdef ANDROID

#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "utils/log.hpp"

#include "../../../lib/irrlicht/source/Irrlicht/CIrrDeviceAndroid.h"

extern int main(int argc, char *argv[]);

struct android_app* global_android_app;

void override_default_params()
{
    // It has an effect only on the first run, when config file is created.
    // So that we can still modify these params in STK options and user's
    // choice will be then remembered.
    
    // Set smaller texture size to avoid high RAM usage
    UserConfigParams::m_max_texture_size = 256;
    UserConfigParams::m_high_definition_textures = false;
    
    // Disable advanced lighting by default to make the game playable
    UserConfigParams::m_dynamic_lights = false;

    // Enable touch steering and screen keyboard
    UserConfigParams::m_multitouch_enabled = true;
    UserConfigParams::m_screen_keyboard = true;
    
    // It shouldn't matter, but STK is always run in fullscreen on android
    UserConfigParams::m_fullscreen = true;
    
    // Make sure that user can play every track even if there are installed
    // only few tracks and it's impossible to finish overworld challenges
    UserConfigParams::m_everything_unlocked = true;
    
    // Create default user istead of showing login screen to make life easier
    UserConfigParams::m_enforce_current_player = true;
    
    // Just for debugging
    UserConfigParams::m_log_errors_to_console = true;
}

void android_main(struct android_app* app) 
{
    Log::info("AndroidMain", "Loading application...");
        
    app_dummy();
    
    // Initialize global Android window state variables
    CIrrDeviceAndroid::onCreate();
    
    override_default_params();

    global_android_app = app;
    main(0, {});

    Log::info("AndroidMain", "Closing STK...");
    
    // TODO: Irrlicht device is properly waiting for destroy event, but
    // some global variables are not initialized/cleared in functions and thus 
    // its state is remembered when the window is restored. We will use exit
    // call to make sure that all variables are cleared until a proper fix will 
    // be done.
    exit(0);
}

#endif
