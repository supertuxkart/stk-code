//  $Id: start_tuxkart.cxx,v 1.43 2004/08/06 00:39:44 jamesgregory Exp $
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

#include <plib/pw.h>

#include "tuxkart.h"

#include "WidgetSet.h"
#include "sound.h"
#include "Loader.h"

#include <vector>
#include <string>


/***********************************\
*                                   *
* These are the PUI widget pointers *
*                                   *
\***********************************/


static ssgSimpleState *introMaterial    ;
std::vector<std::string> trackIdents ;
std::vector<std::string> trackNames ;


void switchToGame (int numLaps, int mirror, int reverse, int track, int nPlayers)
{
  delete introMaterial ;

  tuxkartMain ( numLaps, mirror, reverse, trackIdents[track], nPlayers, 4 ) ;
}

/***********************************\
*                                   *
* This function redisplays the PUI, *
* and flips the double buffer.      *
*                                   *
\***********************************/

static void splashMainLoop (void)
{
  while ( 1 )
  {
   /* 
   //The old splash screen stuff
   
   //Setup for boring 2D rendering
   */
    
    glMatrixMode   ( GL_PROJECTION ) ;
    glLoadIdentity () ;
    glMatrixMode   ( GL_MODELVIEW ) ;
    glLoadIdentity () ;
    glDisable      ( GL_DEPTH_TEST ) ;
    glDisable      ( GL_LIGHTING   ) ;
    glDisable      ( GL_FOG        ) ;
    glDisable      ( GL_CULL_FACE  ) ;
    glDisable      ( GL_ALPHA_TEST ) ;
    //glOrtho        ( 0, 640, 0, 480, 0, 100 ) ;

    //Draw the splash screen

    introMaterial -> force () ;

    glBegin ( GL_QUADS ) ;
    glColor3f    ( 1, 1, 1 ) ;
    glTexCoord2f ( 0, 0 ) ; glVertex2i (   -1,   -1 ) ;
    glTexCoord2f ( 1, 0 ) ; glVertex2i (   1,   -1 ) ;
    glTexCoord2f ( 1, 1 ) ; glVertex2i (   1,   1 ) ;
    glTexCoord2f ( 0, 1 ) ; glVertex2i (   -1,   1 ) ;
    glEnd () ;

    /*
    //Make PUI redraw
    glEnable ( GL_BLEND ) ;
    puDisplay () ;
    */
    
    //glClearColor(0.2, 0.2, 0.2, 0.0);
    //glClear(GL_COLOR_BUFFER_BIT);
    glFlush();
	
    /* Swapbuffers - and off we go again... */

    pollEvents() ;
    updateGUI();
    swapBuffers();
  }
}


static void installMaterial ()
{
  /* Make a simplestate for the title screen texture */

  introMaterial = new ssgSimpleState ;
  introMaterial -> setTexture( 
          loader->createTexture("title_screen.png", true, true, false));
  introMaterial -> enable      ( GL_TEXTURE_2D ) ;
  introMaterial -> disable     ( GL_LIGHTING  ) ;
  introMaterial -> disable     ( GL_CULL_FACE ) ;
  introMaterial -> setOpaque   () ;
  introMaterial -> disable     ( GL_BLEND ) ;
  introMaterial -> setShadeModel ( GL_SMOOTH ) ;
  introMaterial -> disable     ( GL_COLOR_MATERIAL ) ;
  introMaterial -> enable      ( GL_CULL_FACE      ) ;
  introMaterial -> setMaterial ( GL_EMISSION, 0, 0, 0, 1 ) ;
  introMaterial -> setMaterial ( GL_SPECULAR, 0, 0, 0, 1 ) ;
  introMaterial -> setMaterial ( GL_DIFFUSE, 0, 0, 0, 1 ) ;
  introMaterial -> setMaterial ( GL_AMBIENT, 0, 0, 0, 1 ) ;
  introMaterial -> setShininess ( 0 ) ;
}

static std::string loadTrackDescription(const std::string& mapfile)
{
  std::string path = loader->getPath(std::string("data/") + mapfile + ".track");
  FILE* file = fopen(path.c_str(), "r");
  if(file == 0)
    return mapfile;

  char buf[1024];
  if(fgets(buf, 1024, file) == 0)
    buf[0] = 0;
  
  fclose(file);
	
  string ret =  buf;
  ret = ret.substr(0, ret.find('\n'));
  return ret;
}

static void loadTrackList ()
{
  /* Load up a list of tracks - and their names */
  std::set<std::string> files;
  loader->listFiles(files, "data");
  int t = 0;
  for(std::set<std::string>::iterator i = files.begin();
          i != files.end(); ++i) {
    if(i->size() < 6)
      continue;
    if(i->compare(i->size() - 6, 6, ".track") != 0)
      continue;
   
    std::string trackName = i->substr(0, i->size()-6);
    trackIdents.push_back(trackName);

    std::string description = loadTrackDescription(trackName);
    trackNames.push_back(description);

    ++t;
    if(t >= MAX_TRACKS-1)
      break;
  }
}

/* Initialize the datadir */
static void loadDataDir()
{
  loader = new Loader();
  loader->setModelDir("models");
  loader->setTextureDir("images");
    
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
}

/* Load the datadir, tracklist and plib stuff */
static void initTuxKart (int width, int height, int videoFlags)
{
  loadDataDir ();
  loadTrackList () ;

  initVideo ( width, height, videoFlags );
  
  /* Initialise a bunch of PLIB library stuff */

  ssgInit () ;
  ssgSetCurrentOptions(loader);
  registerImageLoaders();
  
  sound      = new SoundSystem ;
  widgetSet        = new WidgetSet ;
}


/* Draw the startScreen */
static void startScreen ( )
{
  guiSwitch = GUIS_MAINMENU;

  installMaterial () ;
  splashMainLoop  () ;
}


void cmdLineHelp (char* invocation)
{
  fprintf ( stdout, 
	    "Usage: %s [OPTIONS]\n\n"

	    "Run TuxKart, a racing game with go-kart that features"
	    " the well-known linux\nmascott Tux. The game is heavily"
	    " inspired by Super-Mario-Kart and Wacky Wheels.\n\n"

	    "Options:\n"
	    "  --no-start-screen  Quick race\n"
	    "  --track n          Start at track number n (see --list-tracks)\n"
            "  --numkarts NUM     Number of karts on the racetrack\n"
	    "  --list-tracks      Show available tracks.\n"
	    "  --laps n           Define number of laps to n\n"
	    "  --players n        Define number of players to either 1, 2 or 4.\n"
	    "  --reverse          Enable reverse mode\n"
	    "  --mirror           Enable mirror mode (when supported)\n"
	    "  --fullscreen       Fullscreen display.\n"
	    "  --screensize WIDTHxHEIGHT\n"
            "                     Set the screen size (e.g. 320x200)\n"
	    "  --version          Show version.\n"
	    "\n"
	    "You can visit TuxKart's homepage at "
	    "http://tuxkart.sourceforge.net\n\n", invocation
	    );
}


int main ( int argc, char *argv[] )
{
  /* Default values */
  int nbrLaps = 3;
  int mirror = 0;
  int reverse = 0;
  int track         = 0;
  int nbrPlayers    = 1;
  int  width  = 800;
  int  height = 600;
  int numKarts = 4;
  bool fullscreen   = false;
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

	  else if( !strcmp(argv[i], "--track") and argc > 2 )
	    {
	      track = atoi(argv[i+1]);

	      if ( track < 0 )
		{
		  fprintf ( stderr,
			    "You choose an invalid track number: %d.\n", track );
		  cmdLineHelp(argv[0]);
		  return 0;
		}

	      fprintf ( stdout, "You choose to start in track: %s.\n", argv[i+1] ) ;
	    }

	  else if( !strcmp(argv[i], "--numkarts") && argc > 2)
            {
	      if (sscanf(argv[i+1], "%d", &numKarts) == 1)
                {
                  if (numKarts < 0 || numKarts > 4)
                    puts("Warning: numkarts must be between 1 and 4");
                }
              else
                {
                  puts("Error: Argument to numkarts must be a number");
                  exit(EXIT_FAILURE);
                }
            }

	  else if( !strcmp(argv[i], "--list-tracks") )
	    {
	      loadDataDir ();
	      loadTrackList () ;

	      fprintf ( stdout, "  Available tracks:\n" );
	      for (uint i = 0; i != trackNames.size(); i++)
		       fprintf ( stdout, "\t%d: %s", i, trackNames[i].c_str() );
	      
		fprintf ( stdout, "\n" );

	      return 0;
	    }

	  else if ( !strcmp(argv[i], "--no-start-screen") )
	    {
	      noStartScreen = true;
	    }

	  else if ( !strcmp(argv[i], "--reverse") )
	    {
	      fprintf ( stdout, "Enabling reverse mode.\n" ) ;
	      reverse = 1;
	    }

	  else if ( !strcmp(argv[i], "--mirror") )
	    {
#ifdef SSG_BACKFACE_COLLISIONS_SUPPORTED
	      fprintf ( stdout, "Enabling mirror mode.\n" ) ;
	      mirror = 1;
#else
	      mirror = 0 ;
#endif
	    }

	  else if ( !strcmp(argv[i], "--laps") and argc > 2 )
	    {
	      fprintf ( stdout, "You choose to have %d laps.\n", atoi(argv[i+1]) ) ;
	      nbrLaps = atoi(argv[i+1]);
	    }

	  else if ( !strcmp(argv[i], "--players") and argc > 2 )
	    {
	      nbrPlayers = atoi(argv[i+1]);

	      if ( nbrPlayers < 0 or nbrPlayers > 4)
		{
		  fprintf ( stderr,
			    "You choose an invalid number of players: %d.\n",
			    nbrPlayers );
		  cmdLineHelp(argv[0]);
		  return 0;
		}

	      fprintf ( stdout, "You choose to have %d players.\n", atoi(argv[i+1]) ) ;
	    }

	  else if ( !strcmp(argv[i], "--fullscreen") )
	    {
              fullscreen = true;
	    }

	  else if ( !strcmp(argv[i], "--screensize") )
	    {
	      if (sscanf(argv[i+1], "%dx%d", &width, &height) == 2)
                fprintf ( stdout, "You choose to be in %dx%d.\n", width, height );
              else
                {
                  fprintf ( stderr, "Error: --screensize argument must be given as WIDTHxHEIGHT\n");
                  exit(EXIT_FAILURE);
                }
	    }
	  #ifdef VERSION
	  else if( !strcmp(argv[i], "--version") )
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

  initTuxKart ( width,  height, fullscreen );

  if ( noStartScreen )
    {
      tuxkartMain ( nbrLaps, mirror, reverse, trackIdents[track], nbrPlayers, numKarts ) ;
    }
  else
    {
      /* Show start screen */
      startScreen ();
    }

  return 0 ;
}
