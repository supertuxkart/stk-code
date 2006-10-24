//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006 Joerg Henrichs, SuperTuxKart-Team, Steve Baker
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

#include <SDL/SDL.h>
#include "race_gui.hpp"
#include "history.hpp"
#include "widget_set.hpp"
#include "world.hpp"
#include "track.hpp"
#include "material_manager.hpp"
#include "menu_manager.hpp"
#include "sdldrv.hpp"

RaceGUI::RaceGUI(): time_left(0.0) {
  if(!config->profile) {
    UpdateKeyboardMappings();
  }   // if !config->profile

  lapLeader     = -1;
  timeOfLeader  = -1.0f;
  xOffForText   = (int)(config->width-220*config->width/800.0f);
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

  // Temporary, we need a better icon here
  SteeringWheelIcon = material_manager->getMaterial("wheel.rgb");
  SteeringWheelIcon->getState()->disable(GL_CULL_FACE);

  for(int i = 0;i < 11;i++) {
    char str [ 256 ];
    sprintf(str,"%d.rgb",i);
    NumberIcons[i] = material_manager->getMaterial(str);
    NumberIcons[i]->getState()->disable(GL_CULL_FACE);
  }

  StIcon = material_manager->getMaterial("st.rgb");
  StIcon->getState()->disable(GL_CULL_FACE);
  NdIcon = material_manager->getMaterial("nd.rgb");
  NdIcon->getState()->disable(GL_CULL_FACE);
  RdIcon = material_manager->getMaterial("rd.rgb");
  RdIcon->getState()->disable(GL_CULL_FACE);
  ThIcon = material_manager->getMaterial("th.rgb");
  ThIcon->getState()->disable(GL_CULL_FACE);
  SlashIcon = material_manager->getMaterial("slash.rgb");
  SlashIcon->getState()->disable(GL_CULL_FACE);
  MinusIcon = material_manager->getMaterial("minus.rgb");
  MinusIcon->getState()->disable(GL_CULL_FACE);
  LapIcon = material_manager->getMaterial("lap.rgb");
  LapIcon->getState()->disable(GL_CULL_FACE);

  fpsCounter = 0;
  fpsString[0]=0;
  fpsTimer.reset();
  fpsTimer.update();
  fpsTimer.setMaxDelta(1000);
  
}   // RaceGUI

// -----------------------------------------------------------------------------
RaceGUI::~RaceGUI() {
	//FIXME: does all that material stuff need freeing somehow?
}   // ~Racegui

// -----------------------------------------------------------------------------
void RaceGUI::UpdateKeyboardMappings() {
  // Clear all entries.
  for(int type = 0;type< (int) IT_LAST+1;type++)
    for(int id0=0;id0<MAX_ID0;id0++)
      for(int id1=0;id1<MAX_ID1;id1++)
        for(int id2=0;id2<MAX_ID2;id2++)
          inputMap[type][id0][id1][id2].kart = NULL;


  // Defines the mappings for player keys to kart and action	
  // To avoid looping over all players to find out what
  // player control key was pressed, a special data structure 
  // is set up: keysToKArt contains for each (player assigned) 
  // key which kart it applies to (and therefore which player),
  // and typeForKey contains the assigned function of that key.
  int num=world->raceSetup.getNumPlayers();
  for(int i=0; i<num; i++)
  {
    PlayerKart* kart = world->getPlayerKart(i);

    for(int ka=(int) KC_LEFT;ka< (int) KC_FIRE+1;ka++)
      putEntry(kart, (KartActions) ka);
  }

}   // UpdateKeyControl

void RaceGUI::putEntry(PlayerKart *kart, KartActions kc)
{
  Player *p = kart->getPlayer();
  Input *i  = p->getInput(kc);

  inputMap[i->type][i->id0][i->id1][i->id2].kart = kart;
  inputMap[i->type][i->id0][i->id1][i->id2].action = kc;
}

bool RaceGUI::handleInput(InputType type, int id0, int id1, int id2, int value)
{
  PlayerKart *k = inputMap[type][id0][id1][id2].kart;

  if (k)
    {
      k->action(inputMap[type][id0][id1][id2].action, value);
      return true;
    }
  else
   return false;
}

// -----------------------------------------------------------------------------
void RaceGUI::update(float dt) {
  assert(world != NULL);
  drawStatusText(world->raceSetup, dt);
}   // update

void RaceGUI::input(InputType type, int id0, int id1, int id2, int value)
{
  switch (type)
  {
    case IT_KEYBOARD:
      if (!handleInput(type, id0, id1, id2, value))
        inputKeyboard(id0, value);
      break;
    default:
      handleInput(type, id0, id1, id2, value);
      break;
  }

}

// -----------------------------------------------------------------------------
void RaceGUI::inputKeyboard(int key, int pressed) {
  if (!pressed)
    return;

  static int isWireframe = FALSE ;
  switch ( key ) {
    case 0x12: // TODO: Which key is that?
      if(world->raceSetup.getNumPlayers()==1) {   // ctrl-r
        Kart* kart = world->getPlayerKart(0);
        kart->setCollectable((rand()%2)?COLLECT_MISSILE :COLLECT_HOMING_MISSILE, 10000);
      }
      break;
    case SDLK_F12:
      config->displayFPS = !config->displayFPS;
	  if(config->displayFPS) {
        fpsTimer.reset();
        fpsTimer.setMaxDelta(1000);
        fpsCounter=0;
      }
      break;
    case SDLK_F11:
      glPolygonMode(GL_FRONT_AND_BACK, isWireframe ? GL_FILL : GL_LINE);
      isWireframe = ! isWireframe;
      break;
#ifndef WIN32
      // For now disable F9 toggling fullscreen, since windows requires
      // to reload all textures, display lists etc. Fullscreen can
      // be toggled from the main menu (options->display).
    case SDLK_F9:
      drv_toggleFullscreen(0);   // 0: do not reset textures
      // Fall through to put the game into pause mode.
#endif
    case SDLK_ESCAPE: // ESC
      widgetSet->tgl_paused();
      menu_manager->pushMenu(MENUID_RACEMENU);
      // The player might have changed the keyboard 
      // configuration, so we need to redefine the mappings
      if(!config->profile) UpdateKeyboardMappings();
      break;
    case SDLK_F10:
      history->Save();
      break;
    default:
      break;
    } // switch
} // keybd

// -----------------------------------------------------------------------------
void RaceGUI::drawFPS () {
  if (++fpsCounter>=50) {
    fpsTimer.update();
    sprintf(fpsString, "%d",(int)(fpsCounter/fpsTimer.getDeltaTime()));
    fpsCounter = 0;
    fpsTimer.setMaxDelta(1000);
  }    
  widgetSet->drawText (fpsString, 36, 0, config->height-50, 255, 255, 255 ) ;
}   // drawFPS

// -----------------------------------------------------------------------------
void RaceGUI::drawInverseDropShadowText ( const char *str, int sz, 
					  int x, int y              ) {
  widgetSet->drawText ( str, sz, x, y, 255, 255, 255 ) ;
  widgetSet->drawText ( str, sz, x+1, y+1, 0, 0, 0 ) ;
}   // drawInverseDropShadowText

// -----------------------------------------------------------------------------
void RaceGUI::drawDropShadowText (const char *str, int sz, 
				  int x, int y, int red, int green, int blue) {
  widgetSet->drawText ( str, sz, x, y, 0, 0, 0 ) ;
  widgetSet->drawText ( str, sz, x+1, y+1, red, green, blue ) ;
}  // drawDropShadowText

// -----------------------------------------------------------------------------
#if 0
//This is not being used..
void RaceGUI::drawTexture(const GLuint texture, int w, int h, 
			  int red, int green, int blue, int x, int y) {
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
}   // drawTexture
#endif
// -----------------------------------------------------------------------------
void RaceGUI::drawTimer () {
  if(world->getPhase()!=World::RACE_PHASE         &&
     world->getPhase()!=World::DELAY_FINISH_PHASE   ) return;
  char str [ 256 ] ;

  assert(world != NULL);
  time_left = world->clock;

  int min     = (int) floor ( time_left / 60.0 ) ;
  int sec     = (int) floor ( time_left - (double) ( 60 * min ) ) ;
  int tenths  = (int) floor ( 10.0f * (time_left - (double)(sec + 60*min)));

  sprintf ( str, "%d:%02d\"%d", min,  sec,  tenths ) ;
  drawDropShadowText ( str, 36, xOffForText, config->height-80) ;
}   // drawTimer

// -----------------------------------------------------------------------------
void RaceGUI::drawScore (const RaceSetup& raceSetup, Kart* player_kart,
			 int offset_x, int offset_y, float ratio_x, 
			 float ratio_y                                 ) {
  char str [ 256 ] ;

  /* Show velocity */
  if ( player_kart->getVelocity()->xyz[1] < 0 )
    sprintf ( str, "Reverse" ) ;
  else {
    if(config->useKPH) {
      sprintf(str,"%d km/h",
	      (int)(player_kart->getVelocity()->xyz[1]/KILOMETERS_PER_HOUR));
    } else {
      sprintf(str,"%d mph",
	      (int)(player_kart->getVelocity()->xyz[1]/MILES_PER_HOUR));
    }   // use KPH
  }   // velocity<0

  int red=255, green=255, blue=255;
  if(!player_kart->isOnGround()) {
    green=0; blue=0;
  }
  drawDropShadowText ( str, (int)(36*ratio_y), 
		       (int)(offset_x+xOffForText         *ratio_x),
		       (int)(offset_y+(config->height-200)*ratio_y),
		       red, green, blue);

  /* Show lap number */
/*  int lap = player_kart->getLap();
  if ( lap < 0 ) {
    sprintf ( str, "Lap:0/%d", raceSetup.numLaps ) ;
  }  else if ( lap < raceSetup.numLaps - 1 ) {
    sprintf ( str, "Lap:%d/%d",
	      lap + 1, raceSetup.numLaps ) ;
  } else if ( lap == raceSetup.numLaps - 1 ) {
    sprintf ( str, "Last lap!" );
  } else {
    sprintf ( str, "Finished!" );
  }

  drawDropShadowText ( str, (int)(38*ratio_y), 
		       (int)(offset_x+xOffForText         *ratio_x),
		       (int)(offset_y+(config->height-250)*ratio_y) );
*/}   // drawScore

// -----------------------------------------------------------------------------
#define TRACKVIEW_SIZE 100

void RaceGUI::drawMap () {
  glDisable ( GL_TEXTURE_2D ) ;
  glColor3f ( 0,0,1 ) ;
  assert(world != NULL);
  int xLeft = 10;
  int yTop   =  10;

  world -> track -> draw2Dview ( xLeft,   yTop   );
  glColor3f ( 1,1,0 ) ;
  world -> track -> draw2Dview ( xLeft+1, yTop+1 );

  glBegin ( GL_QUADS ) ;

  for ( int i = 0 ; i < world->getNumKarts() ; i++ ) {
    sgCoord *c ;

    Kart* kart = world->getKart(i);
    glColor3fv ( *kart->getColor());
    c          = kart->getCoord () ;

    /* If it's a player, draw a bigger sign */
    if (kart -> isPlayerKart ()) {
      world -> track->glVtx ( c->xyz, xLeft+3, yTop+3);
      world -> track->glVtx ( c->xyz, xLeft-2, yTop+3);
      world -> track->glVtx ( c->xyz, xLeft-2, yTop-2);
      world -> track->glVtx ( c->xyz, xLeft+3, yTop-2);
/*      world -> track->glVtx ( c->xyz, xLeft  , yTop-4);
      world -> track->glVtx ( c->xyz, xLeft+4, yTop  );
      world -> track->glVtx ( c->xyz, xLeft  , yTop+4);
      world -> track->glVtx ( c->xyz, xLeft-4, yTop  ); */
    }
    else {
      world -> track->glVtx ( c->xyz, xLeft+2, yTop+2);
      world -> track->glVtx ( c->xyz, xLeft-1, yTop+2);
      world -> track->glVtx ( c->xyz, xLeft-1, yTop-1);
      world -> track->glVtx ( c->xyz, xLeft+2, yTop-1);
    }
  }

  glEnd () ;
  glEnable ( GL_TEXTURE_2D ) ;
}   // drawMap

// -----------------------------------------------------------------------------
void RaceGUI::drawGameOverText (const float dt) {
  static float timer = 0 ;

  /* Calculate a color. This will result in an animation effect. */
  int red   = (int)(255 * sin ( (float)timer/5.1f ) / 2.0f + 0.5f);
  int green = (int)(255 * (sin ( (float)timer/6.3f ) / 2.0f + 0.5f));
  int blue  = (int)(255 * sin ( (float)timer/7.2f ) / 2.0f + 0.5f);
  timer += dt;

  assert(world != NULL);
  int finishing_position = world->getPlayerKart(0)->getPosition();

  if ( finishing_position > 1 ) {
    char s[255];
    sprintf(s,"YOU FINISHED %s",pos_string[finishing_position]);
    widgetSet->drawText ( s  , 50, 130, 300, red, green, blue ) ;
  } else {
    widgetSet->drawText ( "CONGRATULATIONS"  , 50, 130, 300, red, green, blue ) ;
    widgetSet->drawText ( "YOU WON THE RACE!", 50, 130, 210, red, green, blue ) ;
  }
}   // drawGameOverText

// -----------------------------------------------------------------------------

// Draw players position on the race
void RaceGUI::drawPlayerIcons () {
  assert(world != NULL);

  int x = 5;
  int y;
#define ICON_WIDTH 40
#define ICON_PLAYER_WIDHT 50
#define ICON_POS_WIDTH 28

  //glEnable(GL_TEXTURE_2D);
  Material *last_players_gst = 0;
  int bFirst =1;
  for(int i = 0; i < world->getNumKarts() ; i++) {
      Kart* kart   = world->getKart(i);
      int position = kart->getPosition();
      int lap      = kart->getLap();

      y = config->height*3/4-20 - ((position-1)*(ICON_PLAYER_WIDHT+2));

      // draw text
      int red=255, green=255, blue=255;
      int numLaps = world->raceSetup.numLaps;
      if(lap>=numLaps) {  // kart is finished, display in green
        red=0; blue=0;
      } else if(lap>=0 && numLaps>1) {
        green = blue  = 255-(int)((float)lap/((float)numLaps-1.0f)*255.0f);
      }

      glDisable(GL_CULL_FACE);
      if(lap>lapLeader) {
        lapLeader    = lap;
        timeOfLeader = world->clock;
      }

      if(lapLeader>0 &&    // Display position during first lap
         position!=1  &&    // Display position for leader
         (world->clock - kart->getTimeAtLap()<5.0f ||
         lap!=lapLeader)) {  // Display for 5 seconds
        float timeBehind;
        timeBehind = (lap==lapLeader ? kart->getTimeAtLap() : world->clock)
                   - timeOfLeader;
        int min     = (int) floor ( timeBehind / 60.0 ) ;
        int sec     = (int) floor ( timeBehind - (double) ( 60 * min ) ) ;
        int tenths  = (int) floor ( 10.0f * (timeBehind - (double)(sec + 60*min)));
        char str[256];
        sprintf ( str, "%d:%02d\"%d", min,  sec,  tenths ) ;
        drawDropShadowText(str, 20, ICON_PLAYER_WIDHT+x, y+5, 
           red, green, blue);
      }

      glEnable(GL_CULL_FACE);

      bFirst = 0;
      // draw icon
      Material* players_gst = kart->getKartProperties()->getIconMaterial();
      // Hmm - if the same icon is displayed more than once in a row,
      // plib does only do the first setTexture, therefore nothing is
      // displayed for the remaining icons. So we have to call force() if
      // the same icon is displayed more than once in a row.
      if(last_players_gst==players_gst) {
        players_gst->getState()->force();
      }
      //The material of the icons should not have a non-zero alpha_ref value,
      //because if so the next call can make the text look aliased.
      players_gst -> apply ();
      last_players_gst = players_gst;
      glBegin ( GL_QUADS ) ;
        glColor4f    ( 1, 1, 1, 1 ) ;
        if (kart -> isPlayerKart ()) {
          glTexCoord2f ( 0, 0 ) ; glVertex2i ( x                  , y                   ) ;
          glTexCoord2f ( 1, 0 ) ; glVertex2i ( x+ICON_PLAYER_WIDHT, y                   ) ;
          glTexCoord2f ( 1, 1 ) ; glVertex2i ( x+ICON_PLAYER_WIDHT, y+ICON_PLAYER_WIDHT ) ;
          glTexCoord2f ( 0, 1 ) ; glVertex2i ( x                  , y+ICON_PLAYER_WIDHT ) ;
        }
        else {
          glTexCoord2f ( 0, 0 ) ; glVertex2i ( x           , y            ) ;
          glTexCoord2f ( 1, 0 ) ; glVertex2i ( x+ICON_WIDTH, y            ) ;
          glTexCoord2f ( 1, 1 ) ; glVertex2i ( x+ICON_WIDTH, y+ICON_WIDTH ) ;
          glTexCoord2f ( 0, 1 ) ; glVertex2i ( x           , y+ICON_WIDTH ) ;
        }
      glEnd () ;

      // draw position (1st, 2nd...)
      NumberIcons[kart->getPosition()]->getState()->force();
      glBegin ( GL_QUADS ) ;
        glTexCoord2f(0, 0);glVertex2i(x-3               , y+3               );
        glTexCoord2f(1, 0);glVertex2i(x-3+ICON_POS_WIDTH, y+3               );
        glTexCoord2f(1, 1);glVertex2i(x-3+ICON_POS_WIDTH, y+3+ICON_POS_WIDTH);
        glTexCoord2f(0, 1);glVertex2i(x-3               , y+3+ICON_POS_WIDTH);
      glEnd () ;

      if (kart->getPosition() == 1)
        StIcon->getState()->force();
      else if (kart->getPosition() == 2)
        NdIcon->getState()->force();
      else if (kart->getPosition() == 3)
        RdIcon->getState()->force();
      else
        ThIcon->getState()->force();
      glBegin ( GL_QUADS ) ;
        glTexCoord2f(0, 0);glVertex2i(x+(int)(ICON_POS_WIDTH*0.6)                 , y+(int)(ICON_POS_WIDTH*0.6)                 );
        glTexCoord2f(1, 0);glVertex2i(x+(int)(ICON_POS_WIDTH*0.6+ICON_POS_WIDTH/2), y+(int)(ICON_POS_WIDTH*0.6)                 );
        glTexCoord2f(1, 1);glVertex2i(x+(int)(ICON_POS_WIDTH*0.6+ICON_POS_WIDTH/2), y+(int)(ICON_POS_WIDTH*0.6+ICON_POS_WIDTH/2));
        glTexCoord2f(0, 1);glVertex2i(x+(int)(ICON_POS_WIDTH*0.6)                 , y+(int)(ICON_POS_WIDTH*0.6+ICON_POS_WIDTH/2));
      glEnd () ;

    }
}   // drawPlayerIcons

// -----------------------------------------------------------------------------
void RaceGUI::drawEmergencyText (Kart* player_kart, int offset_x,
				 int offset_y, float ratio_x, float ratio_y ) {

  float angle_diff = player_kart->getCoord()->hpr[0] - world->track->angle[player_kart->getHint()];
  if(angle_diff > 180.0f) angle_diff -= 360.0f;
  else if (angle_diff < -180.0f) angle_diff += 360.0f;

  // Display a warning message if the kart is going back way (unless
  // the kart has already finished the race).
  if ((angle_diff > 120.0f || angle_diff < -120.0f)   &&
      player_kart->getVelocity () -> xyz [ 1 ] > 0.0  &&
      !player_kart->raceIsFinished()                       ) {
      static int i = FALSE ;

      int red, green, blue;
      if ( i ) {
        red = blue = 255;
        green = 0;
      } else {
        red = blue = 0;
        green = 255;
      }

      widgetSet->drawText ( "WRONG WAY!", (int)(50*ratio_x), 
			    (int)(130*ratio_x)+offset_x,
			    (int)(210*ratio_y)+offset_y, red, green, blue ) ;
      if ( ! i ) {
        red = blue = 255;
        green = 0;
      } else {
        red = blue = 0;
        green = 255;
      }

      widgetSet->drawText ( "WRONG WAY!", (int)(50*ratio_x), 
			    (int)((130+2)*ratio_x)+offset_x,
			    (int)((210+2)*ratio_y)+offset_y, red, green, blue ) ;

      i = ! i ;
  }
}   //drawEmergencyText

// -----------------------------------------------------------------------------
void RaceGUI::drawCollectableIcons ( Kart* player_kart, int offset_x, 
				     int offset_y, float ratio_x, 
				     float ratio_y                    ) {
  // Originally the hardcoded sizes were 320-32 and 400
  int x1 = (int)((config->width/2-32) * ratio_x) + offset_x ;
  int y1 = (int)(config->height*5/6 * ratio_y)      + offset_y;

  int nSize=(int)(64.0f*std::min(ratio_x, ratio_y));
  // If player doesn't have anything, just let the transparent black square
  Collectable* collectable=player_kart->getCollectable();
  if(collectable->getType() == COLLECT_NOTHING) {
    glDisable(GL_TEXTURE_2D);
    glBegin ( GL_QUADS ) ;
      glColor4f  ( 0.0f, 0.0f, 0.0f, 0.16f          );
      glVertex2i ( x1             , y1              );
      glVertex2i ( x1+(int)(nSize), y1              );
      glVertex2i ( x1+(int)(nSize), y1+(int)(nSize) );
      glVertex2i ( x1             , y1+(int)(nSize) );
    glEnd();
    return;
  }
  collectable->getIcon()->apply();

  int n  = player_kart->getNumCollectables() ;

  if ( n > 5 ) n = 5 ;
  if ( n < 1 ) n = 1 ;

  glBegin(GL_QUADS) ;
    glColor4f(1, 1, 1, 1 );

    for ( int i = 0 ; i < n ; i++ ) {
      glTexCoord2f(0, 0); glVertex2i( i*30 + x1      , y1      );
      glTexCoord2f(1, 0); glVertex2i( i*30 + x1+nSize, y1      );
      glTexCoord2f(1, 1); glVertex2i( i*30 + x1+nSize, y1+nSize);
      glTexCoord2f(0, 1); glVertex2i( i*30 + x1      , y1+nSize);
    }   // for i
  glEnd () ;

}   // drawCollectableIcons

// -----------------------------------------------------------------------------
/* Energy meter that gets filled with coins */

// Meter fluid color (0 - 255)
#define METER_TOP_COLOR    230, 0, 0, 210
#define METER_BOTTOM_COLOR 240, 110, 110, 210 
// Meter border color (0.0 - 1.0)
#define METER_BORDER_COLOR 0.0, 0.0, 0.0

// -----------------------------------------------------------------------------
void RaceGUI::drawEnergyMeter ( Kart *player_kart, int offset_x, int offset_y, 
				float ratio_x, float ratio_y             ) {
  float state = (float)(player_kart->getNumHerring()) /
                        MAX_HERRING_EATEN;
  int x = (int)((config->width-50) * ratio_x) + offset_x;
  int y = (int)(config->height/4 * ratio_y) + offset_y;
  int w = (int)(24 * ratio_x);
  int h = (int)(config->height/2 * ratio_y);
  int wl = (int)(ratio_x);
  if(wl < 1)
    wl = 1;

  glDisable(GL_TEXTURE_2D);
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
  glEnable(GL_TEXTURE_2D);
}   // drawEnergyMeter


// -----------------------------------------------------------------------------
void RaceGUI::drawSteering(Kart* kart, int offset_x, int offset_y,
			   float ratio_x, float ratio_y           ) {

  float minRatio = std::min(ratio_x, ratio_y);
  offset_x += (int)((config->width-220)*ratio_x);
#define WHEELWIDTH 64
  int width  = (int)(WHEELWIDTH*minRatio);
  int height = (int)(WHEELWIDTH*minRatio);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
    glLoadIdentity();
    // for now we display the maximum steering as a 45 degree angle.
    // One the steering angle for all karts are fixed, this should be
    // changed, so that the user gets feedback about how much steering
    // is currently done, since it will vary from kart to kart.
    float displayedAngle = 45.0f * kart->getSteerPercent();

    int tw = width/2; int th = height/2;
    glTranslatef( offset_x+tw,  offset_y+th, 0.0f);
    glRotatef(displayedAngle, 0.0f, 0.0f, 1.0f);
    glTranslatef(-offset_x-tw, -offset_y-th, 0.0f);

    SteeringWheelIcon->getState()->force();
    glBegin ( GL_QUADS ) ;
      glColor4f    ( 1, 1, 1, 1 ) ;
      glTexCoord2f(0, 0);glVertex2i(offset_x      , offset_y       );
      glTexCoord2f(1, 0);glVertex2i(offset_x+width, offset_y       );
      glTexCoord2f(1, 1);glVertex2i(offset_x+width, offset_y+height);
      glTexCoord2f(0, 1);glVertex2i(offset_x      , offset_y+height);
     glEnd () ;

  glPopMatrix();
} // drawSteering

// -----------------------------------------------------------------------------
void RaceGUI::drawPosition(Kart* kart, int offset_x, int offset_y,
			   float ratio_x, float ratio_y           ) {

  float minRatio = std::min(ratio_x, ratio_y);
  offset_x += (int)((config->width-138)*ratio_x);
  offset_y += 0;
#define POSWIDTH 128
  int width  = (int)(POSWIDTH*minRatio);
  int height = (int)(POSWIDTH*minRatio);
  glMatrixMode(GL_MODELVIEW);
    NumberIcons[kart->getPosition()]->getState()->force();
    glBegin ( GL_QUADS ) ;
      glTexCoord2f(0, 0);glVertex2i(offset_x      , offset_y       );
      glTexCoord2f(1, 0);glVertex2i(offset_x+width, offset_y       );
      glTexCoord2f(1, 1);glVertex2i(offset_x+width, offset_y+height);
      glTexCoord2f(0, 1);glVertex2i(offset_x      , offset_y+height);
    glEnd () ;

    if (kart->getPosition() == 1)
      StIcon->getState()->force();
    else if (kart->getPosition() == 2)
      NdIcon->getState()->force();
    else if (kart->getPosition() == 3)
      RdIcon->getState()->force();
    else
      ThIcon->getState()->force();
    glBegin ( GL_QUADS ) ;
      glTexCoord2f(0, 0);glVertex2i(offset_x+(int)(width*0.6)        , offset_y+(int)(height*0.6)         );
      glTexCoord2f(1, 0);glVertex2i(offset_x+(int)(width*0.6+width/2), offset_y+(int)(height*0.6)         );
      glTexCoord2f(1, 1);glVertex2i(offset_x+(int)(width*0.6+width/2), offset_y+(int)(height*0.6+height/2));
      glTexCoord2f(0, 1);glVertex2i(offset_x+(int)(width*0.6)        , offset_y+(int)(height*0.6+height/2));
    glEnd () ;

 } // drawPosition

// -----------------------------------------------------------------------------
void RaceGUI::drawLap(Kart* kart, int offset_x, int offset_y,
			   float ratio_x, float ratio_y           ) {

  float maxRatio = std::max(ratio_x, ratio_y);
  offset_x += (int)(120*ratio_x);
  offset_y += (int)(30*maxRatio);
#define LAPWIDTH 48
  int width  = (int)(LAPWIDTH*maxRatio);
  int height = (int)(LAPWIDTH*maxRatio);
  glMatrixMode(GL_MODELVIEW);

    LapIcon->getState()->force();
    glBegin ( GL_QUADS ) ;
      glTexCoord2f(0, 0);glVertex2i(offset_x                 , offset_y                  );
      glTexCoord2f(1, 0);glVertex2i(offset_x+(int)(1.6*width), offset_y                  );
      glTexCoord2f(1, 1);glVertex2i(offset_x+(int)(1.6*width), offset_y+(int)(1.6*height));
      glTexCoord2f(0, 1);glVertex2i(offset_x                 , offset_y+(int)(1.6*height));
    glEnd () ;

    offset_y -= (int)(LAPWIDTH*0.6*maxRatio);
    offset_x -= (int)(14*maxRatio);
    int lap = kart->getLap();
    if ( lap < 0 )
      NumberIcons[0]->getState()->force();
    else if ( lap >= world->raceSetup.numLaps ) 
      MinusIcon->getState()->force();
    else
      NumberIcons[lap+1]->getState()->force();
    glBegin ( GL_QUADS ) ;
      glTexCoord2f(0, 0);glVertex2i(offset_x      , offset_y       );
      glTexCoord2f(1, 0);glVertex2i(offset_x+width, offset_y       );
      glTexCoord2f(1, 1);glVertex2i(offset_x+width, offset_y+height);
      glTexCoord2f(0, 1);glVertex2i(offset_x      , offset_y+height);
    glEnd () ;

    offset_x += (int)(LAPWIDTH*0.6*maxRatio);
    SlashIcon->getState()->force();
    glBegin ( GL_QUADS ) ;
      glTexCoord2f(0, 0);glVertex2i(offset_x      , offset_y       );
      glTexCoord2f(1, 0);glVertex2i(offset_x+width, offset_y       );
      glTexCoord2f(1, 1);glVertex2i(offset_x+width, offset_y+height);
      glTexCoord2f(0, 1);glVertex2i(offset_x      , offset_y+height);
    glEnd () ;

    offset_x += (int)(LAPWIDTH*0.6*maxRatio);
    if ( lap >= world->raceSetup.numLaps ) 
      MinusIcon->getState()->force();
    else
      NumberIcons[world->raceSetup.numLaps]->getState()->force();
    glBegin ( GL_QUADS ) ;
      glTexCoord2f(0, 0);glVertex2i(offset_x      , offset_y       );
      glTexCoord2f(1, 0);glVertex2i(offset_x+width, offset_y       );
      glTexCoord2f(1, 1);glVertex2i(offset_x+width, offset_y+height);
      glTexCoord2f(0, 1);glVertex2i(offset_x      , offset_y+height);
    glEnd () ;
} // drawLap

// -----------------------------------------------------------------------------
void RaceGUI::drawStatusText (const RaceSetup& raceSetup, const float dt) {
  assert(world != NULL);

  glMatrixMode   ( GL_MODELVIEW ) ;
  glPushMatrix   () ;
  glLoadIdentity () ;

  glMatrixMode   ( GL_PROJECTION ) ;
  glPushMatrix   () ;
  glLoadIdentity () ;

  glPushAttrib   ( GL_ENABLE_BIT | GL_LIGHTING_BIT ) ;
  glDisable      ( GL_DEPTH_TEST   );
  glDisable      ( GL_LIGHTING     );
  glDisable      ( GL_FOG          );
  glDisable      ( GL_CULL_FACE    );
  glEnable       ( GL_ALPHA_TEST   );
  glAlphaFunc    ( GL_GREATER, 0.1f);
  glEnable       ( GL_BLEND        );

  glOrtho        ( 0, config->width, 0, config->height, 0, 100 ) ;
  switch (world->ready_set_go) {
    case 2: widgetSet->drawText ( "Ready!", 80, SCREEN_CENTERED_TEXT, 
				  SCREEN_CENTERED_TEXT, 230, 170, 160 ) ;
            break;
    case 1: widgetSet->drawText ( "Set!", 80, SCREEN_CENTERED_TEXT, 
				  SCREEN_CENTERED_TEXT, 230, 230, 160 ) ;
            break;
    case 0: widgetSet->drawText ( "Go!", 80, SCREEN_CENTERED_TEXT, 
				  SCREEN_CENTERED_TEXT, 100, 210, 100 ) ;
            break;
  }   // switch

  for(int i = 0; i < 10; ++i) {
    if(world->debugtext[i] != "")
      widgetSet->drawText(world->debugtext[i].c_str(), 20, 20, 200 - i*20, 
			  100, 210, 100);
  }
  if(world->getPhase()==World::START_PHASE) {
    for(int i=0; i<raceSetup.getNumPlayers(); i++) {
      if(world->getPlayerKart(i)->earlyStartPenalty()) {
	widgetSet->drawText("Penalty time!!",80, SCREEN_CENTERED_TEXT,
			    200, 200, 10, 10);
      }   // if penalty
    }  // for i < getNumPlayers
  }  // if not RACE_PHASE

  float split_screen_ratio_x, split_screen_ratio_y;
  split_screen_ratio_x = split_screen_ratio_y = 1.0;
  if(raceSetup.getNumPlayers() >= 2)
    split_screen_ratio_y = 0.5;
  if(raceSetup.getNumPlayers() >= 3)
    split_screen_ratio_x = 0.5;

  if ( world->getPhase() == World::FINISH_PHASE ) {
    drawGameOverText(dt) ;
  }   // if FINISH_PHASE
  if ( world->getPhase() == World::RACE_PHASE         ||
       world->getPhase() == World::DELAY_FINISH_PHASE   ) {
    for(int pla = 0; pla < raceSetup.getNumPlayers(); pla++) {
      int offset_x, offset_y;
      offset_x = offset_y = 0;

      if(raceSetup.getNumPlayers() == 2)
      {
          if(pla == 0) offset_y = config->height/2;
      }
      else if(raceSetup.getNumPlayers() > 2)
      {
          if((pla == 0 && raceSetup.getNumPlayers() > 1) || (pla == 1))
              offset_y = config->height/2;

          if((pla == 1) || pla == 3)
              offset_x = config->width/2;
      }

      Kart* player_kart=world->getPlayerKart(pla);
      drawCollectableIcons(player_kart, offset_x, offset_y,
			   split_screen_ratio_x, split_screen_ratio_y );
      drawEnergyMeter     (player_kart, offset_x, offset_y,
			   split_screen_ratio_x, split_screen_ratio_y );
      drawSteering        (player_kart, offset_x, offset_y,
			   split_screen_ratio_x, split_screen_ratio_y );
      drawPosition        (player_kart, offset_x, offset_y,
			   split_screen_ratio_x, split_screen_ratio_y );
      drawLap             (player_kart, offset_x, offset_y,
			   split_screen_ratio_x, split_screen_ratio_y );
      drawScore           (raceSetup, player_kart, offset_x, offset_y,
			   split_screen_ratio_x, split_screen_ratio_y ) ;
      drawEmergencyText   (player_kart, offset_x, offset_y,
			   split_screen_ratio_x, split_screen_ratio_y ) ;
    }   // for pla
    drawTimer ();
    drawMap   ();
    if ( config->displayFPS ) drawFPS ();
    drawPlayerIcons() ;
  }   // if RACE_PHASE

  glPopAttrib  () ;
  glPopMatrix  () ;
  glMatrixMode ( GL_MODELVIEW ) ;
  glPopMatrix  () ;
}   // drawStatusText

/* EOF */
