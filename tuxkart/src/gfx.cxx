
#include  "tuxkart.h"

#ifndef WIN32
#include <unistd.h>
#include <string.h>
#include <sys/io.h>
#include <sys/perm.h>                                                           
#endif

static unsigned int lastKeystroke = 0 ;

static char keyIsDown [ 512 ] ;

void keystroke ( int key, int updown, int, int )
{
  if ( updown == PW_DOWN )
    lastKeystroke = key ;

  keyIsDown [ key ] = (updown == PW_DOWN) ;
}


int isKeyDown ( unsigned int k )
{
  return keyIsDown [ k ] ;
}

int getKeystroke ()
{
  int k = lastKeystroke ;
  lastKeystroke = 0 ;
  return k ;
}

void reshape ( int w, int h )
{
  glViewport ( 0, 0, w, h ) ;
}


GFX::GFX ()
{
  for ( int i = 0 ; i < 512 ; i++ )
    keyIsDown [ i ] = FALSE ;

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



/* Address of the parallel port. */

#define LPBASE 0x378L

/* -1 left-eye, +1 right-eye, 0 center (ie No Stereo) */

static int stereo = 0 ;


int stereoShift ()
{
  return stereo ;
}


void GFX::done ()
{
  pwSwapBuffers () ;
  glBegin ( GL_POINTS ) ;
  glVertex3f ( 0, 0, 0 ) ;
  glEnd () ;
  glFlush () ;

  static int firsttime = TRUE ;

  if ( firsttime )
  {
    firsttime = FALSE ;

    if ( getenv ( "TUXKART_STEREO" ) == NULL )
    {
      stereo = 0 ;
      return ;
    }

    fprintf ( stderr, "Requesting control of parallel printer port...\n" ) ;
 
    int res = ioperm ( LPBASE, 8, 1 ) ;
 
    if ( res != 0 )
    {
      perror ( "parport" ) ;
      fprintf ( stderr, "Need to run as 'root' to get stereo.\n" ) ;
      stereo = 0 ;
    }
    else
    {
      fprintf ( stderr, "Stereo Enabled!\n" ) ;
      stereo = -1 ;
    }
  }

  if ( stereo != 0 )
  {
    outb ( (stereo==-1) ? ~3 : ~2, LPBASE+2 ) ;
    stereo = -stereo ;
  }
}


