//  $Id: RaceGUI.cxx,v 1.29 2004/08/25 21:08:21 oaf_thadres Exp $
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
#include "../constants.h"
#include "../Config.h"
#include "WidgetSet.h"
#include "World.h"
#include "KartDriver.h"
#include "StartScreen.h"

//includes copied from status.cxx, some could be unnecessary
#include <stdarg.h>
#include "RaceSetup.h"
#include "Loader.h"

RaceGUI::RaceGUI():
herringbones_gst(NULL),
herring_gst(NULL),
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
		
	if ( config.displayFPS )
		drawFPS ();
}

void RaceGUI::keybd(const SDL_keysym& key)
{
	static int isWireframe = FALSE ;
	
	//in single player only we have an infinite ammo cheat
	if (key.mod & KMOD_CTRL && World::current()->raceSetup.getNumPlayers() == 1)
	{
          PlayerDriver* driver = dynamic_cast<PlayerDriver*>(World::current()->getPlayerKart(0)->getDriver());
          if (driver)
            driver -> incomingKeystroke ( key ) ;
      	return;
	}
    
	switch ( key.sym )
	{
	case SDLK_F12: config.displayFPS = !config.displayFPS ; return;
	
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
/*  char str [ 256 ] ;

  sprintf ( str, "%3d,%3d,%3d,%3d,%3d,%3d", (int)tt[0],(int)tt[1],(int)tt[2],(int)tt[3],(int)tt[4],(int)tt[5]) ;
  drawDropShadowText ( str, 18, 5, 300 ) ;*/
}

void RaceGUI::drawTimer ()
{
  char str [ 256 ] ;

  time_left = World::current()->clock;

  int min     = (int) floor ( time_left / 60.0 ) ;
  int sec     = (int) floor ( time_left - (double) ( 60 * min ) ) ;
  int tenths  = (int) floor ( 10.0f * (time_left - (double)(sec + 60*min)));

  sprintf ( str, "%3d`%02d\"%d", min,  sec,  tenths ) ;
  drawDropShadowText ( "Time:", 14, 500, 450 ) ;
  drawDropShadowText ( str,     25, 480, 420 ) ;
}

void RaceGUI::drawScore (const RaceSetup& raceSetup)
{
  char str [ 256 ] ;

  KartDriver* player_kart = World::current()->getPlayerKart(0);

#ifdef DEBUG
  /* Show velocity */
  if ( player_kart->getVelocity()->xyz[1] < 0 )
    sprintf ( str, "Reverse" ) ;
  else
    sprintf(str,"%3dmph",(int)(player_kart->getVelocity()->xyz[1]/MILES_PER_HOUR));

  drawDropShadowText ( str, 18, 640-((strlen(str)-1)*18), 0 ) ;
#endif

  /* Show lap number */
  if ( player_kart->getLap() < 0 )
    sprintf ( str, "Not Started Yet!" ) ;
  else
  if ( player_kart->getLap() < raceSetup.numLaps - 1 )
    sprintf ( str, "Lap: %d/%d",
                   player_kart->getLap() + 1, raceSetup.numLaps ) ;
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

  drawDropShadowText ( str, 24, 10, 450 ) ;

  /* Show player's position */
  sprintf ( str, "%s", pos_string [ player_kart->getPosition() ] ) ;
  drawDropShadowText ( str, 55, 22, 22 );
}



void RaceGUI::drawMap ()
{
  glColor3f ( 1,1,1 ) ;
  World::current() ->track -> draw2Dview ( 520, 40, 120, 120, false ) ;

  glBegin ( GL_QUADS ) ;

  for ( int i = 0 ; i < World::current()->getNumKarts() ; ++i )
  {
    sgCoord *c ;

    c = World::current()->getKart(i)->getCoord () ;

    glColor3fv ( World::current()->getKart(i)->getKartProperties().color ) ;

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
    finishing_position = World::current()->getPlayerKart(0)->getPosition() ;

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
  /** Draw players position on the race */

  int x = 10;
  int y;
  char str[256];

  glEnable(GL_TEXTURE_2D);

  for(int i = 0; i < World::current()->getNumKarts() ; i++)
    {
      int position = World::current()->getKart(i)->getPosition();
      if(position > 4)  // only draw the first four karts
        continue;

      y = 310 - ((position-1)*(55+5));

      // draw icon
      Material* players_gst =
          World::current()->getKart(i)->getKartProperties().getIconMaterial();
      players_gst -> apply ();

      glBegin ( GL_QUADS ) ;
      glColor4f    ( 1, 1, 1, 1 ) ;

      glTexCoord2f ( 0, 0 ) ; glVertex2i ( x   , y    ) ;
      glTexCoord2f ( 1, 0 ) ; glVertex2i ( x+55, y    ) ;
      glTexCoord2f ( 1, 1 ) ; glVertex2i ( x+55, y+55 ) ;
      glTexCoord2f ( 0, 1 ) ; glVertex2i ( x   , y+55 ) ;
      glEnd () ;

      // draw text
      sprintf (str, "%s", pos_string[position]);
      drawDropShadowText ( str, 20, 55+x, y+15 ) ;
    }

  glDisable(GL_TEXTURE_2D);
}


void RaceGUI::drawEmergencyText ()
{
  static float wrong_timer = 0.0f ;
  static float last_dist = -1000000.0f ;
  static int last_lap = -1 ;

  float d = World::current()->getPlayerKart(0)-> getDistanceDownTrack () ;
  int   l = World::current()->getPlayerKart(0)-> getLap () ;

  if ( ( l < last_lap || ( l == last_lap && d < last_dist ) ) &&
       World::current()->getPlayerKart(0) -> getVelocity () -> xyz [ 1 ] > 0.0f )
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

  switch ( World::current()->getPlayerKart(0)->getCollectable () )
  {
    case COLLECT_NOTHING        : break ;
    case COLLECT_SPARK          : spark_gst        -> apply () ; break ;
    case COLLECT_MISSILE        : missile_gst      -> apply () ; break ;
    case COLLECT_HOMING_MISSILE : flamemissile_gst -> apply () ; break ;
    case COLLECT_MAGNET         : magnet_gst       -> apply () ; break ;
    case COLLECT_ZIPPER         : zipper_gst       -> apply () ;
                                  zz = TRUE ; break ;
  }

  int x1 =  320-32 ;
  int y1 = 400 ;

  glDisable(GL_TEXTURE_2D);

  glBegin ( GL_QUADS ) ;
    glColor4f ( 0.0, 0.0, 0.0, 0.16 ) ;
    glVertex2i ( x1   , y1    ) ;
    glVertex2i ( x1+64, y1    ) ;
    glVertex2i ( x1+64, y1+64 ) ;
    glVertex2i ( x1   , y1+64 ) ;
  glEnd();

  // If player doesn't have anything, just let the transparent black square
  if(World::current()->getPlayerKart(0)->getCollectable () == COLLECT_NOTHING)
    return;

  int n  = World::current()->getPlayerKart(0)->getNumCollectables() ;

  if ( n > 5 ) n = 5 ;
  if ( n < 1 ) n = 1 ;

  glEnable(GL_TEXTURE_2D);

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

  glDisable(GL_TEXTURE_2D);
}

/* Energy meter that gets filled with coins */

/* These definitions serve to make Energy Meter easy to tweak */
#define METER_POS_X  590
#define METER_POS_Y  130
#define METER_WIDTH  24
#define METER_HEIGHT 220

// Meter fluid color (0 - 255)
#define METER_TOP_COLOR    230, 0, 0, 210
#define METER_BOTTOM_COLOR 240, 110, 110, 210 

#define METER_BORDER_WIDTH 1
// Meter border color (0.0 - 1.0)
#define METER_BORDER_COLOR 0.0, 0.0, 0.0

void RaceGUI::drawEnergyMeter ( float state )
{
  // Draw a Meter border
  // left side
  glBegin ( GL_QUADS ) ;
  glColor3f ( METER_BORDER_COLOR ) ;
    glVertex2i ( METER_POS_X-METER_BORDER_WIDTH, METER_POS_Y-METER_BORDER_WIDTH ) ;
    glVertex2i ( METER_POS_X,   METER_POS_Y-METER_BORDER_WIDTH ) ;
    glVertex2i ( METER_POS_X,   METER_POS_Y + METER_HEIGHT) ;
    glVertex2i ( METER_POS_X-METER_BORDER_WIDTH, METER_POS_Y + METER_HEIGHT ) ;
  glEnd () ;

  // right side
  glBegin ( GL_QUADS ) ;
  glColor3f ( METER_BORDER_COLOR ) ;
    glVertex2i ( METER_POS_X+METER_WIDTH,   METER_POS_Y-METER_BORDER_WIDTH ) ;
    glVertex2i ( METER_POS_X+METER_WIDTH+METER_BORDER_WIDTH, METER_POS_Y-METER_BORDER_WIDTH ) ;
    glVertex2i ( METER_POS_X+METER_WIDTH+METER_BORDER_WIDTH, METER_POS_Y + METER_HEIGHT) ;
    glVertex2i ( METER_POS_X+METER_WIDTH,   METER_POS_Y + METER_HEIGHT ) ;
  glEnd () ;

  // down side
  glBegin ( GL_QUADS ) ;
  glColor3f ( METER_BORDER_COLOR ) ;
    glVertex2i ( METER_POS_X,    METER_POS_Y-METER_BORDER_WIDTH ) ;
    glVertex2i ( METER_POS_X+METER_WIDTH, METER_POS_Y-METER_BORDER_WIDTH ) ;
    glVertex2i ( METER_POS_X+METER_WIDTH, METER_POS_Y ) ;
    glVertex2i ( METER_POS_X,    METER_POS_Y ) ;
  glEnd () ;

  // up side
  glBegin ( GL_QUADS ) ;
  glColor3f ( METER_BORDER_COLOR ) ;
    glVertex2i ( METER_POS_X,    METER_POS_Y+METER_HEIGHT ) ;
    glVertex2i ( METER_POS_X+METER_WIDTH, METER_POS_Y+METER_HEIGHT ) ;
    glVertex2i ( METER_POS_X+METER_WIDTH, METER_POS_Y+METER_HEIGHT+METER_BORDER_WIDTH ) ;
    glVertex2i ( METER_POS_X,    METER_POS_Y+METER_HEIGHT+METER_BORDER_WIDTH ) ;
  glEnd () ;

  // Draw the Meter fluid
  glBegin ( GL_QUADS ) ;
  glColor4ub ( METER_TOP_COLOR ) ;
    glVertex2i ( METER_POS_X,    METER_POS_Y ) ;
    glVertex2i ( METER_POS_X+METER_WIDTH, METER_POS_Y ) ;

  glColor4ub ( METER_BOTTOM_COLOR ) ;
    glVertex2i ( METER_POS_X+METER_WIDTH, METER_POS_Y + (int)(state * METER_HEIGHT));
    glVertex2i ( METER_POS_X,    METER_POS_Y + (int)(state * METER_HEIGHT) ) ;
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
  glAlphaFunc    ( GL_GREATER, 0.1 ) ;
  glEnable       ( GL_BLEND      ) ;

  glOrtho        ( 0, 640, 0, 480, 0, 100 ) ;

  switch (World::current()->ready_set_go)
    {
    case 2:
      glColor3ub ( 230, 170, 160 ) ;
      drawText ( "Ready!", 65, 370-(3*65), 260 ) ;
      break;
    case 1:
      glColor3ub ( 230, 230, 160 ) ;
      drawText ( "Set!", 65, 370-(2*65), 260 ) ;
      break;
    case 0:
      glColor3ub ( 100, 210, 100 ) ;
      drawText ( "Go!", 65, 370-(int)(1.5*65), 260 ) ;
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
      drawEnergyMeter ( (float)(World::current()->getPlayerKart(0)->getNumHerring()) /
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
