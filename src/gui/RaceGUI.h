//  $Id: RaceGUI.h,v 1.9 2005/08/17 22:36:34 joh Exp $
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

#ifndef HEADER_RACEGUI_H
#define HEADER_RACEGUI_H

#include "BaseGUI.h"
#include "Material.h"
#include "Config.h"
#include "PlayerKart.h"
#include "plibdrv.h"

#define MAX_STRING          30
#define MAX_STRING_LENGTH  256

const int TEXTURES_PER_PLAYER = 10;

class RaceGUI: public BaseGUI {
  // A mapping for the assigned keys (like fire, ...) to
  // the kart which is using them
  PlayerKart* keysToKart[MAXKEYS];
  int         typeForKey[MAXKEYS];
public:
       RaceGUI();
       ~RaceGUI();
  void update(float dt);
  void select() {}
  void keybd(int key);
  void point(int x, int y) { (void)x; (void)y; }
  void stick     (const int &whichAxis, const float &value) ;
  void joybuttons(int whichJoy, int hold, int presses, int releases ) ;

private:
    ulClock  fpsTimer;
    int      fpsCounter;
    char     fpsString[10];
    double time_left ;
    char *pos_string [11];

    /* Display informat on screen */
    void drawStatusText        (const RaceSetup& raceSetup);
    void drawEnergyMeter       (Kart *player_kart, 
				int   offset_x, int   offset_y, 
				float ratio_x,  float ratio_y  );
    void drawCollectableIcons  (Kart* player_kart, 
				int   offset_x, int   offset_y, 
				float ratio_x,  float ratio_y  );
    void drawEmergencyText     (Kart* player_kart, 
				int   offset_x, int   offset_y, 
				float ratio_x,  float ratio_y  );
    void drawScore             (const RaceSetup& raceSetup,
				Kart* player_kart, 
				int   offset_x, int   offset_y, 
				float ratio_x,  float ratio_y  );
    void UpdateKeyboardMappings();
    void drawPlayerIcons       ();
    void oldDrawPlayerIcons    ();
    void drawGameOverText      ();
    void drawMap               ();
    void drawTimer             ();
    void drawFPS               ();
    
  /* Text drawing */
  /** Draw text to screen.
      scale_x and scale_y could be used to a simple resize (e.g. for multiplayer
      split screens, though, currently, we reduce fonts size to half).        */
    void drawTexture              (const GLuint texture, int w, int h, int red,
				   int green, int blue, int x, int y);
    void drawDropShadowText       (const char *str, int sz, int x, int y );
    void drawInverseDropShadowText(const char *str, int sz, int x, int y);
    void drawSteering             (Kart* kart, int offset_x, int offset_y,
				   float ratio_x, float ratio_y           );
};

#endif

