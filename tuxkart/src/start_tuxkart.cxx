
#include "start_tuxkart.h"

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

static char *track_names  [ MAX_TRACKS ] ;
static char *track_idents [ MAX_TRACKS ] ;

extern int tuxkart_main ( int nl, char *track ) ;

/**************************************\
*                                      *
* These four functions capture mouse   *
* and keystrokes (special and mundane) *
* from GLUT and pass them on to PUI.   *
*                                      *
\**************************************/

static void specialfn ( int key, int, int )
{
  puKeyboard ( key + PU_KEY_GLUT_SPECIAL_OFFSET, PU_DOWN ) ;
  glutPostRedisplay () ;
}

static void keyfn ( unsigned char key, int, int )
{
  puKeyboard ( key, PU_DOWN ) ;
  glutPostRedisplay () ;
}

static void motionfn ( int x, int y )
{
  puMouse ( x, y ) ;
  glutPostRedisplay () ;
}

static void mousefn ( int button, int updown, int x, int y )
{
  puMouse ( button, updown, x, y ) ;
  glutPostRedisplay () ;
}

/***********************************\
*                                   *
* This function redisplays the PUI, *
* flips the double buffer and then  *
* asks GLUT to post a redisplay     *
* command - so we re-render at      *
* maximum rate.                     *
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

  glutSwapBuffers   () ;
  glutPostRedisplay () ;
}


/***********************************\
*                                   *
* Here are the PUI widget callback  *
* functions.                        *
*                                   *
\***********************************/

static void play_cb ( puObject * )
{
  int t ;
  int nl ;

  trackButtons -> getValue ( & t ) ;
  nl = atoi ( numLapsText->getLegend () ) ;

  delete numLapsSlider ;
  delete numLapsText   ;
  delete playButton    ;
  delete exitButton    ;
  delete trackButtons  ;
  delete intro_gst     ;
  delete sorority      ;
  delete fnt           ;

  tuxkart_main ( nl, track_idents[t] ) ;
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
    if ( access ( "data/levels.dat", F_OK ) == 0 )
      datadir = "." ;
    else
    if ( access ( "../data/levels.dat", F_OK ) == 0 )
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
 
  if ( chdir ( datadir ) == -1 )
  {
    fprintf ( stderr, "Couldn't chdir() to '%s'.\n", datadir ) ;
    exit ( 1 ) ;
  }                                                                             

  loadTrackList () ;

  int fake_argc = 1 ;
  char *fake_argv[3] ;
 
  fake_argv[0] = "Tux Kart" ;
  fake_argv[1] = "Tux Kart by Steve Baker." ;
  fake_argv[2] = NULL ;
 
  glutInitWindowPosition ( 0, 0 ) ;
  glutInitWindowSize     ( 640, 480 ) ;
  glutInit               ( &fake_argc, fake_argv ) ;
  glutInitDisplayMode    ( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH ) ;
  glutCreateWindow       ( fake_argv[1] ) ;

  glutDisplayFunc       ( displayfn ) ;
  glutKeyboardFunc      ( keyfn     ) ;
  glutSpecialFunc       ( specialfn ) ;
  glutMouseFunc         ( mousefn   ) ;
  glutMotionFunc        ( motionfn  ) ;
  glutPassiveMotionFunc ( motionfn  ) ;
  glutIdleFunc          ( displayfn ) ;

  puInit () ;
  ssgInit () ;

  if ( getenv ( "MESA_GLX_FX" ) != NULL )
  {
    puShowCursor () ;
    glutWarpPointer ( 320, 240 ) ;
  }

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
  glutMainLoop () ;
  return 0 ;
}


