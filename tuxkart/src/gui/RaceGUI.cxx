//  $Id: RaceGUI.cxx,v 1.43 2004/09/07 12:44:38 jamesgregory Exp $
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
#include "../widget_image.h"
#include "Loader.h"
#include "RaceSetup.h"
#include <iostream>

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
next_string(0)
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

  nCachedTextures = World::current()->raceSetup.getNumPlayers() * TEXTURES_PER_PLAYER;

	if ((fps_id = widgetSet -> count(0, 1000, GUI_SML, GUI_SE)))
		widgetSet -> layout(fps_id, -1, 1);
}

RaceGUI::~RaceGUI()
{
	widgetSet -> delete_widget(fps_id) ;

	for(FontsCache::iterator i = fonts_cache.begin(); i != fonts_cache.end(); ++i)
		TTF_CloseFont(i->second);

  for (int i = 0; i != nCachedTextures; ++i)
  {
    if (cachedTextures[i].lastUsed != -1)
      glDeleteTextures(1, &(cachedTextures[i].texture));
  }

	//FIXME: does all that material stuff need freeing somehow?
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

void RaceGUI::drawText ( const char *text, int sz, int x, int y, int red, int green, int blue, float scale_x, float scale_y )
{
  TextTexture* glTexture = cacheTexture(text, sz);

  int w = (int)(glTexture->w * scale_x);
  int h = (int)(glTexture->h * scale_y);

  if(x == SCREEN_CENTERED_TEXT)
    x = (640 - w) / 2;
  if(y == SCREEN_CENTERED_TEXT)
    y = (480 - h) / 2;

  drawTexture(glTexture->texture, w, h, red, green, blue, x, y);
}

void RaceGUI::drawInverseDropShadowText ( const char *str, int sz, int x, int y )
{
  drawText ( str, sz, x, y, 255, 255, 255 ) ;
  drawText ( str, sz, x+1, y+1, 0, 0, 0 ) ;
}

void RaceGUI::drawDropShadowText (const char *str, int sz, int x, int y )
{
  drawText ( str, sz, x, y, 0, 0, 0 ) ;
  drawText ( str, sz, x+1, y+1, 255, 255, 255 ) ;
}

void RaceGUI::drawTexture(const GLuint texture, int w, int h, int red, int green, int blue, int x, int y)
{
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, texture);

  glColor3ub ( red, green, blue ) ;
  glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2f(x, (float)h+y);

    glTexCoord2f(1, 0);
    glVertex2f((float)w+x, (float)h+y);

    glTexCoord2f(1, 1);
    glVertex2f((float)w+x, y);

    glTexCoord2f(0, 1);
    glVertex2f(x, y);
  glEnd();

  glDisable(GL_TEXTURE_2D);
}

/** This is a chache system to avoid TTF_Font calls.
      Though I dunno if this is really needed, since I dunno how expensive
      such call is. */
void RaceGUI::cacheFont(int sz)
{
	if(fonts_cache.find(sz) == fonts_cache.end()) // font not yet cached
	{
		#ifdef DEBUG
			std::cout << "RaceGUI: Caching font size: " << sz << std::endl;
		#endif

		std::string path = loader->getPath(GUI_FACE);
		TTF_Font* font = TTF_OpenFont(path.c_str(), sz);  // freeing will be done on exit

		fonts_cache[sz] = font;
	} 
}

TextTexture* RaceGUI::cacheTexture(const char* text, int sz)
{
  for (int i = 0; i != nCachedTextures; ++i)
  {
    if (cachedTextures[i].text == text && cachedTextures[i].sz == sz)
    {
      cachedTextures[i].lastUsed = SDL_GetTicks();
      return &(cachedTextures[i]);
    }
  }

  //not found, going to have to load a new texture

  cacheFont(sz);

  int replace = 0;
  for (int i = 1; i != nCachedTextures; ++i)
  {
    if (cachedTextures[i].lastUsed < cachedTextures[replace].lastUsed)
      replace = i;
  }

  if (cachedTextures[replace].lastUsed != -1)
    glDeleteTextures(1, &(cachedTextures[replace].texture));  

  cachedTextures[replace].texture = make_image_from_font(NULL, NULL, &(cachedTextures[replace].w), &(cachedTextures[replace].h), text, fonts_cache.find(sz)->second);
  
  cachedTextures[replace].text = text;
  cachedTextures[replace].sz = sz;
  cachedTextures[replace].lastUsed = SDL_GetTicks();

  return &(cachedTextures[replace]);
}

void RaceGUI::drawTimer ()
{
  char str [ 256 ] ;

  time_left = World::current()->clock;

  int min     = (int) floor ( time_left / 60.0 ) ;
  int sec     = (int) floor ( time_left - (double) ( 60 * min ) ) ;
  int tenths  = (int) floor ( 10.0f * (time_left - (double)(sec + 60*min)));

  sprintf ( str, "%3d:%02d\"%d", min,  sec,  tenths ) ;
  drawDropShadowText ( "Time:", 28, 500, 430 ) ;
  drawDropShadowText ( str, 36, 460, 400) ;
}

void RaceGUI::drawScore (const RaceSetup& raceSetup, int player_nb, int offset_x, int offset_y, float ratio_x, float ratio_y)
{
  char str [ 256 ] ;

  KartDriver* player_kart = World::current()->getPlayerKart(player_nb);

#ifdef DEBUG
  /* Show velocity */
  if ( player_kart->getVelocity()->xyz[1] < 0 )
    sprintf ( str, "Reverse" ) ;
  else
    sprintf(str,"%3dmph",(int)(player_kart->getVelocity()->xyz[1]/MILES_PER_HOUR));

  drawDropShadowText ( str, (int)(18*ratio_y), (int)((640-((strlen(str)-1)*18))*ratio_x)+offset_x, offset_y ) ;
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
      sprintf ( str, "Last Lap!" );
    else
      sprintf ( str, "%s",
        pos_string [ player_kart->getPosition() ] ) ;
  }

  drawDropShadowText ( str, (int)(38*ratio_y), (int)(10*ratio_x)+offset_x, (int)(422*ratio_y)+offset_y ) ;

  /* Show player's position */
  sprintf ( str, "%s", pos_string [ player_kart->getPosition() ] ) ;
  drawDropShadowText ( str, (int)(74*ratio_y), (int)(16*ratio_x)+offset_x, (int)(2*ratio_y)+offset_y );
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

    glColor3fv ( World::current()->getKart(i)->getKartProperties()->color ) ;

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

  /* Calculate a color. This will result in an animation effect. */
  int red   = (int)(255 * sin ( (float)timer/5.1f ) / 2.0f + 0.5f);
  int green = (int)(255 * (sin ( (float)timer/6.3f ) / 2.0f + 0.5f));
  int blue  = (int)(255 * sin ( (float)timer/7.2f ) / 2.0f + 0.5f);

  if ( finishing_position < 0 )
    finishing_position = World::current()->getPlayerKart(0)->getPosition() ;

  if ( finishing_position > 1 )
  {
    drawText ( "YOU FINISHED"    , 90, 130, 210, red, green, blue ) ;
    drawText ( pos_string [ finishing_position ], 90, 130, 210, red, green, blue ) ;
  }
  else
  {
    drawText ( "CONGRATULATIONS"  , 90, 130, 210, red, green, blue ) ;
    drawText ( "YOU WON THE RACE!", 90, 130, 210, red, green, blue ) ;
  }
}


void RaceGUI::drawPlayerIcons ()
{
  /** Draw players position on the race */

  int x = 10;
  int y;

  for(int i = 0; i < World::current()->getNumKarts() ; i++)
    {
      int position = World::current()->getKart(i)->getPosition();
      if(position > 4)  // only draw the first four karts
        continue;

      y = 310 - ((position-1)*(55+5));

      // draw icon
      Material* players_gst =
          World::current()->getKart(i)->getKartProperties()->getIconMaterial();
      players_gst -> apply ();

      glEnable(GL_TEXTURE_2D);
      glBegin ( GL_QUADS ) ;
      glColor4f    ( 1, 1, 1, 1 ) ;

      glTexCoord2f ( 0, 0 ) ; glVertex2i ( x   , y    ) ;
      glTexCoord2f ( 1, 0 ) ; glVertex2i ( x+55, y    ) ;
      glTexCoord2f ( 1, 1 ) ; glVertex2i ( x+55, y+55 ) ;
      glTexCoord2f ( 0, 1 ) ; glVertex2i ( x   , y+55 ) ;
      glEnd () ;
      glDisable(GL_TEXTURE_2D);

      // draw text
      drawDropShadowText ( pos_string[position], 28, 55+x, y+10 ) ;
    }

}


void RaceGUI::drawEmergencyText ( int player_nb, int offset_x, int offset_y, float ratio_x, float ratio_y )
{
  static float wrong_timer = 0.0f ;
  static float last_dist = -1000000.0f ;
  static int last_lap = -1 ;

  float d = World::current()->getPlayerKart(player_nb)-> getDistanceDownTrack () ;
  int   l = World::current()->getPlayerKart(player_nb)-> getLap () ;

  if ( ( l < last_lap || ( l == last_lap && d < last_dist ) ) &&
       World::current()->getPlayerKart(player_nb) -> getVelocity () -> xyz [ 1 ] > 0.0f )
  {
    wrong_timer += 0.05f; // FIXME: was World::current()->clock -> getDeltaTime () ;

    if ( wrong_timer > 2.0f )
    {
      static int i = FALSE ;

      int red, green, blue;
      if ( i )
        {
        red = blue = 255;
        green = 0;
        }
      else
        {
        red = blue = 0;
        green = 255;
        }

      drawText ( "WRONG WAY!", (int)(90*ratio_y), (int)(130*ratio_x)+offset_x,
                               (int)(210*ratio_y)+offset_y, red, green, blue ) ;
      if ( ! i )
        {
        red = blue = 255;
        green = 0;
        }
      else
        {
        red = blue = 0;
        green = 255;
        }

      drawText ( "WRONG WAY!", (int)(90*ratio_y), (int)((130+2)*ratio_x)+offset_x,
                               (int)((210+2)*ratio_y)+offset_y, red, green, blue ) ;

      i = ! i ;
    }
  }
  else
    wrong_timer = 0.0f ;

  last_dist = d ;
  last_lap  = l ;
}


void RaceGUI::drawCollectableIcons ( int player_nb, int offset_x, int offset_y, float ratio_x, float ratio_y )
{
  int zz = FALSE ;

  switch ( World::current()->getPlayerKart(player_nb)->getCollectable () )
  {
    case COLLECT_NOTHING        : break ;
    case COLLECT_SPARK          : spark_gst        -> apply () ; break ;
    case COLLECT_MISSILE        : missile_gst      -> apply () ; break ;
    case COLLECT_HOMING_MISSILE : flamemissile_gst -> apply () ; break ;
    case COLLECT_MAGNET         : magnet_gst       -> apply () ; break ;
    case COLLECT_ZIPPER         : zipper_gst       -> apply () ;
                                  zz = TRUE ; break ;
  }

  int x1 = (int)((320-32) * ratio_x) + offset_x ;
  int y1 = (int)(400 * ratio_y)      + offset_y;

  glDisable(GL_TEXTURE_2D);

  glBegin ( GL_QUADS ) ;
    glColor4f ( 0.0, 0.0, 0.0, 0.16 ) ;
    glVertex2i ( x1                  , y1    ) ;
    glVertex2i ( x1+(int)(64*ratio_x), y1    ) ;
    glVertex2i ( x1+(int)(64*ratio_x), y1+(int)(64*ratio_y) ) ;
    glVertex2i ( x1                  , y1+(int)(64*ratio_y) ) ;
  glEnd();

  // If player doesn't have anything, just let the transparent black square
  if(World::current()->getPlayerKart(player_nb)->getCollectable () == COLLECT_NOTHING)
    return;

  int n  = World::current()->getPlayerKart(player_nb)->getNumCollectables() ;

  if ( n > 5 ) n = 5 ;
  if ( n < 1 ) n = 1 ;

  glEnable(GL_TEXTURE_2D);

  glBegin ( GL_QUADS ) ;
    glColor4f    ( 1, 1, 1, 1 ) ;

    for ( int i = 0 ; i < n ; i++ )
    {
      if ( zz )
      {
	glTexCoord2f ( 0, 2 ) ; glVertex2i ( i*40 + x1                  , y1    ) ;
	glTexCoord2f ( 0, 0 ) ; glVertex2i ( i*40 + x1+(int)(32*ratio_x), y1    ) ;
	glTexCoord2f ( 2, 0 ) ; glVertex2i ( i*40 + x1+(int)(64*ratio_x), y1+(int)(32*ratio_y) ) ;
	glTexCoord2f ( 2, 2 ) ; glVertex2i ( i*40 + x1+(int)(32*ratio_x), y1+(int)(32*ratio_y) ) ;

	glTexCoord2f ( 0, 2 ) ; glVertex2i ( i*40 + x1+(int)(32*ratio_x), y1+(int)(32*ratio_y) ) ;
	glTexCoord2f ( 0, 0 ) ; glVertex2i ( i*40 + x1+(int)(64*ratio_x), y1+(int)(32*ratio_y) ) ;
	glTexCoord2f ( 2, 0 ) ; glVertex2i ( i*40 + x1+(int)(32*ratio_x), y1+(int)(64*ratio_y) ) ;
	glTexCoord2f ( 2, 2 ) ; glVertex2i ( i*40 + x1                  , y1+(int)(64*ratio_y) ) ;
      }
      else
      {
	glTexCoord2f ( 0, 0 ) ; glVertex2i ( i*30 + x1                  , y1    ) ;
	glTexCoord2f ( 1, 0 ) ; glVertex2i ( i*30 + x1+(int)(64*ratio_x), y1    ) ;
	glTexCoord2f ( 1, 1 ) ; glVertex2i ( i*30 + x1+(int)(64*ratio_x), y1+(int)(64*ratio_y) ) ;
	glTexCoord2f ( 0, 1 ) ; glVertex2i ( i*30 + x1                  , y1+(int)(64*ratio_y) ) ;
      }
    }
  glEnd () ;

  glDisable(GL_TEXTURE_2D);
}

/* Energy meter that gets filled with coins */

// Meter fluid color (0 - 255)
#define METER_TOP_COLOR    230, 0, 0, 210
#define METER_BOTTOM_COLOR 240, 110, 110, 210 
// Meter border color (0.0 - 1.0)
#define METER_BORDER_COLOR 0.0, 0.0, 0.0

void RaceGUI::drawEnergyMeter ( float state, int offset_x, int offset_y, float ratio_x, float ratio_y )
{
  int x = (int)(590 * ratio_x) + offset_x;
  int y = (int)(130 * ratio_y) + offset_y;
  int w = (int)(24 * ratio_x);
  int h = (int)(220 * ratio_y);
  int wl = (int)(1 * ratio_x);
  if(wl < 1)
    wl = 1;

  // Draw a Meter border
  // left side
  glBegin ( GL_QUADS ) ;
  glColor3f ( METER_BORDER_COLOR ) ;
    glVertex2i ( x-wl, y-wl ) ;
    glVertex2i ( x,    y-wl ) ;
    glVertex2i ( x,    y + h) ;
    glVertex2i ( x-wl, y + h ) ;
  glEnd () ;

  // right side
  glBegin ( GL_QUADS ) ;
  glColor3f ( METER_BORDER_COLOR ) ;
    glVertex2i ( x+w,    y-wl ) ;
    glVertex2i ( x+w+wl, y-wl ) ;
    glVertex2i ( x+w+wl, y + h) ;
    glVertex2i ( x+w,    y + h ) ;
  glEnd () ;

  // down side
  glBegin ( GL_QUADS ) ;
  glColor3f ( METER_BORDER_COLOR ) ;
    glVertex2i ( x,   y-wl ) ;
    glVertex2i ( x+w, y-wl ) ;
    glVertex2i ( x+w, y ) ;
    glVertex2i ( x,   y ) ;
  glEnd () ;

  // up side
  glBegin ( GL_QUADS ) ;
  glColor3f ( METER_BORDER_COLOR ) ;
    glVertex2i ( x,   y+h ) ;
    glVertex2i ( x+w, y+h ) ;
    glVertex2i ( x+w, y+h+wl ) ;
    glVertex2i ( x,   y+h+wl ) ;
  glEnd () ;

  // Draw the Meter fluid
  glBegin ( GL_QUADS ) ;
  glColor4ub ( METER_TOP_COLOR ) ;
    glVertex2i ( x,   y ) ;
    glVertex2i ( x+w, y ) ;

  glColor4ub ( METER_BOTTOM_COLOR ) ;
    glVertex2i ( x+w, y + (int)(state * h));
    glVertex2i ( x,   y + (int)(state * h) ) ;
  glEnd () ;
}


void RaceGUI::drawStatusText (const RaceSetup& raceSetup)
{
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
      drawText ( "Ready!", 80, SCREEN_CENTERED_TEXT, SCREEN_CENTERED_TEXT, 230, 170, 160 ) ;
      break;
    case 1:
      drawText ( "Set!", 80, SCREEN_CENTERED_TEXT, SCREEN_CENTERED_TEXT, 230, 230, 160 ) ;
      break;
    case 0:
      drawText ( "Go!", 80, SCREEN_CENTERED_TEXT, SCREEN_CENTERED_TEXT, 100, 210, 100 ) ;
      break;
    }

  float split_screen_ratio_x, split_screen_ratio_y;
  split_screen_ratio_x = split_screen_ratio_y = 1.0;
  if(raceSetup.getNumPlayers() >= 2)
    split_screen_ratio_y = 0.5;
  if(raceSetup.getNumPlayers() >= 3)
    split_screen_ratio_x = 0.5;

  if ( World::current()->getPhase() == World::FINISH_PHASE )
    {
      drawGameOverText     () ;
    }
  else
    {
      for(int pla = 0; pla < raceSetup.getNumPlayers(); pla++)
        {
        int offset_x, offset_y;
        offset_x = offset_y = 0;
        if((pla == 0 && raceSetup.getNumPlayers() > 1) ||
           pla == 2)
          offset_y = 240;
        if((pla == 2 || pla == 3) && raceSetup.getNumPlayers() > 2)
          offset_x = 320;

        drawCollectableIcons ( pla, offset_x, offset_y,
                               split_screen_ratio_x, split_screen_ratio_y ) ;
        drawEnergyMeter ( (float)(World::current()->getPlayerKart(pla)->getNumHerring()) /
                               MAX_HERRING_EATEN, offset_x, offset_y,
                               split_screen_ratio_x, split_screen_ratio_y ) ;
        drawScore       ( raceSetup, pla, offset_x, offset_y,
                               split_screen_ratio_x, split_screen_ratio_y ) ;
        drawEmergencyText    ( pla, offset_x, offset_y,
                               split_screen_ratio_x, split_screen_ratio_y ) ;
        }

      if(raceSetup.getNumPlayers() == 1)
        drawPlayerIcons      () ;

      drawTimer () ;
      drawMap              () ;
    }

  glPopAttrib  () ;
  glPopMatrix  () ;
  glMatrixMode ( GL_MODELVIEW ) ;
  glPopMatrix  () ;
}

/* EOF */
