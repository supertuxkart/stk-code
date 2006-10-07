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

#include "config.hpp"
#include "track_manager.hpp"
#include "track.hpp"
#include "kart_manager.hpp"
#include "kart.hpp"
#include "projectile_manager.hpp"
#include "race_manager.hpp"
#include "loader.hpp"
#include "screen_manager.hpp"
#include "start_screen.hpp"
#include "widget_set.hpp"
#include "material_manager.hpp"
#include "sdldrv.hpp"
#include "hook_manager.hpp"
#include "history.hpp"
#include "herring_manager.hpp"
#include "sound_manager.hpp"
#include "physics_parameters.hpp"

void cmdLineHelp (char* invocation) {
  fprintf ( stdout,
	    "Usage: %s [OPTIONS]\n\n"

	    "Run SuperTuxKart, a racing game with go-kart that features"
	    " the Tux and friends.\n\n"

	    "Options:\n"
	    "  -N,  --no-start-screen  Quick race\n"
	    "  -t,  --track NAME       Start at track NAME (see --list-tracks)\n"
	    "  -l,  --list-tracks      Show available tracks.\n"
	    "  -k,  --numkarts NUM     Number of karts on the racetrack\n"
	    "  --kart NAME             Use kart number NAME\n"
	    "  --list-karts            Show available karts.\n"
	    "  --laps n                Define number of laps to n\n"
//FIXME	    "  --players n             Define number of players to between 1 and 4.\n"
//FIXME	    "  --reverse               Enable reverse mode\n"
//FIXME	    "  --mirror                Enable mirror mode (when supported)\n"
	    "  --herring style         Herring style to use\n"
	    "  -f,  --fullscreen       Fullscreen display.\n"
	    "  -w,  --windowed         Windowed display. (default)\n"
	    "  -s,  --screensize WIDTHxHEIGHT\n"
	    "                          Set the screen size (e.g. 320x200)\n"
	    "  -v,  --version          Show version.\n"
	    // should not be used by unaware users:
            // "  --profile            Enable automatic driven profile mode for 20 seconds\n"
            // "  --profile=n          Enable automatic driven profile mode for n seconds\n"
	    // "  --history            Replay history file 'history.dat'\n"
	    "\n"
	    "You can visit SuperTuxKart's homepage at "
	    "http://supertuxkart.berlios.de\n\n", invocation
	    );
}   // cmdLineHelp

// =============================================================================
int handleCmdLine(int argc, char **argv) {
  int n;
  for(int i=1; i<argc; i++) {
    if(argv[i][0] != '-') continue;
    if ( !strcmp(argv[i], "--help") ||
	 !strcmp(argv[i], "-help" ) ||
	 !strcmp(argv[i], "-h")      ) {
      cmdLineHelp(argv[0]);
      return 0;
    } else if( (!strcmp(argv[i], "--kart") && i+1<argc )) {
      race_manager->setPlayerKart(0, argv[i+1]);
    } else if( (!strcmp(argv[i], "--track") || !strcmp(argv[i], "-t"))
	       && i+1<argc                                              ) {
      race_manager->setTrack(argv[i+1]);
      fprintf ( stdout, "You choose to start in track: %s.\n", argv[i+1] ) ;
    } else if( (!strcmp(argv[i], "--numkarts") || !strcmp(argv[i], "-k")) &&
	       i+1<argc ) {
      race_manager->setNumKarts(config->karts = atoi(argv[i+1]));
      fprintf ( stdout, "You choose to have %s karts.\n", argv[i+1] ) ;
    } else if( !strcmp(argv[i], "--list-tracks") || !strcmp(argv[i], "-l") ) {

      fprintf ( stdout, "  Available tracks:\n" );
      for (size_t i = 0; i != track_manager->getTrackCount(); i++)
	fprintf ( stdout, "\t%10s: %s\n",
		  track_manager->getTrack(i)->getIdent(),
		  track_manager->getTrack(i)->getName());

      fprintf ( stdout, "Use --track N to choose track.\n\n" );
      delete track_manager;
      track_manager = 0;

      return 0;
    } else if( !strcmp(argv[i], "--list-karts") ) {
      kart_manager->loadKartData () ;

      fprintf ( stdout, "  Available karts:\n" );    
      for (unsigned int i = 0; NULL != kart_manager->getKartById(i); i++)
      {
      	const KartProperties* kp= kart_manager->getKartById(i);
		fprintf (stdout, "\t%10s: %s\n", kp->getIdent(), kp->getName());
      }
      fprintf ( stdout, "\n" );

      return 0;
    } else if (    !strcmp(argv[i], "--no-start-screen")
		|| !strcmp(argv[i], "-N")                ) {
      config->noStartScreen = true;
      //FIXME} else if ( !strcmp(argv[i], "--reverse") ) {
      //FIXME:fprintf ( stdout, "Enabling reverse mode.\n" ) ;
      //FIXME:raceSetup.reverse = 1;
    } else if ( !strcmp(argv[i], "--mirror") ) {
  #ifdef SSG_BACKFACE_COLLISIONS_SUPPORTED
      fprintf ( stdout, "Enabling mirror mode.\n" ) ;
      //raceSetup.mirror = 1;
  #else
      //raceSetup.mirror = 0 ;
  #endif
    } else if ( !strcmp(argv[i], "--laps") && i+1<argc ) {
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
    else if ( !strcmp(argv[i], "--fullscreen") || !strcmp(argv[i], "-f")) {
      config->fullscreen = true;
    } else if ( !strcmp(argv[i], "--windowed") || !strcmp(argv[i], "-w")) {
      config->fullscreen = false;
    } 
#endif
    else if ( !strcmp(argv[i], "--screensize") || !strcmp(argv[i], "-s") ) {
      if (sscanf(argv[i+1], "%dx%d", &config->width, &config->height) == 2)
	fprintf ( stdout, "You choose to be in %dx%d.\n", config->width,
		  config->height );
      else {
	fprintf ( stderr, "Error: --screensize argument must be given as WIDTHxHEIGHT\n");
	exit(EXIT_FAILURE);
      }
    }
  #ifdef VERSION
    else if( !strcmp(argv[i], "--version") ||  !strcmp(argv[i], "-v") ) {
      fprintf ( stdout, "SuperTuxkart %s\n", VERSION ) ;
      return 0;
    }
  #endif
    else if( sscanf(argv[i], "--profile=%d",  &n)==1) {
      config->profile=n;
    } else if( !strcmp(argv[i], "--profile") ) {
      config->profile=20;
    } else if( !strcmp(argv[i], "--history") ) {
      config->replayHistory=true;
    } else if( !strcmp(argv[i], "--herring") && i+1<argc ) {
      herring_manager->setUserFilename(argv[i+1]);
    } else {
      fprintf ( stderr, "Invalid parameter: %s.\n\n", argv[i] );
      cmdLineHelp(argv[0]);
      return 0;
    }
  }   // for i <argc
  if(config->profile) {
    printf("Profiling: %d seconds.\n",config->profile);
    config->sfx   = false;  // Disable sound effects and music when profiling
    config->music = false;
  }
  
  return 1;
}   /* handleCmdLine */

// =============================================================================
void InitTuxkart() {
  loader = new Loader();
  loader->setModelDir("models");
  loader->setTextureDir("images");
  loader->setCreateStateCallback(getAppState);
  loader->setCreateBranchCallback(process_userdata);
  config = new Config();
  sound_manager  = new SoundManager();

  // The order here can be important, e.g. KartManager needs 
  // defaultKartProperties.
  history               = new History           ();
  material_manager      = new MaterialManager   ();
  track_manager         = new TrackManager      ();
  physicsParameters     = new PhysicsParameters ();
  const std::string physicsDefault=std::string("data")+DIR_SEPARATOR+ "physics.data";
  physicsParameters->load(physicsDefault);
  kart_manager          = new KartManager       ();
  projectile_manager    = new ProjectileManager ();
  collectable_manager   = new CollectableManager();
  race_manager          = new RaceManager       ();
  screen_manager        = new ScreenManager     ();
  hook_manager          = new HookManager       ();
  herring_manager       = new HerringManager    ();
  track_manager   ->loadTrackList () ;
}

// =============================================================================
int main ( int argc, char **argv ) {
  InitTuxkart();
  //handleCmdLine() needs InitTuxkart() so it can't be called first
  if(!handleCmdLine(argc, argv)) exit(0);

  drv_init();
  // loadMaterials needs ssgLoadTextures (internally), which can
  // only be called after ssgInit (since this adds the actual loader)
  // so this next call can't be in InitTuxkart. And InitPlib needs
  // config, which gets defined in InitTuxkart, so swapping those two
  // calls is not possible either ... so loadMaterial has to be done here :(
  material_manager   ->loadMaterial   ();
  kart_manager       ->loadKartData   ();
  projectile_manager ->loadData       ();
  collectable_manager->loadCollectable();
  herring_manager    ->loadDefaultHerrings();
  startScreen = new StartScreen();
  widgetSet   = new WidgetSet;

  // Replay a race
  // =============
  if(config->replayHistory) {
    // This will setup the race manager etc.
    history->Load();
    startScreen->switchToGame();
    screen_manager->run();
  } else {
    if(!config->profile) {
      if(!config->noStartScreen) {

  // Normal multi window start
  // =========================
	screen_manager->setScreen(startScreen);
      } else {

  // Quickstart (-N)
  // ===============
	race_manager->setNumPlayers(1);
	race_manager->setNumKarts  (4);
	race_manager->setRaceMode  (RaceSetup::RM_QUICK_RACE);
	race_manager->setDifficulty(RD_MEDIUM);
	race_manager->setPlayerKart(0, kart_manager->getKart("tuxkart")->getIdent());
	race_manager->setNumLaps   (3);
	//race_manager->setTrack     ("tuxtrack");
	//race_manager->setTrack     ("sandtrack");
	race_manager->setTrack     ("race");
	startScreen->switchToGame();
      }
    } else {

  // Profiling
  // =========
      race_manager->setNumPlayers(1);
      race_manager->setPlayerKart(0, kart_manager->getKart("tuxkart")->getIdent());
      race_manager->setRaceMode  (RaceSetup::RM_QUICK_RACE);
      race_manager->setDifficulty(RD_MEDIUM);
      race_manager->setNumLaps   (999999); // profile end depends on time
      startScreen->switchToGame  ();
    }
    screen_manager->run();
  }
  config->saveConfig();
  
  drv_deinit();
return 0 ;
}


