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
#include "race/history.hpp"
#include "race/race_manager.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/profiler.hpp"

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
    float dt = 0;
    // If we are doing a replay, use the dt from the history file
    if (World::getWorld() && history->replayHistory() )
    {
        dt = history->updateReplayAndGetDT();
        return dt;
    }


    // In profile mode without graphics, run with a fixed dt of 1/60
    if ((ProfileWorld::isProfileMode() && ProfileWorld::isNoGraphics()) ||
        UserConfigParams::m_arena_ai_stats)
    {
        return 1.0f/60.0f;
    }

    IrrlichtDevice* device = irr_driver->getDevice();
    m_prev_time = m_curr_time;

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

        // Don't allow the game to run slower than a certain amount.
        // when the computer can't keep it up, slow down the shown time instead
        // But this can not be done in networking, otherwise the game time on
        // client and server will not be in synch anymore
        if(!NetworkConfig::get()->isNetworking())
        {
            static const float max_elapsed_time = 3.0f*1.0f / 60.0f*1000.0f; /* time 3 internal substeps take */
            if (dt > max_elapsed_time) dt = max_elapsed_time;
        }

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
    // The race event manager will update world in case of an online race
    if (RaceEventManager::getInstance<RaceEventManager>()->isRunning())
        RaceEventManager::getInstance<RaceEventManager>()->update(dt);
    else
        World::getWorld()->updateWorld(dt);
}   // updateRace

//-----------------------------------------------------------------------------
/** Run the actual main loop.
 *  The sequnce in which various parts of STK are updated is:
 *  - Determine next time step size (`getLimitedDt`). This takes maximum fps
 *    into account (i.e. sleep if the fps would be too high), and will actually
 *    slow down the in-game clock if the fps are too low (if more than 3/60 of
 *    a second have passed, more than 3 physics time steps would be needed, 
 *    and physics do at most 3 time steps).
 *  - if a race is taking place (i.e. not only a menu being shown), call
 *    `updateRace()`, which is a thin wrapper around a call to
 *    `World::updateWorld()`:
 *    - Update history manager (which will either set the kart position and/or
 *      controls when replaying, or store the current info for a replay).
 *      This is mostly for debugging only (though available even in release
 *      mode).
 *    - Updates Replays - either storing data when not replaying, or
 *      updating kart positions/control when replaying).
 *    - Calls `WorldStatus::update()`, which updates the race state (e.g.
 *      go from 'ready' to 'set' etc), and clock.
 *    - Updates the physics (`Physics::update()`). This will simulate all
 *      physical objects for the specified time with bullet.
 *    - Updates all karts (`Kart::update()`). Obviously the update function
 *      does a lot more than what is described here, this is only supposed to
 *      be a _very_ high level overview:
 *      - Updates its rewinder (to store potentially changed controls
 *        as events) in `KartRewinder::update()`.
 *      - Calls `Moveable::update()`, which takes the new position from
 *        the physics and saves it (and computes dependent values, like
 *        heading, local velocity).
 *      - Updates its controller. This is either:
 *        - an AI using `SkiddingController::update()` (which then will
 *          compute the new controls), or 
 *        - a player controller using `PlayerController::update()`, which will
 *          handle smooth steering (in case of digital input devices steering
 *          is adjusted a bit over time to avoid an instant change from all
 *          left to all right). Input events will be handled when updating
 *          the irrlicht driver later at the end of the main loop.
 *      - Updates kart animation (like rescue, ...) if one is shown atm.
 *      - Update attachments.
 *      - update physics, i.e. taking the current steering and updating
 *        the bullet raycast vehicle with that data. The settings are actually
 *        only used in the next frame when the physics are updated.
 *    - Updates all cameras via `Camera::update()`. The camera position and
 *      rotation is adjusted according to the position etc of the kart (and
 *      special circumstances like rescue, falling).
 *    - Updates all projectiles using the projectile manager. Some of the
 *      projectiles are mostly handled by the physics (e.g. a cake will mainly
 *      check if it's out of bounds), others (like basket ball) do all 
 *      their aiming and movement here.
 *    - Updates the rewind manager to store rewind states.
 *  - Updates the music manager.
 *  - Updates the input manager (which only updates internal time, actual
 *    input handling follows late)
 *  - Updates the wiimote manager. This will read the data of all wiimotes
 *    and feed the corresponding events to the irrlicht event system.
 *  - Updates the STK internal gui engine. This updates all widgets, and
 *    e.g. takes care of the rotation of the karts in the KartSelection
 *    screen using the ModelViewWidget.
 *  - Updates STK's irrlicht driver `IrrDriver::update()`:
 *    - Calls Irrlicht's `beginScene()` .
 *    - Renders the scene (several times with different viewport if
 *      split screen is being used)
 *    - Calls `GUIEngine::render()`, which renders all widgets with the
 *      help of Irrlicht's GUIEnvironment (`drawAll()`). This will also
 *      handle all events, i.e. all input is now handled (e.g. steering,
 *      firing etc are all set in the corresponding karts depending on
 *      user input).
 *    - Calls Irrlicht's `endScene()`
 */
void MainLoop::run()
{
    IrrlichtDevice* device = irr_driver->getDevice();

    m_curr_time = device->getTimer()->getRealTime();
    while(!m_abort)
    {
        PROFILER_PUSH_CPU_MARKER("Main loop", 0xFF, 0x00, 0xF7);

        m_prev_time = m_curr_time;
        float dt   = getLimitedDt();
        // Render the previous frame, and also handle all user input.
        PROFILER_PUSH_CPU_MARKER("IrrDriver update", 0x00, 0x00, 0x7F);
        irr_driver->update(dt);
        PROFILER_POP_CPU_MARKER();


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

            // Update sfx and music after graphics, so that graphics code
            // can use as many threads as possible without interfering
            // with audio
            PROFILER_PUSH_CPU_MARKER("Music/input/GUI", 0x7F, 0x00, 0x00);
            SFXManager::get()->update();
            PROFILER_POP_CPU_MARKER();

            PROFILER_PUSH_CPU_MARKER("Protocol manager update", 0x7F, 0x00, 0x7F);
            if (STKHost::existHost())
            {
                if (STKHost::get()->requestedShutdown())
                    STKHost::get()->shutdown();
                else
                    ProtocolManager::getInstance()->update(dt);
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
                ProtocolManager::getInstance()->update(dt);
            PROFILER_POP_CPU_MARKER();

            PROFILER_PUSH_CPU_MARKER("Database polling update", 0x00, 0x7F, 0x7F);
            Online::RequestManager::get()->update(dt);
            PROFILER_POP_CPU_MARKER();
        }

        // Update world time if world exists
        if (World::getWorld())
            World::getWorld()->updateTime(dt);

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
