
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

/* Initialize the datadir, tracklist and plib stuff */
static void initTuxKart (int noBorder)
{
  /* Set tux_aqfh_datadir to the correct directory */
 
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
 
  fprintf ( stderr, "Data files will be fetched from: '%s'\n", datadir ) ;
 
  if ( chDir ( datadir ) )
  {
    fprintf ( stderr, "Couldn't chdir() to '%s'.\n", datadir ) ;
    exit ( 1 ) ;
  }

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
static void startScreen () 
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
  numLapsSlider -> setValue      ( 1.0f*0.05f*(5.0f-1.0f) ) ;
  numLapsSlider -> setCallback   ( numLapsSliderCB  ) ;

  numLapsText = new puButton     ( 160, 80, " 5"    ) ;
  numLapsText -> setStyle        ( PUSTYLE_BOXED    ) ;

  playerButtons = new puButtonBox ( 10, 150, 150, 230, playerOptions, TRUE ) ;
  playerButtons -> setLabel       ( "How Many Players?"   ) ;
  playerButtons -> setLabelPlace  ( PUPLACE_ABOVE    ) ;
  playerButtons -> setValue       ( 0                ) ; 

  trackButtons = new puButtonBox ( 400, 10, 630, 150, trackNames, TRUE ) ;
  trackButtons -> setLabel       ( "Which Track?"   ) ;
  trackButtons -> setLabelPlace  ( PUPLACE_ABOVE    ) ;
  trackButtons -> setValue       ( 0                ) ; 

#ifdef SSG_BACKFACE_COLLISIONS_SUPPORTED
  mirrorButton = new puButton    ( 260, 40, "Mirror Track" ) ;
  mirrorButton -> setValue       ( 0                ) ;
#endif
   
  reverseButton = new puButton    ( 260, 10, "Reverse Track" ) ;
  reverseButton -> setValue       ( 0                ) ;

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
	    "Usage: tuxkart [--track n] [--nbrLaps n] [--reverse] [--mirror]\n"
	    "\t\t[--fullscreen|--screenMode n [--noBorder]]\n"
	    "\n"
	    "Run TuxKart, a racing game with go-kart that features"
	    " the well-known linux\nmascott Tux. The game is heavily"
	    " inspired by Super-Mario-Kart and Wacky Wheels.\n"
	    "\n"
	    "Options:\n"
	    "--track n\tStart at track number n. First track is 0.\n"
	    "--nbrLaps n\tDefine number of laps to n.\n"
	    "--reverse\tEnable reverse mode.\n"
	    "--mirror\tEnable mirror mode (when supported).\n"
	    "--fullscreen\tFullscreen display (doesn't work with --screenMode).\n"
	    "--noBorder\tDisable window borders/decorations.\n"
	    "--screenMode n\tSet the screen mode to:\n"
	    "\t\t 0: 320x240\t5: 960x720\n"
	    "\t\t 1: 400x300\t6: 1024x768\n"
	    "\t\t 2: 512x384\t7: 1152x864\n"
	    "\t\t 3: 640x480\t8: 1280x1024\n"
	    "\t\t 4: 800x600\n"
	    "--version\tShow version.\n"
	    "\n"
	    "You can visit TuxKart's homepage at "
	    "http://tuxkart.sourceforge.net\n\n"
	    );
}

int main ( int argc, char *argv[] )
{
 
  /* Default values */
  int nl = 3;
  int mirror = 0, reverse = 0;
  int t = 0;
  int np = 1;
  int width = 800;
  int height = 600;
  int noBorder = FALSE;
  
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
	      t = atoi(argv[i+1]);

	      if ( t < 0 )
		{
		  fprintf ( stderr, "You choose an invalid track number: %d.\n", t ) ;
		  cmdLineHelp();
		  return 0;
		}

	      fprintf ( stdout, "You choose to start in track: %s.\n", argv[i+1] ) ;
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

	  else if ( !strcmp(argv[i], "--nbrLaps") and argc > 2 )
	    {
	      fprintf ( stdout, "You choose to have %d laps.\n", atoi(argv[i+1]) ) ;
	      nl = atoi(argv[i+1]);
	    }

	  else if ( !strcmp(argv[i], "--fullscreen") )
	    {
	      width = -1;
	      height = -1;
	      noBorder = TRUE;
	    }

	  else if ( !strcmp(argv[i], "--screenMode") and argc > 2 )
	    {
	      switch (atoi(argv[i+1]))
		{
		case 0:
		  width = 320;
		  height = 240;
		  break;
		case 1:
		  width = 400;
		  height = 300;
		  break;
		case 2:
		  width = 512;
		  height = 384;
		  break;
		case 3:
		  width = 640;
		  height = 480;
		  break;
		case 4:
		  width = 800;
		  height = 600;
		  break;
		case 5:
		  width = 960;
		  height = 720;
		  break;
		case 6:
		  width = 1024;
		  height = 768;
		  break;
		case 7:
		  width = 1152;
		  height = 864;
		  break;
		case 8:
		  width = 1280;
		  height = 1024;
		  break;
		}
	      fprintf ( stdout, "You choose to have screen mode %d.\n", atoi(argv[i+1]) ) ;
	    }

	  else if ( !strcmp(argv[i], "--noBorder") )
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
      /* Set screen resolution */
      reshape( width, height );

      /* Load plib stuff */
      initTuxKart (noBorder);
      /* Start the main program */
      tuxkartMain ( nl, mirror, reverse, trackIdents[t], np ) ;
    }

  /* No command line parameters as been specified */
  else
    {
      /* Load plib stuff */
      initTuxKart (noBorder);

      /* Show start screen */
      startScreen ();

      switchToGame () ; 
      return 0 ;
    }
}
