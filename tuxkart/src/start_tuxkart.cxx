
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


int main ( int argc, char **argv )
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
            "Tux Kart by Steve Baker", TRUE, 0 ) ;

  pwSetCallbacks ( startupKeyFn, startupMouseFn, startupMotionFn, NULL, NULL ) ;
  puInit  () ;
  ssgInit () ;

  fnt = new fntTexFont ;
  fnt -> load ( "fonts/sorority.txf" ) ;
  sorority = new puFont ( fnt, 12 ) ;

  puSetDefaultFonts        ( *sorority, *sorority ) ;
  puSetDefaultStyle        ( PUSTYLE_SMALL_SHADED ) ;
  puSetDefaultColourScheme ( 243.0f/255.0f, 140.0f/255.0f, 34.0f/255.0f, 1.0) ;

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
  switchToGame    () ; 
  return 0 ;
}


