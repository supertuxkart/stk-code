//  $Id: status.cxx,v 1.24 2004/08/10 16:22:31 grumbel Exp $
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

#include <stdarg.h>
#include <plib/fnt.h>
#include "tuxkart.h"
#include "status.h"
#include "KartDriver.h"
#include "material.h"
#include "RaceSetup.h"
#include "Loader.h"

#define MAX_STRING          30
#define MAX_STRING_LENGTH  256

double time_left = 0.0 ;

float tt[6]={0,0,0,0,0,0};

//FIXME:
static fntRenderer *text = NULL ;
static fntTexFont *oldfont ;

static char debug_strings [ MAX_STRING ][ MAX_STRING_LENGTH ] ;
static int  next_string  = 0 ;
static int  stats_enabled = FALSE ;
static bool show_fps = FALSE ;

static Material *herringbones_gst = NULL ;
static Material *herring_gst      = NULL ;
static Material *fuzzy_gst        = NULL ;
static Material *spark_gst        = NULL ;
static Material *missile_gst      = NULL ;
static Material *flamemissile_gst = NULL ;
static Material *magnet_gst       = NULL ;
static Material *zipper_gst       = NULL ;


void initStatusDisplay ()
{
  herringbones_gst = getMaterial ( "herringbones.rgb" ) ;
  herring_gst      = getMaterial ( "herring.rgb"      ) ;
  fuzzy_gst        = getMaterial ( "fuzzy.rgb"        ) ;
  spark_gst        = getMaterial ( "spark.rgb"        ) ;
  missile_gst      = getMaterial ( "missile.rgb"      ) ;
  flamemissile_gst = getMaterial ( "flamemissile.rgb" ) ;
  magnet_gst       = getMaterial ( "magnet.rgb"       ) ;
  zipper_gst       = getMaterial ( "zipper.rgb"       ) ;
  
  //FIXME:
  oldfont = new fntTexFont ;
  oldfont -> load ( loader->getPath("fonts/sorority.txf").c_str() ) ;
}

void stToggle ()
{
  if ( stats_enabled )
    stats_enabled = FALSE ;
  else
  {
    stats_enabled = TRUE ;

    for ( int i = 0 ; i < MAX_STRING ; i++ )
       debug_strings [ i ][ 0 ] = '\0' ;

    next_string = 0 ;
  }
}

void fpsToggle () 
{
	show_fps = !show_fps;
}

bool getShowFPS()
{
	return show_fps;
}

void stPrintf ( char *fmt, ... )
{
  char *p = debug_strings [ next_string++ ] ;

  if ( next_string >= MAX_STRING )
    next_string = 0 ;

  va_list ap ;
  va_start ( ap, fmt ) ;
/*
  Ideally, we should use this:

     vsnprintf ( p, MAX_STRING_LENGTH, fmt, ap ) ;

  ...but it's only in Linux   :-(
*/

  vsprintf ( p, fmt, ap ) ;

  va_end ( ap ) ;
}


void drawText ( char *str, int sz, int x, int y )
{
  text -> setFont      ( oldfont ) ;
  text -> setPointSize ( sz ) ;

  text -> begin () ;
    text -> start2f ( x, y ) ;
    text -> puts ( str ) ;
  text -> end () ;
}


void drawInverseDropShadowText ( char *str, int sz, int x, int y )
{
  glColor4f ( 1.0f, 1.0f, 1.0f, 1.0f ) ;
  drawText ( str, sz, x, y ) ;
  glColor4f ( 0.0f, 0.0f, 0.0f, 1.0f ) ;
  drawText ( str, sz, x+1, y+1 ) ;
}


void drawDropShadowText ( char *str, int sz, int x, int y )
{
  glColor4f ( 0.0f, 0.0f, 0.0f, 1.0f ) ;

  drawText ( str, sz, x, y ) ;

  glColor4f ( 1.0f, 1.0f, 1.0f, 1.0f ) ;

  drawText ( str, sz, x+1, y+1 ) ;
}


void drawStatsText ()
{
  char str [ 256 ] ;

  sprintf ( str, "%3d,%3d,%3d,%3d,%3d,%3d", (int)tt[0],(int)tt[1],(int)tt[2],(int)tt[3],(int)tt[4],(int)tt[5]) ;
  drawDropShadowText ( str, 18, 5, 300 ) ;
}

void drawTimer ()
{
  char str [ 256 ] ;

  time_left = fclock->getAbsTime () ;

  int min     = (int) floor ( time_left / 60.0 ) ;
  int sec     = (int) floor ( time_left - (double) ( 60 * min ) ) ;
  int tenths  = (int) floor ( 10.0f * (time_left - (double)(sec + 60*min)));

  sprintf ( str, "%3d:%02d.%d", min,  sec,  tenths ) ;
  drawDropShadowText ( str, 18, 450, 430 ) ;
}

static char *pos_string [] =
{
  "?!?",
  "1st",
  "2nd",
  "3rd",
  "4th",
  "5th",
  "6th",
  "7th",
  "8th",
  "9th"
} ;

void drawScore (RaceSetup& raceSetup)
{
  char str [ 20 ] ;

  if ( kart[0]->getVelocity()->xyz[1] < 0 )
    sprintf ( str, "Reverse" ) ;
  else
    sprintf(str,"%3dmph",(int)(kart[0]->getVelocity()->xyz[1]/MILES_PER_HOUR));

  drawDropShadowText ( str, 18, 450, 410 ) ;

  if ( kart[0]->getLap() < 0 )
    sprintf ( str, "Not Started Yet!" ) ;
  else
  if ( kart[0]->getLap() < raceSetup.numLaps - 1 )
    sprintf ( str, "%s - Lap %d",
      pos_string [ kart[0]->getPosition() ],
                   kart[0]->getLap() + 1 ) ;
  else
  {
    static int flasher = 0 ;

    if ( ++flasher & 32 )
      sprintf ( str, "%s - Last Lap!",
        pos_string [ kart[0]->getPosition() ] ) ;
    else
      sprintf ( str, "%s",
        pos_string [ kart[0]->getPosition() ] ) ;
  }

  drawDropShadowText ( str, 18, 450, 390 ) ;
}



void drawMap ()
{
  glDisable ( GL_TEXTURE_2D ) ;
  glColor3f ( 0,0,1 ) ;
  curr_track -> draw2Dview ( 430+TRACKVIEW_SIZE  , TRACKVIEW_SIZE   ) ;
  glColor3f ( 1,1,0 ) ;
  curr_track -> draw2Dview ( 430+TRACKVIEW_SIZE+1, TRACKVIEW_SIZE+1 ) ;

  glBegin ( GL_QUADS ) ;

  for ( Karts::size_type i = 0 ; i < kart.size() ; ++i )
  {
    sgCoord *c ;

    c = kart[i]->getCoord () ;

    glColor3fv ( kart[i]->getKartProperties().color ) ;

    curr_track->glVtx ( c->xyz, 430+TRACKVIEW_SIZE+3, TRACKVIEW_SIZE+3 ) ;
    curr_track->glVtx ( c->xyz, 430+TRACKVIEW_SIZE+0, TRACKVIEW_SIZE+3 ) ;
    curr_track->glVtx ( c->xyz, 430+TRACKVIEW_SIZE+0, TRACKVIEW_SIZE+0 ) ;
    curr_track->glVtx ( c->xyz, 430+TRACKVIEW_SIZE+3, TRACKVIEW_SIZE+0 ) ;
  }

  glEnd () ;
}


void drawGameOverText ()
{
  static int timer = 0 ;

  glColor4f ( sin ( (float)timer/5.1f ) / 2.0f + 0.5f,
              sin ( (float)timer/6.3f ) / 2.0f + 0.5f,
              sin ( (float)timer/7.2f ) / 2.0f + 0.5f, 0.5 ) ;

  if ( finishing_position < 0 )
    finishing_position = kart[0]->getPosition() ;

  if ( finishing_position > 1 )
  {
    drawText ( "YOU FINISHED"    , 50, 50, 280 ) ;
    drawText ( pos_string [ finishing_position ], 150, 150, 150 ) ;
  }
  else
  {
    drawText ( "CONGRATULATIONS"  , 40, 50, 330 ) ;
    drawText ( "YOU WON THE RACE!", 40, 50, 280 ) ;
  }
}


void drawGameRunningText (RaceSetup& raceSetup)
{
  drawScore (raceSetup) ;
  drawTimer () ;

  glColor4f ( 0.6, 0.0, 0.6, 1.0 ) ;
    
  if ( stats_enabled )
    drawStatsText () ;
}



void drawPlayerIcons ()
{
  int x =  0 ;
  int y = 10 ;

  // FIXME: Use getScreenSize and do more intelligent icon placement
  float w = 640.0f - 64.0f ;

  // FIXME: Draw more intelligent so that player is always on top
  for ( Karts::size_type i = 0; i < kart.size() ; ++i )
    {
      Material* players_gst = kart[i]->getKartProperties().getIconMaterial();
      players_gst -> apply ();

      glBegin ( GL_QUADS ) ;
      glColor4f    ( 1, 1, 1, 1 ) ;

      /* Geeko */
      x = (int) ( w * kart [i] -> getDistanceDownTrack () /
                  curr_track -> getTrackLength () ) ;
      glTexCoord2f ( 0, 0 ) ; glVertex2i ( x   , y    ) ;
      glTexCoord2f ( 1, 0 ) ; glVertex2i ( x+64, y    ) ;
      glTexCoord2f ( 1, 1 ) ; glVertex2i ( x+64, y+64 ) ;
      glTexCoord2f ( 0, 1 ) ; glVertex2i ( x   , y+64 ) ;
      glEnd () ;
    }
}


void drawEmergencyText ()
{
  static float wrong_timer = 0.0f ;
  static float last_dist = -1000000.0f ;
  static int last_lap = -1 ;

  float d = kart [ 0 ] -> getDistanceDownTrack () ;
  int   l = kart [ 0 ] -> getLap () ;

  if ( ( l < last_lap || ( l == last_lap && d < last_dist ) ) &&
       kart [ 0 ] -> getVelocity () -> xyz [ 1 ] > 0.0f )
  {
    wrong_timer += fclock -> getDeltaTime () ;

    if ( wrong_timer > 2.0f )
    {
      static int i = FALSE ;

      if ( i )
        glColor4f ( 1.0f, 0.0f, 1.0f, 1.0f ) ;
      else
        glColor4f ( 0.0f, 1.0f, 0.0f, 1.0f ) ;

      drawText ( "WRONG WAY!", 50, 100, 240 ) ;

      if ( ! i )
        glColor4f ( 1.0f, 0.0f, 1.0f, 1.0f ) ;
      else
        glColor4f ( 0.0f, 1.0f, 0.0f, 1.0f ) ;

      drawText ( "WRONG WAY!", 50, 100+2, 240+2 ) ;

      i = ! i ;
    }
  }
  else
    wrong_timer = 0.0f ;

  last_dist = d ;
  last_lap  = l ;
}


void drawCollectableIcons ()
{
  int zz = FALSE ;

  switch ( kart[0]->getCollectable () )
  {
    case COLLECT_NOTHING        : fuzzy_gst        -> apply () ; break ;
    case COLLECT_SPARK          : spark_gst        -> apply () ; break ;
    case COLLECT_MISSILE        : missile_gst      -> apply () ; break ;
    case COLLECT_HOMING_MISSILE : flamemissile_gst -> apply () ; break ;
    case COLLECT_MAGNET         : magnet_gst       -> apply () ; break ;
    case COLLECT_ZIPPER         : zipper_gst       -> apply () ;
                                  zz = TRUE ; break ;
  }

  int x1 =  20 ;
  int y1 = 400 ;
  int n  = kart[0]->getNumCollectables() ;

  if ( n > 5 ) n = 5 ;
  if ( n < 1 ) n = 1 ;

  glBegin ( GL_QUADS ) ;
    glColor4f    ( 1, 1, 1, 1 ) ;

    for ( int i = 0 ; i < n ; i++ )
    {
      if ( zz )
      {
	glTexCoord2f ( 0, 2 ) ; glVertex2i ( i*40 + x1   , y1    ) ;
	glTexCoord2f ( 0, 0 ) ; glVertex2i ( i*40 + x1+32, y1    ) ;
	glTexCoord2f ( 2, 0 ) ; glVertex2i ( i*40 + x1+64, y1+32 ) ;
	glTexCoord2f ( 2, 2 ) ; glVertex2i ( i*40 + x1+32, y1+32 ) ;

	glTexCoord2f ( 0, 2 ) ; glVertex2i ( i*40 + x1+32, y1+32 ) ;
	glTexCoord2f ( 0, 0 ) ; glVertex2i ( i*40 + x1+64, y1+32 ) ;
	glTexCoord2f ( 2, 0 ) ; glVertex2i ( i*40 + x1+32, y1+64 ) ;
	glTexCoord2f ( 2, 2 ) ; glVertex2i ( i*40 + x1   , y1+64 ) ;
      }
      else
      {
	glTexCoord2f ( 0, 0 ) ; glVertex2i ( i*30 + x1   , y1    ) ;
	glTexCoord2f ( 1, 0 ) ; glVertex2i ( i*30 + x1+64, y1    ) ;
	glTexCoord2f ( 1, 1 ) ; glVertex2i ( i*30 + x1+64, y1+64 ) ;
	glTexCoord2f ( 0, 1 ) ; glVertex2i ( i*30 + x1   , y1+64 ) ;
      }
    }
  glEnd () ;
}


void drawPartlyDigestedHerring ( float state )
{
  herringbones_gst -> apply () ;
 
  glBegin ( GL_QUADS ) ;
  glColor3f    ( 1, 1, 1 ) ;
  glTexCoord2f ( 0, 0 ) ; glVertex2i ( 200, 400 ) ;
  glTexCoord2f ( 1, 0 ) ; glVertex2i ( 300, 400 ) ;
  glTexCoord2f ( 1, 1 ) ; glVertex2i ( 300, 440 ) ;
  glTexCoord2f ( 0, 1 ) ; glVertex2i ( 200, 440 ) ;
  glEnd () ;
 
  herring_gst -> apply () ;
 
  glBegin ( GL_QUADS ) ;
  glColor3f    ( 0.7, 1, 1 ) ;
  glTexCoord2f ( 0, 0 ) ; glVertex2i ( 200,  400 ) ;
  glTexCoord2f ( state, 0 ) ; glVertex2i ( 200 + (int)(state * 100.0f), 400 ) ;
  glTexCoord2f ( state, 1 ) ; glVertex2i ( 200 + (int)(state * 100.0f), 440 ) ;
  glTexCoord2f ( 0, 1 ) ; glVertex2i ( 200, 440 ) ;
  glEnd () ;                                                                    
}


void drawStatusText (RaceSetup& raceSetup)
{
  if ( text == NULL )
    text = new fntRenderer () ;

  glMatrixMode   ( GL_MODELVIEW ) ;
  glPushMatrix   () ;
  glLoadIdentity () ;

  glMatrixMode   ( GL_PROJECTION ) ;
  glPushMatrix   () ;
  glLoadIdentity () ;

  glPushAttrib   ( GL_ENABLE_BIT | GL_LIGHTING_BIT ) ;
  glDisable      ( GL_DEPTH_TEST ) ;
  glDisable      ( GL_LIGHTING   ) ;
  glDisable      ( GL_FOG        ) ;
  glDisable      ( GL_CULL_FACE  ) ;
  glEnable       ( GL_ALPHA_TEST ) ;
  glAlphaFunc    ( GL_GREATER, 0.2 ) ;
  glEnable       ( GL_BLEND      ) ;

  glOrtho        ( 0, 640, 0, 480, 0, 100 ) ;

  if ( kart[0]->getLap () >= raceSetup.numLaps )
    drawGameOverText     () ;
  else
  {
    drawGameRunningText  (raceSetup) ;
    drawEmergencyText    () ;
    drawCollectableIcons () ;
    drawPartlyDigestedHerring ( (float)(kart[0]->getNumHerring()) /
                                                     MAX_HERRING_EATEN ) ;
    drawPlayerIcons      () ;
    drawMap              () ;
  }

  glPopAttrib  () ;
  glPopMatrix  () ;
  glMatrixMode ( GL_MODELVIEW ) ;
  glPopMatrix  () ;
}

