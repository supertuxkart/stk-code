//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015 Ingo Ruhnke <grumbel@gmx.de>
//  Copyright (C) 2006-2015 SuperTuxKart-Team
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

#include "main_loop.hpp"

#include <assert.h>

#include "audio/sfx_manager.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material_manager.hpp"
#include "guiengine/engine.hpp"
#include "input/input_manager.hpp"
#include "input/wiimote_manager.hpp"
#include "modes/profile_world.hpp"
#include "modes/world.hpp"
#include "network/network_config.hpp"
#include "network/protocol_manager.hpp"
#include "network/race_event_manager.hpp"
#include "network/stk_host.hpp"
#include "online/request_manager.hpp"
#include "race/race_manager.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/profiler.hpp"

#ifdef ANDROID
#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>
extern void* global_android_app;
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#endif

MainLoop* main_loop = 0;

MainLoop::MainLoop() :
m_abort(false)
{
    m_curr_time = 0;
    m_prev_time = 0;
    m_throttle_fps = true;
}  // MainLoop

//-----------------------------------------------------------------------------
MainLoop::~MainLoop()
{
}   // ~MainLoop

//-----------------------------------------------------------------------------
/** Returns the current dt, which guarantees a limited frame rate. If dt is
 *  too low (the frame rate too high), the process will sleep to reach the
 *  maxium frame rate.
 */
float MainLoop::getLimitedDt()
{
    IrrlichtDevice* device = irr_driver->getDevice();
    m_prev_time = m_curr_time;

    float dt;  // needed outside of the while loop
    while( 1 )
    {
        m_curr_time = device->getTimer()->getRealTime();
        dt = (float)(m_curr_time - m_prev_time);
        const World* const world = World::getWorld();
        if (UserConfigParams::m_fps_debug && world)
        {
            const LinearWorld *lw = dynamic_cast<const LinearWorld*>(world);
            if (lw)
            {
                Log::verbose("fps", "time %f distance %f dt %f fps %f",
                             lw->getTime(),
                             lw->getDistanceDownTrackForKart(0),
                             dt*0.001f, 1000.0f / dt);
            }
            else
            {
                Log::verbose("fps", "time %f dt %f fps %f",
                             world->getTime(), dt*0.001f, 1000.0f / dt);
            }

        }

        // don't allow the game to run slower than a certain amount.
        // when the computer can't keep it up, slow down the shown time instead
        static const float max_elapsed_time = 3.0f*1.0f/60.0f*1000.0f; /* time 3 internal substeps take */
        if(dt > max_elapsed_time) dt=max_elapsed_time;

        // Throttle fps if more than maximum, which can reduce
        // the noise the fan on a graphics card makes.
        // When in menus, reduce FPS much, it's not necessary to push to the maximum for plain menus
        const int max_fps = (StateManager::get()->throttleFPS() ? 30 : UserConfigParams::m_max_fps);
        const int current_fps = (int)(1000.0f/dt);
        if (m_throttle_fps && current_fps > max_fps && !ProfileWorld::isProfileMode())
        {
            int wait_time = 1000/max_fps - 1000/current_fps;
            if(wait_time < 1) wait_time = 1;

            PROFILER_PUSH_CPU_MARKER("Throttle framerate", 0, 0, 0);
            StkTime::sleep(wait_time);
            PROFILER_POP_CPU_MARKER();
        }
        else break;
    }
    dt *= 0.001f;
    return dt;
}   // getLimitedDt

//-----------------------------------------------------------------------------
/** Updates all race related objects.
 *  \param dt Time step size.
 */
void MainLoop::updateRace(float dt)
{
    if(ProfileWorld::isProfileMode()) dt=1.0f/60.0f;

    // The race event manager will update world in case of an online race
    if (RaceEventManager::getInstance<RaceEventManager>()->isRunning())
        RaceEventManager::getInstance<RaceEventManager>()->update(dt);
    else
        World::getWorld()->updateWorld(dt);
}   // updateRace

bool pressed[4] = { false, false, false, false};

void post_key(int key, bool end) {
    irr::SEvent irrevent;
    switch(key)
    {
    case 0:
        irrevent.KeyInput.Key = irr::KEY_UP;
        break;
    case 1:
        irrevent.KeyInput.Key = irr::KEY_DOWN;
        break;
    case 2:
        irrevent.KeyInput.Key = irr::KEY_LEFT;
        break;
    case 3:
        irrevent.KeyInput.Key = irr::KEY_RIGHT;
        break;
    }
	if(pressed[key] != end) {
		irrevent.EventType = irr::EET_KEY_INPUT_EVENT;
		irrevent.KeyInput.PressedDown = end;
		pressed[key] = end;
		irr_driver->getDevice()->postEventFromUser(irrevent);
	}
}
//-----------------------------------------------------------------------------
/** Run the actual main loop.
 */
void MainLoop::run()
{
    IrrlichtDevice* device = irr_driver->getDevice();

    m_curr_time = device->getTimer()->getRealTime();
    
    #if defined(ANDROID)
	auto sensorManager = ASensorManager_getInstance();
	auto accelerometerSensor = ASensorManager_getDefaultSensor(sensorManager,
                                                    ASENSOR_TYPE_ACCELEROMETER);
    auto sensorEventQueue = ASensorManager_createEventQueue(sensorManager,
                                    ((android_app*)global_android_app)->looper, 
                                    LOOPER_ID_USER, NULL, NULL);
	ASensorEventQueue_enableSensor(sensorEventQueue, accelerometerSensor);
	// We'd like to get 60 events per second (in us).
	ASensorEventQueue_setEventRate(sensorEventQueue, accelerometerSensor, 
                                   (1000L/1)*1000);
    #endif
    
    while(!m_abort)
    {
        #if defined(ANDROID)
        int ident;
        int events;
        struct android_poll_source* source;
        while ((ident = ALooper_pollAll(0, NULL, &events,
                (void**)&source)) >= 0) 
        {
			// Process this event.
            if (source != NULL) 
            {
                source->process((android_app*)global_android_app, source);
            }

            // If a sensor has data, process it now.
            if (ident == LOOPER_ID_USER) 
            {
                if (accelerometerSensor != NULL) 
                {
                    ASensorEvent event;
                    while (ASensorEventQueue_getEvents(sensorEventQueue,
                           &event, 1) > 0) 
                    {
                        LOGI("accelerometer: x=%f y=%f z=%f",
                             event.acceleration.x, event.acceleration.y,
                             event.acceleration.z);
						post_key(1, event.acceleration.z < 0);
						post_key(0, event.acceleration.z > 4);
						post_key(3, event.acceleration.y > 2);
						post_key(2, event.acceleration.y < -2);
                    }
                }
            }

        }
        #endif
        PROFILER_PUSH_CPU_MARKER("Main loop", 0xFF, 0x00, 0xF7);

        m_prev_time = m_curr_time;
        float dt   = getLimitedDt();

        if (World::getWorld())  // race is active if world exists
        {
            PROFILER_PUSH_CPU_MARKER("Update race", 0, 255, 255);
            updateRace(dt);
            PROFILER_POP_CPU_MARKER();
        }   // if race is active

        // We need to check again because update_race may have requested
        // the main loop to abort; and it's not a good idea to continue
        // since the GUI engine is no more to be called then.
        // Also only do music, input, and graphics update if graphics are
        // enabled.
        if (!m_abort && !ProfileWorld::isNoGraphics())
        {
            PROFILER_PUSH_CPU_MARKER("Music/input/GUI", 0x7F, 0x00, 0x00);
            input_manager->update(dt);

            #ifdef ENABLE_WIIUSE
                wiimote_manager->update();
            #endif
            
            GUIEngine::update(dt);
            PROFILER_POP_CPU_MARKER();

            PROFILER_PUSH_CPU_MARKER("IrrDriver update", 0x00, 0x00, 0x7F);
            irr_driver->update(dt);
            PROFILER_POP_CPU_MARKER();

            // Update sfx and music after graphics, so that graphics code
            // can use as many threads as possible without interfering
            // with audia
            PROFILER_PUSH_CPU_MARKER("Music/input/GUI", 0x7F, 0x00, 0x00);
            SFXManager::get()->update();
            PROFILER_POP_CPU_MARKER();

            PROFILER_PUSH_CPU_MARKER("Protocol manager update", 0x7F, 0x00, 0x7F);
            if (STKHost::existHost())
            {
                if (STKHost::get()->requestedShutdown())
                    STKHost::get()->shutdown();
                else
                    ProtocolManager::getInstance()->update();
            }
            PROFILER_POP_CPU_MARKER();

            PROFILER_PUSH_CPU_MARKER("Database polling update", 0x00, 0x7F, 0x7F);
            Online::RequestManager::get()->update(dt);
            PROFILER_POP_CPU_MARKER();
        }
        else if (!m_abort && ProfileWorld::isNoGraphics())
        {
            PROFILER_PUSH_CPU_MARKER("Protocol manager update", 0x7F, 0x00, 0x7F);
            if(NetworkConfig::get()->isNetworking())
                ProtocolManager::getInstance()->update();
            PROFILER_POP_CPU_MARKER();

            PROFILER_PUSH_CPU_MARKER("Database polling update", 0x00, 0x7F, 0x7F);
            Online::RequestManager::get()->update(dt);
            PROFILER_POP_CPU_MARKER();
        }

        PROFILER_POP_CPU_MARKER();
        PROFILER_SYNC_FRAME();
    }  // while !m_abort

}   // run

//-----------------------------------------------------------------------------
/** Set the abort flag, causing the mainloop to be left.
 */
void MainLoop::abort()
{
    m_abort = true;
}   // abort

/* EOF */
