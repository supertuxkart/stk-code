
#include "tuxkart.h"

/***********************************\
*                                   *
* These are the PUI widget pointers *
*                                   *
\***********************************/

#define MAX_TRACKS 10


static puSlider       *numLapsSlider    ;
static puButton       *numLapsText      ;
static puButton       *playButton       ;
static puButton       *exitButton       ;
static puButton       *mirrorButton     ;
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
static char           *datadir        =  0 ;
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

  tuxkartMain ( nl, mirror, reverse, trackIdents[t], np ) ;
}

/*********************************\
*                                 *
* These functions capture mouse   *
* and keystrokes and pass them on *
* to PUI.                         *
*                                 *
\*********************************/

static void startupKeyFn ( int key, int updown, int, int )
{
  puKeyboard ( key, updown ) ;
}

static void startupMotionFn ( int x, int y )
{
  puMouse ( x, y ) ;
}

static void startupMouseFn ( int button, int updown, int x, int y )
{
  puMouse ( button, updown, x, y ) ;
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

    pwSwapBuffers   () ;

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
  exit ( 1 ) ;
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
  introMaterial -> setTexture  ( "images/title_screen.rgb", TRUE, TRUE ) ;
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


static void loadTrackList ()
{
  /* Load up a list of tracks - and their names */

  char *fname = "data/levels.dat" ;

  if ( getenv ( "TUXKART_TRACKLIST" ) != NULL )
    fname = getenv ( "TUXKART_TRACKLIST" ) ;

  FILE *fd = fopen ( fname, "ra" ) ;

  if ( fd == NULL )
  {
    fprintf ( stderr, "tuxkart: Can't open '%s'\n", fname ) ;
    exit ( 1 ) ;
  }

  int i = 0 ;

  while ( i < MAX_TRACKS && ! feof ( fd ) )
  {
    char *p ;
    char  s [ 1024 ] ;

    if ( fgets ( s, 1023, fd ) == NULL )
      break ;
 
    if ( *s == '#' )
      continue ;

    for ( p = s ; *p > ' ' && *p != '\0' ; p++ )
      /* Search for a space */ ;

    if ( *p == ' ' )
    {
      *p = '\0' ;
      trackIdents [ i ] = new char [ strlen(s)+1 ] ;
      strcpy ( trackIdents [ i ], s ) ;
      p++ ;

      while ( *p <= ' ' && *p != '\0' )
        p++ ; 

      trackNames [ i ] = new char [ strlen(p)+1 ] ;
      strcpy ( trackNames [ i ], p ) ;

      i++ ;
    }
  }

  trackNames  [ i ] = NULL ;
  trackIdents [ i ] = NULL ;

  fclose ( fd ) ;
}


/* Initialize the datadir */
static void loadDataDir (int debug)
{
  /* Set to the correct directory */
 
  if ( datadir == NULL )
  {
    if ( getenv ( "TUXKART_DATADIR" ) != NULL )
      datadir = getenv ( "TUXKART_DATADIR" ) ;
    else
    if ( canAccess ( "data/levels.dat" ) )
      datadir = "." ;
    else
    if ( canAccess ( "../data/levels.dat" ) )
      datadir = ".." ;
    else
      datadir = TUXKART_DATADIR ;
  }
 
  if ( debug )
    fprintf ( stderr, "Data files will be fetched from: '%s'\n", datadir ) ;
 
  if ( chDir ( datadir ) )
  {
    fprintf ( stderr, "Couldn't chdir() to '%s'.\n", datadir ) ;
    exit ( 1 ) ;
  }
}


/* Load the datadir, tracklist and plib stuff */
static void initTuxKart (int noBorder)
{
  loadDataDir ( TRUE );
  loadTrackList () ;

  /* Initialise a bunch of PLIB library stuff */

  pwInit  ( 0, 0, getScreenWidth(), getScreenHeight(), FALSE, 
	    "Tux Kart by Steve Baker", !noBorder, 0 ) ;

  pwSetCallbacks ( startupKeyFn, startupMouseFn, startupMotionFn, NULL, NULL ) ;
  puInit  () ;
  ssgInit () ;

  fnt = new fntTexFont ;
  fnt -> load ( "fonts/sorority.txf" ) ;
  sorority = new puFont ( fnt, 12 ) ;

  puSetDefaultFonts        ( *sorority, *sorority ) ;
  puSetDefaultStyle        ( PUSTYLE_SMALL_SHADED ) ;
  puSetDefaultColourScheme ( 243.0f/255.0f, 140.0f/255.0f, 34.0f/255.0f, 1.0) ;
}


/* Draw the startScreen */
static void startScreen ( int nbrLaps, int mirror, int reverse,
			  int track, int nbrPlayers )
{
  /* Create all of the GUI elements */

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


void cmdLineHelp ()
{
  fprintf ( stdout, 
	    "Usage: tuxkart [options...]\n\n"

	    "Run TuxKart, a racing game with go-kart that features"
	    " the well-known linux\nmascott Tux. The game is heavily"
	    " inspired by Super-Mario-Kart and Wacky Wheels.\n\n"

	    "Options:\n"
	    "--no-start-screen" "\t\tQuick race.\n"
	    "--track n"         "\t\t\tStart at track number n (see --list-tracks).\n"
	    "--list-tracks"     "\t\t\tShow available tracks.\n"
	    "--laps n"          "\t\t\tDefine number of laps to n.\n"
	    "--players n"       "\t\t\tDefine number of players to either 1, 2 or 4.\n"
	    "--reverse"        "\t\t\tEnable reverse mode.\n"
	    "--mirror"         "\t\t\tEnable mirror mode (when supported).\n"
	    "--fullscreen"     "\t\t\tFullscreen display.\n"
	    "--no-borders"     "\t\t\tDisable window borders/decorations.\n"
	    "--screensize WIDTH HEIGHT" "\tSet the screen size (e.g. 320 200).\n"
	    "--version"        "\t\t\tShow version.\n"
	    "\n"
	    "You can visit TuxKart's homepage at "
	    "http://tuxkart.sourceforge.net\n\n"
	    );
}


int main ( int argc, char *argv[] )
{
 
  /* Default values */
  int nbrLaps = 3;
  int mirror = 0, reverse = 0;
  int track = 0;
  int nbrPlayers = 1;
  int width = 800;
  int height = 600;
  int noBorder = FALSE;
  int noStartScreen = FALSE;
  
  /* Testing if we've given arguments */
  if ( argc > 1) 
    {
      for(int i = 1; i<argc; i++)
	{
	  if ( argv[i][0] != '-') continue;

	  if ( !strcmp(argv[i], "--help") or
	       !strcmp(argv[i], "-help") or
	       !strcmp(argv[i], "-h") )
	    {
	      cmdLineHelp();
	      return 0;
	    }

	  else if( !strcmp(argv[i], "--track") and argc > 2 )
	    {
	      track = atoi(argv[i+1]);

	      if ( track < 0 )
		{
		  fprintf ( stderr,
			    "You choose an invalid track number: %d.\n", track );
		  cmdLineHelp();
		  return 0;
		}

	      fprintf ( stdout, "You choose to start in track: %s.\n", argv[i+1] ) ;
	    }

	  else if( !strcmp(argv[i], "--list-tracks") )
	    {
	      loadDataDir ( FALSE );
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
	      noStartScreen = TRUE;
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
		  cmdLineHelp();
		  return 0;
		}

	      fprintf ( stdout, "You choose to have %d players.\n", atoi(argv[i+1]) ) ;
	    }

	  else if ( !strcmp(argv[i], "--fullscreen") )
	    {
	      width = -1;
	      height = -1;
	      noBorder = TRUE;
	      // Needs some thinking, borders can't be switched on and off.
	      noStartScreen = TRUE;
	    }

	  else if ( !strcmp(argv[i], "--screensize") and argc > 3 )
	    {
	      width = ( atoi(argv[i+1]) > 0 ) ? atoi(argv[i+1]) : width;
	      height = ( atoi(argv[i+2]) > 0 ) ? atoi(argv[i+2]) : height;

	      fprintf ( stdout, "You choose to be in %dx%d.\n",
			atoi(argv[i+1]), atoi(argv[i+2]) ) ;
	    }

	  else if ( !strcmp(argv[i], "--no-borders") )
	    {
	      fprintf ( stdout, "Disabling window borders.\n" ) ;
	      noBorder = TRUE;
	    }

	  else if( !strcmp(argv[i], "--version") )
	    {
	      fprintf ( stdout, "Tuxkart %s\n", VERSION ) ;
	      return 0;
	    }

	  else
	    {
	      fprintf ( stderr, "Invalid parameter: %s.\n\n", argv[i] );
	      cmdLineHelp();
	      return 0;
	    }
	}
    }


  if ( noStartScreen == TRUE )
    {
      /* Set screen size */
      reshape( width, height );
      /* Load plib stuff */
      initTuxKart ( noBorder );

      tuxkartMain ( nbrLaps, mirror, reverse, trackIdents[track], nbrPlayers ) ;
    }
  else
    {
      /* Load plib stuff */
      initTuxKart ( noBorder );

      /* Show start screen */
      startScreen ( nbrLaps, mirror, reverse, track, nbrPlayers );

      /* Set screen size */
      reshape( width, height );
      pwSetSize( getScreenWidth(), getScreenHeight() );

      switchToGame () ; 
    }

  return 0 ;
}
