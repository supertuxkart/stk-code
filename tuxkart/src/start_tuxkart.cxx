//  $Id: start_tuxkart.cxx,v 1.78 2004/09/05 20:09:59 matzebraun Exp $
//
//  TuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
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

#include <iostream>
#include <plib/pw.h>
#include <vector>
#include <string>
#include <stdexcept>

#include "tuxkart.h"
#include "sdldrv.h"
#include "ScreenManager.h"
#include "TrackManager.h"
#include "StringUtils.h"
#include "WidgetSet.h"
#include "sound.h"
#include "RaceSetup.h"
#include "Loader.h"
#include "WorldScreen.h"
#include "WorldLoader.h"
#include "material.h"
#include "KartManager.h"
#include "StartScreen.h"
#include "TuxkartError.h"
#include "RaceManager.h"
#include "Config.h"

// Initialize the datadir
static void initializeLoader()
{
  if(!loader) {
    loader = new Loader();
    loader->setModelDir("models");
    loader->setTextureDir("images");

    /*create $HOME/.tuxkart if necessary*/
    loader->initConfigDir();

    /* initialize search path of the loader */
    if ( getenv ( "TUXKART_DATADIR" ) != NULL )
      loader->addSearchPath(getenv ( "TUXKART_DATADIR" )) ;
    if (getenv ("HOME") != NULL) {
      std::string homepath = getenv("HOME");
      homepath += "/.tuxkart";
      loader->addSearchPath(homepath);
    }
    loader->addSearchPath(".");
    loader->addSearchPath("..");
    loader->addSearchPath(TUXKART_DATADIR);

    ssgSetCurrentOptions(loader);
    registerImageLoaders();
  }
}

// Load the datadir, tracklist and plib stuff
static void initTuxKart (int width, int height, int videoFlags)
{
  initVideo ( width, height, videoFlags );
  ssgInit();
  
  initializeLoader();

  initMaterials () ;
  track_manager.loadTrackList () ;
  kart_manager.loadKartData();
  
  sound     = new SoundSystem ;
  widgetSet = new WidgetSet ;
}

void deinitTuxKart()
{
  delete gui;
  delete widgetSet;

  /*write configuration to disk*/
  config.saveConfig();
}

void cmdLineHelp (char* invocation)
{
  fprintf ( stdout, 
	    "Usage: %s [OPTIONS]\n\n"

	    "Run TuxKart, a racing game with go-kart that features"
	    " the well-known linux\nmascott Tux. The game is heavily"
	    " inspired by Super-Mario-Kart and Wacky Wheels.\n\n"

	    "Options:\n"
	    "  -N,  --no-start-screen  Quick race\n"
	    "  -t,  --track NAME       Start at track NAME (see --list-tracks)\n"
	    "  -l,  --list-tracks      Show available tracks.\n"
	    "  -k,  --numkarts NUM     Number of karts on the racetrack\n"
	    "  --kart NAME             Use kart number NAME\n"
	    "  --list-karts            Show available karts.\n"
	    "  --laps n                Define number of laps to n\n"
	    "  --players n             Define number of players to between 1 and 4.\n"
	    "  --reverse               Enable reverse mode\n"
	    "  --mirror                Enable mirror mode (when supported)\n"
	    "  -f,  --fullscreen       Fullscreen display.\n"
	    "  -w,  --windowed         Windowed display. (default)\n"
	    "  -s,  --screensize WIDTHxHEIGHT\n"
	    "                          Set the screen size (e.g. 320x200)\n"
	    "  -v,  --version          Show version.\n"
	    "\n"
	    "You can visit TuxKart's homepage at "
	    "http://tuxkart.sourceforge.net\n\n", invocation
	    );
}


int main ( int argc, char *argv[] )
{
#ifndef DEBUG // in debugmode we wanna have a backtrace of the exception in gdb
  try {
#endif
    // Initialize SDL
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_TIMER) < 0) {
      std::stringstream msg;
      msg << "Couldn't initialize SDL: " << SDL_GetError();
      throw std::runtime_error(msg.str());                                   
    }

    /*load options from configuration file*/
    config.loadConfig();
    RaceManager::instance()->setNumKarts(config.karts);

    bool noStartScreen = false;
    /* Testing if we've given arguments */
    if ( argc > 1) 
      {
        for(int i = 1; i < argc; ++i)
          {
            if ( argv[i][0] != '-') continue;

            if ( !strcmp(argv[i], "--help") or
                 !strcmp(argv[i], "-help") or
                 !strcmp(argv[i], "-h") )
              {
                cmdLineHelp(argv[0]);
                return 0;
              }

            else if( (!strcmp(argv[i], "--kart") and argc > 2 ))
              {
                RaceManager::instance()->setPlayerKart(0, argv[i+1]);
              }

            else if( (!strcmp(argv[i], "--track") or !strcmp(argv[i], "-t")) and argc > 2 )
              {
                RaceManager::instance()->setTrack(argv[i+1]);
                fprintf ( stdout, "You choose to start in track: %s.\n", argv[i+1] ) ;
              }

            else if( (!strcmp(argv[i], "--numkarts") or !strcmp(argv[i], "-k")) && argc > 2)
              {
                RaceManager::instance()->setNumKarts(config.karts = atoi(argv[i+1]));
                fprintf ( stdout, "You choose to have %s karts.\n", argv[i+1] ) ;
              }

            else if( !strcmp(argv[i], "--list-tracks") or !strcmp(argv[i], "-l") )
              {
                initializeLoader ();
                track_manager.loadTrackList () ;

                fprintf ( stdout, "  Available tracks:\n" );
                for (unsigned int i = 0; i != track_manager.tracks.size(); i++)
                  fprintf ( stdout, "\t%10s: %s\n", 
                            track_manager.tracks[i].ident.c_str(),
                            track_manager.tracks[i].name.c_str());
                
                fprintf ( stdout, "Use --track N to choose track.\n\n" );

                return 0;
              }
              
             else if( !strcmp(argv[i], "--list-karts") )
              {
                initializeLoader ();
                kart_manager.loadKartData () ;

                fprintf ( stdout, "  Available karts:\n" );
                for (unsigned int i = 0; i != kart_manager.karts.size(); i++)
                  fprintf ( stdout, "\t%10s: %s\n", 
                            kart_manager.karts[i]->ident.c_str(),
                            kart_manager.karts[i]->name.c_str() );
                
                fprintf ( stdout, "\n" );

                return 0;
              }

            else if ( !strcmp(argv[i], "--no-start-screen") or !strcmp(argv[i], "-N") )
              {
                noStartScreen = true;
              }

            else if ( !strcmp(argv[i], "--reverse") )
              {
                fprintf ( stdout, "Enabling reverse mode.\n" ) ;
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

            else if ( !strcmp(argv[i], "--laps") and argc > 2 )
              {
                fprintf ( stdout, "You choose to have %d laps.\n", atoi(argv[i+1]) ) ;
                RaceManager::instance()->setNumLaps(atoi(argv[i+1]));
              }
            /* FIXME:
            else if ( !strcmp(argv[i], "--players") and argc > 2 )
              {
                raceSetup.numPlayers = atoi(argv[i+1]);

                if ( raceSetup.numPlayers < 0 or raceSetup.numPlayers > 4)
                  {
                    fprintf ( stderr,
                              "You choose an invalid number of players: %d.\n",
                              raceSetup.numPlayers );
                    cmdLineHelp(argv[0]);
                    return 0;
                  }

                fprintf ( stdout, "You choose to have %d players.\n", atoi(argv[i+1]) ) ;
              }
            */
            else if ( !strcmp(argv[i], "--fullscreen") or !strcmp(argv[i], "-f"))
              {
               config.fullscreen = true;
              }

            else if ( !strcmp(argv[i], "--windowed") or !strcmp(argv[i], "-w"))
              {
                config.fullscreen = false;
              }

            else if ( !strcmp(argv[i], "--screensize") or !strcmp(argv[i], "-s") )
              {
                if (sscanf(argv[i+1], "%dx%d", &config.width, &config.height) == 2)
                  fprintf ( stdout, "You choose to be in %dx%d.\n", config.width, config.height );
                else
                  {
                    fprintf ( stderr, "Error: --screensize argument must be given as WIDTHxHEIGHT\n");
                    exit(EXIT_FAILURE);
                  }
              }
  #ifdef VERSION
            else if( !strcmp(argv[i], "--version") or  !strcmp(argv[i], "-v") )
              {
                fprintf ( stdout, "Tuxkart %s\n", VERSION ) ;
                return 0;
              }
  #endif

            else
              {
                fprintf ( stderr, "Invalid parameter: %s.\n\n", argv[i] );
                cmdLineHelp(argv[0]);
                return 0;
              }
          }
      }

    // now we can really start the game
    initTuxKart ( config.width, config.height, config.fullscreen );

    /* Set the SSG loader options */
    loader -> setCreateStateCallback  ( getAppState ) ;
    loader -> setCreateBranchCallback ( process_userdata ) ;

    /* Set Window's title */
    char title[256];
    sprintf(title, "TuxKart %s", VERSION);
    SDL_WM_SetCaption(title, title);

    screenManager = new ScreenManager();

    if ( noStartScreen )
      {
        RaceManager::instance()->start();
      }
    else
      {
        startScreen = new StartScreen();
        screenManager->setScreen(startScreen);      
      }

    screenManager->run();
    delete startScreen;
    delete screenManager;

    deinitTuxKart();

    // shutdown SDL
    SDL_Quit();
#ifndef DEBUG
  } catch (const TuxkartError& err) {
    std::cout << "TuxkartError: " << err.what() << std::endl;
  } catch (const std::exception& err) {
    std::cout << "Error: " << err.what() << std::endl;
  }
#endif

  return 0 ;
}

