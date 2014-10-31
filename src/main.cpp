
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2013 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2011-2013 Joerg Henrichs, Marianne Gagnon
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
 race -> modes
 race -> tracks
 race -> karts
 modes -> tracks
 modes -> karts
 tracks -> graphics
 karts -> graphics
 tracks -> items
 graphics -> irrlicht
 guiengine -> irrlicht
 states_screens -> guiengine
 states_screens -> input
 guiengine -> input
 karts->physics
 tracks->physics
 karts -> controller
 input->controller
 tracks -> animations
 physics -> animations
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
#  define _WINSOCKAPI_
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  ifdef _MSC_VER
#    include <direct.h>
#  endif
#else
#  include <unistd.h>
#endif
#include <stdexcept>
#include <cstdio>
#include <string>
#include <cstring>
#include <sstream>
#include <algorithm>

#include <IEventReceiver.h>

#include "main_loop.hpp"
#include "achievements/achievements_manager.hpp"
#include "addons/addons_manager.hpp"
#include "addons/news_manager.hpp"
#include "audio/music_manager.hpp"
#include "audio/sfx_manager.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/hardware_stats.hpp"
#include "config/player_manager.hpp"
#include "config/player_profile.hpp"
#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "graphics/hardware_skinning.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/particle_kind_manager.hpp"
#include "graphics/referee.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/event_handler.hpp"
#include "guiengine/dialog_queue.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "input/keyboard_device.hpp"
#include "input/wiimote_manager.hpp"
#include "io/file_manager.hpp"
#include "items/attachment_manager.hpp"
#include "items/item_manager.hpp"
#include "items/projectile_manager.hpp"
#include "karts/controller/ai_base_controller.hpp"
#include "karts/kart_properties.hpp"
#include "karts/kart_properties_manager.hpp"
#include "modes/demo_world.hpp"
#include "modes/profile_world.hpp"
#include "network/client_network_manager.hpp"
#include "network/network_manager.hpp"
#include "network/protocol_manager.hpp"
#include "network/protocols/server_lobby_room_protocol.hpp"
#include "network/client_network_manager.hpp"
#include "network/server_network_manager.hpp"
#include "network/protocol_manager.hpp"
#include "network/protocols/server_lobby_room_protocol.hpp"
#include "online/profile_manager.hpp"
#include "online/request_manager.hpp"
#include "online/servers_manager.hpp"
#include "race/grand_prix_manager.hpp"
#include "race/highscore_manager.hpp"
#include "race/history.hpp"
#include "race/race_manager.hpp"
#include "replay/replay_play.hpp"
#include "replay/replay_recorder.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "states_screens/register_screen.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/user_screen.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/command_line.hpp"
#include "utils/constants.hpp"
#include "utils/crash_reporting.hpp"
#include "utils/leak_check.hpp"
#include "utils/log.hpp"
#include "utils/translation.hpp"

static void cleanSuperTuxKart();
static void cleanUserConfig();

// ============================================================================
//                        gamepad visualisation screen
// ============================================================================

void gamepadVisualisation()
{

    core::array<SJoystickInfo>          irrlicht_gamepads;
    irr_driver->getDevice()->activateJoysticks(irrlicht_gamepads);


    struct Gamepad
    {
        s16   m_axis[SEvent::SJoystickEvent::NUMBER_OF_AXES];
        bool  m_button_state[SEvent::SJoystickEvent::NUMBER_OF_BUTTONS];
    };

    #define GAMEPAD_COUNT 8 // const won't work

    class EventReceiver : public IEventReceiver
    {
    public:
        Gamepad m_gamepads[GAMEPAD_COUNT];

        EventReceiver()
        {
            for (int n=0; n<GAMEPAD_COUNT; n++)
            {
                Gamepad& g = m_gamepads[n];
                for (int i=0; i<SEvent::SJoystickEvent::NUMBER_OF_AXES; i++)
                    g.m_axis[i] = 0;
                for (int i=0; i<SEvent::SJoystickEvent::NUMBER_OF_BUTTONS; i++)
                    g.m_button_state[i] = false;
            }
        }

        virtual bool OnEvent (const irr::SEvent &event)
        {
            switch (event.EventType)
            {
                case EET_JOYSTICK_INPUT_EVENT :
                {
                    const SEvent::SJoystickEvent& evt = event.JoystickEvent;
                    if (evt.Joystick >= GAMEPAD_COUNT) return true;

                    Gamepad& g = m_gamepads[evt.Joystick];
                    for (int i=0; i<SEvent::SJoystickEvent::NUMBER_OF_AXES;i++)
                    {
                        g.m_axis[i] = evt.Axis[i];
                    }
                    for (int i=0; i<SEvent::SJoystickEvent::NUMBER_OF_BUTTONS;
                         i++)
                    {
                        g.m_button_state[i] = evt.IsButtonPressed(i);
                    }
                    break;
                }

                case EET_KEY_INPUT_EVENT:
                {
                    const SEvent::SKeyInput& evt = event.KeyInput;

                    if (evt.PressedDown)
                    {
                        if (evt.Key == KEY_RETURN || evt.Key == KEY_ESCAPE ||
                            evt.Key == KEY_SPACE)
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

        video::IVideoDriver* driver = irr_driver->getVideoDriver();
        const core::dimension2du size = driver ->getCurrentRenderTargetSize();

        driver->beginScene(true, true, video::SColor(255,0,0,0));

        for (int n=0; n<GAMEPAD_COUNT; n++)
        {
            Gamepad& g = events->m_gamepads[n];

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

                if (g.m_button_state[b])
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
                const float rate = g.m_axis[a] / 32767.0f;

                core::position2di pos(axis_x, axis_y + a*axis_h);
                core::dimension2di size(axis_w, axis_h);

                const bool deadzone = (abs(g.m_axis[a]) < DEADZONE_JOYSTICK);

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
        kart_properties_manager->setHatMeshName("christmas_hat.b3d");
}   // handleXmasMode
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
        race_manager->setLocalKartInfo(0, UserConfigParams::m_default_kart.getDefaultValue());
    }
    else
    {
        // Set up race manager appropriately
        race_manager->setLocalKartInfo(0, UserConfigParams::m_default_kart);
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
    "Run SuperTuxKart, a racing game with go-kart that features"
    " the Tux and friends.\n\n"
    "Options:\n"
    "  -N,  --no-start-screen  Immediately start race without showing a "
                              "menu.\n"
    "  -R,  --race-now         Same as -N but also skip the ready-set-go phase"
                              " and the music.\n"
    "  -t,  --track=NAME       Start at track NAME.\n"
    "       --gp=NAME          Start the specified Grand Prix.\n"
    "       --add-gp-dir=DIR   Load Grand Prix in DIR. Setting will be saved "
                              "inconfig.xml under additional_gp_directory. Use "
                              "--add-gp-dir=\"\" to unset.\n"
    "       --stk-config=FILE  use ./data/FILE instead of "
                              "./data/stk_config.xml\n"
    "  -k,  --numkarts=NUM     Number of karts on the racetrack.\n"
    "       --kart=NAME        Use kart number NAME.\n"
    "       --ai=a,b,...       Use the karts a, b, ... for the AI.\n"
    "       --laps=N           Define number of laps to N.\n"
    "       --mode=N           N=1 novice, N=2 driver, N=3 racer.\n"
    "       --type=N           N=0 Normal, N=1 Time trial, N=2 FTL\n"
    "       --reverse          Play track in reverse (if allowed)\n"
    "  -f,  --fullscreen       Select fullscreen display.\n"
    "  -w,  --windowed         Windowed display (default).\n"
    "  -s,  --screensize=WxH   Set the screen size (e.g. 320x200).\n"
    "  -v,  --version          Show version of SuperTuxKart.\n"
    "       --trackdir=DIR     A directory from which additional tracks are "
                              "loaded.\n"
    "       --profile-laps=n   Enable automatic driven profile mode for n "
                              "laps.\n"
    "       --profile-time=n   Enable automatic driven profile mode for n "
                              "seconds.\n"
    "       --no-graphics      Do not display the actual race.\n"
    "       --with-profile     Enables the profile mode.\n"
    "       --demo-mode=t      Enables demo mode after t seconds idle time in "
                               "main menu.\n"
    "       --demo-tracks=t1,t2 List of tracks to be used in demo mode. No\n"
    "                          spaces are allowed in the track names.\n"
    "       --demo-laps=n      Number of laps in a demo.\n"
    "       --demo-karts=n     Number of karts to use in a demo.\n"
    "       --ghost            Replay ghost data together with one player kart.\n"
    // "       --history          Replay history file 'history.dat'.\n"
    // "       --history=n        Replay history file 'history.dat' using:\n"
    // "                            n=1: recorded positions\n"
    // "                            n=2: recorded key strokes\n"
    "       --server           Start a server (not a playing client).\n"
    "       --login=s          Automatically sign in (set the login).\n"
    "       --password=s       Automatically sign in (set the password).\n"
    "       --port=n           Port number to use.\n"
    "       --max-players=n    Maximum number of clients (server only).\n"
    "       --no-console       Does not write messages in the console but to\n"
    "                          stdout.log.\n"
    "       --console          Write messages in the console and files\n"
    "  -h,  --help             Show this help.\n"
    "\n"
    "You can visit SuperTuxKart's homepage at "
    "http://supertuxkart.sourceforge.net\n\n",
    CommandLine::getExecName().c_str()
    );
}   // cmdLineHelp

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
    if (CommandLine::has("--help") || CommandLine::has("-help") ||
        CommandLine::has("-h"))
    {
        cmdLineHelp();
        cleanUserConfig();
        exit(0);
    }

    if(CommandLine::has("--version") || CommandLine::has("-v"))
    {
        Log::info("main", "==============================");
        Log::info("main", "SuperTuxKart, %s.", STK_VERSION ) ;
        // IRRLICHT_VERSION_SVN
        Log::info("main", "Irrlicht version %i.%i.%i (%s)",
                          IRRLICHT_VERSION_MAJOR , IRRLICHT_VERSION_MINOR,
                          IRRLICHT_VERSION_REVISION, IRRLICHT_SDK_VERSION );
        Log::info("main", "==============================");
        cleanUserConfig();
        exit(0);
    }

    if(CommandLine::has("--gamepad-visualisation") ||   // only BE
       CommandLine::has("--gamepad-visualization")    ) // both AE and BE
        UserConfigParams::m_gamepad_visualisation=true;
    if(CommandLine::has("--debug=memory"))
        UserConfigParams::m_verbosity |= UserConfigParams::LOG_MEMORY;
    if(CommandLine::has("--debug=addons"))
        UserConfigParams::m_verbosity |= UserConfigParams::LOG_ADDONS;
    if(CommandLine::has("--debug=mgui"))
        UserConfigParams::m_verbosity |= UserConfigParams::LOG_GUI;
    if(CommandLine::has("--debug=flyable"))
        UserConfigParams::m_verbosity |= UserConfigParams::LOG_FLYABLE;
    if(CommandLine::has("--debug=mist"))
        UserConfigParams::m_verbosity |= UserConfigParams::LOG_MISC;
    if(CommandLine::has("--debug=all") )
        UserConfigParams::m_verbosity |= UserConfigParams::LOG_ALL;
    if(CommandLine::has("--console"))
        UserConfigParams::m_log_errors_to_console=true;
    if(CommandLine::has("--no-console"))
        UserConfigParams::m_log_errors_to_console=false;
    if(CommandLine::has("--online"))
        MainMenuScreen::m_enable_online=true;
    if(CommandLine::has("--log=nocolor"))
    {
        Log::disableColor();
        Log::verbose("main", "Colours disabled.");
    }

    std::string s;
    if(CommandLine::has("--stk-config", &s))
    {
        stk_config->load(file_manager->getAsset(s));
        Log::info("main", "STK config will be read from %s.",s.c_str());
    }
    if(CommandLine::has("--trackdir", &s))
        TrackManager::addTrackSearchDir(s);
    if(CommandLine::has("--kartdir", &s))
        KartPropertiesManager::addKartSearchDir(s);

    if(CommandLine::has("--no-graphics") || CommandLine::has("-l"))
    {
        ProfileWorld::disableGraphics();
        UserConfigParams::m_log_errors_to_console=true;
    }

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
                UserConfigParams::m_prev_width =
                    UserConfigParams::m_width = width;
                UserConfigParams::m_prev_height =
                    UserConfigParams::m_height = height;
                Log::verbose("main", "You choose to use %dx%d.",
                    (int)UserConfigParams::m_width,
                    (int)UserConfigParams::m_height );
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
    if(CommandLine::has("--log", &n))
        Log::setLogLevel(n);

    return 0;
}   // handleCmdLinePreliminary

// ============================================================================
/** Handles command line options.
 *  \param argc Number of command line options
 */
int handleCmdLine()
{
    // Some generic variables used in scanning:
    int n;
    std::string s;

    bool try_login = false;
    irr::core::stringw login, password;

    if(CommandLine::has("--gamepad-debug"))
        UserConfigParams::m_gamepad_debug=true;
    if(CommandLine::has("--wiimote-debug"))
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
            UserConfigParams::m_slipstream_debug=true;
    if(CommandLine::has("--rendering-debug"))
        UserConfigParams::m_rendering_debug=true;
    if(CommandLine::has("--ai-debug"))
        AIBaseController::enableDebug();

    if(UserConfigParams::m_artist_debug_mode)
    {
       if(CommandLine::has("--camera-wheel-debug"))
           UserConfigParams::m_camera_debug=2;
        if(CommandLine::has("--camera-debug"))
            UserConfigParams::m_camera_debug=1;
        if(CommandLine::has("--physics-debug"))
            UserConfigParams::m_physics_debug=1;
        if(CommandLine::has("--check-debug"))
            UserConfigParams::m_check_debug=true;
    }

    // Networking command lines
    if(CommandLine::has("--server") )
    {
        NetworkManager::getInstance<ServerNetworkManager>();
        Log::info("main", "Creating a server network manager.");
    }   // -server

    if(CommandLine::has("--max-players", &n))
        UserConfigParams::m_server_max_players=n;

    if(CommandLine::has("--login", &s) )
    {
        login = s.c_str();
        try_login = true;
    }   // --login

    if(CommandLine::has("--password", &s))
        password = s.c_str();

    // Race parameters
    if(CommandLine::has("--kartsize-debug"))
    {
        for(unsigned int i=0;
            i<kart_properties_manager->getNumberOfKarts(); i++)
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
        const PlayerProfile *player = PlayerManager::getCurrentPlayer();

        if(player && !player->isLocked(s))
        {
            const KartProperties *prop =
                kart_properties_manager->getKart(s);
            if(prop)
            {
                UserConfigParams::m_default_kart = s;

                // if a player was added with -N, change its kart.
                // Otherwise, nothing to do, kart choice will be picked
                // up upon player creation.
                if (StateManager::get()->activePlayerCount() > 0)
                {
                    race_manager->setLocalKartInfo(0, s);
                }
                Log::verbose("main", "You chose to use kart '%s'.",
                             s.c_str() ) ;
            }
            else
            {
                Log::warn("main", "Kart '%s' not found, ignored.",
                          s.c_str());
            }
        }
        else   // kart locked
        {
            if (player)
                Log::warn("main", "Kart '%s' has not been unlocked yet.",
                          s.c_str());
            else
                Log::warn("main",
                        "A default player must exist in order to use --kart.");
        }   // if kart locked
    }   // if --kart

    if(CommandLine::has("--ai", &s))
    {
        const std::vector<std::string> l=StringUtils::split(std::string(s),',');
        race_manager->setDefaultAIKartList(l);
        // Add 1 for the player kart
        race_manager->setNumKarts((int)l.size()+1);
    }   // --ai

    if(CommandLine::has( "--mode", &s))
    {
        int n = atoi(s.c_str());
        if(n<0 || n>RaceManager::DIFFICULTY_LAST)
            Log::warn("main", "Invalid difficulty '%s' - ignored.\n",
                      s.c_str());
        else
            race_manager->setDifficulty(RaceManager::Difficulty(n));
    }   // --mode

    if(CommandLine::has("--type", &n))
    {
        switch (n)
        {
        case 0: race_manager->setMinorMode(RaceManager::MINOR_MODE_NORMAL_RACE);
                break;
        case 1: race_manager->setMinorMode(RaceManager::MINOR_MODE_TIME_TRIAL);
                break;
        case 2: race_manager->setMinorMode(RaceManager::MINOR_MODE_FOLLOW_LEADER);
                break;
        default:
                Log::warn("main", "Invalid race type '%d' - ignored.", n);
        }
    }   // --type

    if(CommandLine::has("--track", &s) || CommandLine::has("-t", &s))
    {
        const PlayerProfile *player = PlayerManager::getCurrentPlayer();
        if (player && !player->isLocked(s))
        {
            race_manager->setTrack(s);
            Log::verbose("main", "You choose to start in track '%s'.",
                         s.c_str());

            Track* t = track_manager->getTrack(s);
            if (!t)
            {
                Log::warn("main", "Can't find track named '%s'.", s.c_str());
            }
            else if (t->isArena())
            {
                //if it's arena, don't create ai karts
                const std::vector<std::string> l;
                race_manager->setDefaultAIKartList(l);
                // Add 1 for the player kart
                race_manager->setNumKarts(1);
                race_manager->setMinorMode(RaceManager::MINOR_MODE_3_STRIKES);
            }
            else if(t->isSoccer())
            {
                //if it's soccer, don't create ai karts
                const std::vector<std::string> l;
                race_manager->setDefaultAIKartList(l);
                // Add 1 for the player kart
                race_manager->setNumKarts(1);
                race_manager->setMinorMode(RaceManager::MINOR_MODE_SOCCER);
            }
        }
        else
        {
            if (player)
                Log::warn("main", "Track '%s' has not been unlocked yet.",
                          s.c_str());
            else
                Log::warn("main",
                       "A default player must exist in order to use --track.");
        }
    }   // --track


    if(CommandLine::has("--gp", &s))
    {
        race_manager->setMajorMode(RaceManager::MAJOR_MODE_GRAND_PRIX);
        const GrandPrixData *gp = grand_prix_manager->getGrandPrix(s);

        if (!gp)
        {
            Log::warn("main", "There is no GP named '%s'.", s.c_str());
            return 0;
        }
        race_manager->setGrandPrix(*gp);
    }   // --gp

    if(CommandLine::has("--numkarts", &n) ||CommandLine::has("-k", &n))
    {
        UserConfigParams::m_num_karts = n;
        if(UserConfigParams::m_num_karts > stk_config->m_max_karts)
        {
            Log::warn("main", "Number of karts reset to maximum number %d.",
                      stk_config->m_max_karts);
            UserConfigParams::m_num_karts = stk_config->m_max_karts;
        }
        race_manager->setNumKarts( UserConfigParams::m_num_karts );
        Log::verbose("main", "%d karts will be used.",
                     (int)UserConfigParams::m_num_karts);
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
            Log::verbose("main", "You choose to have %d laps.", laps);
            race_manager->setNumLaps(laps);
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
            race_manager->setNumLaps(n);
        }
    }   // --profile-laps

    if(CommandLine::has("--profile-time",  &n))
    {
        Log::verbose("main", "Profiling: %d seconds.", n);
        UserConfigParams::m_no_start_screen = true;
        ProfileWorld::setProfileModeTime((float)n);
        race_manager->setNumLaps(999999); // profile end depends on time
    }   // --profile-time

    if(CommandLine::has("--with-profile") )
    {
        // Set default profile mode of 1 lap if we haven't already set one
        if (!ProfileWorld::isProfileMode()) {
            UserConfigParams::m_no_start_screen = true;
            ProfileWorld::setProfileModeLaps(1);
            race_manager->setNumLaps(1);
        }
    }   // --with-profile

    if(CommandLine::has("--ghost"))
        ReplayPlay::create();

    if(CommandLine::has("--history",  &n))
    {
        history->doReplayHistory( (History::HistoryReplayMode)n);
        // Force the no-start screen flag, since this initialises
        // the player structures correctly.
        UserConfigParams::m_no_start_screen = true;
    }   // --history=%d

    if(CommandLine::has("--history"))  // handy default for --history=1
    {
        history->doReplayHistory(History::HISTORY_POSITION);
        // Force the no-start screen flag, since this initialises
        // the player structures correctly.
        UserConfigParams::m_no_start_screen = true;
    }   // --history

    // Demo mode
    if(CommandLine::has("--demo-mode", &s))
    {
        float t;
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

    CommandLine::reportInvalidParameters();

    if(ProfileWorld::isProfileMode())
    {
        UserConfigParams::m_sfx = false;  // Disable sound effects
        UserConfigParams::m_music = false;// and music when profiling
    }

    if (try_login)
    {
        irr::core::stringw s;
        Online::XMLRequest* request =
                PlayerManager::requestSignIn(login, password);

        if (request->isSuccess())
        {
            Log::info("Main", "Logged in from command line.");
        }
    }

    return 1;
}   // handleCmdLine

//=============================================================================
/** Initialises the minimum number of managers to get access to user_config.
 */
void initUserConfig()
{
    irr_driver              = new IrrDriver();
    file_manager            = new FileManager();
    user_config             = new UserConfig();     // needs file_manager
    user_config->loadConfig();
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
void initRest()
{
    stk_config->load(file_manager->getAsset("stk_config.xml"));

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

    GUIEngine::init(device, driver, StateManager::get());

    // This only initialises the non-network part of the addons manager. The
    // online section of the addons manager will be initialised from a
    // separate thread running in network http.
    addons_manager          = new AddonsManager();
    Online::ProfileManager::create();

    // The request manager will start the login process in case of a saved
    // session, so we need to read the main data from the players.xml file.
    // The rest will be read later (since the rest needs the unlock- and
    // achievement managers to be created, which can only be created later).
    PlayerManager::create();
    Online::RequestManager::get()->startNetworkThread();
    NewsManager::get();   // this will create the news manager

    music_manager           = new MusicManager();
    SFXManager::create();
    // The order here can be important, e.g. KartPropertiesManager needs
    // defaultKartProperties, which are defined in stk_config.
    history                 = new History              ();
    ReplayRecorder::create();
    material_manager        = new MaterialManager      ();
    track_manager           = new TrackManager         ();
    kart_properties_manager = new KartPropertiesManager();
    projectile_manager      = new ProjectileManager    ();
    powerup_manager         = new PowerupManager       ();
    attachment_manager      = new AttachmentManager    ();
    highscore_manager       = new HighscoreManager     ();
    KartPropertiesManager::addKartSearchDir(
                 file_manager->getAddonsFile("karts/"));
    track_manager->addTrackSearchDir(
                 file_manager->getAddonsFile("tracks/"));

    track_manager->loadTrackList();
    music_manager->addMusicToTracks();

    GUIEngine::addLoadingIcon(irr_driver->getTexture(FileManager::GUI,
                                                     "notes.png"      ) );

    grand_prix_manager      = new GrandPrixManager     ();
    // Consistency check for challenges, and enable all challenges
    // that have all prerequisites fulfilled
    grand_prix_manager->checkConsistency();
    GUIEngine::addLoadingIcon( irr_driver->getTexture(FileManager::GUI,
                                                      "cup_gold.png"    ) );

    race_manager            = new RaceManager          ();
    // default settings for Quickstart
    race_manager->setNumLocalPlayers(1);
    race_manager->setNumLaps   (3);
    race_manager->setMajorMode (RaceManager::MAJOR_MODE_SINGLE);
    race_manager->setMinorMode (RaceManager::MINOR_MODE_NORMAL_RACE);
    race_manager->setDifficulty(
                 (RaceManager::Difficulty)(int)UserConfigParams::m_difficulty);
    if(track_manager->getTrack(UserConfigParams::m_last_track))
        race_manager->setTrack(UserConfigParams::m_last_track);

}   // initRest

//=============================================================================
void askForInternetPermission()
{
    if (UserConfigParams::m_internet_status ==
        Online::RequestManager::IPERM_NOT_ASKED)
    {
        class ConfirmServer :
            public MessageDialog::IConfirmDialogListener
        {
        public:
            virtual void onConfirm()
            {
                // Typically internet is disabled here (just better safe
                // than sorry). If internet should be allowed, the news
                // manager needs to be started (which in turn activates
                // the addons manager).
                bool need_to_start_news_manager = 
                     UserConfigParams::m_internet_status!=
                                       Online::RequestManager::IPERM_ALLOWED;
                UserConfigParams::m_internet_status =
                    Online::RequestManager::IPERM_ALLOWED;
                if(need_to_start_news_manager)
                    NewsManager::get()->init(false);
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

        new MessageDialog(_("SuperTuxKart may connect to a server "
            "to download add-ons and notify you of updates. We also collect "
            "anonymous hardware statistics to help with the development of STK. "
            "Would you like this feature to be enabled? (To change this setting "
            "at a later time, go to options, select tab "
            "'User Interface', and edit \"Allow STK to connect to the "
            "Internet\" and \"Allow STK to send anonymous HW statistics\")."),
            MessageDialog::MESSAGE_DIALOG_YESNO,
            new ConfirmServer(), true);
    }

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
int main(int argc, char *argv[] )
{
    CommandLine::init(argc, argv);

    CrashReporting::installHandlers();

    srand(( unsigned ) time( 0 ));

    try
    {
        std::string s;
        if(CommandLine::has("--root", &s))
        {
            FileManager::addRootDirs(s);
        }

        // Init the minimum managers so that user config exists, then
        // handle all command line options that do not need (or must
        // not have) other managers initialised:
        initUserConfig();

        handleCmdLinePreliminary();

        initRest();

        input_manager = new InputManager ();

#ifdef ENABLE_WIIUSE
        wiimote_manager = new WiimoteManager();
#endif

        // Get into menu mode initially.
        input_manager->setMode(InputManager::MENU);
        main_loop = new MainLoop();
        material_manager->loadMaterial();

        // Load the font textures - they are all lazily loaded
        // so no need to push a texture search path. They will actually
        // be loaded from ScalableFont.
        material_manager->addSharedMaterial(
                   file_manager->getAsset(FileManager::FONT,"materials.xml"));

        GUIEngine::addLoadingIcon( irr_driver->getTexture(FileManager::GUI,
                                                          "options_video.png"));
        kart_properties_manager -> loadAllKarts    ();
        handleXmasMode();

        // Needs the kart and track directories to load potential challenges
        // in those dirs, so it can only be created after reading tracks
        // and karts.
        unlock_manager = new UnlockManager();
        AchievementsManager::create();

        // Reading the rest of the player data needs the unlock manager to
        // initialise the game slots of all players and the AchievementsManager
        // to initialise the AchievementsStatus, so it is done only now.
        PlayerManager::get()->initRemainingData();

        GUIEngine::addLoadingIcon( irr_driver->getTexture(FileManager::GUI,
                                                          "gui_lock.png"  ) );
        projectile_manager->loadData();

        // Both item_manager and powerup_manager load models and therefore
        // textures from the model directory. To avoid reading the
        // materials.xml twice, we do this here once for both:
        file_manager->pushTextureSearchPath(file_manager->getAsset(FileManager::MODEL,""));
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
        powerup_manager         -> loadAllPowerups ();
        ItemManager::loadDefaultItemMeshes();

        GUIEngine::addLoadingIcon( irr_driver->getTexture(FileManager::GUI,
                                                          "gift.png")       );

        file_manager->popTextureSearchPath();

        attachment_manager->loadModels();

        GUIEngine::addLoadingIcon( irr_driver->getTexture(FileManager::GUI,
                                                          "banana.png")    );

        //handleCmdLine() needs InitTuxkart() so it can't be called first
        if(!handleCmdLine()) exit(0);

        // load the network manager
        // If the server has been created (--server option), this will do nothing (just a warning):
        NetworkManager::getInstance<ClientNetworkManager>();
        if (NetworkManager::getInstance()->isServer())
        {
            ServerNetworkManager::getInstance()->setMaxPlayers(
                    UserConfigParams::m_server_max_players);
        }
        NetworkManager::getInstance()->run();
        if (NetworkManager::getInstance()->isServer())
        {
            ProtocolManager::getInstance()->requestStart(new ServerLobbyRoomProtocol());
        }

        addons_manager->checkInstalledAddons();

        // Load addons.xml to get info about addons even when not
        // allowed to access the internet
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
                    Log::warn("Addons", "Exception thrown when initializing addons manager : %s", e.what());
                }
            }
        }

        // Note that on the very first run of STK internet status is set to
        // "not asked", so the report will only be sent in the next run.
        if(UserConfigParams::m_internet_status==Online::RequestManager::IPERM_ALLOWED)
        {
            HardwareStats::reportHardwareStats();
        }

        if(!UserConfigParams::m_no_start_screen)
        {
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
                if(PlayerManager::get()->getNumPlayers()==0)
                    RegisterScreen::getInstance()->push();
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

        // If an important news message exists it is shown in a popup dialog.
        const core::stringw important_message =
                                     NewsManager::get()->getImportantMessage();
        if(important_message!="")
        {
            new MessageDialog(important_message,
                              MessageDialog::MESSAGE_DIALOG_OK,
                              NULL, true);
        }   // if important_message


        // Replay a race
        // =============
        if(history->replayHistory())
        {
            // This will setup the race manager etc.
            history->Load();
            race_manager->setupPlayerKartInfo();
            race_manager->startNew(false);
            main_loop->run();
            // well, actually run() will never return, since
            // it exits after replaying history (see history::GetNextDT()).
            // So the next line is just to make this obvious here!
            exit(-3);
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
                race_manager->setupPlayerKartInfo();
                race_manager->startNew(false);
            }
        }
        else  // profile
        {
            // Profiling
            // =========
            race_manager->setMajorMode (RaceManager::MAJOR_MODE_SINGLE);
            race_manager->setupPlayerKartInfo();
            race_manager->startNew(false);
        }
        main_loop->run();

    }  // try
    catch (std::exception &e)
    {
        Log::error("main", "Exception caught : %s.",e.what());
        Log::error("main", "Aborting SuperTuxKart.");
    }

    /* Program closing...*/

#ifdef ENABLE_WIIUSE
    if(wiimote_manager)
        delete wiimote_manager;
#endif

    // If the window was closed in the middle of a race, remove players,
    // so we don't crash later when StateManager tries to access input devices.
    StateManager::get()->resetActivePlayers();
    if(input_manager) delete input_manager; // if early crash avoid delete NULL
    NetworkManager::getInstance()->abort();

    cleanSuperTuxKart();

#ifdef DEBUG
    MemoryLeaks::checkForLeaks();
#endif

#ifndef WIN32
    if (user_config) //close logfiles
    {
        Log::closeOutputFiles();
#endif
        fclose(stderr);
        fclose(stdout);
#ifndef WIN32
    }
#endif

    return 0 ;
}   // main

// ============================================================================
#ifdef WIN32
//routine for running under windows
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                     LPTSTR lpCmdLine, int nCmdShow)
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

    SFXManager::get()->stopThread();
    irr_driver->updateConfigIfRelevant();
    AchievementsManager::destroy();
    Referee::cleanup();
    if(ReplayPlay::get())       ReplayPlay::destroy();
    if(race_manager)            delete race_manager;
    if(grand_prix_manager)      delete grand_prix_manager;
    if(highscore_manager)       delete highscore_manager;
    if(attachment_manager)      delete attachment_manager;
    ItemManager::removeTextures();
    if(powerup_manager)         delete powerup_manager;
    if(projectile_manager)      delete projectile_manager;
    if(kart_properties_manager) delete kart_properties_manager;
    if(track_manager)           delete track_manager;
    if(material_manager)        delete material_manager;
    if(history)                 delete history;
    ReplayRecorder::destroy();
    SFXManager::destroy();
    if(music_manager)           delete music_manager;
    delete ParticleKindManager::get();
    PlayerManager::destroy();
    if(unlock_manager)          delete unlock_manager;
    Online::ProfileManager::destroy();
    GUIEngine::DialogQueue::deallocate();

    // Now finish shutting down objects which a separate thread. The
    // RequestManager has been signaled to shut down as early as possible,
    // the NewsManager thread should have finished quite early on anyway.
    // But still give them some additional time to finish. It avoids a
    // race condition where a thread might access the file manager after it
    // was deleted (in cleanUserConfig below), but before STK finishes and
    // the os takes all threads down.

    if(!NewsManager::get()->waitForReadyToDeleted(2.0f))
    {
        Log::info("Thread", "News manager not stopping, exiting anyway.");
    }
    NewsManager::deallocate();

    if(!Online::RequestManager::get()->waitForReadyToDeleted(5.0f))
    {
        Log::info("Thread", "Request Manager not aborting in time, aborting.");
    }
    Online::RequestManager::deallocate();

    // The addons manager might still be called from a currenty running request
    // in the request manager, so it can not be deleted earlier.
    if(addons_manager)  delete addons_manager;

    // FIXME: do we need to wait for threads there, can they be
    // moved further up?
    Online::ServersManager::deallocate();
    NetworkManager::kill();

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

    if(file_manager)            delete file_manager;
    if(irr_driver)              delete irr_driver;
}
