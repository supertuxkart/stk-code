
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2011-2015 Joerg Henrichs, Marianne Gagnon
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


/**
 * \mainpage SuperTuxKart developer documentation
 *
 * This document contains the developer documentation for SuperTuxKart,
 * including the list of modules, the list of classes, the API reference,
 * and some pages that describe in more depth some parts of the code/engine.
 *
 * \section Overview
 *
 * Here is an overview of the high-level interactions between modules :
 \dot
 digraph interaction {
# race -> modes
 race -> tracks
 race -> karts
# modes -> tracks
# modes -> karts
 tracks -> graphics
 karts -> graphics
 tracks -> items
 items -> graphics
 animations -> graphics
 graphics -> "Antarctica/irrlicht"
# guiengine -> irrlicht
# states_screens -> guiengine
# input -> states_screens
 input -> guiengine
 guiengine -> font_system
 karts -> physics
 physics -> karts
 tracks -> physics
 ai -> controller
 controller -> karts
 input -> controller
 tracks -> animations
 physics -> animations
 animations -> physics
 karts -> audio
 physics -> audio
 "translations\n(too many connections\nto draw)"
 "configuration\n(too many connections\nto draw)"
 addons -> tracks
 addons -> karts
 guiengine -> addons
 guiengine -> race
 addons -> online_manager
 challenges -> race
# challenges -> modes
 guiengine -> challenges
 online_manager -> addons
 online_manager -> "STK Server"
 "STK Server" -> online_manager
 karts -> replay
 replay 
 # force karts and tracks on the same level, looks better this way
 subgraph { 
  rank = same; karts; tracks; 
 } 

}
 \enddot

 Note that this graph is only an approximation because the real one would be
 much too complicated :)


 \section Modules

 \li \ref addonsgroup :
   Handles add-ons that can be downloaded.
 \li \ref animations :
   This module manages interpolation-based animation (of position, rotation
   and/or scale)
 \li \ref audio :
   This module handles audio (sound effects and music).
 \li \ref challenges :
   This module handles the challenge system, which locks features (tracks, karts
   modes, etc.) until the user completes some task.
 \li \ref config :
   This module handles the user configuration, the supertuxkart configuration
   file (which contains options usually not edited by the player) and the input
   configuration file.
 \li \ref font :
   This module stores font files and tools used to draw characters in STK.
 \li \ref graphics :
   This module contains the core graphics engine, that is mostly a thin layer
   on top of irrlicht providing some additional features we need for STK
   (like particles, more scene node types, mesh manipulation tools, material
   management, etc...)
 \li \ref guiengine :
   Contains the generic GUI engine (contains the widgets and the backing logic
   for event handling, the skin, screens and dialogs). See module @ref states_screens
   for the actual STK GUI screens. Note that all input comes through this module
   too.
 \li \ref widgetsgroup :
   Contains the various types of widgets supported by the GUI engine.
 \li \ref input :
   Contains classes for input management (keyboard and gamepad)
 \li \ref io :
  Contains generic utility classes for file I/O (especially XML handling).
 \li \ref items :
   Defines the various collectibles and weapons of STK.
 \li \ref karts :
   Contains classes that deal with the properties, models and physics
   of karts.
 \li \ref controller :
   Contains kart controllers, which are either human players or AIs
   (this module thus contains the AIs)
 \li \ref modes :
   Contains the logic for the various game modes (race, follow the leader,
   battle, etc.)
 \li \ref physics :
   Contains various physics utilities.
 \li \ref race :
   Contains the race information that is conceptually above what you can find
   in group Modes. Handles highscores, grands prix, number of karts, which
   track was selected, etc.
 \li \ref states_screens :
   Contains the various screens and dialogs of the STK user interface,
   using the facilities of the guiengine module. Also contains the
   stack of menus and handles state management (in-game vs menu).
 \li \ref tracks :
   Contains information about tracks, namely drivelines, checklines and track
   objects.
 \li \ref tutorial :
   Work in progress
 */

#ifdef WIN32
#  ifdef __CYGWIN__
#    include <unistd.h>
#  endif
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  ifdef _MSC_VER
#    include <direct.h>
#  endif
#else
#  include <signal.h>
#  include <unistd.h>
#endif

#ifdef ANDROID
#include <SDL_system.h>
#include <jni.h>
#endif

#ifdef __SWITCH__
extern "C" {
  #include <sys/iosupport.h>
  #include <switch/kernel/svc.h>
  #include <switch/runtime/nxlink.h>
  #include <switch/runtime/diag.h>
  #include <switch/services/ssl.h>
#define Event libnx_Event
  #include <switch/services/set.h>
  #include <switch/services/applet.h>
  #include <switch/services/nifm.h>
  #include <switch/runtime/pad.h>
#undef Event
  #include <switch/runtime/devices/socket.h>
}

#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#endif

#include <stdexcept>
#include <cstdio>
#include <string>
#include <cstring>
#include <sstream>
#include <algorithm>
#include <limits>

#include <IEventReceiver.h>

#include "main_loop.hpp"
#include "achievements/achievements_manager.hpp"
#include "addons/addons_manager.hpp"
#include "addons/news_manager.hpp"
#include "audio/music_manager.hpp"
#include "audio/sfx_manager.hpp"
#include "challenges/story_mode_timer.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/hardware_stats.hpp"
#include "config/player_manager.hpp"
#include "config/player_profile.hpp"
#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "font/font_manager.hpp"
#include "graphics/camera.hpp"
#include "graphics/camera_debug.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/graphics_restrictions.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/particle_kind_manager.hpp"
#include "graphics/referee.hpp"
#include "graphics/sp/sp_base.hpp"
#include "graphics/sp/sp_shader.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/event_handler.hpp"
#include "guiengine/dialog_queue.hpp"
#include "guiengine/message_queue.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "input/keyboard_device.hpp"
#include "input/wiimote_manager.hpp"
#include "io/file_manager.hpp"
#include "items/attachment_manager.hpp"
#include "items/item_manager.hpp"
#include "items/network_item_manager.hpp"
#include "items/powerup_manager.hpp"
#include "items/projectile_manager.hpp"
#include "karts/combined_characteristic.hpp"
#include "karts/controller/ai_base_controller.hpp"
#include "karts/controller/network_ai_controller.hpp"
#include "karts/kart_model.hpp"
#include "karts/kart_properties.hpp"
#include "karts/kart_properties_manager.hpp"
#include "karts/official_karts.hpp"
#include "modes/cutscene_world.hpp"
#include "modes/demo_world.hpp"
#include "network/protocols/connect_to_server.hpp"
#include "network/protocols/client_lobby.hpp"
#include "network/protocols/server_lobby.hpp"
#include "network/network.hpp"
#include "network/network_config.hpp"
#include "network/network_string.hpp"
#include "network/protocols/connect_to_server.hpp"
#include "network/protocols/client_lobby.hpp"
#include "network/protocols/server_lobby.hpp"
#include "network/race_event_manager.hpp"
#include "network/rewind_manager.hpp"
#include "network/rewind_queue.hpp"
#include "network/server.hpp"
#include "network/server_config.hpp"
#include "network/servers_manager.hpp"
#include "network/socket_address.hpp"
#include "network/stk_host.hpp"
#include "network/stk_peer.hpp"
#include "online/profile_manager.hpp"
#include "online/request_manager.hpp"
#include "race/grand_prix_manager.hpp"
#include "race/highscore_manager.hpp"
#include "race/history.hpp"
#include "race/race_manager.hpp"
#include "replay/replay_play.hpp"
#include "replay/replay_recorder.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "states_screens/online/networking_lobby.hpp"
#include "states_screens/online/register_screen.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/options/user_screen.hpp"
#include "states_screens/dialogs/init_android_dialog.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "tips/tips_manager.hpp"
#include "tracks/arena_graph.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/command_line.hpp"
#include "utils/constants.hpp"
#include "utils/crash_reporting.hpp"
#include "utils/leak_check.hpp"
#include "utils/log.hpp"
#include "mini_glm.hpp"
#include "utils/profiler.hpp"
#include "utils/stk_process.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"
#include "io/rich_presence.hpp"

#include <IrrlichtDevice.h>

static void cleanSuperTuxKart();
static void cleanUserConfig();
void runUnitTests();

// ============================================================================
//                        gamepad visualisation screen
// ============================================================================

void gamepadVisualisation()
{
#ifndef SERVER_ONLY

    input_manager = new InputManager();
    #define GAMEPAD_COUNT 8 // const won't work

    class EventReceiver : public IEventReceiver
    {
    public:
        virtual bool OnEvent (const irr::SEvent &event)
        {
            switch (event.EventType)
            {
                case EET_KEY_INPUT_EVENT:
                {
                    const SEvent::SKeyInput& evt = event.KeyInput;

                    if (evt.PressedDown)
                    {
                        if (evt.Key == IRR_KEY_RETURN || 
                            evt.Key == IRR_KEY_ESCAPE ||
                            evt.Key == IRR_KEY_SPACE)
                        {
                            exit(0);
                        }
                    }

                }

                default:
                    // don't care about others
                    break;
            }
            return true;
        }
    };

    EventReceiver* events = new EventReceiver();
    irr_driver->getDevice()->setEventReceiver(events);

    while (true)
    {
        if (!irr_driver->getDevice()->run()) break;

        input_manager->update(0);
        video::IVideoDriver* driver = irr_driver->getVideoDriver();
        const core::dimension2du size = driver ->getCurrentRenderTargetSize();

        driver->beginScene(true, true, video::SColor(255,0,0,0));

        for (unsigned n = 0; n < input_manager->getGamepadCount(); n++)
        {
            if (n >= GAMEPAD_COUNT)
                break;
            const irr::SEvent& g = input_manager->getEventForGamePad(n);

            const int MARGIN = 10;
            const int x = (n & 1 ? size.Width/2 + MARGIN : MARGIN );
            const int w = size.Width/2 - MARGIN*2;
            const int h = size.Height/(GAMEPAD_COUNT/2) - MARGIN*2;
            const int y = size.Height/(GAMEPAD_COUNT/2)*(n/2) + MARGIN;

            driver->draw2DRectangleOutline( core::recti(x, y, x+w, y+h) );

            const int btn_y = y + 5;
            const int btn_x = x + 5;
            const int BTN_SIZE =
                (w - 10)/SEvent::SJoystickEvent::NUMBER_OF_BUTTONS;

            for (int b=0; b<SEvent::SJoystickEvent::NUMBER_OF_BUTTONS; b++)
            {
                core::position2di pos(btn_x + b*BTN_SIZE, btn_y);
                core::dimension2di size(BTN_SIZE, BTN_SIZE);

                if (g.JoystickEvent.IsButtonPressed(b))
                {
                    driver->draw2DRectangle (video::SColor(255,255,0,0),
                                             core::recti(pos, size));
                }

                driver->draw2DRectangleOutline( core::recti(pos, size) );
            }

            const int axis_y = btn_y + BTN_SIZE + 5;
            const int axis_x = btn_x;
            const int axis_w = w - 10;
            const int axis_h = (h - BTN_SIZE - 15)
                            / SEvent::SJoystickEvent::NUMBER_OF_AXES;

            for (int a=0; a<SEvent::SJoystickEvent::NUMBER_OF_AXES; a++)
            {
                const float rate = g.JoystickEvent.Axis[a] / 32767.0f;

                core::position2di pos(axis_x, axis_y + a*axis_h);
                core::dimension2di size(axis_w, axis_h);

                // Assume a default deadzone value of 4096
                const bool deadzone = (abs(g.JoystickEvent.Axis[a]) < 4096);

                core::recti fillbar(core::position2di(axis_x + axis_w/2,
                                                      axis_y + a*axis_h),
                                    core::dimension2di( (int)(axis_w/2*rate),
                                                        axis_h)               );
                fillbar.repair(); // dimension may be negative
                driver->draw2DRectangle (deadzone ? video::SColor(255,255,0,0)
                                                  : video::SColor(255,0,255,0),
                                         fillbar);
                driver->draw2DRectangleOutline( core::recti(pos, size) );
            }
        }

        driver->endScene();
    }
#endif
}   // gamepadVisualisation

// ============================================================================
/** Sets the hat mesh name depending on the current christmas mode
 *  m_xmas_mode (0: use current date, 1: always on, 2: always off).
 */
void handleXmasMode()
{
    bool xmas = false;
    switch(UserConfigParams::m_xmas_mode)
    {
    case 0:
        {
            int day, month;
            StkTime::getDate(&day, &month);
            // Christmat hats are shown between 17. of December
            // and 5th of January
            xmas = (month == 12 && day>=17)  || (month ==  1 && day <=5);
            break;
        }
    case 1:  xmas = true;  break;
    default: xmas = false; break;
    }   // switch m_xmas_mode

    if(xmas)
        kart_properties_manager->setHatMeshName("christmas_hat.spm");
}   // handleXmasMode
// ============================================================================
/** Determines if Easter Ears should be used
 *  m_easter_ear_mode (0: use current date, 1: always on, 2: always off).
 */
bool isEasterMode(int day, int month, int year, int before_after_days)
{
    if (UserConfigParams::m_easter_ear_mode == 1) {
        return true;
    }

    if (UserConfigParams::m_easter_ear_mode == 0)
    {
        // Compute Easter date, based on wikipedia formula
        // http://en.wikipedia.org/wiki/Computus
        int a = year % 19;
        int b = year >> 2;
        int c = int(floor(b / 25)) + 1;
        int d = (c * 3) >> 2;
        int e = ((a * 19) - int(floor((c * 8 + 5) / 25)) + d + 15) % 30;
        e += (29578 - a - e * 32) >> 10;
        e -= ((year % 7) + b - d + e + 2) % 7;
        d = e >> 5;
        int easter_day = e - d * 31;
        int easter_month = d + 3;

        int easter_start_day = easter_day - before_after_days;
        int easter_start_month = easter_month;
        if (easter_start_day < 1)
        {
            easter_start_day += 31; // Month is April, going into March
            easter_start_month--;
        }
        int easter_end_day = easter_day + before_after_days;
        int easter_end_month = easter_month;
        int month_length = easter_end_month == 3 ? 31 : 30;
        if (easter_end_day > month_length)
        {
            easter_end_day -= month_length;
            easter_end_month++;
        }
        return (month > easter_start_month || (month == easter_start_month && day >= easter_start_day)) &&
               (month < easter_end_month   || (month == easter_end_month   && day <= easter_end_day));
    }

    return false;
}   // isEasterMode(day, month, year, before_after_days)

// ============================================================================
/** Wrapper around handleEasterEarMode(day, month, year, before_after_days).
 */
void handleEasterEarMode()
{
    int day, month, year;
    StkTime::getDate(&day, &month, &year);
    if (isEasterMode(day, month, year, /*before_after_days*/5))
        kart_properties_manager->setHatMeshName("easter_ears.spm");
}   // handleEasterMode

// ============================================================================
/** This function sets up all data structure for an immediate race start.
 *  It is used when the -N or -R command line options are used.
 */
void setupRaceStart()
{
    // Skip the start screen. This esp. means that no login screen is
    // displayed (if necessary), so we have to make sure there is
    // a current player
    PlayerManager::get()->enforceCurrentPlayer();

    InputDevice *device;

    // Use keyboard 0 by default in --no-start-screen
    device = input_manager->getDeviceManager()->getKeyboard(0);

    // Create player and associate player with keyboard
    StateManager::get()->createActivePlayer(
        PlayerManager::get()->getPlayer(0), device);

    if (!kart_properties_manager->getKart(UserConfigParams::m_default_kart))
    {
        Log::warn("main", "Kart '%s' is unknown so will use the "
            "default kart.",
            UserConfigParams::m_default_kart.c_str());
        RaceManager::get()->setPlayerKart(0,
                           UserConfigParams::m_default_kart.getDefaultValue());
    }
    else
    {
        // Set up race manager appropriately
        if (RaceManager::get()->getNumPlayers() > 0)
            RaceManager::get()->setPlayerKart(0, UserConfigParams::m_default_kart);
    }

    // ASSIGN should make sure that only input from assigned devices
    // is read.
    input_manager->getDeviceManager()->setAssignMode(ASSIGN);
}   // setupRaceStart

// ----------------------------------------------------------------------------
/** Prints help for command line options to stdout.
 */
void cmdLineHelp()
{
    fprintf(stdout,
    "Usage: %s [OPTIONS]\n\n"
    "Run SuperTuxKart, a go-kart racing game that features "
    "Tux and friends.\n\n"
    "Options:\n"
    "  -N,  --no-start-screen  Immediately start race without showing a "
                              "menu.\n"
    "  -R,  --race-now         Same as -N but also skip the ready-set-go phase"
                              " and the music.\n"
    "  -t,  --track=NAME       Start track NAME.\n"
    "       --gp=NAME          Start the specified Grand Prix.\n"
    "       --add-gp-dir=DIR   Load Grand Prix files in DIR. Setting will be saved\n"
                              "in config.xml under additional_gp_directory. Use\n"
                              "--add-gp-dir=\"\" to unset.\n"
    "       --stk-config=FILE  use ./data/FILE instead of "
                              "./data/stk_config.xml\n"
    "  -k,  --numkarts=NUM     Set number of karts on the racetrack.\n"
    "       --kart=NAME        Use kart NAME.\n"
    "       --ai=a,b,...       Use the karts a, b, ... for the AI, and additional player kart.\n"
    "       --aiNP=a,b,...     Use the karts a, b, ... for the AI, no additional player kart.\n"
    "       --laps=N           Define number of laps to N, if used in a server all races will use this value.\n"
    "       --mode=N           N=0 Normal, N=1 Time trial, N=2 Battle, N=3 Soccer,\n"
    "                          N=4 Follow The Leader, N=5 Capture The Flag. In configure server use --battle-mode=n\n"
    "                          for battle server and --soccer-timed / goals for soccer server\n"
    "                          to control verbosely, see below:\n"
    "       --difficulty=N     N=0 Beginner, N=1 Intermediate, N=2 Expert, N=3 SuperTux.\n"
    "       --battle-mode=n    Specify battle mode in network, 0 is Free-For-All and\n"
    "                          1 is Capture The Flag.\n"
    "       --soccer-timed     Use time limit mode in network soccer game.\n"
    "       --soccer-goals     Use goals limit mode in network soccer game.\n"
    "       --capture-limit    Specify capture limit for CTF.\n"
    "       --time-limit       Specify time limit for current game mode.\n"
    "       --reverse          Play track in reverse (if allowed)\n"
    "  -f,  --fullscreen       Use fullscreen display.\n"
    "  -w,  --windowed         Use windowed display (default).\n"
    "  -s,  --screensize=WxH   Set the display size (e.g. 320x200).\n"
    "  -v,  --version          Print version of SuperTuxKart.\n"
    "       --trackdir=DIR     A directory from which additional tracks are "
                              "loaded.\n"
    "       --seed=n           Seed for random number generation to provide reproducible behavior.\n"
    "       --profile-laps=n   Enable automatic driven profile mode for n "
                              "laps.\n"
    "       --profile-time=n   Enable automatic driven profile mode for n "
                              "seconds.\n"
    "       --unlock-all       Permanently unlock all karts and tracks for testing.\n"
    "       --no-unlock-all    Disable unlock-all (i.e. base unlocking on player achievement).\n"
    "       --xmas=n           Toggle Xmas/Christmas mode. n=0 Use current date, n=1, Always enable,\n"
    "                          n=2, Always disable.\n"
    "       --easter=n         Toggle Easter ears mode. n=0 Use current date, n=1, Always enable,\n"
    "                          n=2, Always disable.\n"
    "       --no-graphics      Do not display the actual race.\n"
    "       --sp-shader-debug  Enables debug in sp shader, it will print all unavailable uniforms.\n"
    "       --demo-mode=t      Enables demo mode after t seconds of idle time in "
                               "main menu.\n"
    "       --demo-tracks=t1,t2 List of tracks to be used in demo mode. No\n"
    "                          spaces are allowed in the track names.\n"
    "       --demo-laps=n      Number of laps to use in a demo.\n"
    "       --demo-karts=n     Number of karts to use in a demo.\n"
    "       --history          Replay history file 'history.dat'.\n"
    "       --server-config=file Specify the server_config.xml for server hosting, it will create\n"
    "                            one if not found.\n"
    "       --network-console  Enable network console.\n"
    "       --wan-server=name  Start a Wan server (not a playing client).\n"
    "       --public-server    Allow direct connection to the server (without stk server)\n"
    "       --lan-server=name  Start a LAN server (not a playing client).\n"
    "       --server-password= Sets a password for a server (both client and server).\n"
    "       --connect-now=ip   Connect to a server with IP or domain known now\n"
    "                          (in format x.x.x.x:xxx(optional port)), the port should be its\n"
    "                          public port, you can use [::] to replace x.x.x.x for IPv6 address.\n"
    "       --server-id=n      Server id in stk addons for --connect-now.\n"
    "       --network-ai=n     Numbers of AI for connecting to linear race server, used\n"
    "                          together with --connect-now.\n"
    "       --login=s          Automatically log in (set the login).\n"
    "       --password=s       Automatically log in (set the password).\n"
    "       --init-user        Save the above login and password (if set) in config.\n"
    "       --disable-polling  Don't poll for logged in user.\n"
    "       --port=n           Port number to use.\n"
    "       --auto-connect     Automatically connect to first server and start race\n"
    "       --max-players=n    Maximum number of clients (server only).\n"
    "       --min-players=n    Minimum number of clients for ownerless server(server only).\n"
    "       --motd             Message showing in all lobby of clients, can specify a .txt file.\n"
    "       --auto-end         Automatically end network game after 1st player finished\n"
    "                          for some time (currently his finished time * 0.25 + 15.0). \n"
    "       --no-auto-end      Don't automatically end network game after 1st player finished\n"
    "       --team-choosing    Enable choosing team in lobby for team game.\n"
    "       --no-team-choosing Disable choosing team in lobby for team game.\n"
    "       --network-gp=n     Specify number of tracks used in network grand prix.\n"
    "       --graphical-server Enable graphical view in server.\n"
    "       --no-validation    Allow non validated and unencrypted connection in wan.\n"
    "       --ranked           Server will submit ranking to stk addons server.\n"
    "       --no-ranked        Server will not submit ranking to stk addons server.\n"
    "                          You require permission for that.\n"
    "       --owner-less       Race will autostart and no one can kick players in server.\n"
    "       --no-owner-less    Race will not autostart and server owner can kick players in server.\n"
    "       --firewalled-server Turn on all stun related code in server.\n"
    "       --no-firewalled-server Turn off all stun related code in server.\n"
    "       --connection-debug Print verbose info for sending or receiving packets.\n"
    "       --no-console-log   Does not write messages in the console but to\n"
    "                          stdout.log.\n"
    "  -h,  --help             Show this help.\n"
    "       --help-debug       Show help for debugging options.\n"
    "       --log=N            Set the verbosity to a value between\n"
    "                          0 (Debug) and 5 (Only Fatal messages)\n"
    "       --logbuffer=N      Buffers up to N lines log lines before writing.\n"
    "       --root=DIR         Path to add to the list of STK root directories.\n"
    "                          You can specify more than one by separating them\n"
    "                          with colons (:).\n"
    "       --cutscene=NAME    Launch the specified track as a cutscene.\n"
    "                          This is for internal debugging use only.\n"
    "       --enable-glow      Enable glow effect.\n"
    "       --disable-glow     Disable glow effect.\n"
    "       --enable-bloom     Enable bloom effect.\n"
    "       --disable-bloom    Disable bloom effect.\n"
    "       --enable-light-shaft Enable light shafts (God rays).\n"
    "       --disable-light-shaft Disable light shafts (God rays).\n"
    "       --enable-dof       Enable depth of field.\n"
    "       --disable-dof      Disable depth of field.\n"
    "       --enable-particles  Enable particles.\n"
    "       --disable-particles Disable particles.\n"
    "       --enable-animated-characters  Enable animated characters.\n"
    "       --disable-animated-characters Disable animated characters.\n"
    "       --enable-motion-blur Enable motion blur.\n"
    "       --disable-motion-blur Disable motion blur.\n"
    "       --enable-mlaa      Enable anti-aliasing.\n"
    "       --disable-mlaa     Disable anti-aliasing.\n"
    "       --enable-texture-compression Enable texture compression.\n"
    "       --disable-texture-compression Disable texture compression.\n"
    "       --enable-ssao      Enable screen space ambient occlusion.\n"
    "       --disable-ssao     Disable screen space ambient occlusion.\n"
    "       --enable-ibl       Enable image based lighting.\n"
    "       --disable-ibl      Disable image based lighting.\n"
    "       --enable-hd-textures Enable high definition textures.\n"
    "       --disable-hd-textures Disable high definition textures.\n"
    "       --enable-dynamic-lights Enable advanced pipeline.\n"
    "       --disable-dynamic-lights Disable advanced pipeline.\n"
    "       --anisotropic=n     Anisotropic filtering quality (0 to disable).\n"
    "                           Takes precedence over trilinear or bilinear\n"
    "                           texture filtering.\n"
    "       --shadows=n         Set resolution of shadows (0 to disable).\n"
    "       --render-driver=n   Render driver to use (gl or directx9).\n"
    "       --disable-addon-karts Disable loading of addon karts.\n"
    "       --disable-addon-tracks Disable loading of addon tracks.\n"
    "       --dump-official-karts Dump official karts for current stk-assets.\n"
    "       --apitrace          This will disable buffer storage and\n"
    "                           writing gpu query strings to opengl, which\n"
    "                           can be seen later in apitrace.\n"
#ifdef ENABLE_WIIUSE
    "       --wii               Enable usage of Wii Remotes.\n"
#endif
#if defined(__linux__) || defined(__FreeBSD__)
    "\n"
    "Environment variables:\n"
    "       IRR_DEVICE_TYPE=[x11, wayland] Force x11/wayland device\n"
    "       IRR_DISABLE_NETWM=1            Force to use legacy fullscreen\n"
    "       IRR_VIDEO_OUTPUT=output_name   Force to use selected monitor for\n"
    "                                      fullscreen window, eg. HDMI-0\n"
#endif
    "\n"
    "You can visit SuperTuxKart's homepage at "
    "https://supertuxkart.net\n\n",
    CommandLine::getExecName().c_str()
    );
}   // cmdLineHelp

void cmdDebugHelp()
{
    fprintf(stdout,
    "Usage: %s [OPTIONS]\n\n"
    "Run SuperTuxKart, a go-kart racing game that features "
    "Tux and friends.\n\n"
    "Debug options (some work only if artist debug mode is enabled):\n"
    "       --debug=s                   s=all Log everything, s=addons Log addons management,\n"
    "                                   s=gui Log GUI events, s=flyable Log flyables,\n"
    "                                   s=memory Log memory usage, s=misc Log other events.\n"
    "       --gamepad-visuals           Debug gamepads by visualising their values.\n"
    "       --no-high-scores            Disable writing high scores.\n"
    "       --unit-testing              Run unit tests and exit.\n"
    "       --gamepad-debug             Enable verbose logging of gamepad button presses.\n"
    "       --keyboard-debug            Enable verbose logging of keyboard key presses.\n"
    "       --wiimote-debug             Enable verbose logging of Wii Remote button presses.\n"
    "       --tutorial-debug            Enable verbose logging of the tutorial mode.\n"
    "       --track-debug               Enable displaying the driveline in tracks.\n"
    "       --check-debug               Enable displaying the checklines in tracks.\n"
    "       --kartsize-debug            Enable verbose logging of kart dimensions\n"
    "                                   and mesh buffer counts.\n"
    "       --physics-debug             Enable verbose logging of the physics system.\n"
    "       --material-debug            Enable verbose logging of terrain specific slowdowns.\n"
    "       --ftl-debug                 Enable verbose logging in follow-the-leader mode.\n"
    "       --slipstream-debug          Enable displaying of slipstreams more prominently.\n"
    "       --rendering-debug           Enable displaying of ambient/diffuse/specularity\n"
    "                                   in RGB & all anisotropic.\n"
    "       --ai-debug                  Enable displaying of AI controllers as on-screen text.\n"
    "                                   Makes it easier to distinguish between different AI controllers.\n"
    "       --fps-debug                 Enable verbose logging of the FPS counter on every frame.\n"
    "       --rewind                    Enable the rewind manager.\n"
    "       --battle-ai-stats           Enable verbose logging of AI karts in battle modes.\n"
    "       --soccer-ai-stats           Enable verbose logging of AI karts in soccer mode.\n"
    "       --test-ai=n                 Use the test-ai for every n-th AI kart.\n"
    "                                   (so n=1 means all Ais will be the test ai)\n"
    "       --disable-item-collection   Disable item collection. Useful for\n"
    "                                   debugging client/server item management.\n"
    "       --network-item-debugging    Print item handling debug information.\n"
    "\n"
    "You can visit SuperTuxKart's homepage at "
    "https://supertuxkart.net\n\n",
    CommandLine::getExecName().c_str()
    );
}   // cmdDebugHelp

//=============================================================================
/** For base options that modify the output (loglevel/color) or exit right
 * after being processed (version/help).
 */
int handleCmdLineOutputModifier()
{
    if (CommandLine::has("--help") || CommandLine::has("-help") ||
        CommandLine::has("-h"))
    {
        cmdLineHelp();
        cleanUserConfig();
        exit(0);
    }

    if (CommandLine::has("--help-debug"))
    {
        cmdDebugHelp();
        cleanUserConfig();
        exit(0);
    }

    if(CommandLine::has("--version") || CommandLine::has("-v"))
    {
        Log::info("main", "==============================");
        Log::info("main", "SuperTuxKart, %s.", STK_VERSION ) ;
        Log::info("main", "==============================");
        cleanUserConfig();
        exit(0);
    }

    int n;
    if(CommandLine::has("--log", &n))
        Log::setLogLevel(n);
    if (CommandLine::has("--logbuffer", &n))
        Log::setBufferSize(n);

    if(CommandLine::has("--log=nocolor"))
    {
        Log::disableColor();
        Log::verbose("main", "Colours disabled.");
    }
    if(CommandLine::has("--no-console-log"))
        Log::toggleConsoleLog(false);

    return 0;
}

//=============================================================================
/** For base options that don't need much to be inited (and, in some cases,
 *  that need to be read before initing stuff) - it only assumes that
 *  user config is loaded (necessary to check for blacklisted screen
 *  resolutions), but nothing else (esp. not kart_properties_manager and
 *  track_manager, since their search path might be extended by command
 *  line options).
 */
int handleCmdLinePreliminary()
{
    if(CommandLine::has("--gamepad-visuals"))
        UserConfigParams::m_gamepad_visualisation=true;
    if(CommandLine::has("--debug=memory"))
        UserConfigParams::m_verbosity |= UserConfigParams::LOG_MEMORY;
    if(CommandLine::has("--debug=addons"))
        UserConfigParams::m_verbosity |= UserConfigParams::LOG_ADDONS;
    if(CommandLine::has("--debug=gui"))
        UserConfigParams::m_verbosity |= UserConfigParams::LOG_GUI;
    if(CommandLine::has("--debug=flyable"))
        UserConfigParams::m_verbosity |= UserConfigParams::LOG_FLYABLE;
    if(CommandLine::has("--debug=misc"))
        UserConfigParams::m_verbosity |= UserConfigParams::LOG_MISC;
    if(CommandLine::has("--debug=all") )
        UserConfigParams::m_verbosity |= UserConfigParams::LOG_ALL;
    if(CommandLine::has("--online"))
        History::m_online_history_replay = true;
#if !(defined(SERVER_ONLY) || defined(ANDROID))
    if(CommandLine::has("--apitrace"))
    {
        SP::sp_apitrace = true;
    }
#endif

    std::string s;
    if(CommandLine::has("--stk-config", &s))
    {
        stk_config->load(file_manager->getAsset(s));
        Log::info("main", "STK config will be read from %s.",s.c_str());
    }
    if(CommandLine::has("--render-driver", &s))
        UserConfigParams::m_render_driver = s;
    if(CommandLine::has("--trackdir", &s))
        TrackManager::addTrackSearchDir(s);
    if(CommandLine::has("--kartdir", &s))
        KartPropertiesManager::addKartSearchDir(s);

    if (CommandLine::has("--sp-shader-debug"))
        SP::SPShader::m_sp_shader_debug = true;

    if(CommandLine::has("--screensize", &s) || CommandLine::has("-s", &s))
    {
        //Check if fullscreen and new res is blacklisted
        int width, height;
        if (sscanf(s.c_str(), "%dx%d", &width, &height) == 2)
        {
            // Reassemble the string in case that the original width or
            // height contained a leading 0
            std::ostringstream o;
            o << width << "x" << height;
            std::string res = o.str();
            if (!UserConfigParams::m_fullscreen ||
                std::find(UserConfigParams::m_blacklist_res.begin(),
                UserConfigParams::m_blacklist_res.end(),res) ==
                UserConfigParams::m_blacklist_res.end())
            {
                UserConfigParams::m_prev_real_width =
                    UserConfigParams::m_real_width = width;
                UserConfigParams::m_prev_real_height =
                    UserConfigParams::m_real_height = height;
                Log::verbose("main", "You choose to use %dx%d.",
                    (int)UserConfigParams::m_real_width,
                    (int)UserConfigParams::m_real_height );
            }
            else
                Log::warn("main", "Resolution %s has been blacklisted, so "
                "it is not available!", res.c_str());
        }
        else
        {
            Log::fatal("main", "Error: --screensize argument must be "
                "given as WIDTHxHEIGHT");
        }
    }

    if(CommandLine::has("--fullscreen") || CommandLine::has("-f"))
    {
        // Check that current res is not blacklisted
        std::ostringstream o;
        o << UserConfigParams::m_width << "x" << UserConfigParams::m_height;
        std::string res = o.str();
        if (std::find(UserConfigParams::m_blacklist_res.begin(),
                      UserConfigParams::m_blacklist_res.end(),res)
                   == UserConfigParams::m_blacklist_res.end())
            UserConfigParams::m_fullscreen = true;
        else
            Log::warn("main", "Resolution %s has been blacklisted, so it "
            "is not available!", res.c_str());
    }

    if(CommandLine::has("--windowed") || CommandLine::has("-w"))
        UserConfigParams::m_fullscreen = false;

    // toggle graphical options
    if (CommandLine::has("--enable-glow"))
        UserConfigParams::m_glow = true;
    else if (CommandLine::has("--disable-glow"))
        UserConfigParams::m_glow = false;
    if (CommandLine::has("--enable-bloom"))
        UserConfigParams::m_bloom = true;
    else if (CommandLine::has("--disable-bloom"))
        UserConfigParams::m_bloom = false;
    if (CommandLine::has("--enable-light-shaft"))
        UserConfigParams::m_light_shaft = true;
    else if (CommandLine::has("--disable-light-shaft"))
        UserConfigParams::m_light_shaft = false;
    if (CommandLine::has("--enable-dynamic-lights"))
        UserConfigParams::m_dynamic_lights = true;
    else if (CommandLine::has("--disable-dynamic-lights"))
        UserConfigParams::m_dynamic_lights = false;
    // depth of field
    if (CommandLine::has("--enable-dof"))
        UserConfigParams::m_dof = true;
    else if (CommandLine::has("--disable-dof"))
        UserConfigParams::m_dof = false;
    // particles effects
    if (CommandLine::has("--enable-particles"))
        UserConfigParams::m_particles_effects = 2;
    else if (CommandLine::has("--disable-particles"))
        UserConfigParams::m_particles_effects = 0;
    // animated characters
    if (CommandLine::has("--enable-animated-characters"))
        UserConfigParams::m_animated_characters = true;
    else if (CommandLine::has("--disable-animated-characters"))
        UserConfigParams::m_animated_characters = false;
    if (CommandLine::has("--enable-motion-blur"))
        UserConfigParams::m_motionblur = true;
    else if (CommandLine::has("--disable-motion-blur"))
        UserConfigParams::m_motionblur = false;
    // anti-aliasing
    if (CommandLine::has("--enable-mlaa"))
        UserConfigParams::m_mlaa = true;
    else if (CommandLine::has("--disable-mlaa"))
        UserConfigParams::m_mlaa = false;
    if (CommandLine::has("--enable-texture-compression"))
        UserConfigParams::m_texture_compression = true;
    else if (CommandLine::has("--disable-texture-compression"))
        UserConfigParams::m_texture_compression = false;
    // screen space ambient occluison
    if (CommandLine::has("--enable-ssao"))
        UserConfigParams::m_ssao = true;
    else if (CommandLine::has("--disable-ssao"))
        UserConfigParams::m_ssao = false;
    // image based lighting
    if (CommandLine::has("--enable-ibl"))
        UserConfigParams::m_degraded_IBL = false;
    else if (CommandLine::has("--disable-ibl"))
        UserConfigParams::m_degraded_IBL = true;
    // high definition textures user choice
    if (CommandLine::has("--enable-hd-textures"))
        UserConfigParams::m_high_definition_textures =  2 | 1;
    else if (CommandLine::has("--disable-hd-textures"))
        UserConfigParams::m_high_definition_textures = 2;

    // Enable loading grand prix from local directory
    if(CommandLine::has("--add-gp-dir", &s))
    {
        // Ensure that the path ends with a /
        if (s[s.size()] == '/')
            UserConfigParams::m_additional_gp_directory = s;
        else
            UserConfigParams::m_additional_gp_directory = s + "/";

        Log::info("main", "Additional Grand Prix's will be loaded from %s",
                           UserConfigParams::m_additional_gp_directory.c_str());
    }

    int n;
    if(CommandLine::has("--xmas", &n))
        UserConfigParams::m_xmas_mode = n;
    if (CommandLine::has("--easter", &n))
        UserConfigParams::m_easter_ear_mode = n;
    if (CommandLine::has("--shadows", &n))
        UserConfigParams::m_shadows_resolution = n;
    if (CommandLine::has("--anisotropic", &n))
        UserConfigParams::m_anisotropic = n;

    // Useful for debugging: the temple navmesh needs 12 minutes in debug
    // mode to compute the distance matrix!!
    if(CommandLine::has("--dont-load-navmesh"))
        Track::m_dont_load_navmesh = true;

    if (CommandLine::has("--no-sound"))
        UserConfigParams::m_enable_sound = false;

    if (CommandLine::has("--seed", &n))
    {
        srand(n);
        Log::info("main", "STK using random seed (%d)", n);
    }

    if (CommandLine::has("--disable-addon-karts"))
        UserConfigParams::m_disable_addon_karts = true;
    if (CommandLine::has("--disable-addon-tracks"))
        UserConfigParams::m_disable_addon_tracks = true;

    return 0;
}   // handleCmdLinePreliminary

// ============================================================================
/** Handles command line options.
 *  \param argc Number of command line options
 */
int handleCmdLine(bool has_server_config, bool has_parent_process)
{
    // Some generic variables used in scanning:
    int n;
    std::string s;

    irr::core::stringw login, password;

    if (CommandLine::has("--no-high-scores"))
        UserConfigParams::m_no_high_scores=true;
    if (CommandLine::has("--unit-testing"))
        UserConfigParams::m_unit_testing = true;
    if (CommandLine::has("--gamepad-debug"))
        UserConfigParams::m_gamepad_debug=true;
    if (CommandLine::has("--keyboard-debug"))
        UserConfigParams::m_keyboard_debug = true;
    if (CommandLine::has("--wiimote-debug"))
        UserConfigParams::m_wiimote_debug = true;
    if(CommandLine::has("--tutorial-debug"))
            UserConfigParams::m_tutorial_debug = true;
    if(CommandLine::has( "--track-debug",&n))
        UserConfigParams::m_track_debug=n;
    if(CommandLine::has( "--track-debug"))
        UserConfigParams::m_track_debug=1;
    if(CommandLine::has("--material-debug"))
        UserConfigParams::m_material_debug = true;
    if(CommandLine::has("--ftl-debug"))
        UserConfigParams::m_ftl_debug = true;
    if(CommandLine::has("--slipstream-debug"))
        UserConfigParams::m_slipstream_debug = true;
    if(CommandLine::has("--rendering-debug"))
        UserConfigParams::m_rendering_debug=true;
    if(CommandLine::has("--ai-debug"))
        AIBaseController::enableDebug();
    if(CommandLine::has("--test-ai", &n))
        AIBaseController::setTestAI(n);
    if (CommandLine::has("--fps-debug"))
        UserConfigParams::m_fps_debug = true;
    if (CommandLine::has("--rewind") )
        RewindManager::setEnable(true);
    if(CommandLine::has("--soccer-ai-stats"))
    {
        UserConfigParams::m_arena_ai_stats=true;
        RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_SOCCER);
        std::vector<std::string> l;
        for (int i = 0; i < 8; i++)
            l.push_back("tux");
        RaceManager::get()->setDefaultAIKartList(l);
        RaceManager::get()->setNumKarts(9);
        RaceManager::get()->setMaxGoal(30);
        RaceManager::get()->setTrack("soccer_field");
        RaceManager::get()->setDifficulty(RaceManager::Difficulty(3));
        UserConfigParams::m_no_start_screen = true;
        UserConfigParams::m_race_now = true;
        UserConfigParams::m_sfx = false;
        UserConfigParams::m_music = false;
    }
    if(CommandLine::has("--battle-ai-stats"))
    {
        std::string track;
        if (!CommandLine::has("--track", &track))
            track = "temple";
        UserConfigParams::m_arena_ai_stats=true;
        RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_3_STRIKES);
        std::vector<std::string> l;
        for (int i = 0; i < 8; i++)
            l.push_back("tux");
        RaceManager::get()->setDefaultAIKartList(l);
        RaceManager::get()->setTrack(track);
        RaceManager::get()->setNumKarts(8);
        RaceManager::get()->setDifficulty(RaceManager::Difficulty(3));
        UserConfigParams::m_no_start_screen = true;
        UserConfigParams::m_race_now = true;
        UserConfigParams::m_sfx = false;
        UserConfigParams::m_music = false;
    }

    if (UserConfigParams::m_artist_debug_mode)
    {
        if(CommandLine::has("--physics-debug"))
            UserConfigParams::m_physics_debug=1;
        if(CommandLine::has("--check-debug"))
            UserConfigParams::m_check_debug=true;
    }

    bool init_user = CommandLine::has("--init-user");
    if (init_user)
    {
        PlayerManager::get()->enforceCurrentPlayer();
        PlayerManager::getCurrentPlayer()->setRememberPassword(true);
        UserConfigParams::m_internet_status = Online::RequestManager::IPERM_ALLOWED;
    }
    if (CommandLine::has("--login", &s))
        login = s.c_str();
    if (CommandLine::has("--password", &s))
        password = s.c_str();

    bool can_wan = false;
    if (!login.empty() && !password.empty())
    {
        if (!PlayerManager::getCurrentPlayer())
        {
            Log::error("Main", "Run supertuxkart with --init-user");
            cleanSuperTuxKart();
            return false;
        }
        irr::core::stringw s;
        PlayerManager::requestSignIn(login, password);
        uint64_t started_time = StkTime::getMonoTimeMs();
        while (PlayerManager::getCurrentOnlineState() != PlayerProfile::OS_SIGNED_IN)
        {
            Online::RequestManager::get()->update(0.0f);
            StkTime::sleep(1);
            if (StkTime::getMonoTimeMs() > started_time + 20000)
            {
                Log::error("Main",
                    "Timed out trying login, check login info or connection "
                    "to stk addons.");
                cleanSuperTuxKart();
                return false;
            }
        }
        Log::info("Main", "Logged in from command-line.");
        if (init_user)
            PlayerManager::getCurrentPlayer()->setWasOnlineLastTime(true);
        can_wan = true;
    }
    if (init_user)
    {
        Log::info("Main", "Done saving user, leaving");
        cleanSuperTuxKart();
        return false;
    }

    if (CommandLine::has( "--network-ai-freq", &n))
        NetworkAIController::setAIFrequency(n);
    else
        NetworkAIController::setAIFrequency(30);

    if (!can_wan && CommandLine::has("--login-id", &n) &&
        CommandLine::has("--token", &s))
    {
        NetworkConfig::get()->setCurrentUserId(n);
        NetworkConfig::get()->setCurrentUserToken(s);
        can_wan = true;
    }

    if (CommandLine::has( "--difficulty", &s))
    {
        int n = atoi(s.c_str());
        if (n < 0 || n > RaceManager::DIFFICULTY_LAST)
        {
            Log::warn("main", "Invalid difficulty '%s', use easy.\n",
                      s.c_str());
            RaceManager::get()->setDifficulty(RaceManager::Difficulty(0));
            ServerConfig::m_server_difficulty = 0;
        }
        else
        {
            RaceManager::get()->setDifficulty(RaceManager::Difficulty(n));
            ServerConfig::m_server_difficulty = n;
        }
    }   // --difficulty

    if (CommandLine::has("--mode", &n))
    {
        switch (n)
        {
        // The order here makes server creation screen easier
        case 0:
        {
            ServerConfig::m_server_mode = 3;
            RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_NORMAL_RACE);
            break;
        }
        case 1:
        {
            ServerConfig::m_server_mode = 4;
            RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_TIME_TRIAL);
            break;
        }
        case 2:
        {
            ServerConfig::m_server_mode = 7;
            RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_FREE_FOR_ALL);
            break;
        }
        case 3:
        {
            ServerConfig::m_server_mode = 6;
            RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_SOCCER);
            break;
        }
        case 4:
        {
            RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_FOLLOW_LEADER);
            break;
        }
        case 5:
        {
            ServerConfig::m_server_mode = 8;
            RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_CAPTURE_THE_FLAG);
            break;
        }
        default:
            Log::warn("main", "Invalid race mode '%d' - ignored.", n);
        }
    }   // --mode

    const bool is_soccer =
        RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_SOCCER;
    const bool is_battle = RaceManager::get()->isBattleMode();

    if (!has_server_config)
    {
        if (CommandLine::has("--soccer-timed") && is_soccer)
            ServerConfig::m_soccer_goal_target = false;
        else if (CommandLine::has("--soccer-goals") && is_soccer)
            ServerConfig::m_soccer_goal_target = true;
        else if (CommandLine::has("--network-gp", &n))
        {
            ServerConfig::m_gp_track_count = n;
            if (ServerConfig::m_server_mode == 3)
                ServerConfig::m_server_mode = 0;
            else
                ServerConfig::m_server_mode = 1;
        }
        else if (CommandLine::has("--battle-mode", &n) && is_battle)
        {
            switch (n)
            {
            case 0:
                ServerConfig::m_server_mode = 7;
                break;
            case 1:
                ServerConfig::m_server_mode = 8;
                break;
            default:
                break;
            }
        }
        else if (is_battle)
        {
            Log::warn("main", "Set to ffa for battle server");
            ServerConfig::m_server_mode = 7;
        }
        else if (is_soccer)
        {
            Log::warn("main", "Set to goal target for soccer server");
            ServerConfig::m_soccer_goal_target = true;
        }
    }

    if (CommandLine::has("--network-console"))
    {
        ServerConfig::m_enable_console = true;
        STKHost::m_enable_console = true;
    }
    else if (ServerConfig::m_enable_console &&
        NetworkConfig::get()->isServer() && !has_parent_process)
    {
        STKHost::m_enable_console = true;
    }

    if (CommandLine::has("--disable-item-collection"))
        ItemManager::disableItemCollection();

    if (CommandLine::has("--network-item-debugging"))
        NetworkItemManager::m_network_item_debugging = true;
    
    std::string server_password;
    if (CommandLine::has("--server-password", &s))
    {
        ServerConfig::m_private_server_password = s;
        server_password = s;
    }

    if (CommandLine::has("--motd", &s))
        ServerConfig::m_motd = s;

    if (CommandLine::has("--team-choosing"))
    {
        ServerConfig::m_team_choosing = true;
    }
    if (CommandLine::has("--no-team-choosing"))
    {
        ServerConfig::m_team_choosing = false;
    }
    if (CommandLine::has("--ranked"))
    {
        ServerConfig::m_ranked = true;
    }
    if (CommandLine::has("--no-ranked"))
    {
        ServerConfig::m_ranked = false;
    }
    if (CommandLine::has("--auto-end"))
    {
        ServerConfig::m_auto_end = true;
    }
    if (CommandLine::has("--no-auto-end"))
    {
        ServerConfig::m_auto_end = false;
    }
    if (CommandLine::has("--owner-less"))
    {
        ServerConfig::m_owner_less = true;
    }
    if (CommandLine::has("--no-owner-less"))
    {
        ServerConfig::m_owner_less = false;
    }
    if (CommandLine::has("--firewalled-server"))
    {
        ServerConfig::m_firewalled_server = true;
    }
    if (CommandLine::has("--no-firewalled-server"))
    {
        ServerConfig::m_firewalled_server = false;
    }
    if (CommandLine::has("--connection-debug"))
    {
        Network::m_connection_debug = true;
    }
    if (CommandLine::has("--server-id-file", &s))
    {
        NetworkConfig::get()->setServerIdFile(
            file_manager->getUserConfigFile(s));
        ServerConfig::m_server_configurable = true;
    }
    if (CommandLine::has("--disable-polling"))
    {
        Online::RequestManager::m_disable_polling = true;
    }
    if (CommandLine::has("--max-players", &n))
    {
        ServerConfig::m_server_max_players = n;
    }
    if (ServerConfig::m_server_max_players < 1)
    {
        ServerConfig::m_server_max_players = 1;
    }

    if (CommandLine::has("--min-players", &n))
    {
        if (n > ServerConfig::m_server_max_players)
            n = 1;
        ServerConfig::m_min_start_game_players = n;
    }
    if (CommandLine::has("--port", &n))
    {
        // We don't know if this instance is going to be a client
        // or server, so just set both ports, only one will be used anyway
        NetworkConfig::get()->setClientPort(n);
        ServerConfig::m_server_port = n;
    }
    if (CommandLine::has("--public-server"))
    {
        NetworkConfig::get()->setIsPublicServer();
    }

    unsigned server_id = 0;
    if ((NetworkConfig::get()->isServer() && ServerConfig::m_wan_server) ||
        CommandLine::has("--server-id", &server_id))
    {
        PlayerProfile* player = PlayerManager::getCurrentPlayer();
        // Try to use saved user token if exists
        if (!can_wan && player && player->wasOnlineLastTime() &&
            player->wasOnlineLastTime() && player->hasSavedSession())
        {
            uint64_t started_time = StkTime::getMonoTimeMs();
            while (PlayerManager::getCurrentOnlineState() !=
                PlayerProfile::OS_SIGNED_IN)
            {
                Online::RequestManager::get()->update(0.0f);
                StkTime::sleep(1);
                if (StkTime::getMonoTimeMs() > started_time + 20000)
                {
                    Log::error("Main",
                        "Timed out trying to login saved session, check "
                        "connection to stk addons or rerun --init-user.");
                    cleanSuperTuxKart();
                    return false;
                }
            }
            can_wan = true;
        }
        else if (!can_wan)
        {
            Log::warn("main","No saved online player session to create "
                "or connect to a wan server.");
        }
    }

    int ai_num = 0;
    if (CommandLine::has("--server-ai", &ai_num))
        NetworkConfig::get()->setNumFixedAI(ai_num);

    std::string addr;
    bool has_addr = CommandLine::has("--connect-now", &addr);
    if (has_addr)
    {
        NetworkConfig::get()->setIsServer(false);
        if (CommandLine::has("--network-ai", &n))
        {
            // We need an existing current player
            PlayerManager::get()->enforceCurrentPlayer();
            NetworkConfig::get()->setNetworkAIInstance(true);
            PlayerManager::get()->createGuestPlayers(n);
            for (int i = 0; i < n; i++)
            {
                NetworkConfig::get()->addNetworkPlayer(
                    NULL, PlayerManager::get()->getPlayer(i),
                    HANDICAP_NONE);
            }
        }
        else
        {
            NetworkConfig::get()->addNetworkPlayer(
                input_manager->getDeviceManager()->getLatestUsedDevice(),
                PlayerManager::getCurrentPlayer(), HANDICAP_NONE);
        }
        SocketAddress server_addr(addr);
        if (server_addr.getIP() == 0 && !server_addr.isIPv6())
        {
            Log::error("Main", "Invalid server address: %s", addr.c_str());
            cleanSuperTuxKart();
            return false;
        }
        SocketAddress ipv4_addr = server_addr;
        if (server_addr.isIPv6())
            ipv4_addr.setIP(0);
        auto server = std::make_shared<UserDefinedServer>(
            StringUtils::utf8ToWide(addr), ipv4_addr,
            !server_password.empty());
        if (server_addr.isIPv6())
        {
            server->setIPV6Address(server_addr);
            server->setIPV6Connection(true);
        }
        NetworkConfig::get()->doneAddingNetworkPlayers();
        if (server_id != 0)
        {
            NetworkConfig::get()->setIsWAN();
            server->setServerId(server_id);
            server->setSupportsEncryption(true);
        }
        else
            NetworkConfig::get()->setIsLAN();
        STKHost::create();
        if (!GUIEngine::isNoGraphics())
            NetworkingLobby::getInstance()->setJoinedServer(server);
        else if (NetworkConfig::get()->isClient())
            std::make_shared<ConnectToServer>(server)->requestStart();
    }

    if (NetworkConfig::get()->isServer())
    {
        PlayerManager::get()->enforceCurrentPlayer();
        const std::string& server_name = ServerConfig::m_server_name;
        if (ServerConfig::m_wan_server)
        {
            if (can_wan)
            {
                // Server owner online account will keep online as long as
                // server is live
                Online::RequestManager::m_disable_polling = true;
                // For server we assume it is an IPv4 one, because if it fails
                // to detect the server won't start at all
                if (UserConfigParams::m_default_ip_type == NetworkConfig::IP_NONE)
                {
                    NetworkConfig::get()->setIPType(NetworkConfig::IP_V4);
                    NetworkConfig::get()->queueIPDetection();
                }
                // Longer timeout for server creation
                NetworkConfig::get()->getIPDetectionResult(4000);
                NetworkConfig::get()->setIsWAN();
                NetworkConfig::get()->setIsPublicServer();
                ServerConfig::loadServerLobbyFromConfig();
                Log::info("main", "Creating a WAN server '%s'.",
                    server_name.c_str());
            }
        }
        else
        {
            NetworkConfig::get()->setIsLAN();
            ServerConfig::loadServerLobbyFromConfig();
            Log::info("main", "Creating a LAN server '%s'.",
                server_name.c_str());
        }
    }

    if (CommandLine::has("--auto-connect"))
    {
        NetworkConfig::get()->setAutoConnect(true);
    }

    if (CommandLine::has("--capture-limit", &n))
    {
        RaceManager::get()->setHitCaptureTime(n, 0.0f);
    }

    if (CommandLine::has("--time-limit", &n))
    {
        RaceManager::get()->setTimeTarget(n);
    }

    // Race parameters
    if(CommandLine::has("--kartsize-debug"))
    {
        for(unsigned int i=0; i<kart_properties_manager->getNumberOfKarts();
           i++)
        {
            const KartProperties *km =
                kart_properties_manager->getKartById(i);
            Log::info("main", "%s:\t%swidth: %f length: %f height: %f "
                      "mesh-buffer count %d",
                      km->getIdent().c_str(),
                      (km->getIdent().size()<7) ? "\t" : "",
                      km->getMasterKartModel().getWidth(),
                      km->getMasterKartModel().getLength(),
                      km->getMasterKartModel().getHeight(),
                      km->getMasterKartModel().getModel()
                        ->getMeshBufferCount());
        }    // for i
    }   // --kartsize-debug

    if(CommandLine::has("--kart", &s))
    {
        const KartProperties *prop = kart_properties_manager->getKart(s);
        if (prop)
        {
            UserConfigParams::m_default_kart = s;

            // if a player was added with -N, change its kart.
            // Otherwise, nothing to do, kart choice will be picked
            // up upon player creation.
            if (StateManager::get()->activePlayerCount() > 0)
            {
                RaceManager::get()->setPlayerKart(0, s);
            }
            Log::verbose("main", "You chose to use kart '%s'.",
                         s.c_str());
        }
        else
        {
            Log::warn("main", "Kart '%s' not found, ignored.",
                      s.c_str());
        }
    }   // if --kart

    if(CommandLine::has("--ai", &s))
    {
        const std::vector<std::string> l=StringUtils::split(std::string(s),',');
        RaceManager::get()->setDefaultAIKartList(l);
        // Add 1 for the player kart
        RaceManager::get()->setNumKarts((int)l.size()+1);
    }   // --ai

    if(CommandLine::has("--aiNP", &s))
    {
        const std::vector<std::string> l=StringUtils::split(std::string(s),',');
        RaceManager::get()->setDefaultAIKartList(l);
        RaceManager::get()->setNumKarts((int)l.size());
    }   // --aiNP

    if(CommandLine::has("--reverse"))
    {
        RaceManager::get()->setReverseTrack(true);
    }

    if(CommandLine::has("--track", &s) || CommandLine::has("-t", &s))
    {
        RaceManager::get()->setTrack(s);
        Log::verbose("main", "You chose to start in track '%s'.",
                     s.c_str());

        Track* t = track_manager->getTrack(s);
        if (!t)
        {
            Log::warn("main", "Can't find track named '%s'.", s.c_str());
        }
        else if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_CAPTURE_THE_FLAG)
        {
            // CTF has no ai support atm
            const std::vector<std::string> l;
            RaceManager::get()->setDefaultAIKartList(l);
            // Add 1 for the player kart
            RaceManager::get()->setNumKarts(1);
        }
        else if (t->isArena())
        {
            //if it's arena, don't create AI karts
            const std::vector<std::string> l;
            RaceManager::get()->setDefaultAIKartList(l);
            // Add 1 for the player kart
            RaceManager::get()->setNumKarts(1);
            RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_3_STRIKES);
        }
        else if (t->isSoccer())
        {
            //if it's soccer, don't create AI karts
            const std::vector<std::string> l;
            RaceManager::get()->setDefaultAIKartList(l);
            // Add 1 for the player kart
            RaceManager::get()->setNumKarts(1);
            RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_SOCCER);
        }
    }   // --track

    // used only for debugging/testing
    if (CommandLine::has("--cutscene", &s))
    {
        UserConfigParams::m_no_start_screen = true; // Purple menu background otherwise
        RaceManager::get()->setTrack(s);
        StateManager::get()->enterGameState();
        RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_CUTSCENE);
        RaceManager::get()->setNumKarts(0);
        RaceManager::get()->setNumPlayers(0);
        RaceManager::get()->setNumLaps(999);
    } // --cutscene

    if(CommandLine::has("--gp", &s))
    {
        RaceManager::get()->setMajorMode(RaceManager::MAJOR_MODE_GRAND_PRIX);
        const GrandPrixData *gp = grand_prix_manager->getGrandPrix(s);

        if (!gp)
        {
            Log::warn("main", "There is no GP named '%s'.", s.c_str());
            return 0;
        }
        RaceManager::get()->setGrandPrix(*gp);
    }   // --gp

    if(CommandLine::has("--numkarts", &n) ||CommandLine::has("-k", &n))
    {
        UserConfigParams::m_default_num_karts = n;
        if(UserConfigParams::m_default_num_karts > stk_config->m_max_karts)
        {
            Log::warn("main", "Number of karts reset to maximum number %d.",
                      stk_config->m_max_karts);
            UserConfigParams::m_default_num_karts = stk_config->m_max_karts;
        }
        RaceManager::get()->setNumKarts( UserConfigParams::m_default_num_karts );
        Log::verbose("main", "%d karts will be used.",
                     (int)UserConfigParams::m_default_num_karts);
    }   // --numkarts

    if(CommandLine::has( "--no-start-screen") ||
        CommandLine::has("-N")                   )
        UserConfigParams::m_no_start_screen = true;
    if(CommandLine::has("--race-now") || CommandLine::has("-R"))
    {
        UserConfigParams::m_no_start_screen = true;
        UserConfigParams::m_race_now = true;
    }   // --race-now

    if(CommandLine::has("--laps", &s))
    {
        int laps = atoi(s.c_str());
        if (laps < 0)
        {
            Log::error("main", "Invalid number of laps: %s.\n", s.c_str());
            return 0;
        }
        else
        {
            Log::verbose("main", "You chose to have %d laps.", laps);
            if (NetworkConfig::get()->isServer())
                ServerLobby::m_fixed_laps = laps;
            else
                RaceManager::get()->setNumLaps(laps);
        }
    }   // --laps

    if(CommandLine::has("--profile-laps",  &n))
    {
        if (n < 0)
        {
            Log::error("main", "Invalid number of profile-laps: %i.", n );
            return 0;
        }
        else
        {
            Log::verbose("main", "Profiling %d laps.",n);
            UserConfigParams::m_no_start_screen = true;
            ProfileWorld::setProfileModeLaps(n);
            RaceManager::get()->setNumLaps(n);
        }
    }   // --profile-laps
    
    if(CommandLine::has("--unlock-all"))
    {
        UserConfigParams::m_unlock_everything = 2;
    } // --unlock-all
    
    if(CommandLine::has("--no-unlock-all"))
    {
        UserConfigParams::m_unlock_everything = 0;
    } // --no-unlock-all
    
    if(CommandLine::has("--profile-time",  &n))
    {
        Log::verbose("main", "Profiling: %d seconds.", n);
        UserConfigParams::m_no_start_screen = true;
        ProfileWorld::setProfileModeTime((float)n);
        RaceManager::get()->setNumLaps(999999); // profile end depends on time
    }   // --profile-time

    if(CommandLine::has("--history"))
    {
        history->setReplayHistory(true);
        // Force the no-start screen flag, since this initialises
        // the player structures correctly.
        if (!History::m_online_history_replay)
            UserConfigParams::m_no_start_screen = true;
    }   // --history

    // Demo mode
    if(CommandLine::has("--demo-mode", &s))
    {
        float t = 0;
        StringUtils::fromString(s, t);
        DemoWorld::enableDemoMode(t);
        // The default number of laps is taken from ProfileWorld and
        // is 0. So set a more useful default for demo mode.
        DemoWorld::setNumLaps(2);
    }   // --demo-mode

    if(CommandLine::has("--demo-laps", &n))
    {
        // Note that we use a separate setting for demo mode to avoid the
        // problem that someone plays a game, and in further demos then
        // the wrong (i.e. last selected) number of laps would be used
        DemoWorld::setNumLaps(n);
    }   // --demo-laps

    if(CommandLine::has("--demo-karts", &n))
    {
        // Note that we use a separate setting for demo mode to avoid the
        // problem that someone plays a game, and in further demos then
        // the wrong (i.e. last selected) number of karts would be used
        DemoWorld::setNumKarts(n);
    }   // --demo-karts

    if(CommandLine::has("--demo-tracks", &s))
        DemoWorld::setTracks(StringUtils::split(s,','));

#ifdef ENABLE_WIIUSE
    if(CommandLine::has("--wii"))
        WiimoteManager::enable();
#endif

#ifdef __APPLE__
    // on OS X, sometimes the Finder will pass a -psn* something parameter
    // to the application --> ignore it
    CommandLine::has("-psn");
#endif

    if (CommandLine::has("--dump-official-karts"))
    {
        OfficialKarts::dumpOfficialKarts();
        return 0;
    }

    CommandLine::reportInvalidParameters();

    if (ProfileWorld::isProfileMode() || GUIEngine::isNoGraphics())
    {
        UserConfigParams::m_sfx = false;  // Disable sound effects
        UserConfigParams::m_music = false;// and music when profiling
    }

    return 1;
}   // handleCmdLine

//=============================================================================
/** Initialises the minimum number of managers to get access to user_config.
 */
void initUserConfig()
{
    file_manager = new FileManager();
    user_config  = new UserConfig();     // needs file_manager
    user_config->loadConfig();
    // Some parts of the file manager needs user config (paths for models
    // depend on artist debug flag). So init the rest of the file manager
    // after reading the user config file.
    file_manager->init();
    if (UserConfigParams::m_language.toString() != "system")
    {
#ifdef WIN32
        std::string s=std::string("LANGUAGE=")
                     +UserConfigParams::m_language.c_str();
        _putenv(s.c_str());
#else
        setenv("LANGUAGE", UserConfigParams::m_language.c_str(), 1);
#endif
    }

    translations            = new Translations();   // needs file_manager
    stk_config              = new STKConfig();      // in case of --stk-config
                                                    // command line parameters
}   // initUserConfig

//=============================================================================
void clearGlobalVariables()
{
    // In android sometimes global variables is not reset when restart the app
    // we clear it here as much as possible
    STKProcess::reset();
    StateManager::clear();
    NetworkConfig::clear();
    STKHost::clear();
    RaceManager::clear();
    ProjectileManager::clear();
    RaceEventManager::clear();
    music_manager = NULL;
    irr_driver = NULL;
#ifdef ENABLE_WIIUSE
    wiimote_manager = NULL;
#endif
    World::clear();
    GUIEngine::resetGlobalVariables();
}   // clearGlobalVariables

//=============================================================================
void initRest()
{
    SP::setMaxTextureSize();
    irr_driver = new IrrDriver();

    if (irr_driver->getDevice() == NULL)
    {
        Log::fatal("main", "Couldn't initialise irrlicht device. Quitting.\n");
    }

    StkTime::init();   // grabs the timer object from the irrlicht device

    // Now create the actual non-null device in the irrlicht driver
    irr_driver->initDevice();

    // Init GUI
    IrrlichtDevice* device = irr_driver->getDevice();
    video::IVideoDriver* driver = device->getVideoDriver();

    if (UserConfigParams::m_gamepad_visualisation)
    {
        gamepadVisualisation();
        exit(0);
    }

    font_manager = new FontManager(); // Fonts are loaded in GUIEngine::init

    input_manager = new InputManager();
#ifdef __SWITCH__
    input_manager->addJoystick();
#endif
    // Get into menu mode initially.
    input_manager->setMode(InputManager::MENU);
    // Input manager set first so it recieves SDL joystick event
    GUIEngine::init(device, driver, StateManager::get());
    GUIEngine::renderLoading(true, true, false);
    GUIEngine::flushRenderLoading(true/*launching*/);

#ifdef ANDROID
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    assert(env);
    jobject activity = (jobject)SDL_AndroidGetActivity();
    if (activity != NULL)
    {
        jclass clazz = env->GetObjectClass(activity);
        if (clazz != NULL)
        {
            jmethodID method_id = env->GetMethodID(clazz, "hideSplashScreen",
                "()V");
            if (method_id != NULL)
                env->CallVoidMethod(activity, method_id);
            env->DeleteLocalRef(clazz);
        }
        env->DeleteLocalRef(activity);
    }
#endif

    stk_config->initMusicFiles();
    // This only initialises the non-network part of the add-ons manager. The
    // online section of the add-ons manager will be initialised from a
    // separate thread running in network HTTP.
#ifndef SERVER_ONLY
    addons_manager = NULL;
    if (!GUIEngine::isNoGraphics())
    {
        // Need to load shader after downloading assets as it reads prefilled
        // textures
        if (CVS->isGLSL())
            SP::loadShaders();
        addons_manager = new AddonsManager();
    }
#endif
    Online::ProfileManager::create();

    // The request manager will start the login process in case of a saved
    // session, so we need to read the main data from the players.xml file.
    // The rest will be read later (since the rest needs the unlock- and
    // achievement managers to be created, which can only be created later).
    PlayerManager::create();
    Online::RequestManager::get()->startNetworkThread();
#ifndef SERVER_ONLY
    if (!GUIEngine::isNoGraphics())
        NewsManager::get();   // this will create the news manager
#endif

    music_manager = new MusicManager();
    SFXManager::create();
    // The order here can be important, e.g. KartPropertiesManager needs
    // defaultKartProperties, which are defined in stk_config.
    history                 = new History              ();
    ReplayPlay::create();
    ReplayRecorder::create();
    material_manager        = new MaterialManager      ();
    track_manager           = new TrackManager         ();
    kart_properties_manager = new KartPropertiesManager();
    ProjectileManager::create();
    powerup_manager         = new PowerupManager       ();
    attachment_manager      = new AttachmentManager    ();
    highscore_manager       = new HighscoreManager     ();

    // The maximum texture size can not be set earlier, since
    // e.g. the background image needs to be loaded in high res.
    irr_driver->setMaxTextureSize();
    if (!UserConfigParams::m_disable_addon_karts)
    {
        KartPropertiesManager::addKartSearchDir(
            file_manager->getAddonsFile("karts/"));
    }
    if (!UserConfigParams::m_disable_addon_tracks)
    {
        track_manager->addTrackSearchDir(
            file_manager->getAddonsFile("tracks/"));
    }

    {
        XMLNode characteristicsNode(file_manager->getAsset("kart_characteristics.xml"));
        kart_properties_manager->loadCharacteristics(&characteristicsNode);
    }

    track_manager->loadTrackList();
    music_manager->addMusicToTracks();

    GUIEngine::addLoadingIcon(irr_driver->getTexture(FileManager::GUI_ICON,
                                                     "notes.png"      ) );

    grand_prix_manager      = new GrandPrixManager     ();
    // Consistency check for challenges, and enable all challenges
    // that have all prerequisites fulfilled
    grand_prix_manager->checkConsistency();
    GUIEngine::addLoadingIcon( irr_driver->getTexture(FileManager::GUI_ICON,
                                                      "cup_gold.png"    ) );

    RaceManager::create();
    // default settings for Quickstart
    RaceManager::get()->setNumPlayers(1);
    RaceManager::get()->setNumLaps   (3);
    RaceManager::get()->setMajorMode (RaceManager::MAJOR_MODE_SINGLE);
    RaceManager::get()->setMinorMode (RaceManager::MINOR_MODE_NORMAL_RACE);
    RaceManager::get()->setDifficulty(
                 (RaceManager::Difficulty)(int)UserConfigParams::m_difficulty);

    if (!track_manager->getTrack(UserConfigParams::m_last_track))
        UserConfigParams::m_last_track.revertToDefaults();

    RaceManager::get()->setTrack(UserConfigParams::m_last_track);

}   // initRest

//=============================================================================
void askForInternetPermission()
{
    if (UserConfigParams::m_internet_status !=
        Online::RequestManager::IPERM_NOT_ASKED)
        return;

    class ConfirmServer :
          public MessageDialog::IConfirmDialogListener
    {
    public:
        virtual void onConfirm()
        {
            // Typically internet is disabled here (just better safe
            // than sorry). If internet should be allowed, the news
            // manager needs to be started (which in turn activates
            // the add-ons manager).
#ifndef SERVER_ONLY
            bool need_to_start_news_manager =
                UserConfigParams::m_internet_status !=
                Online::RequestManager::IPERM_ALLOWED &&
                !GUIEngine::isNoGraphics();
            UserConfigParams::m_internet_status =
                                  Online::RequestManager::IPERM_ALLOWED;
            if (need_to_start_news_manager)
                NewsManager::get()->init(false);
#endif
            user_config->saveConfig();
            GUIEngine::ModalDialog::dismiss();
        }   // onConfirm
        // --------------------------------------------------------
        virtual void onCancel()
        {
            UserConfigParams::m_internet_status =
                Online::RequestManager::IPERM_NOT_ALLOWED;
            GUIEngine::ModalDialog::dismiss();
        }   // onCancel
    };   // ConfirmServer

    MessageDialog *dialog =
    new MessageDialog(_("SuperTuxKart may connect to a server "
        "to download add-ons and notify you of updates. "
        "Please read our privacy policy at https://supertuxkart.net/Privacy. "
        "Would you like this feature to be enabled? (To change this setting "
        "at a later time, go to options, select tab "
        "'General', and edit \"Connect to the "
        "Internet\")."),
        MessageDialog::MESSAGE_DIALOG_YESNO,
        new ConfirmServer(), true, true, 0.85f, 0.85f);

    // Changes the default focus to be 'cancel' ('ok' as default is not
    // GDPR compliant, see #3378).
    dialog->setFocusCancel();
    GUIEngine::DialogQueue::get()->pushDialog(dialog, false);
}   // askForInternetPermission

//=============================================================================

#if defined(WIN32) && defined(_MSC_VER)
    #ifdef DEBUG
        #pragma comment(linker, "/SUBSYSTEM:console")
    #else
        #pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
    #endif
#endif

// ----------------------------------------------------------------------------
#ifdef ANDROID
extern "C"
{
#endif
void main_abort()
{
    if (main_loop)
    {
        main_loop->requestAbort();
    }
}
#ifdef ANDROID
}
#endif

#ifdef __SWITCH__
ssize_t dotab_stdout_fn(struct _reent *r,void *fd,const char *ptr, size_t len)
{
    svcOutputDebugString(ptr, len);
    return len;
}

void debugLoop()
{
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);

    PadState pad;
    padInitializeDefault(&pad);

    while(1)
    {
        padUpdate(&pad);
        uint64_t down = padGetButtons(&pad);
        if(down & HidNpadButton_Up)
        {
            diagAbortWithResult(0);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}
#endif

// ----------------------------------------------------------------------------
#if defined(ANDROID)
int android_main(int argc, char *argv[])
#elif defined(IOS_STK)
int ios_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
#ifdef __SWITCH__
    constexpr devoptab_t dotab_stdout = {
        .name    = "con",
        .write_r = dotab_stdout_fn,
    };

    devoptab_list[STD_OUT] = &dotab_stdout;
    devoptab_list[STD_ERR] = &dotab_stdout;

    // Up number of maximum concurrent sockets, otherwise we can fail while loading with nxlink
    const SocketInitConfig socketConfig = {
        .tcp_tx_buf_size        = 0x8000,
        .tcp_rx_buf_size        = 0x10000,
        .tcp_tx_buf_max_size    = 0x40000,
        .tcp_rx_buf_max_size    = 0x40000,

        .udp_tx_buf_size = 0x2400,
        .udp_rx_buf_size = 0xA500,

        .sb_efficiency = 4,
        .num_bsd_sessions = 16,
        .bsd_service_type = BsdServiceType_User,
    };

    // Initialize socket, needed for networking and nxlink stdio
    socketInitialize(&socketConfig);
    // Initialize settings, needed to grab language
    setInitialize();
    // Needed to get ip address
    nifmInitialize(NifmServiceType_User);

    // Boost CPU while loading:
    appletSetCpuBoostMode(ApmCpuBoostMode_FastLoad);

    // Crashes on Reujinx
#ifdef DEBUG_NXLINK
    nxlinkStdio();
    std::thread debugThread = std::thread(debugLoop);
#endif
#endif
  
    clearGlobalVariables();
    CommandLine::init(argc, argv);

    CrashReporting::installHandlers();
#ifndef WIN32
    signal(SIGTERM, [](int signum)
        {
            main_abort();
        });
#endif
    srand(( unsigned ) time( 0 ));

    try
    {
        std::string s, server_config;

        handleCmdLineOutputModifier();

        if (CommandLine::has("--server-config", &s))
        {
            const std::string& base_name = StringUtils::getBasename(s);
            if (base_name.find(".xml") != std::string::npos)
            {
                server_config = s;
                FileManager::setStdoutName(
                    StringUtils::removeExtension(base_name) + ".log");
            }
        }

        if(CommandLine::has("--root", &s))
            FileManager::addRootDirs(s);
        if (CommandLine::has("--stdout", &s))
            FileManager::setStdoutName(s);
        if (CommandLine::has("--stdout-dir", &s))
            FileManager::setStdoutDir(s);

#ifndef SERVER_ONLY
        if(CommandLine::has("--no-graphics") || CommandLine::has("-l"))
#endif
            GUIEngine::disableGraphics();

        // Init the minimum managers so that user config exists, then
        // handle all command line options that do not need (or must
        // not have) other managers initialised:
        initUserConfig();

        CommandLine::addArgsFromUserConfig();

        handleCmdLinePreliminary();

        // ServerConfig will use stk_config for server version testing
        stk_config->load(file_manager->getAsset("stk_config.xml"));
        NetworkConfig::initSystemIP();
        // Client port depends on user config file and stk_config
        NetworkConfig::get()->initClientPort();
        bool no_graphics = !CommandLine::has("--graphical-server");

#ifndef SERVER_ONLY
        TipsManager::create();
#endif

        // Load current server config first, if any option is specified than
        // override it later
        // Disable sound if found server-config or wan/lan server name
        if (!server_config.empty())
        {
            if (no_graphics)
            {
                GUIEngine::disableGraphics();
                UserConfigParams::m_enable_sound = false;
            }
            ServerConfig::loadServerConfig(server_config);
            NetworkConfig::get()->setIsServer(true);
        }
        else
            ServerConfig::loadServerConfig();

        if (CommandLine::has("--wan-server", &s))
        {
            if (no_graphics)
            {
                GUIEngine::disableGraphics();
                UserConfigParams::m_enable_sound = false;
            }
            NetworkConfig::get()->setIsServer(true);
            ServerConfig::m_server_name = s;
            ServerConfig::m_wan_server = true;
            if (CommandLine::has("--no-validation"))
                ServerConfig::m_validating_player = false;
            else
                ServerConfig::m_validating_player = true;
        }
        else if (CommandLine::has("--lan-server", &s))
        {
            if (no_graphics)
            {
                GUIEngine::disableGraphics();
                UserConfigParams::m_enable_sound = false;
            }
            NetworkConfig::get()->setIsServer(true);
            ServerConfig::m_server_name = s;
            ServerConfig::m_wan_server = false;
            ServerConfig::m_validating_player = false;
        }

        if (!GUIEngine::isNoGraphics())
            profiler.init();
        // Create the story mode timer with empty setting first, it will
        // be reset later after story mode status and player manager is loaded
        story_mode_timer = new StoryModeTimer();
        initRest();

#ifdef ENABLE_WIIUSE
        wiimote_manager = new WiimoteManager();
#endif

        int parent_pid;
        bool has_parent_process = false;
        if (CommandLine::has("--parent-process", &parent_pid))
        {
            main_loop = new MainLoop(parent_pid);
            has_parent_process = true;
        }
        else
            main_loop = new MainLoop(0/*parent_pid*/);
        material_manager->loadMaterial();

        // Preload the explosion effects (explode.png)
        ParticleKindManager::get()->getParticles("explosion.xml");
        ParticleKindManager::get()->getParticles("explosion_bomb.xml");
        ParticleKindManager::get()->getParticles("explosion_cake.xml");
        ParticleKindManager::get()->getParticles("jump_explosion.xml");

        GUIEngine::addLoadingIcon( irr_driver->getTexture(FileManager::GUI_ICON,
                                                          "options_video.png"));
        kart_properties_manager -> loadAllKarts    ();
        kart_properties_manager->onDemandLoadKartTextures(
            { UserConfigParams::m_default_kart }, false/*unload_unused*/);
        OfficialKarts::load();
        handleXmasMode();
        handleEasterEarMode();

        // Needs the kart and track directories to load potential challenges
        // in those dirs, so it can only be created after reading tracks
        // and karts.
        unlock_manager = new UnlockManager();
        AchievementsManager::create();

        // Reading the rest of the player data needs the unlock manager to
        // initialise the game slots of all players and the AchievementsManager
        // to initialise the AchievementsStatus, so it is done only now.
        PlayerManager::get()->initRemainingData();

        GUIEngine::addLoadingIcon( irr_driver->getTexture(FileManager::GUI_ICON,
                                                          "gui_lock.png"  ) );
        ProjectileManager::get()->loadData();

        // Both item_manager and powerup_manager load models and therefore
        // textures from the model directory. To avoid reading the
        // materials.xml twice, we do this here once for both:
        file_manager->pushTextureSearchPath(file_manager->getAsset(FileManager::MODEL,""), "models");
        const std::string materials_file =
            file_manager->getAsset(FileManager::MODEL,"materials.xml");
        if(materials_file!="")
        {
            // Some of the materials might be needed later, so just add
            // them all permanently (i.e. as shared). Adding them temporary
            // will actually not be possible: powerup_manager adds some
            // permanent icon materials, which would (with the current
            // implementation) make the temporary materials permanent anyway.
            material_manager->addSharedMaterial(materials_file);
        }
        Referee::init();
        powerup_manager->loadPowerupsModels();
        ItemManager::loadDefaultItemMeshes();

        GUIEngine::addLoadingIcon( irr_driver->getTexture(FileManager::GUI_ICON,
                                                          "gift.png")       );

        attachment_manager->loadModels();
        file_manager->popTextureSearchPath();

        GUIEngine::addLoadingIcon( irr_driver->getTexture(FileManager::GUI_ICON,
                                                          "banana.png")    );

        //handleCmdLine() needs InitTuxkart() so it can't be called first
        if (!handleCmdLine(!server_config.empty(), has_parent_process))
            exit(0);

#ifndef SERVER_ONLY
        if (!GUIEngine::isNoGraphics())
        {
            addons_manager->checkInstalledAddons();

            // Load addons.xml to get info about add-ons even when not
            // allowed to access the Internet
            if (UserConfigParams::m_internet_status !=
                Online::RequestManager::IPERM_ALLOWED)
            {
                std::string xml_file = file_manager->getAddonsFile("addonsX.xml");
                if (file_manager->fileExists(xml_file))
                {
                    try
                    {
                        const XMLNode *xml = new XMLNode(xml_file);
                        addons_manager->initAddons(xml);
                    }
                    catch (std::runtime_error& e)
                    {
                        Log::warn("Addons", "Exception thrown when initializing add-ons manager : %s", e.what());
                    }
                }
            }
        }
#endif

        if(UserConfigParams::m_unit_testing)
        {
            runUnitTests();
            exit(0);
        }

#ifndef SERVER_ONLY
        if (!GUIEngine::isNoGraphics())
        {
            // Some Android devices have only 320x240 and height >= 480 is bare
            // minimum to make the GUI working as expected.
            if (irr_driver->getActualScreenSize().Height < 480)
            {
                if (UserConfigParams::m_old_driver_popup)
                {
                    MessageDialog *dialog =
                        new MessageDialog(_("Your screen resolution is too "
                                            "low to run STK."),
                                            /*from queue*/ true);
                    GUIEngine::DialogQueue::get()->pushDialog(dialog);
                }
                Log::warn("main", "Screen size is too small!");
            }
            else
            {
                irr_driver->getDevice()->setWindowMinimumSize(480, 480);
            }
            #ifdef MOBILE_STK
            if (UserConfigParams::m_multitouch_controls == MULTITOUCH_CONTROLS_UNDEFINED)
            {
                bool android_tv = false;
#ifdef ANDROID
                // For some android tv sdl returns a touch screen device even it
                // doesn't have
                android_tv = SDL_IsAndroidTV();
#endif
                if (!android_tv && irr_driver->getDevice()->supportsTouchDevice())
                {
                    InitAndroidDialog* init_android = new InitAndroidDialog(
                                                                    0.8f, 0.8f);
                    GUIEngine::DialogQueue::get()->pushDialog(init_android);
                }
            }
            #endif

            class DriverDialog :
                  public MessageDialog::IConfirmDialogListener
            {
            public:
                virtual void onConfirm()
                {
                    GUIEngine::ModalDialog::dismiss();
                }   // onConfirm
                // --------------------------------------------------------
                virtual void onCancel()
                {
                    UserConfigParams::m_old_driver_popup = false;
                    GUIEngine::ModalDialog::dismiss();
                }   // onCancel
            };   // DriverDialog

            if (GraphicsRestrictions::isDisabled(
                GraphicsRestrictions::GR_DRIVER_RECENT_ENOUGH))
            {
                if (UserConfigParams::m_old_driver_popup)
                {
                    MessageDialog *dialog =
                        new MessageDialog(_("Your driver version is too old. "
                                            "Please install the latest video "
                                            "drivers."),
                        MessageDialog::MESSAGE_DIALOG_OK_DONTSHOWAGAIN,
                        new DriverDialog(), /*delete_listener*/ true, /*from queue*/ true);
                    GUIEngine::DialogQueue::get()->pushDialog(dialog);
                }
                Log::warn("OpenGL", "Driver is too old!");
            }
            else if (!CVS->isGLSL())
            {
                #if !defined(MOBILE_STK)
                if (UserConfigParams::m_old_driver_popup)
                {
                    #ifdef USE_GLES2
                    irr::core::stringw version = "OpenGL ES 3.0";
                    #else
                    irr::core::stringw version = "OpenGL 3.3";
                    #endif
                    MessageDialog *dialog = new MessageDialog(_(
                        "Your graphics driver appears to be very old. Please "
                        "check if an update is available. SuperTuxKart "
                        "recommends a driver supporting %s or better. The game "
                        "will likely still run, but in a reduced-graphics mode.",
                        version),
                        MessageDialog::MESSAGE_DIALOG_OK_DONTSHOWAGAIN,
                        new DriverDialog(), /*delete_listener*/ true, /*from queue*/ true);
                    GUIEngine::DialogQueue::get()->pushDialog(dialog);
                }
                #endif
                Log::warn("OpenGL", "OpenGL version is too old!");
            }

            // Note that on the very first run of STK internet status is set to
            // "not asked", so the report will only be sent in the next run.
            if(UserConfigParams::m_internet_status==Online::RequestManager::IPERM_ALLOWED)
            {
                HardwareStats::reportHardwareStats();
            }
        }
#endif

        if (STKHost::existHost())
        {
            if (!GUIEngine::isNoGraphics())
                NetworkingLobby::getInstance()->push();
        }
        else if (!UserConfigParams::m_no_start_screen)
        {
            if (UserConfigParams::m_enforce_current_player)
            {
                PlayerManager::get()->enforceCurrentPlayer();
            }

            // If there is a current player, it was saved in the config file,
            // so we immediately start the main menu (unless it was requested
            // to always show the login screen). Otherwise show the login
            // screen first.
            if(PlayerManager::getCurrentPlayer() && !
                UserConfigParams::m_always_show_login_screen)
            {
                MainMenuScreen::getInstance()->push();
            }
            else
            {
                UserScreen::getInstance()->push();
                // If there is no player, push the RegisterScreen on top of
                // the login screen. This way on first start players are
                // forced to create a player.
                if (PlayerManager::get()->getNumPlayers() == 0)
                {
                    RegisterScreen::getInstance()->push();
                    RegisterScreen::getInstance()->setParent(UserScreen::getInstance());
                }
            }
#ifdef ENABLE_WIIUSE
            // Show a dialog to allow connection of wiimotes. */
            if(WiimoteManager::isEnabled())
            {
                wiimote_manager->askUserToConnectWiimotes();
            }
#endif
            askForInternetPermission();
        }
        else
        {
            setupRaceStart();
            // Go straight to the race
            StateManager::get()->enterGameState();
        }

#ifndef SERVER_ONLY
        // If an important news message exists it is shown in a popup dialog.
        if (!GUIEngine::isNoGraphics())
        {
            const core::stringw important_message =
                                        NewsManager::get()->getImportantMessage();
            if (important_message!="")
            {
                new MessageDialog(important_message,
                                MessageDialog::MESSAGE_DIALOG_OK,
                                NULL, true);
            }   // if important_message
        }
#endif

        // Reset the story mode timer before going in the main loop
        // as it needs to be able to run continuously
        // Now the story mode status and player manager is loaded
        story_mode_timer->reset();

        // Replay a race
        // =============
        if(history->replayHistory())
        {
            // This will setup the race manager etc.
            history->Load();
            if (!History::m_online_history_replay)
            {
                RaceManager::get()->setupPlayerKartInfo();
                RaceManager::get()->startNew(false);
                main_loop->run();
                // The run() function will only return if the user aborts.
                Log::flushBuffers();
                exit(-3);
            }   // if !online
        }

        // Not replaying
        // =============
        if(!ProfileWorld::isProfileMode())
        {
            if(UserConfigParams::m_no_start_screen)
            {
                // Quickstart (-N)
                // ===============
                // all defaults are set in InitTuxkart()
                RaceManager::get()->setupPlayerKartInfo();
                RaceManager::get()->startNew(false);
            }
        }
        else  // profile
        {
            // Profiling
            // =========
            RaceManager::get()->setMajorMode (RaceManager::MAJOR_MODE_SINGLE);
            RaceManager::get()->setupPlayerKartInfo();
            RaceManager::get()->startNew(false);
        }

#ifdef __SWITCH__
        // Game loaded, bring CPU / GPU clock back to normal
        appletSetCpuBoostMode(ApmCpuBoostMode_Normal);
#endif

        main_loop->run();

    }  // try
    catch (std::exception &e)
    {
        Log::flushBuffers();
        Log::error("main", "Exception caught : %s.",e.what());
        Log::error("main", "Aborting SuperTuxKart.");
        Log::flushBuffers();
    }

    /* Program closing...*/

#ifdef ENABLE_WIIUSE
    if(wiimote_manager)
        delete wiimote_manager;
#endif

    // If the window was closed in the middle of a race, remove players,
    // so we don't crash later when StateManager tries to access input devices.
    StateManager::get()->resetActivePlayers();
    if (input_manager)
    {
        delete input_manager;
        input_manager = NULL;
    }

    if (STKHost::existHost())
        STKHost::get()->shutdown();
    ClientLobby::destroyBackgroundDownload();

    cleanSuperTuxKart();
    NetworkConfig::destroy();

    RichPresenceNS::RichPresence::destroy();

#ifdef DEBUG
    MemoryLeaks::checkForLeaks();
#endif

    Log::flushBuffers();

#ifndef WIN32
    if (user_config) //close logfiles
    {
        Log::closeOutputFiles();
#endif
#ifndef ANDROID
        fclose(stderr);
        fclose(stdout);
#endif
#ifndef WIN32
    }
#endif

    delete file_manager;

#ifdef __SWITCH__
    // De-initialize stuff!
    setExit();
    socketExit();
    nifmExit();
#endif

#ifdef IOS_STK
    // App store may not like this, but this can happen if player uses keyboard to quit stk
    exit(0);
    return 0;
#else
    return 0 ;
#endif
}   // main

// ============================================================================
#ifdef WIN32
//routine for running under windows
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine, int nCmdShow)
{
    return main(__argc, __argv);
}
#endif

//=============================================================================
/** Frees all manager and their associated memory.
 */
static void cleanSuperTuxKart()
{

    delete main_loop;

    if(Online::RequestManager::isRunning())
        Online::RequestManager::get()->stopNetworkThread();

#ifndef SERVER_ONLY
    TipsManager::destroy();
#endif

    // Stop music (this request will go into the sfx manager queue, so it needs
    // to be done before stopping the thread).
    if (music_manager)
        music_manager->stopMusic();
    if (SFXManager::get())
        SFXManager::get()->stopThread();
    irr_driver->updateConfigIfRelevant();
    AchievementsManager::destroy();
    Referee::cleanup();

    if (SFXManager::get() &&
        !SFXManager::get()->waitForReadyToDeleted(2.0f))
    {
        Log::info("Thread", "SFXManager not stopping, exiting anyway.");
    }
    SFXManager::destroy();

    // Music manager can not be deleted before the SFX thread is stopped
    // (since SFX commands can contain music information, which are
    // deleted by the music manager).
    delete music_manager;

    // Race manager needs to be deleted after sfx manager as it checks for
    // the kart size structure from race manager
    RaceManager::destroy();
    if(grand_prix_manager)      delete grand_prix_manager;
    if(highscore_manager)       delete highscore_manager;
    if(attachment_manager)      delete attachment_manager;
    ItemManager::removeTextures();
    if(powerup_manager)         delete powerup_manager;
    ProjectileManager::destroy();
    if(kart_properties_manager) delete kart_properties_manager;
    if(track_manager)           delete track_manager;
    if(material_manager)        delete material_manager;
    if(history)                 delete history;
    ReplayPlay::destroy();
    ReplayRecorder::destroy();
    delete ParticleKindManager::get();
    PlayerManager::destroy();
    if(unlock_manager)          delete unlock_manager;
    Online::ProfileManager::destroy();
    GUIEngine::DialogQueue::deallocate();
    GUIEngine::clear();
    GUIEngine::cleanUp();
    GUIEngine::clearScreenCache();
    if(font_manager)            delete font_manager;
    if(story_mode_timer)        delete story_mode_timer;

    // Now finish shutting down objects which a separate thread. The
    // RequestManager has been signaled to shut down as early as possible,
    // the NewsManager thread should have finished quite early on anyway.
    // But still give them some additional time to finish. It avoids a
    // race condition where a thread might access the file manager after it
    // was deleted (in cleanUserConfig below), but before STK finishes and
    // the OS takes all threads down.

#ifndef SERVER_ONLY
    if (!GUIEngine::isNoGraphics())
    {
        if (UserConfigParams::m_internet_status == Online::RequestManager::
            IPERM_ALLOWED && NewsManager::isRunning() &&
            !NewsManager::get()->waitForReadyToDeleted(2.0f))
        {
            Log::info("Thread", "News manager not stopping, exiting anyway.");
        }
        NewsManager::deallocate();
    }
#endif

    if (Online::RequestManager::isRunning())
    {
        if (Online::RequestManager::get()->waitForReadyToDeleted(5.0f))
        {
            Online::RequestManager::deallocate();
        }
        else
        {
            Log::warn("Thread", "Request Manager not aborting in time, proceeding without cleanup.");
        }
    }

    // The add-ons manager might still be called from a currenty running request
    // in the request manager, so it can not be deleted earlier.
#ifndef SERVER_ONLY
    if(addons_manager)  delete addons_manager;
#endif

    ServersManager::deallocate();
    cleanUserConfig();

    StateManager::deallocate();
    GUIEngine::EventHandler::deallocate();
}   // cleanSuperTuxKart

//=============================================================================
/**
 * Frees all the memory of initUserConfig()
 */
static void cleanUserConfig()
{
    if(stk_config)              delete stk_config;
    if(translations)            delete translations;
    if (user_config)
    {
        // In case that abort is triggered before user_config exists
        if (UserConfigParams::m_crashed) UserConfigParams::m_crashed = false;
        user_config->saveConfig();
        delete user_config;
    }

    if(irr_driver)              delete irr_driver;
}   // cleanUserConfig

//=============================================================================
void runUnitTests()
{
    Log::info("UnitTest", "Starting unit testing");
    Log::info("UnitTest", "=====================");
    Log::info("UnitTest", "MiniGLM");
    MiniGLM::unitTesting();
    Log::info("UnitTest", "GraphicsRestrictions");
    GraphicsRestrictions::unitTesting();
    Log::info("UnitTest", "NetworkString");
    NetworkString::unitTesting();
    Log::info("UnitTest", "SocketAddress");
    SocketAddress::unitTesting();
    Log::info("UnitTest", "StringUtils::versionToInt");
    StringUtils::unitTesting();

    Log::info("UnitTest", "Easter detection");
    // Test easter mode: in 2015 Easter is 5th of April - check with 0 days
    // before and after
    int saved_easter_mode = UserConfigParams::m_easter_ear_mode;
    UserConfigParams::m_easter_ear_mode = 0;   // disable always on or off mode
    assert( isEasterMode( 5, 4, 2015, 0));
    assert(!isEasterMode( 4, 4, 2015, 0));
    assert(!isEasterMode( 6, 4, 2015, 0));

    assert( isEasterMode( 1, 4, 2018, 0));
    assert( isEasterMode(27, 3, 2016, 0));

    // Check days before/after
    assert( isEasterMode( 2, 4, 2015, 3));
    assert(!isEasterMode( 1, 4, 2015, 3));
    assert( isEasterMode( 8, 4, 2015, 3));
    assert(!isEasterMode( 9, 4, 2015, 3));

    // Check if going to previous month is handled correctly: 1/4/2018
    assert( isEasterMode(27, 3, 2018, 5));
    assert(!isEasterMode(26, 3, 2018, 5));
    assert( isEasterMode( 6, 4, 2018, 5));
    assert(!isEasterMode( 7, 4, 2018, 5));

    // Check if going to previous month is handled correctly: 1/4/2018
    assert( isEasterMode( 1, 4, 2016, 5));
    assert(!isEasterMode( 2, 4, 2016, 5));
    assert( isEasterMode(22, 3, 2016, 5));
    assert(!isEasterMode(21, 3, 2016, 5));
    UserConfigParams::m_easter_ear_mode = saved_easter_mode;

    Log::info("UnitTest", "PowerupManager");
    PowerupManager::unitTesting();

    Log::info("UnitTest", "Kart characteristics");
    CombinedCharacteristic::unitTesting();

    Log::info("UnitTest", "Arena Graph");
    ArenaGraph::unitTesting();

    Log::info("UnitTest", "Fonts for translation");
    font_manager->unitTesting();

    Log::info("UnitTest", "RewindQueue");
    RewindQueue::unitTesting();

    Log::info("UnitTest", "=====================");
    Log::info("UnitTest", "Testing successful   ");
    Log::info("UnitTest", "=====================");
}   // runUnitTests
