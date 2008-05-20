// $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006 Joerg Henrichs, Steve Baker
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#ifdef __APPLE__
// Necessary for Macs when using SDL without Xwindows: this include
// will rename main to SDLmain, and a new main program will be linked
// in from the library, causing a correct framework to be set up
#  include "SDL/SDL.h"
#endif

#ifdef WIN32
#  ifdef __CYGWIN__
#    include <unistd.h>
#  endif
#  include <windows.h>
#  ifdef _MSC_VER
#    include <io.h>
#    include <direct.h>
#  endif
#else
#  include <unistd.h>
#endif
#include <stdexcept>
#include <cstdio>
#include <string>
#include <sstream>
#include <algorithm>

#include "user_config.hpp"
#include "unlock_manager.hpp"
#include "track_manager.hpp"
#include "track.hpp"
#include "kart_properties_manager.hpp"
#include "kart.hpp"
#include "projectile_manager.hpp"
#include "race_manager.hpp"
#include "file_manager.hpp"
#include "loader.hpp"
#include "game_manager.hpp"
#include "widget_manager.hpp"
#include "material_manager.hpp"
#include "sdldrv.hpp"
#include "callback_manager.hpp"
#include "history.hpp"
#include "herring_manager.hpp"
#include "attachment_manager.hpp"
#include "sound_manager.hpp"
#include "stk_config.hpp"
#include "translation.hpp"
#include "highscore_manager.hpp"
#include "gui/menu_manager.hpp"
#include "scene.hpp"

// Only needed for bullet debug!
#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

void cmdLineHelp (char* invocation)
{
    fprintf ( stdout,
    "Usage: %s [OPTIONS]\n\n"
    "Run SuperTuxKart, a racing game with go-kart that features"
    " the Tux and friends.\n\n"
    "Options:\n"
    "  -N,  --no-start-screen  Quick race\n"
    "  -t,  --track NAME       Start at track NAME (see --list-tracks)\n"
    "       --stk-config FILE  use ./data/FILE instead of ./data/stk_config.data\n"
    "  -l,  --list-tracks      Show available tracks\n"
    "  -k,  --numkarts NUM     Number of karts on the racetrack\n"
    "       --kart NAME        Use kart number NAME (see --list-karts)\n"
    "       --list-karts       Show available karts\n"
    "       --laps N           Define number of laps to N\n"
    "       --mode N           N=1 novice, N=2 driver, N=3 racer\n"
    "  --players n             Define number of players to between 1 and 4.\n"
    //FIXME     "  --reverse               Enable reverse mode\n"
    //FIXME     "  --mirror                Enable mirror mode (when supported)\n"
    "       --herring STYLE    Use STYLE as your herring style\n"
    "  -f,  --fullscreen       Fullscreen display\n"
    "  -w,  --windowed         Windowed display (default)\n"
    "  -s,  --screensize WxH   Set the screen size (e.g. 320x200)\n"
    "  -v,  --version          Show version\n"
    // should not be used by unaware users:
    // "  --profile            Enable automatic driven profile mode for 20 seconds\n"
    // "  --profile=n          Enable automatic driven profile mode for n seconds\n"
    // "                       if n<0 --> (-n) = number of laps to drive
    // "  --history            Replay history file 'history.dat'\n"
    "  --log=terminal          Write messages to screen\n"
    "  --log=file              Write messages/warning to log files stdout.log/stderr.log\n"
    "  -h,  --help             Show this help\n"
    "\n"
    "You can visit SuperTuxKart's homepage at "
    "http://supertuxkart.sourceforge.net\n\n", invocation
    );
}   // cmdLineHelp

//=============================================================================
int handleCmdLine(int argc, char **argv)
{
    int n;
    for(int i=1; i<argc; i++)
    {
        if(argv[i][0] != '-') continue;
        if ( !strcmp(argv[i], "--help"    ) ||
             !strcmp(argv[i], "-help"     ) ||
             !strcmp(argv[i], "--help" ) ||
             !strcmp(argv[i], "-help" ) ||
             !strcmp(argv[i], "-h")       )
        {
            cmdLineHelp(argv[0]);
            return 0;
        }
        else if(!strcmp(argv[i], "--keyboard-debug"))
        {
            user_config->m_keyboard_debug=true;
        }
        else if(sscanf(argv[i], "--track-debug=%d",&n)==1)
        {
            user_config->m_track_debug=n;
        }
        else if(!strcmp(argv[i], "--track-debug"))
        {
            user_config->m_track_debug=1;
        }
        else if(!strcmp(argv[i], "--bullet-debug"))
        {
            user_config->m_bullet_debug=1;
        }
        else if( (!strcmp(argv[i], "--kart") && i+1<argc ))
        {
            std::string filename=file_manager->getKartFile(std::string(argv[i+1])+".tkkf");
            if(filename!="")
            {
                race_manager->setPlayerKart(0, argv[i+1]);
                fprintf ( stdout, "You choose to use kart '%s'.\n", argv[i+1] ) ;
            }
            else
            {
	            fprintf(stdout, "Kart '%s' not found, ignored.\n",
		                argv[i+1]);
            }
        }
        else if( (!strcmp(argv[i], "--mode") && i+1<argc ))
        {
            switch (atoi(argv[i+1]))
            {
            case 1:
                race_manager->setDifficulty(RaceManager::RD_EASY);
                break;
            case 2:
                // FIXME: no medium AI atm race_manager->setDifficulty(RaceManager::RD_MEDIUM);
                race_manager->setDifficulty(RaceManager::RD_HARD);
                break;
            case 3:
                race_manager->setDifficulty(RaceManager::RD_HARD);
                break;
            }
        }
        else if( (!strcmp(argv[i], "--track") || !strcmp(argv[i], "-t"))
                 && i+1<argc                                              )
        {
            if (!unlock_manager->isLocked(argv[i+1]))
            {
                race_manager->setTrack(argv[i+1]);
                fprintf ( stdout, "You choose to start in track: %s.\n", argv[i+1] ) ;
            }
            else 
            {
                fprintf(stdout, "Track %s has not been unlocked yet. \n", argv[i+1]);
                fprintf(stdout, "Use --list-tracks to list available tracks.\n\n");
                return 0;
            }    
        }
        else if( (!strcmp(argv[i], "--stk-config")) && i+1<argc )
        {
            stk_config->load(file_manager->getConfigFile(argv[i+1]));
            fprintf ( stdout, "STK config will be read from %s.\n", argv[i+1] ) ;
        }
        else if( (!strcmp(argv[i], "--numkarts") || !strcmp(argv[i], "-k")) &&
                 i+1<argc )
        {
            user_config->m_karts = atoi(argv[i+1]);
            if(user_config->m_karts>stk_config->m_max_karts) {
                fprintf(stdout, "Number of karts reset to maximum number %d\n",
                                  stk_config->m_max_karts);
                user_config->m_karts = stk_config->m_max_karts;
            }
            race_manager->setNumKarts(user_config->m_karts );
            fprintf ( stdout, "%d karts will be used.\n", user_config->m_karts);
            i++;
        }
        else if( !strcmp(argv[i], "--list-tracks") || !strcmp(argv[i], "-l") )
        {

            fprintf ( stdout, "  Available tracks:\n" );
            for (size_t i = 0; i != track_manager->getTrackCount(); i++)
            {
                const Track *track = track_manager->getTrack(i);
                if (!unlock_manager->isLocked(track->getIdent()))
                {
                    fprintf ( stdout, "\t%10s: %s\n",
                              track->getIdent().c_str(),
                              track->getName());
                }
            }    

            fprintf ( stdout, "Use --track N to choose track.\n\n");
            delete track_manager;
            track_manager = 0;

            return 0;
        }
        else if( !strcmp(argv[i], "--list-karts") )
        {
            bool dont_load_models=true;
            kart_properties_manager->loadKartData(dont_load_models) ;

            fprintf ( stdout, "  Available karts:\n" );
            for (unsigned int i = 0; NULL != kart_properties_manager->getKartById(i); i++)
            {
                const KartProperties* KP= kart_properties_manager->getKartById(i);
                fprintf (stdout, "\t%10s: %s\n", KP->getIdent().c_str(), KP->getName().c_str());
            }
            fprintf ( stdout, "\n" );

            return 0;
        }
        else if (    !strcmp(argv[i], "--no-start-screen")
                     || !strcmp(argv[i], "-N")                )
        {
            user_config->m_no_start_screen = true;
            //FIXME} else if ( !strcmp(argv[i], "--reverse") ) {
            //FIXME:fprintf ( stdout, "Enabling reverse mode.\n" ) ;
            //FIXME:raceSetup.reverse = 1;
        }
        else if ( !strcmp(argv[i], "--mirror") )
        {
#ifdef SSG_BACKFACE_COLLISIONS_SUPPORTED
            fprintf ( stdout, "Enabling mirror mode.\n" ) ;
            //raceSetup.mirror = 1;
#else
            //raceSetup.mirror = 0 ;
#endif

        }
        else if ( !strcmp(argv[i], "--laps") && i+1<argc )
        {
            fprintf ( stdout, "You choose to have %d laps.\n", atoi(argv[i+1]) ) ;
            race_manager->setNumLaps(atoi(argv[i+1]));
        }
        /* FIXME:
        else if ( !strcmp(argv[i], "--players") && i+1<argc ) {
          raceSetup.numPlayers = atoi(argv[i+1]);

          if ( raceSetup.numPlayers < 0 || raceSetup.numPlayers > 4) {
        fprintf ( stderr,
        "You choose an invalid number of players: %d.\n",
        raceSetup.numPlayers );
        cmdLineHelp(argv[0]);
        return 0;
          }
          fprintf ( stdout, "You choose to have %d players.\n", atoi(argv[i+1]) ) ;
        }
        */
#if !defined(WIN32) && !defined(__CYGWIN)
        else if ( !strcmp(argv[i], "--fullscreen") || !strcmp(argv[i], "-f"))
        {
            // Check that current res is not blacklisted
            std::ostringstream o;
    		o << user_config->m_width << "x" << user_config->m_height;
    		std::string res = o.str();
            if (std::find(user_config->m_blacklist_res.begin(), 
              user_config->m_blacklist_res.end(),res) == user_config->m_blacklist_res.end())         
            	user_config->m_fullscreen = true;
          	else 
          		fprintf ( stdout, "Resolution %s has been blacklisted, so it is not available!\n", res.c_str());
        }
        else if ( !strcmp(argv[i], "--windowed") || !strcmp(argv[i], "-w"))
        {
            user_config->m_fullscreen = false;
        }
#endif
        else if ( !strcmp(argv[i], "--screensize") || !strcmp(argv[i], "-s") )
        {
            //Check if fullscreen and new res is blacklisted
            int width, height;
            if (sscanf(argv[i+1], "%dx%d", &width, &height) == 2)
            {
            	std::ostringstream o;
    			o << width << "x" << height;
    			std::string res = o.str();
                if (!user_config->m_fullscreen || std::find(user_config->m_blacklist_res.begin(), 
              	  user_config->m_blacklist_res.end(),res) == user_config->m_blacklist_res.end())
                {
                	user_config->m_prev_width = user_config->m_width = width;
               		user_config->m_prev_height = user_config->m_height = height;
                	fprintf ( stdout, "You choose to be in %dx%d.\n", user_config->m_width,
                    	 user_config->m_height );
               	}
               	else
               		fprintf ( stdout, "Resolution %s has been blacklisted, so it is not available!\n", res.c_str());
            }
            else
            {
                fprintf(stderr, "Error: --screensize argument must be given as WIDTHxHEIGHT\n");
                exit(EXIT_FAILURE);
            }
        }
        else if( !strcmp(argv[i], "--version") ||  !strcmp(argv[i], "-v") )
        {
#ifdef VERSION
            fprintf ( stdout, "SuperTuxKart, %s.\n", VERSION ) ;
#endif
#ifdef SVNVERSION
            fprintf ( stdout, "SuperTuxKart, SVN revision number '%s'.\n", SVNVERSION ) ;
#endif
            return 0;
        }
        else if( !strcmp(argv[i], "--log=terminal"))
        {
            user_config->m_log_errors=false;
        }
        else if( !strcmp(argv[i], "--log=file"))
        {
            user_config->m_log_errors=true;
        }else if( sscanf(argv[i], "--profile=%d",  &n)==1)
        {
            user_config->m_profile=n;
	    if(n<0) 
	    {
                fprintf(stdout,"Profiling %d laps\n",-n);
                race_manager->setNumLaps(-n);
	    }
	    else
            {
	        printf("Profiling: %d seconds.\n",user_config->m_profile);
                race_manager->setNumLaps   (999999); // profile end depends on time
            }
        }
        else if( !strcmp(argv[i], "--profile") )
        {
            user_config->m_profile=20;
        }
        else if( !strcmp(argv[i], "--history") )
        {
            user_config->m_replay_history=true;
        }
        else if( !strcmp(argv[i], "--herring") && i+1<argc )
        {
            herring_manager->setUserFilename(argv[i+1]);
        }
        else
        {
            fprintf ( stderr, "Invalid parameter: %s.\n\n", argv[i] );
            cmdLineHelp(argv[0]);
            return 0;
        }
    }   // for i <argc
    if(user_config->m_profile)
    {
        user_config->setSFX(UserConfig::UC_DISABLE);  // Disable sound effects 
        user_config->setMusic(UserConfig::UC_DISABLE);// and music when profiling
    }

    return 1;
}   /* handleCmdLine */

//=============================================================================
void InitTuxkart()
{
    file_manager            = new FileManager();
    translations            = new Translations();
    loader                  = new Loader();
    loader->setCreateStateCallback(getAppState);
    // unlock manager is needed when reading the config file
    unlock_manager          = new UnlockManager();
    user_config             = new UserConfig();
    sound_manager           = new SoundManager();

    // The order here can be important, e.g. KartPropertiesManager needs
    // defaultKartProperties.
    history                 = new History              ();
    material_manager        = new MaterialManager      ();
    track_manager           = new TrackManager         ();
    stk_config              = new STKConfig            ();
    kart_properties_manager = new KartPropertiesManager();
    projectile_manager      = new ProjectileManager    ();
    collectable_manager     = new CollectableManager   ();
    callback_manager        = new CallbackManager      ();
    herring_manager         = new HerringManager       ();
    attachment_manager      = new AttachmentManager    ();
    highscore_manager       = new HighscoreManager     ();
    track_manager->loadTrackList();
    sound_manager->addMusicToTracks();

    stk_config->load(file_manager->getConfigFile("stk_config.data"));
    race_manager            = new RaceManager          ();
    // default settings for Quickstart
    race_manager->setNumPlayers(1);
    race_manager->setNumLaps   (3);
    race_manager->setRaceMode  (RaceManager::RM_QUICK_RACE);
    race_manager->setDifficulty(RaceManager::RD_HARD);
}

//=============================================================================

int main(int argc, char *argv[] ) 
{
    try {
        glutInit(&argc, argv);
        InitTuxkart();

        //handleCmdLine() needs InitTuxkart() so it can't be called first
        if(!handleCmdLine(argc, argv)) exit(0);
        
        if (user_config->m_log_errors) //Enable logging of stdout and stderr to logfile
        {
            std::string logoutfile = file_manager->getLogFile("stdout.log");
            std::string logerrfile = file_manager->getLogFile("stderr.log");
            std::cout << "Error messages and other text output will be logged to " ;
            std::cout << logoutfile << " and "<<logerrfile;
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
        
        //FIXME: this needs a better organization
        inputDriver = new SDLDriver ();
        ssgInit () ;
        
        game_manager = new GameManager ();
        // loadMaterials needs ssgLoadTextures (internally), which can
        // only be called after ssgInit (since this adds the actual file_manager)
        // so this next call can't be in InitTuxkart. And InitPlib needs
        // config, which gets defined in InitTuxkart, so swapping those two
        // calls is not possible either ... so loadMaterial has to be done here :(
        material_manager        -> loadMaterial       ();
        kart_properties_manager -> loadKartData       ();
        projectile_manager      -> loadData           ();
        collectable_manager     -> loadCollectables   ();
        herring_manager         -> loadDefaultHerrings();
        attachment_manager      -> loadModels         ();
        scene = new Scene();

        //For some reason, calling this before the material loading screws
        //the background picture.
        fntInit();
        init_fonts();

        widget_manager   = new WidgetManager;
        menu_manager->switchToMainMenu();

        // Replay a race
        // =============
        if(user_config->m_replay_history)
        {
            // This will setup the race manager etc.
            history->Load();
            race_manager->startNew();
            game_manager->run();
            // well, actually run() will never return, since
            // it exits after replaying history (see history::GetNextDT()).
            // So the next line is just to make this obvious here!
            exit(-3);
        }
        
        // Not replaying
        // =============
        if(!user_config->m_profile)
        {
            if(user_config->m_no_start_screen)
            {
                // Quickstart (-N)
                // ===============
                // all defaults are set in InitTuxkart()
                race_manager->startNew();
            }
        }
        else  // profile
        {

            // Profiling
            // =========
            race_manager->setNumPlayers(1);
            race_manager->setPlayerKart(0, kart_properties_manager->getKart("tuxkart")->getIdent());
            race_manager->setRaceMode  (RaceManager::RM_QUICK_RACE);
            race_manager->setDifficulty(RaceManager::RD_HARD);
            race_manager->startNew();
        }
        game_manager->run();

    }  // try
    catch (std::exception &e)
    {
        fprintf(stderr,e.what());
        fprintf(stderr,"\nAborting SuperTuxKart\n");
    }

    /* Program closing...*/

    if (user_config->m_crashed) user_config->m_crashed = false;
    user_config->saveConfig();
    delete inputDriver;
    
    if (user_config->m_log_errors) //close logfiles
    {
        fclose(stderr);
        fclose(stdout);
    }

    delete highscore_manager;
    delete_fonts();

    return 0 ;
}

