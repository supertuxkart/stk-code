
#include "start_tuxkart.h"

int width=800,height=600,fullscreen=0;
extern int player;

/***********************************\
*                                   *
* These are the PUI widget pointers *
*                                   *
\***********************************/

static int numLaps = 5 ;

static char        numLapsLegend [ 100 ] ;
static char        *datadir = 0 ;

static puSlider    *numLapsSlider ;
static puButton    *numLapsText   ;
static puButton    *playButton    ;
static puButton    *exitButton    ;
static puButtonBox *trackButtons  ;
static fntTexFont  *fnt ;
static puFont      *sorority      ;

static ssgSimpleState *intro_gst ;

#define MAX_TRACKS 10

static puButton    *pleaseWaitButton    ;
static int startup_counter = 0 ;

static char *track_names  [ MAX_TRACKS ] ;
static char *track_idents [ MAX_TRACKS ] ;

extern int tuxkart_main ( int nl, char *track ) ;

static void switch_to_game ()
{
  int t ;
  int nl ;

  trackButtons -> getValue ( & t ) ;
  nl = atoi ( numLapsText->getLegend () ) ;

  puDeleteObject ( pleaseWaitButton ) ;
  puDeleteObject ( numLapsSlider ) ;
  puDeleteObject ( numLapsText   ) ;
  puDeleteObject ( playButton    ) ;
  puDeleteObject ( exitButton    ) ;
  puDeleteObject ( trackButtons  ) ;
  delete intro_gst ;
  delete sorority  ;
  delete fnt       ;

  tuxkart_main ( nl, track_idents[t] ) ;
}

/*********************************\
*                                 *
* These functions capture mouse   *
* and keystrokes and pass them on *
* to PUI.                         *
*                                 *
\*********************************/

static void keyfn ( int key, int updown, int, int )
{
  puKeyboard ( key, updown ) ;
}

static void motionfn ( int x, int y )
{
  puMouse ( x, y ) ;
}

static void mousefn ( int button, int updown, int x, int y )
{
  puMouse ( button, updown, x, y ) ;
}

/***********************************\
*                                   *
* This function redisplays the PUI, *
* and flips the double buffer.      *
*                                   *
\***********************************/

static void displayfn (void)
{
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

  intro_gst -> force () ;

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
  
  /* Off we go again... */

  pwSwapBuffers   () ;

  if ( startup_counter > 0 )
  {
    if ( --startup_counter <= 0 )
      switch_to_game () ;
  }
}


/***********************************\
*                                   *
* Here are the PUI widget callback  *
* functions.                        *
*                                   *
\***********************************/

static void play_cb ( puObject * )
{
  puSetDefaultColourScheme ( 123.0f/255.0f, 0.0f/255.0f, 34.0f/255.0f, 1.0) ;
  pleaseWaitButton = new puButton ( 100, 240,
                               "LOADING: PLEASE WAIT FOR A MINUTE OR TWO"  ) ;

  startup_counter = 3 ;
}


static void exit_cb ( puObject * )
{
  fprintf ( stderr, "Exiting TuxKart starter program.\n" ) ;
  exit ( 1 ) ;
}



static void numLapsSlider_cb ( puObject *)
{
  float d ;

  numLapsSlider->getValue ( & d ) ;

  numLaps = 1 + (int)( d / 0.05f ) ;

  if ( numLaps <  1 ) numLaps =  1 ;
  if ( numLaps > 20 ) numLaps = 20 ;

  sprintf ( numLapsLegend, "%2d", numLaps ) ;
  numLapsText->setLegend ( numLapsLegend ) ;
}

static void install_material ()
{
  intro_gst = new ssgSimpleState ;
 
  if ( getenv ( "MESA_GLX_FX" ) != NULL )
    intro_gst -> setTexture ( "images/title_screen_small.rgb", TRUE, TRUE ) ;
  else
    intro_gst -> setTexture ( "images/title_screen.rgb", TRUE, TRUE ) ;

  intro_gst -> enable      ( GL_TEXTURE_2D ) ;
  intro_gst -> disable     ( GL_LIGHTING  ) ;
  intro_gst -> disable     ( GL_CULL_FACE ) ;
  intro_gst -> setOpaque   () ;
  intro_gst -> disable     ( GL_BLEND ) ;
  intro_gst -> setShadeModel ( GL_SMOOTH ) ;
  intro_gst -> disable     ( GL_COLOR_MATERIAL ) ;
  intro_gst -> enable      ( GL_CULL_FACE      ) ;
  intro_gst -> setMaterial ( GL_EMISSION, 0, 0, 0, 1 ) ;
  intro_gst -> setMaterial ( GL_SPECULAR, 0, 0, 0, 1 ) ;
  intro_gst -> setMaterial ( GL_DIFFUSE, 0, 0, 0, 1 ) ;
  intro_gst -> setMaterial ( GL_AMBIENT, 0, 0, 0, 1 ) ;
  intro_gst -> setShininess ( 0 ) ;
}
                                                                                
static void loadTrackList ()
{
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
      track_idents [ i ] = new char [ strlen(s)+1 ] ;
      strcpy ( track_idents [ i ], s ) ;
      p++ ;

      while ( *p <= ' ' && *p != '\0' )
        p++ ; 

      track_names [ i ] = new char [ strlen(p)+1 ] ;
      strcpy ( track_names [ i ], p ) ;

      i++ ;
    }
  }

  track_names  [ i ] = NULL ;
  track_idents [ i ] = NULL ;

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
#ifdef _MSC_VER
    if ( _access ( "data/levels.dat", 04 ) == 0 )
#else
    if ( access ( "data/levels.dat", F_OK ) == 0 )
#endif
      datadir = "." ;
    else
#ifdef _MSC_VER
    if ( _access ( "../data/levels.dat", 04 ) == 0 )
#else
    if ( access ( "../data/levels.dat", F_OK ) == 0 )
#endif
      datadir = ".." ;
    else
#ifdef TUXKART_DATADIR
      datadir = TUXKART_DATADIR ;
#else
      datadir = "/usr/local/share/games/tuxkart" ;
#endif
  }
 
  fprintf ( stderr, "Data files will be fetched from: '%s'\n",
                                                    datadir ) ;
 
#ifdef _MSC_VER
  if ( _chdir ( datadir ) == -1 )
#else
  if ( chdir ( datadir ) == -1 )
#endif
  {
    fprintf ( stderr, "Couldn't chdir() to '%s'.\n", datadir ) ;
    exit ( 1 ) ;
  }                                                                             

  loadTrackList () ;

  pwInit ( 0, 0, width, height, FALSE, "Tux Kart by Steve Baker", TRUE, 0 ) ;

  pwSetCallbacks ( keyfn, mousefn, motionfn, NULL, NULL ) ;

  puInit () ;
  ssgInit () ;

  fnt = new fntTexFont ;
  fnt -> load ( "fonts/sorority.txf" ) ;
  sorority = new puFont ( fnt, 12 ) ;

  puSetDefaultFonts        ( *sorority, *sorority ) ;
  puSetDefaultStyle        ( PUSTYLE_SMALL_SHADED ) ;
  puSetDefaultColourScheme ( 243.0f/255.0f, 140.0f/255.0f, 34.0f/255.0f, 1.0) ;

  playButton = new puButton     ( 10, 10, 150, 50 ) ;
  playButton->setLegend         ( "Start Game"  ) ;
  playButton->setCallback       ( play_cb ) ;
  playButton->makeReturnDefault ( TRUE ) ;

  exitButton = new puButton     ( 180, 10, 250, 50 ) ;
  exitButton->setLegend         ( "Quit"  ) ;
  exitButton->setCallback       ( exit_cb ) ;
   
  numLapsSlider = new puSlider  ( 10, 80, 150 ) ;
  numLapsSlider->setLabelPlace ( PUPLACE_ABOVE ) ;
  numLapsSlider->setLabel  ( "How Many Laps?" ) ;
  numLapsSlider->setDelta  ( 0.05 ) ;
  numLapsSlider->setCBMode ( PUSLIDER_ALWAYS ) ;
  numLapsSlider->setValue  ( 1.0f*0.05f*(5.0f-1.0f) ) ;
  numLapsSlider->setCallback ( numLapsSlider_cb ) ;

  numLapsText = new puButton ( 160, 80, " 5" ) ;
  numLapsText->setStyle ( PUSTYLE_BOXED ) ;

  trackButtons = new puButtonBox ( 400, 10, 630, 150, track_names, TRUE ) ;
  trackButtons -> setLabel ( "Which Track?" ) ;
  trackButtons -> setLabelPlace ( PUPLACE_ABOVE ) ;
  trackButtons -> setValue ( 0 ) ; 

  install_material () ;

  signal ( 11, SIG_DFL ) ;

  while ( 1 )
    displayfn () ;

  return 0 ;
}


