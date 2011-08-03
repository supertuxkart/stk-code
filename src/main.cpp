// $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2006 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2011 Joerg Henrichs, Marianne Gagnon
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


#ifdef WIN32
#  ifdef __CYGWIN__
#    include <unistd.h>
#  endif
#  define _WINSOCKAPI_
#  include <windows.h>
#  ifdef _MSC_VER
#    include <io.h>
#    include <direct.h>
#    include <time.h>
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
#include "addons/addons_manager.hpp"
#include "addons/network_http.hpp"
#include "addons/news_manager.hpp"
#include "audio/music_manager.hpp"
#include "audio/sfx_manager.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "config/player.hpp"
#include "graphics/hardware_skinning.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material_manager.hpp"
#include "guiengine/engine.hpp"
#include "input/input_manager.hpp"
#include "input/device_manager.hpp"
#include "io/file_manager.hpp"
#include "items/attachment_manager.hpp"
#include "items/item_manager.hpp"
#include "items/projectile_manager.hpp"
#include "karts/kart_properties_manager.hpp"
#include "karts/kart.hpp"
#include "modes/profile_world.hpp"
#include "network/network_manager.hpp"
#include "race/grand_prix_manager.hpp"
#include "race/highscore_manager.hpp"
#include "race/history.hpp"
#include "race/race_manager.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "tutorial/tutorial_manager.hpp"
#include "utils/constants.hpp"
#include "utils/translation.hpp"

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
                for (int i=0; i<SEvent::SJoystickEvent::NUMBER_OF_AXES; i++) g.m_axis[i] = 0;
                for (int i=0; i<SEvent::SJoystickEvent::NUMBER_OF_BUTTONS; i++) g.m_button_state[i] = false;
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
                    for (int i=0; i<SEvent::SJoystickEvent::NUMBER_OF_AXES; i++)
                    {
                        g.m_axis[i] = evt.Axis[i];
                    }
                    for (int i=0; i<SEvent::SJoystickEvent::NUMBER_OF_BUTTONS; i++)
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
                        if (evt.Key == KEY_RETURN || evt.Key == KEY_ESCAPE || evt.Key == KEY_SPACE)
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
        const core::dimension2d<u32> size = driver ->getCurrentRenderTargetSize();
        
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
            const int BTN_SIZE = (w - 10)/SEvent::SJoystickEvent::NUMBER_OF_BUTTONS;
            
            for (int b=0; b<SEvent::SJoystickEvent::NUMBER_OF_BUTTONS; b++)
            {
                position2di pos(btn_x + b*BTN_SIZE, btn_y);
                dimension2di size(BTN_SIZE, BTN_SIZE);
                
                if (g.m_button_state[b])
                {
                    driver->draw2DRectangle (video::SColor(255,255,0,0), core::recti(pos, size));
                }
                
                driver->draw2DRectangleOutline( core::recti(pos, size) );
            }
                        
            const int axis_y = btn_y + BTN_SIZE + 5;
            const int axis_x = btn_x;
            const int axis_w = w - 10;
            const int axis_h = (h - BTN_SIZE - 15) / SEvent::SJoystickEvent::NUMBER_OF_AXES;
            
            for (int a=0; a<SEvent::SJoystickEvent::NUMBER_OF_AXES; a++)
            {
                const float rate = g.m_axis[a] / 32767.0f;
                
                position2di pos(axis_x, axis_y + a*axis_h);
                dimension2di size(axis_w, axis_h);
                
                const bool deadzone = (abs(g.m_axis[a]) < DEADZONE_JOYSTICK);
                
                core::recti fillbar(position2di(axis_x + axis_w/2, 
                                                axis_y + a*axis_h),
                                    dimension2di( (int)(axis_w/2*rate), 
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
}
// ============================================================================


void cmdLineHelp (char* invocation)
{
    fprintf ( stdout,
    "Usage: %s [OPTIONS]\n\n"
    "Run SuperTuxKart, a racing game with go-kart that features"
    " the Tux and friends.\n\n"
    "Options:\n"
    "  -N,  --no-start-screen  Immediately start race without showing a menu.\n"
    "  -R,  --race-now         Same as -N but also skip the ready-set-go phase and the music.\n"
    "  -t,  --track NAME       Start at track NAME (see --list-tracks).\n"
    "  --gp name               Start the specified Grand Prix.\n"
    "       --stk-config FILE  use ./data/FILE instead of ./data/stk_config.xml\n"
    "  -l,  --list-tracks      Show available tracks.\n"
    "  -k,  --numkarts NUM     Number of karts on the racetrack.\n"
    "       --kart NAME        Use kart number NAME (see --list-karts).\n"
    "       --ai=a,b,...       Use the karts a, b, ... for the AI.\n" 
    "       --list-karts       Show available karts.\n"
    "       --laps N           Define number of laps to N.\n"
    "       --mode N           N=1 novice, N=2 driver, N=3 racer.\n"
    // TODO: add back "--players" switch
    //"     --players n             Define number of players to between 1 and 4.\n"
    "       --item STYLE       Use STYLE as your item style.\n"
    "  -f,  --fullscreen       Select fullscreen display.\n"
    "  -w,  --windowed         Windowed display (default).\n"
    "  -s,  --screensize WxH   Set the screen size (e.g. 320x200).\n"
    "  -v,  --version          Show version of SuperTuxKart.\n"
    "  --trackdir DIR          A directory from which additional tracks are loaded.\n"
    "  --renderer NUM          Choose the renderer. Valid renderers are:"
    "                          (Default: 0, OpenGL: 1, Direct3D9: 2, \n"
    "                           Direct3D8: 3, Software: 4, \n"
    "                           Burning's Software: 5, Null device: 6).\n"
    "  --animations=n          Play karts' animations (All: 2, Humans only: 1, Nobody: 0).\n"
    "  --gfx=n                 Play other graphical effects like impact stars dance,\n"
    "                           water animations or explosions (Enable: 1, Disable: 0).\n"
    "  --weather=n             Show weather effects like rain or snow (0 or 1 as --gfx).\n"
    "  --camera-style=n        Flexible (0) or hard like v0.6 (1) kart-camera link.\n"
    "  --profile-laps=n        Enable automatic driven profile mode for n laps.\n"
    "  --profile-time=n        Enable automatic driven profile mode for n seconds.\n"
    "  --no-graphics           Do not display the actual race.\n"
    // "  --history            Replay history file 'history.dat'.\n"
    // "  --history=n          Replay history file 'history.dat' using mode:\n"
    // "                       n=1: use recorded positions\n"
    // "                       n=2: use recorded key strokes\n"
    "  --server[=port]         This is the server (running on the specified port).\n"
    "  --client=ip             This is a client, connect to the specified ip address.\n"
    "  --port=n                Port number to use.\n"
    "  --numclients=n          Number of clients to wait for (server only).\n"
    "  --log=terminal          Write messages to screen.\n"
    "  --log=file              Write messages/warning to log files stdout.log/stderr.log.\n"
    "  -h,  --help             Show this help.\n"
    "\n"
    "You can visit SuperTuxKart's homepage at "
    "http://supertuxkart.sourceforge.net\n\n", invocation
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
int handleCmdLinePreliminary(int argc, char **argv)
{
    for(int i=1; i<argc; i++)
    {
        if(argv[i][0] != '-') continue;
        if (!strcmp(argv[i], "--help" ) ||
            !strcmp(argv[i], "-help"  ) ||
            !strcmp(argv[i], "--help" ) ||
            !strcmp(argv[i], "-help"  ) ||
            !strcmp(argv[i], "-h"     )     )
        {
            cmdLineHelp(argv[0]);
            exit(0);
        }
        else if(!strcmp(argv[i], "--gamepad-visualisation") ||
                !strcmp(argv[i], "--gamepad-visualization")   )
        {
            UserConfigParams::m_gamepad_visualisation=true;
        }
        else if ( !strcmp(argv[i], "--debug=memory") )
        {
            UserConfigParams::m_verbosity |= UserConfigParams::LOG_MEMORY;
        }
        else if ( !strcmp(argv[i], "--debug=addons") )
        {
            UserConfigParams::m_verbosity |= UserConfigParams::LOG_ADDONS;
        }
        else if ( !strcmp(argv[i], "--debug=gui") )
        {
            UserConfigParams::m_verbosity |= UserConfigParams::LOG_GUI;
        }
        else if ( !strcmp(argv[i], "--debug=misc") )
        {
            UserConfigParams::m_verbosity |= UserConfigParams::LOG_MISC;
        }
        else if ( !strcmp(argv[i], "--debug=all") )
        {
            UserConfigParams::m_verbosity |= UserConfigParams::LOG_ALL;
        }
        else if( (!strcmp(argv[i], "--stk-config")) && i+1<argc )
        {
            stk_config->load(file_manager->getDataFile(argv[i+1]));
            fprintf ( stdout, "STK config will be read from %s.\n", argv[i+1] ) ;
            i++;
        }
        else if( !strcmp(argv[i], "--trackdir") && i+1<argc )
        {
            TrackManager::addTrackSearchDir(argv[i+1]);
            i++;
        }
        else if( !strcmp(argv[i], "--kartdir") && i+1<argc )
        {
            KartPropertiesManager::addKartSearchDir(argv[i+1]);
            i++;
        }
#if !defined(WIN32) && !defined(__CYGWIN)
        else if ( !strcmp(argv[i], "--fullscreen") || !strcmp(argv[i], "-f"))
        {
            // Check that current res is not blacklisted
            std::ostringstream o;
            o << UserConfigParams::m_width << "x" << UserConfigParams::m_height;
            std::string res = o.str();
            if (std::find(UserConfigParams::m_blacklist_res.begin(),
                          UserConfigParams::m_blacklist_res.end(),res) == UserConfigParams::m_blacklist_res.end())
                UserConfigParams::m_fullscreen = true;
            else 
                fprintf ( stdout, "Resolution %s has been blacklisted, so it is not available!\n", res.c_str());
        }
        else if ( !strcmp(argv[i], "--windowed") || !strcmp(argv[i], "-w"))
        {
            UserConfigParams::m_fullscreen = false;
        }
#endif
        else if( !strcmp(argv[i], "--renderer") && (i+1 < argc)  )
        {
            printf("You chose renderer %i\n", atoi(argv[i+1]));
            UserConfigParams::m_renderer = atoi(argv[i+1]);
            i++;
        }
        else if ( (!strcmp(argv[i], "--screensize") || !strcmp(argv[i], "-s") )
             && i+1<argc)
        {
            //Check if fullscreen and new res is blacklisted
            int width, height;
            if (sscanf(argv[i+1], "%dx%d", &width, &height) == 2)
            {
                std::ostringstream o;
                o << width << "x" << height;
                std::string res = o.str();
                if (!UserConfigParams::m_fullscreen || std::find(UserConfigParams::m_blacklist_res.begin(), 
                                                            UserConfigParams::m_blacklist_res.end(),res) == UserConfigParams::m_blacklist_res.end())
                {
                    UserConfigParams::m_prev_width = UserConfigParams::m_width = width;
                    UserConfigParams::m_prev_height = UserConfigParams::m_height = height;
                    fprintf ( stdout, "You choose to be in %dx%d.\n", (int)UserConfigParams::m_width,
                             (int)UserConfigParams::m_height );
                }
                else
                    fprintf ( stdout, "Resolution %s has been blacklisted, so it is not available!\n", res.c_str());
                i++;
            }
            else
            {
                fprintf(stderr, "Error: --screensize argument must be given as WIDTHxHEIGHT\n");
                exit(EXIT_FAILURE);
            }
        }
        else if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0)
        {
            printf("==============================\n");
            fprintf ( stdout, "SuperTuxKart, %s.\n", STK_VERSION ) ;
#ifdef SVNVERSION
            fprintf ( stdout, "SuperTuxKart, SVN revision number '%s'.\n", SVNVERSION ) ;
#endif
            
            // IRRLICHT_VERSION_SVN
            fprintf ( stdout, "Irrlicht version %i.%i.%i (%s)\n", IRRLICHT_VERSION_MAJOR , IRRLICHT_VERSION_MINOR,
                                                                  IRRLICHT_VERSION_REVISION, IRRLICHT_SDK_VERSION );
            
            printf("==============================\n");
            exit(0);
        }
    }
    return 0;
}

// ============================================================================
/** Handles command line options.
 *  \param argc Number of command line options
 *  \param argv Command line options.
 */
int handleCmdLine(int argc, char **argv)
{
    int n;
    char s[1024];

    for(int i=1; i<argc; i++)
    {

        if(!strcmp(argv[i], "--gamepad-debug"))
        {
            UserConfigParams::m_gamepad_debug=true;
        }
        else if (!strcmp(argv[i], "--tutorial-debug"))
        {
            UserConfigParams::m_tutorial_debug = true;
        }
        else if(sscanf(argv[i], "--track-debug=%d",&n)==1)
        {
            UserConfigParams::m_track_debug=n;
        }
        else if(!strcmp(argv[i], "--track-debug"))
        {
            UserConfigParams::m_track_debug=1;
        }
        else if(!strcmp(argv[i], "--material-debug"))
        {
            UserConfigParams::m_material_debug = true;
        }
        else if(!strcmp(argv[i], "--ftl-debug"))
        {
            UserConfigParams::m_ftl_debug = true;
        }
        else if(UserConfigParams::m_artist_debug_mode && !strcmp(argv[i], "--camera-debug"))
        {
            UserConfigParams::m_camera_debug=1;
        }
        else if(!strcmp(argv[i], "--kartsize-debug"))
        {
            for(unsigned int i=0; i<kart_properties_manager->getNumberOfKarts();
                i++)
            {
                const KartProperties *km = kart_properties_manager->getKartById(i);
                 printf("%s:\t%swidth: %f length: %f height: %f mesh-buffer count %d\n",
                       km->getIdent().c_str(),
                       (km->getIdent().size()<7) ? "\t" : "",
                       km->getMasterKartModel().getWidth(),
                       km->getMasterKartModel().getLength(),
                       km->getMasterKartModel().getHeight(),
                       km->getMasterKartModel().getModel()->getMeshBufferCount());
            }
        }
        else if(UserConfigParams::m_artist_debug_mode && !strcmp(argv[i], "--check-debug"))
        {
            UserConfigParams::m_check_debug=true;
        }
        else if(!strcmp(argv[i], "--slipstream-debug"))
        {
            UserConfigParams::m_slipstream_debug=true;
        }
        else if(!strcmp(argv[i], "--rendering-debug"))
        {
            UserConfigParams::m_rendering_debug=true;
        }
        else if(sscanf(argv[i], "--server=%d",&n)==1)
        {
            network_manager->setMode(NetworkManager::NW_SERVER);
            UserConfigParams::m_server_port = n;
        }
        else if( !strcmp(argv[i], "--server") )
        {
            network_manager->setMode(NetworkManager::NW_SERVER);
        }
        else if( sscanf(argv[i], "--port=%d", &n) )
        {
            UserConfigParams::m_server_port=n;
        }
        else if( sscanf(argv[i], "--client=%s", s) )
        {
            network_manager->setMode(NetworkManager::NW_CLIENT);
            UserConfigParams::m_server_address=s;
        }
        else if ( sscanf(argv[i], "--gfx=%d", &n) )
        {
            if (n)
            {
                UserConfigParams::m_graphical_effects = true;
            }
            else
            {
                UserConfigParams::m_graphical_effects = false;
            }
        }
        else if ( sscanf(argv[i], "--weather=%d", &n) )
        {
            if (n)
            {
                UserConfigParams::m_weather_effects = true;
            }
            else
            {
                UserConfigParams::m_weather_effects = false;
            }
        }
        else if ( sscanf(argv[i], "--animations=%d", &n) )
        {
            UserConfigParams::m_show_steering_animations = n;
        }

        else if ( sscanf(argv[i], "--camera-style=%d", &n) )
        {
            UserConfigParams::m_camera_style = n;
        }
        else if( (!strcmp(argv[i], "--kart") && i+1<argc ))
        {
            if (!unlock_manager->isLocked(argv[i+1]))
            {
                const KartProperties *prop = kart_properties_manager->getKart(argv[i+1]);
                if(prop)
                {
                    UserConfigParams::m_default_kart = argv[i+1];
                    
                    // if a player was added with -N, change its kart. Otherwise, nothing to do,
                    // kart choice will be picked up upon player creation.
                    if (StateManager::get()->activePlayerCount() > 0)
                    {
                        race_manager->setLocalKartInfo(0, argv[i+1]);
                    }
                    fprintf ( stdout, "You chose to use kart '%s'.\n", argv[i+1] ) ;
                    i++;
                }
                else
                {
                    fprintf(stdout, "Kart '%s' not found, ignored.\n",
                            argv[i+1]);
                }
            }
            else
            {
                fprintf(stdout, "Kart %s has not been unlocked yet. \n", argv[i+1]);
                fprintf(stdout, "Use --list-karts to list available karts.\n\n");
                return 0;
            }
        }
        else if( sscanf(argv[i], "--ai=%s",  &s)==1)
        {
            const std::vector<std::string> l=
                StringUtils::split(std::string(s),',');
            race_manager->setDefaultAIKartList(l);
            // Add 1 for the player kart
            race_manager->setNumKarts(l.size()+1);
        }
        else if( (!strcmp(argv[i], "--mode") && i+1<argc ))
        {
            switch (atoi(argv[i+1]))
            {
            case 1:
                race_manager->setDifficulty(RaceManager::RD_EASY);
                break;
            case 2:
                race_manager->setDifficulty(RaceManager::RD_MEDIUM);
                break;
            case 3:
                race_manager->setDifficulty(RaceManager::RD_HARD);
                break;
            }
            i++;
        }
        else if( (!strcmp(argv[i], "--track") || !strcmp(argv[i], "-t"))
                 && i+1<argc                                              )
        {
            if (!unlock_manager->isLocked(argv[i+1]))
            {
                race_manager->setTrack(argv[i+1]);
                fprintf ( stdout, "You choose to start in track: %s.\n", argv[i+1] ) ;
                
                Track* t = track_manager->getTrack(argv[i+1]);
                if (t == NULL)
                {
                    fprintf(stderr, "Can't find track named <%s>\n", argv[i+1]);
                    exit(1);
                }
                if (t->isArena())
                {
                    race_manager->setMinorMode(RaceManager::MINOR_MODE_3_STRIKES);
                }
            }
            else
            {
                fprintf(stdout, "Track %s has not been unlocked yet. \n", argv[i+1]);
                fprintf(stdout, "Use --list-tracks to list available tracks.\n\n");
                return 0;
            }
            i++;
        }
        else if( (!strcmp(argv[i], "--gp")) && i+1<argc)
        {
            race_manager->setMajorMode(RaceManager::MAJOR_MODE_GRAND_PRIX);
            const GrandPrixData *gp = grand_prix_manager->getGrandPrix(argv[i+1]);
            race_manager->setGrandPrix(*gp);
            i++;
        }
        else if( (!strcmp(argv[i], "--numkarts") || !strcmp(argv[i], "-k")) &&
                 i+1<argc )
        {
            UserConfigParams::m_num_karts = atoi(argv[i+1]);
            if(UserConfigParams::m_num_karts > stk_config->m_max_karts)
            {
                fprintf(stdout, "Number of karts reset to maximum number %d\n",
                                  stk_config->m_max_karts);
                UserConfigParams::m_num_karts = stk_config->m_max_karts;
            }
            race_manager->setNumKarts( UserConfigParams::m_num_karts );
            fprintf(stdout, "%d karts will be used.\n",  (int)UserConfigParams::m_num_karts);
            i++;
        }
        else if( !strcmp(argv[i], "--list-tracks") || !strcmp(argv[i], "-l") )
        {

            fprintf ( stdout, "  Available tracks:\n" );
            for (size_t i = 0; i != track_manager->getNumberOfTracks(); i++)
            {
                const Track *track = track_manager->getTrack(i);
                if (!unlock_manager->isLocked(track->getIdent()))
                {
                    fprintf ( stdout, "\t%10s: %ls\n",
                              track->getIdent().c_str(),
                              track->getName());
                }
            }

            fprintf ( stdout, "Use --track N to choose track.\n\n");
        }
        else if( !strcmp(argv[i], "--list-karts") )
        {
            fprintf ( stdout, "  Available karts:\n" );
            for (unsigned int i = 0; NULL != kart_properties_manager->getKartById(i); i++)
            {
                const KartProperties* KP= kart_properties_manager->getKartById(i);
                if (!unlock_manager->isLocked(KP->getIdent()))
                {
                    fprintf (stdout, "\t%10s: %ls\n", KP->getIdent().c_str(), KP->getName());
                }
            }
            fprintf ( stdout, "\n" );
        }
        else if (    !strcmp(argv[i], "--no-start-screen")
                     || !strcmp(argv[i], "-N")                )
        {
            UserConfigParams::m_no_start_screen = true;
        }
        else if (    !strcmp(argv[i], "--race-now")
                     || !strcmp(argv[i], "-R")                )
        {
            UserConfigParams::m_no_start_screen = true;
            UserConfigParams::m_race_now = true;
        }
        else if ( !strcmp(argv[i], "--laps") && i+1<argc )
        {
            fprintf ( stdout, "You choose to have %d laps.\n", atoi(argv[i+1]) ) ;
            race_manager->setNumLaps(atoi(argv[i+1]));
            i++;
        }
        else if( !strcmp(argv[i], "--log=terminal"))
        {
            UserConfigParams::m_log_errors=false;
        }
        else if( !strcmp(argv[i], "--log=file"))
        {
            UserConfigParams::m_log_errors=true;
        } else if( sscanf(argv[i], "--profile-laps=%d",  &n)==1)
        {
            printf("Profiling %d laps\n",n);
            UserConfigParams::m_no_start_screen = true;
            ProfileWorld::setProfileModeLaps(n);
            race_manager->setNumLaps(n);
        }
        else if( sscanf(argv[i], "--profile-time=%d",  &n)==1)
        {
            printf("Profiling: %d seconds.\n", n);
            UserConfigParams::m_no_start_screen = true;
            ProfileWorld::setProfileModeTime((float)n);
            race_manager->setNumLaps(999999); // profile end depends on time
        }
        else if( !strcmp(argv[i], "--no-graphics") )
        {
            ProfileWorld::disableGraphics();
        }
        else if( sscanf(argv[i], "--history=%d",  &n)==1)
        {
            history->doReplayHistory( (History::HistoryReplayMode)n);
            // Force the no-start screen flag, since this initialises
            // the player structures correctly.
            UserConfigParams::m_no_start_screen = true;

        }
        else if( !strcmp(argv[i], "--history") )
        {
            history->doReplayHistory(History::HISTORY_POSITION);
            // Force the no-start screen flag, since this initialises
            // the player structures correctly.
            UserConfigParams::m_no_start_screen = true;

        }
        else if( !strcmp(argv[i], "--item") && i+1<argc )
        {
            item_manager->setUserFilename(argv[i+1]);
            i++;
        }
        // these commands are already processed in handleCmdLinePreliminary, but repeat this
        // just so that we don't get error messages about unknown commands
        else if( !strcmp(argv[i], "--stk-config")&& i+1<argc ) { i++; }
        else if( !strcmp(argv[i], "--trackdir")  && i+1<argc ) { i++; }
        else if( !strcmp(argv[i], "--kartdir")   && i+1<argc ) { i++; }
        else if( !strcmp(argv[i], "--renderer")  && i+1<argc ) { i++; }
        else if( !strcmp(argv[i], "--debug=memory")                        ) {}
        else if( !strcmp(argv[i], "--debug=addons")                        ) {}
        else if( !strcmp(argv[i], "--debug=gui"   )                        ) {}
        else if( !strcmp(argv[i], "--debug=misc"  )                        ) {}
        else if( !strcmp(argv[i], "--debug=all"   )                        ) {}
        else if( !strcmp(argv[i], "--screensize") || !strcmp(argv[i], "-s")) {i++;}
        else if( !strcmp(argv[i], "--fullscreen") || !strcmp(argv[i], "-f")) {}
        else if( !strcmp(argv[i], "--windowed")   || !strcmp(argv[i], "-w")) {}
        else if( !strcmp(argv[i], "--version")    || !strcmp(argv[i], "-v")) {}
#ifdef __APPLE__
        // on OS X, sometimes the Finder will pass a -psn* something parameter to the application
        else if( strncmp(argv[i], "-psn", 3) == 0) {}
#endif
        else
        {
            fprintf ( stderr, "Invalid parameter: %s.\n\n", argv[i] );
            cmdLineHelp(argv[0]);
            return 0;
        }
    }   // for i <argc
    if(ProfileWorld::isProfileMode())
    {
        UserConfigParams::m_sfx = false;  // Disable sound effects 
        UserConfigParams::m_music = false;// and music when profiling
    }

    return 1;
}   // handleCmdLine

//=============================================================================
/** Initialises the minimum number of managers to get access to user_config.
 */
void initUserConfig(char *argv[])
{
    file_manager            = new FileManager(argv);
    user_config             = new UserConfig();     // needs file_manager
    const bool config_ok    = user_config->loadConfig();
    
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
    
    if (!config_ok || UserConfigParams::m_all_players.size() == 0)
    {
        user_config->addDefaultPlayer();
        user_config->saveConfig();
    }
    
}   // initUserConfig

//=============================================================================
void initRest()
{
    stk_config->load(file_manager->getDataFile("stk_config.xml"));

    irr_driver              = new IrrDriver();
    
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
    news_manager            = new NewsManager();
    addons_manager          = new AddonsManager();
    network_http            = new NetworkHttp();
    // Note that the network thread must be started after the assignment
    // to network_http (since the thread might use network_http, otherwise
    // a race condition can be introduced resulting in a crash).
    network_http->startNetworkThread();
    music_manager           = new MusicManager();
    sfx_manager             = new SFXManager();
    // The order here can be important, e.g. KartPropertiesManager needs
    // defaultKartProperties, which are defined in stk_config.
    history                 = new History              ();
    material_manager        = new MaterialManager      ();
    track_manager           = new TrackManager         ();
    kart_properties_manager = new KartPropertiesManager();
    projectile_manager      = new ProjectileManager    ();
    powerup_manager         = new PowerupManager       ();
    item_manager            = new ItemManager          ();
    attachment_manager      = new AttachmentManager    ();
    highscore_manager       = new HighscoreManager     ();
    network_manager         = new NetworkManager       ();

    KartPropertiesManager::addKartSearchDir(
                 file_manager->getAddonsFile("karts"));
    track_manager->addTrackSearchDir(
                 file_manager->getAddonsFile("tracks"));

    track_manager->loadTrackList();
    music_manager->addMusicToTracks();

    GUIEngine::addLoadingIcon(
        irr_driver->getTexture(file_manager->getTextureFile("notes.png")) );

    grand_prix_manager      = new GrandPrixManager     ();
    // Consistency check for challenges, and enable all challenges
    // that have all prerequisites fulfilled
    grand_prix_manager->checkConsistency();
    GUIEngine::addLoadingIcon( irr_driver->getTexture(file_manager->getTextureFile("cup_gold.png")) );

    race_manager            = new RaceManager          ();
    // default settings for Quickstart
    race_manager->setNumLocalPlayers(1);
    race_manager->setNumLaps   (3);
    race_manager->setMajorMode (RaceManager::MAJOR_MODE_SINGLE);
    race_manager->setMinorMode (RaceManager::MINOR_MODE_NORMAL_RACE);
    race_manager->setDifficulty((RaceManager::Difficulty)(int)UserConfigParams::m_difficulty);
    // race_manager->setDifficulty(RaceManager::RD_HARD);

    //menu_manager= new MenuManager();

}   // initRest

//=============================================================================
void cleanTuxKart()
{
    if(network_http)
        network_http->stopNetworkThread();
    //delete in reverse order of what they were created in.
    //see InitTuxkart()
    if(race_manager)            delete race_manager;
    if(network_http)            delete network_http;
    if(news_manager)            delete news_manager;
    if(network_manager)         delete network_manager;
    if(grand_prix_manager)      delete grand_prix_manager;
    if(highscore_manager)       delete highscore_manager;
    if(attachment_manager)      delete attachment_manager;
    if(item_manager)            delete item_manager;
    if(powerup_manager)         delete powerup_manager;   
    if(projectile_manager)      delete projectile_manager;
    if(kart_properties_manager) delete kart_properties_manager;
    if(track_manager)           delete track_manager;
    if(material_manager)        delete material_manager;
    if(history)                 delete history;
    if(sfx_manager)             delete sfx_manager;
    if(music_manager)           delete music_manager;
    if(stk_config)              delete stk_config;
    if(user_config)             delete user_config;
    if(unlock_manager)          delete unlock_manager;
    if(translations)            delete translations;
    if(file_manager)            delete file_manager;
    if(irr_driver)              delete irr_driver;
}   // cleanTuxKart

//=============================================================================

int main(int argc, char *argv[] )
{
    srand(( unsigned ) time( 0 ));

    try {
        // Init the minimum managers so that user config exists, then
        // handle all command line options that do not need (or must
        // not have) other managers initialised:
        initUserConfig(argv); // argv passed so config file can be
                              // found more reliably
        handleCmdLinePreliminary(argc, argv);

        initRest();

        if (UserConfigParams::m_log_errors) //Enable logging of stdout and stderr to logfile
        {
            std::string logoutfile = file_manager->getLogFile("stdout.log");
            std::string logerrfile = file_manager->getLogFile("stderr.log");
            printf("Error messages and other text output will be logged to %s and %s\n", logoutfile.c_str(), logerrfile.c_str());
            if(freopen (logoutfile.c_str(),"w",stdout)!=stdout)
            {
                fprintf(stderr, "Can not open log file '%s'. Writing to stdout instead.\n",
                        logoutfile.c_str());
            }
            if(freopen (logerrfile.c_str(),"w",stderr)!=stderr)
            {
                fprintf(stderr, "Can not open log file '%s'. Writing to stderr instead.\n",
                        logerrfile.c_str());
            }
        }

        input_manager = new InputManager ();

        // Get into menu mode initially.
        input_manager->setMode(InputManager::MENU);

        main_loop = new MainLoop();
        material_manager        -> loadMaterial    ();
        GUIEngine::addLoadingIcon( irr_driver->getTexture(file_manager->getGUIDir() + "/options_video.png") );
        kart_properties_manager -> loadAllKarts    ();
        unlock_manager          = new UnlockManager();
        //m_tutorial_manager      = new TutorialManager();
        GUIEngine::addLoadingIcon( irr_driver->getTexture(file_manager->getTextureFile("gui_lock.png")) );
        projectile_manager      -> loadData        ();

        // Both item_manager and powerup_manager load models and therefore
        // textures from the model directory. To avoid reading the 
        // materials.xml twice, we do this here once for both:
        file_manager->pushTextureSearchPath(file_manager->getModelFile(""));
        const std::string materials_file = file_manager->getModelFile("materials.xml");
        if(materials_file!="")
        {
            // Some of the materials might be needed later, so just add
            // them all permanently (i.e. as shared). Adding them temporary
            // will actually not be possible: powerup_manager adds some
            // permanent icon materials, which would (with the current
            // implementation) make the temporary materials permanent anyway.
            material_manager->addSharedMaterial(materials_file);
        }
        powerup_manager         -> loadAllPowerups ();
        item_manager            -> loadDefaultItems();

        GUIEngine::addLoadingIcon( irr_driver->getTexture(file_manager->getGUIDir() + "/gift.png") );

        file_manager->popTextureSearchPath();

        attachment_manager      -> loadModels      ();

        GUIEngine::addLoadingIcon( irr_driver->getTexture(file_manager->getGUIDir() + "/banana.png") );

        //handleCmdLine() needs InitTuxkart() so it can't be called first
        if(!handleCmdLine(argc, argv)) exit(0);

        if(!UserConfigParams::m_no_start_screen)
        {
            StateManager::get()->pushScreen(MainMenuScreen::getInstance());
            if(UserConfigParams::m_internet_status==NetworkHttp::IPERM_NOT_ASKED)
            {
                class ConfirmServer : public MessageDialog::IConfirmDialogListener
                {
                public:
                    virtual void onConfirm()
                    {
                        delete network_http;
                        UserConfigParams::m_internet_status = NetworkHttp::IPERM_ALLOWED;
                        GUIEngine::ModalDialog::dismiss();
                        network_http = new NetworkHttp();
                        // Note that the network thread must be started after 
                        // the assignment to network_http (since the thread 
                        // might use network_http, otherwise a race condition 
                        // can be introduced resulting in a crash).
                        network_http->startNetworkThread();

                    }   // onConfirm
                    // --------------------------------------------------------
                    virtual void onCancel()
                    {
                        delete network_http;
                        UserConfigParams::m_internet_status = NetworkHttp::IPERM_NOT_ALLOWED;
                        GUIEngine::ModalDialog::dismiss();
                        network_http = new NetworkHttp();
                        network_http->startNetworkThread();
                    }   // onCancel
                };   // ConfirmServer
                
                new MessageDialog(_("SuperTuxKart may connect to a server "
                    "to download add-ons and notify you of updates. Would you like this feature to be "
                    "enabled? (To change this setting at a later time, go to options, select tab "
                    "'User Interface', and edit \"Internet STK news\")."), new ConfirmServer(), true);
            }
        }
        else 
        {
            InputDevice *device;

            // Use keyboard 0 by default in --no-start-screen
            device = input_manager->getDeviceList()->getKeyboard(0);

            // Create player and associate player with keyboard
            StateManager::get()->createActivePlayer( UserConfigParams::m_all_players.get(0), device );

            // Set up race manager appropriately
            race_manager->setLocalKartInfo(0, UserConfigParams::m_default_kart);

            // ASSIGN should make sure that only input from assigned devices
            // is read.
            input_manager->getDeviceList()->setAssignMode(ASSIGN);

            // Go straight to the race
            StateManager::get()->enterGameState();
        }

        // Replay a race
        // =============
        if(history->replayHistory())
        {
            // This will setup the race manager etc.
            history->Load();
            network_manager->setupPlayerKartInfo();
            race_manager->startNew();
            main_loop->run();
            // well, actually run() will never return, since
            // it exits after replaying history (see history::GetNextDT()).
            // So the next line is just to make this obvious here!
            exit(-3);
        }

        // Initialise connection in case that a command line option was set
        // configuring a client or server. Otherwise this function does nothing
        // here (and will be called again from the network gui).
        if(!network_manager->initialiseConnections())
        {
            fprintf(stderr, "Problems initialising network connections,\n"
                            "Running in non-network mode.\n");
        }
        // On the server start with the network information page for now
        if(network_manager->getMode()==NetworkManager::NW_SERVER)
        {
            // TODO - network menu
            //menu_manager->pushMenu(MENUID_NETWORK_GUI);
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
                network_manager->setupPlayerKartInfo();
                race_manager->startNew();
            }
        }
        else  // profile
        {
            // Profiling
            // =========
            race_manager->setMajorMode (RaceManager::MAJOR_MODE_SINGLE);
            race_manager->setMinorMode (RaceManager::MINOR_MODE_NORMAL_RACE);
            race_manager->setDifficulty(RaceManager::RD_HARD);
            network_manager->setupPlayerKartInfo();
            race_manager->startNew();
        }
        main_loop->run();

    }  // try
    catch (std::exception &e)
    {
        fprintf(stderr,"Exception caught : %s\n",e.what());
        fprintf(stderr,"Aborting SuperTuxKart\n");
    }

    /* Program closing...*/

    if(user_config)
    {
        // In case that abort is triggered before user_config exists
        if (UserConfigParams::m_crashed) UserConfigParams::m_crashed = false;
        user_config->saveConfig();
    }
    if(input_manager) delete input_manager;  // if early crash avoid delete NULL

    if (user_config && UserConfigParams::m_log_errors) //close logfiles
    {
        fclose(stderr);
        fclose(stdout);
    }

    cleanTuxKart();

    return 0 ;
}

