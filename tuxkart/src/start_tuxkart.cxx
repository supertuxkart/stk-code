//  $Id: start_tuxkart.cxx,v 1.39 2004/08/05 14:35:42 grumbel Exp $
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
#include <plib/puSDL.h>

#include "tuxkart.h"

#include "oldgui.h"
#include "WidgetSet.h"
#include "sound.h"
#include "Loader.h"


/***********************************\
*                                   *
* These are the PUI widget pointers *
*                                   *
\***********************************/

#define MAX_TRACKS 100

static puSlider       *numLapsSlider    ;
static puButton       *numLapsText      ;
static puButton       *playButton       ;
static puButton       *exitButton       ;
#ifdef SSG_BACKFACE_COLLISIONS_SUPPORTED
static puButton       *mirrorButton     ;
#endif
static puButton       *reverseButton    ;
static puButtonBox    *trackButtons     ;
static puButtonBox    *playerButtons    ;
static puButton       *pleaseWaitButton ;
static puFont         *sorority         ;
static fntTexFont     *fnt              ;
static ssgSimpleState *introMaterial    ;
static char           *trackNames    [ MAX_TRACKS ] ;
static char           *playerOptions [      4     ] ;
static char           *trackIdents   [ MAX_TRACKS ] ;
static char            numLapsLegend [     100    ] ;
static int             numLaps        =  5 ;
static int             startupCounter = -1 ;


static void switchToGame ()
{
  /* Collect the selected track and number of laps */

  int t ;
  int nl, np  ;
  int mirror  ;
  int reverse ;

  trackButtons -> getValue ( & t ) ;
  nl = atoi ( numLapsText->getLegend () ) ;
  playerButtons -> getValue ( & np ) ;

  np = 1 << np ;

#ifdef SSG_BACKFACE_COLLISIONS_SUPPORTED
  mirrorButton -> getValue ( & mirror ) ;
#else
  mirror = 0 ;
#endif

  reverseButton -> getValue ( & reverse ) ;

  /* Get rid of all the GUI widgets */

  puDeleteObject ( pleaseWaitButton ) ;
  puDeleteObject ( numLapsSlider  ) ;
  puDeleteObject ( numLapsText    ) ;
#ifdef SSG_BACKFACE_COLLISIONS_SUPPORTED
  puDeleteObject ( mirrorButton   ) ;
#endif
  puDeleteObject ( reverseButton  ) ;
  puDeleteObject ( playButton     ) ;
  puDeleteObject ( exitButton     ) ;
  puDeleteObject ( trackButtons   ) ;
  puDeleteObject ( playerButtons  ) ;
  delete introMaterial ;
  delete sorority  ;
  delete fnt       ;

  /* Start the main program */

  tuxkartMain ( nl, mirror, reverse, trackIdents[t], np, 4 ) ;
}

/***********************************\
*                                   *
* This function redisplays the PUI, *
* and flips the double buffer.      *
*                                   *
\***********************************/

static void splashMainLoop (void)
{
  while ( startupCounter != 0 )
  {
    /* Setup for boring 2D rendering */

    glMatrixMode   ( GL_PROJECTION ) ;
    glLoadIdentity () ;
    glMatrixMode   ( GL_MODELVIEW ) ;
    glLoadIdentity () ;
    glDisable      ( GL_DEPTH_TEST ) ;
    glDisable      ( GL_LIGHTING   ) ;
    glDisable      ( GL_FOG        ) ;
    glDisable      ( GL_CULL_FACE  ) ;
    glDisable      ( GL_ALPHA_TEST ) ;
    glOrtho        ( 0, 640, 0, 480, 0, 100 ) ;

    /* Draw the splash screen */

    introMaterial -> force () ;

    glBegin ( GL_QUADS ) ;
    glColor3f    ( 1, 1, 1 ) ;
    glTexCoord2f ( 0, 0 ) ; glVertex2i (   0,   0 ) ;
    glTexCoord2f ( 1, 0 ) ; glVertex2i ( 640,   0 ) ;
    glTexCoord2f ( 1, 1 ) ; glVertex2i ( 640, 480 ) ;
    glTexCoord2f ( 0, 1 ) ; glVertex2i (   0, 480 ) ;
    glEnd () ;

    /* Make PUI redraw */

    glEnable ( GL_BLEND ) ;
    puDisplay () ;
  
    /* Swapbuffers - and off we go again... */

    pollEvents() ;
    updateGUI();
    swapBuffers();
    
    if ( startupCounter > 0 ) startupCounter-- ;
  }
}


/***********************************\
*                                   *
* Here are the PUI widget callback  *
* functions.                        *
*                                   *
\***********************************/


static void playCB ( puObject * )
{
  puSetDefaultColourScheme ( 123.0f/255.0f, 0.0f/255.0f, 34.0f/255.0f, 1.0) ;
  pleaseWaitButton = new puButton ( 100, 240,
                               "LOADING: PLEASE WAIT FOR A MINUTE OR TWO"  ) ;

  /*
    Set up a few frames of delay to ensure the above message gets onto
    the front-buffer and onto the screen before we flip out to the game.
  */

  startupCounter = 3 ;
}


static void exitCB ( puObject * )
{
  fprintf ( stderr, "Exiting TuxKart starter program.\n" ) ;
  shutdown();
}


static void numLapsSliderCB ( puObject *)
{
  float d ;

  numLapsSlider->getValue ( & d ) ;

  numLaps = 1 + (int)( d / 0.05f ) ;

  if ( numLaps <  1 ) numLaps =  1 ;
  if ( numLaps > 20 ) numLaps = 20 ;

  sprintf ( numLapsLegend, "%2d", numLaps ) ;
  numLapsText->setLegend ( numLapsLegend ) ;
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

  return std::string(buf);  
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
    trackIdents[t] = new char[trackName.size() + 1];
    strcpy(trackIdents[t], trackName.c_str());

    std::string description = loadTrackDescription(trackName);
    trackNames[t] = new char[description.size() + 1];
    strcpy(trackNames[t], description.c_str());

    ++t;
    if(t >= MAX_TRACKS-1)
      break;
  }
  trackNames[t] = 0;
  trackIdents[t] = 0;
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

  puInit  () ;
  ssgInit () ;
  ssgSetCurrentOptions(loader);
  registerImageLoaders();

  fnt = new fntTexFont ;
  fnt -> load ( loader->getPath("fonts/sorority.txf").c_str()) ;
  sorority = new puFont ( fnt, 12 ) ;

  puSetDefaultFonts        ( *sorority, *sorority ) ;
  puSetDefaultStyle        ( PUSTYLE_SMALL_SHADED ) ;
  puSetDefaultColourScheme ( 243.0f/255.0f, 140.0f/255.0f, 34.0f/255.0f, 1.0) ;
  
  sound      = new SoundSystem ;
  widgetSet        = new WidgetSet ;
  oldgui        = new OldGUI ;
}


/* Draw the startScreen */
static void startScreen ( int nbrLaps, int mirror, int reverse,
			  int track, int nbrPlayers )
{
  (void)mirror;

  /* Create all of the GUI elements */
  guiSwitch = GUIS_MAINMENU;

  playButton = new puButton      ( 10, 10, 150, 50  ) ;
  playButton -> setLegend        ( "Start Game"     ) ;
  playButton -> setCallback      ( playCB           ) ;
  playButton -> makeReturnDefault( TRUE             ) ;

  exitButton = new puButton      ( 180, 10, 250, 50 ) ;
  exitButton -> setLegend        ( "Quit"           ) ;
  exitButton -> setCallback      ( exitCB           ) ;

  playerOptions [ 0 ] = "1 Player"  ;
  playerOptions [ 1 ] = "2 Players" ;
  playerOptions [ 2 ] = "4 Players" ;
  playerOptions [ 3 ] = NULL ;

  numLapsSlider = new puSlider   ( 10, 80, 150      ) ;
  numLapsSlider -> setLabelPlace ( PUPLACE_ABOVE    ) ;
  numLapsSlider -> setLabel      ( "How Many Laps?" ) ;
  numLapsSlider -> setDelta      ( 0.05 ) ;
  numLapsSlider -> setCBMode     ( PUSLIDER_ALWAYS  ) ;
  numLapsSlider -> setValue      ( (int) ( nbrLaps - 1) * 0.05f ) ;
  //  numLapsSlider -> setValue      ( 1.0f*0.05f*(5.0f-1.0f) ) ;
  numLapsSlider -> setCallback   ( numLapsSliderCB  ) ;

  numLapsText = new puButton     ( 160, 80, " 5" ) ;
  numLapsText -> setStyle        ( PUSTYLE_BOXED    ) ;
  sprintf ( numLapsLegend, "%2d", nbrLaps ) ;
  numLapsText->setLegend ( numLapsLegend ) ;

  playerButtons = new puButtonBox ( 10, 150, 150, 230, playerOptions, TRUE ) ;
  playerButtons -> setLabel       ( "How Many Players?"   ) ;
  playerButtons -> setLabelPlace  ( PUPLACE_ABOVE    ) ;
  playerButtons -> setValue       ( nbrPlayers/2     ) ; 

  trackButtons = new puButtonBox ( 400, 10, 630, 150, trackNames, TRUE ) ;
  trackButtons -> setLabel       ( "Which Track?"   ) ;
  trackButtons -> setLabelPlace  ( PUPLACE_ABOVE    ) ;
  trackButtons -> setValue       ( track            ) ; 

#ifdef SSG_BACKFACE_COLLISIONS_SUPPORTED
  mirrorButton = new puButton    ( 260, 40, "Mirror Track" ) ;
  mirrorButton -> setValue       ( mirror           ) ;
#endif
   
  reverseButton = new puButton    ( 260, 10, "Reverse Track" ) ;
  reverseButton -> setValue       ( reverse         ) ;

  /*
    Load up the splash screen texture,
    loop until user hits the START button,
    then start the game running.
  */

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
	      for (int i = 0; i < MAX_TRACKS; i++)
		{
		  if ( trackNames[i] != '\0' )
		       fprintf ( stdout, "\t%d: %s", i, trackNames[i] );
		}
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
      startScreen ( nbrLaps, mirror, reverse, track, nbrPlayers );
      switchToGame () ; 
    }

  return 0 ;
}
