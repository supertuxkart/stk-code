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

#include <plib/pw.h>

#include "race_gui.hpp"
#include "history.hpp"
#include "widget_set.hpp"
#include "world.hpp"
#include "track.hpp"
#include "material_manager.hpp"
#include "menu_manager.hpp"

#define TEXT_START_X  (config->width-220)

RaceGUI::RaceGUI(): time_left(0.0) {
  if(!config->profile) {
    UpdateKeyboardMappings();
  }   // if !config->profile

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
  // Defines the mappings for player keys to kart and action	
  // To avoid looping over all players to find out what
  // player control key was pressed, a special data structure 
  // is set up: keysToKArt contains for each (player assigned) 
  // key which kart it applies to (and therefore which player),
  // and typeForKey contains the assigned function of that key.
  for(int i=0; i<MAXKEYS; i++) {
    keysToKart[i]=0;
    typeForKey[i]=0;
  }
  
  for(int i=0; i<world->raceSetup.getNumPlayers(); i++) {
    assert(world != NULL);
    PlayerKart* kart = world->getPlayerKart(i);
    Player* p        = kart->getPlayer();
    
    keysToKart[p->getKey(KC_WHEELIE)] = kart;
    keysToKart[p->getKey(KC_JUMP)   ] = kart;
    keysToKart[p->getKey(KC_RESCUE) ] = kart;
    keysToKart[p->getKey(KC_FIRE)   ] = kart;
    typeForKey[p->getKey(KC_WHEELIE)] = KC_WHEELIE;
    typeForKey[p->getKey(KC_JUMP)   ] = KC_JUMP;
    typeForKey[p->getKey(KC_RESCUE) ] = KC_RESCUE;
    typeForKey[p->getKey(KC_FIRE)   ] = KC_FIRE;
  }
}   // UpdateKeyControl

// -----------------------------------------------------------------------------
void RaceGUI::update(float dt) {
  assert(world != NULL);
  drawStatusText(world->raceSetup, dt);
}   // update

// -----------------------------------------------------------------------------
void RaceGUI::keybd(int key) {
  static int isWireframe = FALSE ;
  switch ( key ) {
    case 0x12:
      if(world->raceSetup.getNumPlayers()==1) {   // ctrl-r
        Kart* kart = world->getPlayerKart(0);
        kart->setCollectable((rand()%2)?COLLECT_MISSILE :COLLECT_HOMING_MISSILE, 10000);
      }
      break;
    case PW_KEY_F12:
      config->displayFPS = !config->displayFPS;
	  if(config->displayFPS) {
        fpsTimer.reset();
        fpsTimer.setMaxDelta(1000);
        fpsCounter=0;
      }
      break;
    case PW_KEY_F11:
      glPolygonMode(GL_FRONT_AND_BACK, isWireframe ? GL_FILL : GL_LINE);
      isWireframe = ! isWireframe;
      break;
    case 27: // ESC
      widgetSet->tgl_paused();
      menu_manager->pushMenu(MENUID_RACEMENU);
      // The player might have changed the keyboard 
      // configuration, so we need to redefine the mappings
      UpdateKeyboardMappings();
      break;
    case PW_KEY_F10:
      history->Save();
      break;
    default:
      // Check if it's a user assigned key
      if (keysToKart[key] != 0) {
        keysToKart[key]->action(typeForKey[key]);
      }
      break;
    } // switch
} // keybd

// -----------------------------------------------------------------------------
void RaceGUI::stick(const int &whichAxis, const float &value){
  KartControl controls;
  controls.data[whichAxis] = value;
  assert(world != NULL);
  world -> getPlayerKart(0) -> incomingJoystick ( controls );
}   // stick

// -----------------------------------------------------------------------------
void RaceGUI::joybuttons( int whichJoy, int hold, int presses, int releases ) {
  KartControl controls;
  controls.buttons = hold;
  controls.presses = presses;
  controls.releases = releases;
  assert(world != NULL);
  world -> getPlayerKart(whichJoy) -> incomingJoystick ( controls );
}   // joybuttons

// -----------------------------------------------------------------------------
void RaceGUI::drawFPS () {
  if (++fpsCounter>=50) {
    fpsTimer.update();
    sprintf(fpsString, "%d",(int)(fpsCounter/fpsTimer.getDeltaTime()));
    fpsCounter = 0;
    fpsTimer.setMaxDelta(1000);
  }    
  widgetSet->drawText (fpsString, 36, 0, config->height-36, 255, 255, 255 ) ;
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

// -----------------------------------------------------------------------------
void RaceGUI::drawTimer () {
  if(world->getPhase()!=World::RACE_PHASE) return;
  char str [ 256 ] ;

  assert(world != NULL);
  time_left = world->clock;

  int min     = (int) floor ( time_left / 60.0 ) ;
  int sec     = (int) floor ( time_left - (double) ( 60 * min ) ) ;
  int tenths  = (int) floor ( 10.0f * (time_left - (double)(sec + 60*min)));

  sprintf ( str, "%d:%02d\"%d", min,  sec,  tenths ) ;
  drawDropShadowText ( str, 36, TEXT_START_X, config->height-80) ;
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
		       (int)(offset_x+TEXT_START_X        *ratio_x),
		       (int)(offset_y+(config->height-200)*ratio_y),
		       red, green, blue);

  /* Show lap number */
  if ( player_kart->getLap() < 0 ) {
    sprintf ( str, "Lap:0/%d", raceSetup.numLaps ) ;
  }  else if ( player_kart->getLap() < raceSetup.numLaps - 1 ) {
    sprintf ( str, "Lap:%d/%d",
	      player_kart->getLap() + 1, raceSetup.numLaps ) ;
  } else if ( player_kart->getLap() == raceSetup.numLaps - 1 ) {
    sprintf ( str, "Last lap!" );
  } else {
    sprintf ( str, "Finished!" );
  }

  drawDropShadowText ( str, (int)(38*ratio_y), 
		       (int)(offset_x+TEXT_START_X        *ratio_x),
		       (int)(offset_y+(config->height-250)*ratio_y) );

  /* Show player's position */
  sprintf ( str, "%s", pos_string [ player_kart->getPosition() ] ) ;
  drawDropShadowText ( str, (int)(38*ratio_y), 
  		       (int)(offset_x+TEXT_START_X        *ratio_x), 
  		       (int)(offset_y+(config->height-300)*ratio_y) );
}   // drawScore

// -----------------------------------------------------------------------------
#define TRACKVIEW_SIZE 100

void RaceGUI::drawMap () {
  glDisable ( GL_TEXTURE_2D ) ;
  glColor3f ( 0,0,1 ) ;
  assert(world != NULL);
  world -> track -> draw2Dview ( 430+TRACKVIEW_SIZE  , TRACKVIEW_SIZE   ) ;
  glColor3f ( 1,1,0 ) ;
  world -> track -> draw2Dview ( 430+TRACKVIEW_SIZE+1, TRACKVIEW_SIZE+1 ) ;

  glBegin ( GL_QUADS ) ;

  for ( int i = 0 ; i < world->getNumKarts() ; i++ ) {
    sgCoord *c ;

    Kart* kart = world->getKart(i);
    glColor3fv ( *kart->getColor());
    c          = kart->getCoord () ;

    world -> track->glVtx ( c->xyz, 430+TRACKVIEW_SIZE+3, TRACKVIEW_SIZE+3 ) ;
    world -> track->glVtx ( c->xyz, 430+TRACKVIEW_SIZE+0, TRACKVIEW_SIZE+3 ) ;
    world -> track->glVtx ( c->xyz, 430+TRACKVIEW_SIZE+0, TRACKVIEW_SIZE+0 ) ;
    world -> track->glVtx ( c->xyz, 430+TRACKVIEW_SIZE+3, TRACKVIEW_SIZE+0 ) ;
  }

  glEnd () ;
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
void RaceGUI::oldDrawPlayerIcons () {
  assert(world != NULL);
  
  int   x =  0 ;
  int   y = 10 ;
  float w = 640.0f - 64.0f ;

    Material *last_players_gst=0;
    for(int i=0; i<world->getNumKarts(); i++) {
      x = (int) ( w * world->getKart(i) -> getDistanceDownTrack () /
		  world -> track -> getTrackLength () ) ;
      Material* players_gst =
	        world->getKart(i)->getKartProperties()->getIconMaterial();
      // Hmm - if the same icon is displayed more than once in a row,
      // plib does only do the first setTexture, therefore nothing is
      // displayed for the remaining icons. So we have to call force() if
      // the same icon is displayed more than once in a row.
      if(last_players_gst==players_gst) {
	players_gst->getState()->force();
      }
      players_gst -> apply ();
      last_players_gst=players_gst;
      glBegin ( GL_QUADS ) ;
        glColor4f    ( 1, 1, 1, 1 ) ;
	glTexCoord2f (  0, 0 ) ; glVertex2i ( x   , y    ) ;
	glTexCoord2f (  1, 0 ) ; glVertex2i ( x+64, y    ) ;
	glTexCoord2f (  1, 1 ) ; glVertex2i ( x+64, y+64 ) ;
	glTexCoord2f (  0, 1 ) ; glVertex2i ( x   , y+64 ) ;
      glEnd () ;

    }   // for i

}   // oldDrawPlayerIcons

// -----------------------------------------------------------------------------

// Draw players position on the race
void RaceGUI::drawPlayerIcons () {
  assert(world != NULL);
  
  int x = 10;
  int y;

  glEnable(GL_TEXTURE_2D);
  Material *last_players_gst = 0;
  for(int i = 0; i < world->getNumKarts() ; i++)
    {
      int position = world->getKart(i)->getPosition();
      if(position > 4)  // only draw the first four karts
        continue;

      y = config->width/2-20 - ((position-1)*(55+5));

      // draw text
      drawDropShadowText ( pos_string[position], 28, 55+x, y+10 ) ;

      // draw icon
      Material* players_gst =
	        world->getKart(i)->getKartProperties()->getIconMaterial();
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

        glTexCoord2f ( 0, 0 ) ; glVertex2i ( x   , y    ) ;
        glTexCoord2f ( 1, 0 ) ; glVertex2i ( x+55, y    ) ;
        glTexCoord2f ( 1, 1 ) ; glVertex2i ( x+55, y+55 ) ;
        glTexCoord2f ( 0, 1 ) ; glVertex2i ( x   , y+55 ) ;
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

      widgetSet->drawText ( "WRONG WAY!", (int)(50*ratio_y), 
			    (int)(130*ratio_x)+offset_x,
			    (int)(210*ratio_y)+offset_y, red, green, blue ) ;
      if ( ! i ) {
        red = blue = 255;
        green = 0;
      } else {
        red = blue = 0;
        green = 255;
      }

      widgetSet->drawText ( "WRONG WAY!", (int)(50*ratio_y), 
			    (int)((130+2)*ratio_x)+offset_x,
			    (int)((210+2)*ratio_y)+offset_y, red, green, blue ) ;

      i = ! i ;
  }
}   //drawEmergencyText

// -----------------------------------------------------------------------------
void RaceGUI::drawCollectableIcons ( Kart* player_kart, int offset_x, 
				     int offset_y, float ratio_x, 
				     float ratio_y                    ) {
  int zz = FALSE ;
  // Originally the hardcoded sizes were 320-32 and 400
  int x1 = (int)((config->width/2-32) * ratio_x) + offset_x ;
  int y1 = (int)(config->height*5/6 * ratio_y)      + offset_y;

  // If player doesn't have anything, just let the transparent black square
  Collectable* collectable=player_kart->getCollectable();
  if(collectable->getType() == COLLECT_NOTHING) {
    glDisable(GL_TEXTURE_2D);
    glBegin ( GL_QUADS ) ;
      glColor4f ( 0.0, 0.0, 0.0, 0.16 ) ;
      glVertex2i ( x1                  , y1    ) ;
      glVertex2i ( x1+(int)(64*ratio_x), y1    ) ;
      glVertex2i ( x1+(int)(64*ratio_x), y1+(int)(64*ratio_y) ) ;
      glVertex2i ( x1                  , y1+(int)(64*ratio_y) ) ;
    glEnd();
    return;
  }
  collectable->getIcon()->apply();

  int n  = player_kart->getNumCollectables() ;

  if ( n > 5 ) n = 5 ;
  if ( n < 1 ) n = 1 ;

  glEnable(GL_TEXTURE_2D);

  glBegin ( GL_QUADS ) ;
    glColor4f    ( 1, 1, 1, 1 ) ;

    for ( int i = 0 ; i < n ; i++ ) {
      if ( zz ) {
	glTexCoord2f ( 0, 2 ) ; glVertex2i ( i*40 + x1                  , y1    ) ;
	glTexCoord2f ( 0, 0 ) ; glVertex2i ( i*40 + x1+(int)(32*ratio_x), y1    ) ;
	glTexCoord2f ( 2, 0 ) ; glVertex2i ( i*40 + x1+(int)(64*ratio_x), y1+(int)(32*ratio_y) ) ;
	glTexCoord2f ( 2, 2 ) ; glVertex2i ( i*40 + x1+(int)(32*ratio_x), y1+(int)(32*ratio_y) ) ;

	glTexCoord2f ( 0, 2 ) ; glVertex2i ( i*40 + x1+(int)(32*ratio_x), y1+(int)(32*ratio_y) ) ;
	glTexCoord2f ( 0, 0 ) ; glVertex2i ( i*40 + x1+(int)(64*ratio_x), y1+(int)(32*ratio_y) ) ;
	glTexCoord2f ( 2, 0 ) ; glVertex2i ( i*40 + x1+(int)(32*ratio_x), y1+(int)(64*ratio_y) ) ;
	glTexCoord2f ( 2, 2 ) ; glVertex2i ( i*40 + x1                  , y1+(int)(64*ratio_y) ) ;
      } else {
	glTexCoord2f ( 0, 0 ) ; glVertex2i ( i*30 + x1                  , y1    ) ;
	glTexCoord2f ( 1, 0 ) ; glVertex2i ( i*30 + x1+(int)(64*ratio_x), y1    ) ;
	glTexCoord2f ( 1, 1 ) ; glVertex2i ( i*30 + x1+(int)(64*ratio_x), y1+(int)(64*ratio_y) ) ;
	glTexCoord2f ( 0, 1 ) ; glVertex2i ( i*30 + x1                  , y1+(int)(64*ratio_y) ) ;
      }
    }   // for i
  glEnd () ;

  glDisable(GL_TEXTURE_2D);
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
  int x = (int)(config->width-50 * ratio_x) + offset_x;
  int y = (int)(config->height/4 * ratio_y) + offset_y;
  int w = (int)(24 * ratio_x);
  int h = (int)(config->height/2 * ratio_y);
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
}   // drawEnergyMeter


// -----------------------------------------------------------------------------
void RaceGUI::drawSteering(Kart* kart, int offset_x, int offset_y,
			   float ratio_x, float ratio_y           ) {

  offset_x += (int)((config->width-220)*ratio_x);
#define WHEELWIDTH 64  
  int width  = (int)(WHEELWIDTH*ratio_x);
  int height = (int)(WHEELWIDTH*ratio_y);
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
    glEnable(GL_TEXTURE_2D);
    SteeringWheelIcon->apply();
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
void RaceGUI::drawStatusText (const RaceSetup& raceSetup, const float dt) {
  assert(world != NULL);

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
  if ( world->getPhase() == World::RACE_PHASE   ) {
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
      drawScore           (raceSetup, player_kart, offset_x, offset_y,
			   split_screen_ratio_x, split_screen_ratio_y ) ;
      drawEmergencyText   (player_kart, offset_x, offset_y,
			   split_screen_ratio_x, split_screen_ratio_y ) ;
    }   // for pla
    drawTimer ();
    drawMap   ();
    if ( config->displayFPS ) drawFPS ();
    if(config->oldStatusDisplay) {
      oldDrawPlayerIcons();
    } else {
      drawPlayerIcons() ;
      //drawSteering        (world->getPlayerKart(0), 100, 100, 1.0, 1.0 );
    }
  }   // if RACE_PHASE

  glPopAttrib  () ;
  glPopMatrix  () ;
  glMatrixMode ( GL_MODELVIEW ) ;
  glPopMatrix  () ;
}   // drawStatusText

/* EOF */
