
#include  "tuxkart.h"

static unsigned int lastGLUTKeystroke = 0 ;

static char keyIsDown [ 512 ] ;

static void getGLUTUpSpecialKeystroke ( int key, int, int )
{
  keyIsDown [ 256 + key ] = FALSE ;
}

static void getGLUTUpKeystroke ( unsigned char key, int, int )
{
  keyIsDown [ key ] = FALSE ;
}

static void getGLUTSpecialKeystroke ( int key, int, int )
{
  lastGLUTKeystroke = 256 + key ;
  keyIsDown [ 256 + key ] = TRUE ;
}

static void getGLUTKeystroke ( unsigned char key, int, int )
{
  lastGLUTKeystroke = key ;
  keyIsDown [ key ] = TRUE ;
}

int isGLUTKeyDown ( unsigned int k )
{
  return keyIsDown [ k ] ;
}

int getGLUTKeystroke ()
{
  int k = lastGLUTKeystroke ;
  lastGLUTKeystroke = 0 ;
  return k ;
}

void reshape ( int w, int h )
{
  glViewport ( 0, 0, w, h ) ;
}

void initWindow ( int w, int h )
{
/*
  Already done in start_tuxkart

  int fake_argc = 1 ;
  char *fake_argv[3] ;

  for ( int i = 0 ; i < 512 ; i++ )
    keyIsDown [ i ] = FALSE ;

  fake_argv[0] = "Tux Kart" ;
  fake_argv[1] = "Tux Kart by Steve Baker." ;
  fake_argv[2] = NULL ;

  glutInitWindowPosition ( 0, 0 ) ;
  glutInitWindowSize     ( w, h ) ;
  glutInit               ( &fake_argc, fake_argv ) ;
  glutInitDisplayMode    ( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH ) ;
  glutCreateWindow       ( fake_argv[1] ) ;
*/
  glutDisplayFunc        ( tuxKartMainLoop ) ;
  glutKeyboardFunc       ( getGLUTKeystroke ) ;
  glutSpecialFunc        ( getGLUTSpecialKeystroke ) ;
  glutKeyboardUpFunc     ( getGLUTUpKeystroke ) ;
  glutSpecialUpFunc      ( getGLUTUpSpecialKeystroke ) ;
  glutReshapeFunc        ( reshape ) ;
#ifndef WIN32
  glutIdleFunc           ( glutPostRedisplay ) ;
#endif
}


GFX::GFX ()
{
  initWindow ( 640, 480 ) ;

/*
  Already done in start_tuxkart

  ssgInit  () ;
*/
 
  static int firsttime = 1 ;

  if ( firsttime )
  {
    firsttime = 0 ;
    initMaterials () ;
  }

  ssgSetFOV ( 75.0f, 0.0f ) ;
  ssgSetNearFar ( 0.05f, 1000.0f ) ;

  sgCoord cam ;
  sgSetVec3 ( cam.xyz, 0, 0, 0 ) ;
  sgSetVec3 ( cam.hpr, 0, 0, 0 ) ;
  ssgSetCamera ( & cam ) ;
}


void GFX::update ()
{
  sgVec3 sunposn   ;
  sgVec4 skyfogcol ;
  sgVec4 ambientcol ;
  sgVec4 specularcol ;
  sgVec4 diffusecol ;

  sgSetVec3 ( sunposn, 0.4, 0.4, 0.4 ) ;

  sgSetVec4 ( skyfogcol  , 0.3, 0.7, 0.9, 1.0 ) ;
  sgSetVec4 ( ambientcol , 0.5, 0.5, 0.5, 1.0 ) ;
  sgSetVec4 ( specularcol, 1.0, 1.0, 1.0, 1.0 ) ;
  sgSetVec4 ( diffusecol , 1.0, 1.0, 1.0, 1.0 ) ;

  ssgGetLight ( 0 ) -> setPosition ( sunposn ) ;
  ssgGetLight ( 0 ) -> setColour ( GL_AMBIENT , ambientcol  ) ;
  ssgGetLight ( 0 ) -> setColour ( GL_DIFFUSE , diffusecol  ) ;
  ssgGetLight ( 0 ) -> setColour ( GL_SPECULAR, specularcol ) ;

  /* Clear the screen */

  glClearColor ( skyfogcol[0], skyfogcol[1], skyfogcol[2], skyfogcol[3] ) ;
  glClear      ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

  glEnable ( GL_DEPTH_TEST ) ;

  glFogf ( GL_FOG_DENSITY, 0.005 / 100.0f ) ;
  glFogfv( GL_FOG_COLOR  , skyfogcol ) ;
  glFogf ( GL_FOG_START  , 0.0       ) ;
  glFogi ( GL_FOG_MODE   , GL_EXP2   ) ;
  glHint ( GL_FOG_HINT   , GL_NICEST ) ;

/*
  sgCoord cam ;
  sgSetVec3 ( cam.xyz, 0, 0, 0 ) ;
  sgSetVec3 ( cam.hpr, 0, 0, 0 ) ;
  ssgSetCamera ( & cam ) ;
*/
  glEnable ( GL_FOG ) ;
  ssgCullAndDraw ( scene ) ;
  glDisable ( GL_FOG ) ;
}


void GFX::done ()
{
  glutPostRedisplay () ;
  glutSwapBuffers () ;
}

