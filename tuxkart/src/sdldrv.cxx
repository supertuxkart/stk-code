//  $Id: sdldrv.cxx,v 1.35 2004/08/29 19:50:45 oaf_thadres Exp $
//
//  TuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 James Gregory <james.gregory@btinternet.com>
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

#include <iostream>
#include <SDL.h>
#include <vector>

#include "sdldrv.h"
#include "WidgetSet.h"
#include "gui/BaseGUI.h"
#include "tuxkart.h"
#include "Driver.h"
#include "KartDriver.h"
#include "PlayerDriver.h"
#include "RaceSetup.h"
#include "World.h"
#include "Config.h"

using std::cout;
using std::vector;

const unsigned int MOUSE_HIDE_TIME = 2000;

Uint8 *keyState = 0;
SDL_Surface *sdl_screen = 0;


static vector<SDL_Joystick*> joys;


void initVideo(int screenWidth, int screenHeight, bool fullscreen)
{
  int videoFlags = SDL_OPENGL;

  if (fullscreen)
    videoFlags |= SDL_FULLSCREEN;

  if ( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) == -1 )
    {
      cout << "SDL_Init() failed: " << SDL_GetError();
      exit(1);
    }
  
  if ( ( sdl_screen = SDL_SetVideoMode( screenWidth, screenHeight, 0, videoFlags )) == 0 )
    {
      cout << "SDL_SetVideoMode() failed: " << SDL_GetError();
      exit(1);
    }
    
	setupControls();
}

void setupControls()
{
	keyState = SDL_GetKeyState(NULL);


	int numJoys = SDL_NumJoysticks();
	
	int i;
	for (i = 0; i != numJoys; ++i)
	{
		joys.push_back( SDL_JoystickOpen( i ) );
		/*open all joysticks, but don't access players that don't exist*/
		if(i < PLAYERS)
			config.player[i].useJoy = true;
	}

	for (; i != PLAYERS; ++i)
		config.player[i].useJoy = false;

	/*player 1 default keyboard settings*/
	config.player[0].keys[CD_KEYBOARD][KC_LEFT]    = SDLK_LEFT;
	config.player[0].keys[CD_KEYBOARD][KC_RIGHT]   = SDLK_RIGHT;
	config.player[0].keys[CD_KEYBOARD][KC_UP]      = SDLK_UP;
	config.player[0].keys[CD_KEYBOARD][KC_DOWN]    = SDLK_DOWN;
	config.player[0].keys[CD_KEYBOARD][KC_WHEELIE] = SDLK_a;
	config.player[0].keys[CD_KEYBOARD][KC_JUMP]    = SDLK_s;
	config.player[0].keys[CD_KEYBOARD][KC_RESCUE]  = SDLK_d;
	config.player[0].keys[CD_KEYBOARD][KC_FIRE]    = SDLK_f;

	/*player 2 default keyboard settings*/
	config.player[1].keys[CD_KEYBOARD][KC_LEFT]    = SDLK_j;
	config.player[1].keys[CD_KEYBOARD][KC_RIGHT]   = SDLK_l;
	config.player[1].keys[CD_KEYBOARD][KC_UP]      = SDLK_i;
	config.player[1].keys[CD_KEYBOARD][KC_DOWN]    = SDLK_k;
	config.player[1].keys[CD_KEYBOARD][KC_WHEELIE] = SDLK_q;
	config.player[1].keys[CD_KEYBOARD][KC_JUMP]    = SDLK_w;
	config.player[1].keys[CD_KEYBOARD][KC_RESCUE]  = SDLK_e;
	config.player[1].keys[CD_KEYBOARD][KC_FIRE]    = SDLK_r;
}

void shutdownVideo ()
{	
	for (unsigned int i = 0; i != joys.size(); ++i)
    		SDL_JoystickClose ( joys[i] );
		
	SDL_Quit( );
}

void pollEvents ()
{
  static SDL_Event event;
  static int lastMouseMove;
  
  while ( SDL_PollEvent(&event) )
  {
    switch (event.type)
    {
      case SDL_KEYDOWN:
		if (gui)
		{
			for (int i = 0; i != 4; ++i)
			{
				if ( event.key.keysym.sym == config.player[i].keys[CD_KEYBOARD][KC_FIRE] )
				{
					gui -> select();
					break;
				}
			}
			
			gui -> keybd(event.key.keysym);
		}
		break;
	
	case SDL_MOUSEBUTTONDOWN:
		if (gui && event.button.button == SDL_BUTTON_LEFT)
			gui -> select();
		else if (guiStack.size() > 1 && event.button.button == SDL_BUTTON_RIGHT)
			guiStack.pop_back();
	  break;
	 
	case SDL_MOUSEMOTION:
		SDL_ShowCursor(SDL_ENABLE);
		lastMouseMove = SDL_GetTicks();
		if (gui)
	  		gui -> point ( event.motion.x, getScreenHeight() - event.motion.y );
	  	break;
	 
	 case SDL_JOYAXISMOTION:
		{
			int x = 0;
			int y = 0;
			
			if (event.jaxis.axis == 0)
				x = event.jaxis.value;
			else if (event.jaxis.axis == 1)
				y = event.jaxis.value;
			if (gui)
				gui -> stick ( x,  y );
		}
		break;
	  
	case SDL_JOYBALLMOTION:
	  if (gui)
	  	gui -> point ( event.jball.xrel,  getScreenHeight() - event.jball.yrel );
	  break;
	  
      case SDL_JOYBUTTONDOWN:
		if (gui && event.jbutton.button == 0)
			gui -> select();
		else if (guiStack.size() > 1 && event.jbutton.button == 1)
			guiStack.pop_back();
		break;
	  
	case SDL_QUIT:
	  deinitTuxKart();
	  break;
    }
  }
  
  if (SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE && SDL_GetTicks() - lastMouseMove > MOUSE_HIDE_TIME)
  	SDL_ShowCursor(SDL_DISABLE);
}

void kartInput(RaceSetup& raceSetup)
{	
	static JoyInfo ji;
	
	for (int i = 0; i != int(raceSetup.players.size()); ++i)
	{
		memset(&ji, 0, sizeof(ji));
		
		Player& plyr = config.player[i];
		
		if ( plyr.useJoy )
		{			
			ji.lr = static_cast<float>(SDL_JoystickGetAxis(joys[i], 0 )) / JOY_MAX;
			ji.accel   = SDL_JoystickGetButton (joys[i], 0);
			ji.brake   = SDL_JoystickGetButton (joys[i], 1);
			ji.wheelie = SDL_JoystickGetButton (joys[i], 2);
			ji.jump    = SDL_JoystickGetButton (joys[i], 3);
			ji.rescue  = SDL_JoystickGetButton (joys[i], 4);
			ji.fire    = SDL_JoystickGetButton (joys[i], 5);
		}
		
		if ( keyState [ plyr.keys[CD_KEYBOARD][KC_LEFT] ] )  ji.lr = -1.0f ;
		if ( keyState [ plyr.keys[CD_KEYBOARD][KC_RIGHT] ] ) ji.lr =  1.0f ;
		if ( keyState [ plyr.keys[CD_KEYBOARD][KC_UP] ] )    ji.accel = true ;
		if ( keyState [ plyr.keys[CD_KEYBOARD][KC_DOWN] ] )  ji.brake = true ;
		
		if ( keyState [ plyr.keys[CD_KEYBOARD][KC_WHEELIE] ] ) ji.wheelie = true ;
		if ( keyState [ plyr.keys[CD_KEYBOARD][KC_JUMP] ] )    ji.jump = true ;
		if ( keyState [ plyr.keys[CD_KEYBOARD][KC_RESCUE] ] )  ji.rescue = true ;	
		if ( keyState [ plyr.keys[CD_KEYBOARD][KC_FIRE] ] )    ji.fire = true ;
	
		PlayerDriver* driver = dynamic_cast<PlayerDriver*>(World::current()->getPlayerKart(i)->getDriver());
                if (driver)
                  driver -> incomingJoystick ( ji ) ;
	}
}

void swapBuffers()
{
  SDL_GL_SwapBuffers();
}

int  getScreenWidth()
{
  return sdl_screen->w;
}

int  getScreenHeight()
{
  return sdl_screen->h;
}

/* EOF */
