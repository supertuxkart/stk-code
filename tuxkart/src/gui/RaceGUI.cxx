//  $Id: RaceGUI.cxx,v 1.17 2004/08/19 12:29:17 grumbel Exp $
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

#include "RaceGUI.h"
#include "tuxkart.h"
#include "../PlayerDriver.h"
#include "../Track.h"
#include "WidgetSet.h"
#include "World.h"
#include "KartDriver.h"
#include "StartScreen.h"

//includes copied from status.cxx, some could be unnecessary
#include <stdarg.h>
#include "RaceSetup.h"
#include "Loader.h"

RaceGUI::RaceGUI():
show_fps(false),
herringbones_gst(NULL),
herring_gst(NULL),
fuzzy_gst(NULL),
spark_gst(NULL),
missile_gst(NULL),
flamemissile_gst(NULL),
magnet_gst(NULL),
zipper_gst(NULL),
time_left(0.0),
stats_enabled(false),
next_string(0),
text(NULL)
{
	pos_string[0] = "?!?";
	pos_string[1] = "1st";
	pos_string[2] = "2nd";
	pos_string[3] = "3rd";
	pos_string[4] = "4th";
	pos_string[5] = "5th";
	pos_string[6] = "6th";
	pos_string[7] = "7th";
	pos_string[8] = "8th";
	pos_string[9] = "9th";
	pos_string[10] = "10th";
	
	memset(tt, 0, sizeof(float) * 6);

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
  
	if ((fps_id = widgetSet -> count(0, 1000, GUI_SML, GUI_SE)))
		widgetSet -> layout(fps_id, -1, 1);
}

RaceGUI::~RaceGUI()
{
	widgetSet -> delete_widget(fps_id) ;
	
	//FIXME: does all that material stuff need freeing somehow?
	//also need to unload oldfont somehow
}
	
void RaceGUI::update(float dt)
{
	widgetSet -> timer(fps_id, dt) ;
	
	drawStatusText (World::current()->raceSetup) ;
		
	if ( show_fps )
		drawFPS ();
}

void RaceGUI::keybd(const SDL_keysym& key)
{
	static int isWireframe = FALSE ;
	
	if (key.mod & KMOD_CTRL)
	{
          // FIXME: '0' alone can't be correct, can it?
          PlayerDriver* driver = dynamic_cast<PlayerDriver*>(World::current()->kart [ 0 ]->getDriver());
          if (driver)
            driver -> incomingKeystroke ( key ) ;
      	return;
	}
    
	switch ( key.sym )
	{
	case SDLK_F12: show_fps = !show_fps ; return;
	
	case SDLK_F11 : 
		if ( isWireframe )
			glPolygonMode ( GL_FRONT_AND_BACK, GL_FILL ) ;
		else
      		glPolygonMode ( GL_FRONT_AND_BACK, GL_LINE ) ;
      	isWireframe = ! isWireframe ;
		return ;
	
	#ifdef DEBUG
	case SDLK_F10 : stToggle () ; return ;
	#endif
	
	case SDLK_ESCAPE:
		widgetSet -> tgl_paused();
		guiStack.push_back(GUIS_RACEMENU);
		break;
		
	default: break;
	}
}

void RaceGUI::drawFPS ()
{
  static int fpsCounter;
  static int fpsSave = 0;
  static int fpsTimer = SDL_GetTicks();
  
  int now = SDL_GetTicks();

  if (now - fpsTimer > 1000)
    {
      fpsSave = fpsCounter;
      fpsCounter = 0;
      fpsTimer = now;
	
	widgetSet -> set_count(fps_id, fpsSave);
    }
  else
    ++fpsCounter;
    
  widgetSet -> paint(fps_id) ;
}

void RaceGUI::stToggle ()
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

void RaceGUI::stPrintf ( char *fmt, ... )
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

void RaceGUI::drawText ( char *str, int sz, int x, int y )
{
  text -> setFont      ( oldfont ) ;
  text -> setPointSize ( sz ) ;

  text -> begin () ;
    text -> start2f ( x, y ) ;
    text -> puts ( str ) ;
  text -> end () ;
}


void RaceGUI::drawInverseDropShadowText ( char *str, int sz, int x, int y )
{
  glColor4f ( 1.0f, 1.0f, 1.0f, 1.0f ) ;
  drawText ( str, sz, x, y ) ;
  glColor4f ( 0.0f, 0.0f, 0.0f, 1.0f ) ;
  drawText ( str, sz, x+1, y+1 ) ;
}

void RaceGUI::drawDropShadowText ( char *str, int sz, int x, int y )
{
  glColor4f ( 0.0f, 0.0f, 0.0f, 1.0f ) ;

  drawText ( str, sz, x, y ) ;

  glColor4f ( 1.0f, 1.0f, 1.0f, 1.0f ) ;

  drawText ( str, sz, x+1, y+1 ) ;
}


void RaceGUI::drawStatsText ()
{
  char str [ 256 ] ;

  sprintf ( str, "%3d,%3d,%3d,%3d,%3d,%3d", (int)tt[0],(int)tt[1],(int)tt[2],(int)tt[3],(int)tt[4],(int)tt[5]) ;
  drawDropShadowText ( str, 18, 5, 300 ) ;
}

void RaceGUI::drawTimer ()
{
  char str [ 256 ] ;

  time_left = World::current()->clock;

  int min     = (int) floor ( time_left / 60.0 ) ;
  int sec     = (int) floor ( time_left - (double) ( 60 * min ) ) ;
  int tenths  = (int) floor ( 10.0f * (time_left - (double)(sec + 60*min)));

  sprintf ( str, "%3d:%02d.%d", min,  sec,  tenths ) ;
  drawDropShadowText ( str, 18, 450, 430 ) ;
}

void RaceGUI::drawScore (const RaceSetup& raceSetup)
{
  char str [ 256 ] ;

  KartDriver* player_kart = World::current()->kart[0];

  if ( player_kart->getVelocity()->xyz[1] < 0 )
    sprintf ( str, "Reverse" ) ;
  else
    sprintf(str,"%3dmph",(int)(player_kart->getVelocity()->xyz[1]/MILES_PER_HOUR));

  drawDropShadowText ( str, 18, 450, 410 ) ;

  if ( player_kart->getLap() < 0 )
    sprintf ( str, "Not Started Yet!" ) ;
  else
  if ( player_kart->getLap() < raceSetup.numLaps - 1 )
    sprintf ( str, "%s - Lap %d",
      pos_string [ player_kart->getPosition() ],
                   player_kart->getLap() + 1 ) ;
  else
  {
    static int flasher = 0 ;

    if ( ++flasher & 32 )
      sprintf ( str, "%s - Last Lap!",
        pos_string [ player_kart->getPosition() ] ) ;
    else
      sprintf ( str, "%s",
        pos_string [ player_kart->getPosition() ] ) ;
  }

  drawDropShadowText ( str, 18, 450, 390 ) ;
}



void RaceGUI::drawMap ()
{
  glDisable ( GL_TEXTURE_2D ) ;
  glColor3f ( 1,1,1 ) ;
  World::current() ->track -> draw2Dview ( 200*2, 200  ) ;

  glBegin ( GL_QUADS ) ;

  for ( World::Karts::size_type i = 0 ; i < World::current()->kart.size() ; ++i )
  {
    sgCoord *c ;

    c = World::current()->kart[i]->getCoord () ;

    glColor3fv ( World::current()->kart[i]->getKartProperties().color ) ;

    /* 
       FIXME:
       curr_track->glVtx ( c->xyz, 430+TRACKVIEW_SIZE+3, TRACKVIEW_SIZE+3 ) ;
       curr_track->glVtx ( c->xyz, 430+TRACKVIEW_SIZE+0, TRACKVIEW_SIZE+3 ) ;
       curr_track->glVtx ( c->xyz, 430+TRACKVIEW_SIZE+0, TRACKVIEW_SIZE+0 ) ;
       curr_track->glVtx ( c->xyz, 430+TRACKVIEW_SIZE+3, TRACKVIEW_SIZE+0 ) ;
    */
  }

  glEnd () ;
}


void RaceGUI::drawGameOverText ()
{
  static int timer = 0 ;

  glColor4f ( sin ( (float)timer/5.1f ) / 2.0f + 0.5f,
              sin ( (float)timer/6.3f ) / 2.0f + 0.5f,
              sin ( (float)timer/7.2f ) / 2.0f + 0.5f, 0.5 ) ;

  if ( finishing_position < 0 )
    finishing_position = World::current()->kart[0]->getPosition() ;

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


void RaceGUI::drawGameRunningText (const RaceSetup& raceSetup)
{
  drawScore (raceSetup) ;
  drawTimer () ;

  glColor4f ( 0.6, 0.0, 0.6, 1.0 ) ;
    
  if ( stats_enabled )
    drawStatsText () ;
}



void RaceGUI::drawPlayerIcons ()
{
  int x =  0 ;
  int y = 10 ;

  // FIXME: Use getScreenSize and do more intelligent icon placement
  float w = 640.0f - 64.0f ;

  // FIXME: Draw more intelligent so that player is always on top
  for ( World::Karts::size_type i = 0; i < World::current()->kart.size() ; ++i )
    {
      Material* players_gst = World::current()->kart[i]->getKartProperties().getIconMaterial();
      players_gst -> apply ();

      glBegin ( GL_QUADS ) ;
      glColor4f    ( 1, 1, 1, 1 ) ;

      /* Geeko */
      x = (int) ( w * World::current()->kart [i] -> getDistanceDownTrack () /
                  World::current() ->track -> getTrackLength () ) ;
      glTexCoord2f ( 0, 0 ) ; glVertex2i ( x   , y    ) ;
      glTexCoord2f ( 1, 0 ) ; glVertex2i ( x+64, y    ) ;
      glTexCoord2f ( 1, 1 ) ; glVertex2i ( x+64, y+64 ) ;
      glTexCoord2f ( 0, 1 ) ; glVertex2i ( x   , y+64 ) ;
      glEnd () ;
    }
}


void RaceGUI::drawEmergencyText ()
{
  static float wrong_timer = 0.0f ;
  static float last_dist = -1000000.0f ;
  static int last_lap = -1 ;

  float d = World::current()->kart [ 0 ] -> getDistanceDownTrack () ;
  int   l = World::current()->kart [ 0 ] -> getLap () ;

  if ( ( l < last_lap || ( l == last_lap && d < last_dist ) ) &&
       World::current()->kart [ 0 ] -> getVelocity () -> xyz [ 1 ] > 0.0f )
  {
    wrong_timer += 0.05f; // FIXME: was World::current()->clock -> getDeltaTime () ;

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


void RaceGUI::drawCollectableIcons ()
{
  int zz = FALSE ;

  switch ( World::current()->kart[0]->getCollectable () )
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
  int n  = World::current()->kart[0]->getNumCollectables() ;

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


void RaceGUI::drawPartlyDigestedHerring ( float state )
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


void RaceGUI::drawStatusText (const RaceSetup& raceSetup)
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

  switch (World::current()->ready_set_go)
    {
    case 2:
      drawText ( "Ready!", 40, 50, 280 ) ;
      break;
    case 1:
      drawText ( "Set!", 40, 50, 280 ) ;
      break;
    case 0:
      drawText ( "Go!", 40, 50, 280 ) ;
      break;
    }

  if ( World::current()->getPhase() == World::FINISH_PHASE )
    {
      drawGameOverText     () ;
    }
  else
    {
      drawGameRunningText  (raceSetup) ;
      drawEmergencyText    () ;
      drawCollectableIcons () ;
      drawPartlyDigestedHerring ( (float)(World::current()->kart[0]->getNumHerring()) /
                                  MAX_HERRING_EATEN ) ;
      drawPlayerIcons      () ;
      drawMap              () ;
    }

  glPopAttrib  () ;
  glPopMatrix  () ;
  glMatrixMode ( GL_MODELVIEW ) ;
  glPopMatrix  () ;
}

/* EOF */
