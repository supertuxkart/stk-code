//  $Id: sdldrv.cxx,v 1.31 2004/08/17 21:06:39 grumbel Exp $
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

using std::cout;
using std::vector;

const unsigned int MOUSE_HIDE_TIME = 2000;

Uint8 *keyState = 0;
SDL_Surface *sdl_screen = 0;
static vector<SDL_Joystick*> joys;

std::vector<ControlConfig> controlCon;

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
	
	controlCon.resize(4);
  
	int numJoys = SDL_NumJoysticks();
	
	int i;
	for (i = 0; i != numJoys; ++i)
	{
  		joys.push_back( SDL_JoystickOpen( i ) );
		controlCon[i].useJoy = true;
	}
	
	for (; i != 4; ++i)
		controlCon[i].useJoy = false;
	
	controlCon[0].keys[KC_LEFT] = SDLK_LEFT;
	controlCon[0].keys[KC_RIGHT] = SDLK_RIGHT;
	controlCon[0].keys[KC_UP] = SDLK_UP;
	controlCon[0].keys[KC_DOWN] = SDLK_DOWN;
	controlCon[0].keys[KC_WHEELIE] = SDLK_a;
	controlCon[0].keys[KC_JUMP] = SDLK_s;
	controlCon[0].keys[KC_RESCUE] = SDLK_d;
	controlCon[0].keys[KC_FIRE] = SDLK_f;
	
	controlCon[1].keys[KC_LEFT] = SDLK_j;
	controlCon[1].keys[KC_RIGHT] = SDLK_l;
	controlCon[1].keys[KC_UP] = SDLK_i;
	controlCon[1].keys[KC_DOWN] = SDLK_k;
	controlCon[1].keys[KC_WHEELIE] = SDLK_q;
	controlCon[1].keys[KC_JUMP] = SDLK_w;
	controlCon[1].keys[KC_RESCUE] = SDLK_e;
	controlCon[1].keys[KC_FIRE] = SDLK_r;
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
				if ( event.key.keysym.sym == controlCon[i].keys[KC_FIRE] )
				{
					gui -> select();
					break;
				}
			}
			
			gui -> keybd(event.key.keysym);
		}
		break;
	
	case SDL_MOUSEBUTTONDOWN:
		if (gui)
			gui -> select();
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
	
	for (int i = 0; i != raceSetup.numPlayers; ++i)
	{
		memset(&ji, 0, sizeof(ji));
		
		ControlConfig& cc = controlCon[i];
		
		if ( cc.useJoy )
		{			
			ji.lr = static_cast<float>(SDL_JoystickGetAxis(joys[i], 0 )) / JOY_MAX;
			ji.accel   = SDL_JoystickGetButton (joys[i], 0);
			ji.brake   = SDL_JoystickGetButton (joys[i], 1);
			ji.wheelie = SDL_JoystickGetButton (joys[i], 2);
			ji.jump    = SDL_JoystickGetButton (joys[i], 3);
			ji.rescue  = SDL_JoystickGetButton (joys[i], 4);
			ji.fire    = SDL_JoystickGetButton (joys[i], 5);
		}
		
		if ( keyState [ cc.keys[KC_LEFT] ] )  ji.lr = -1.0f ;
		if ( keyState [ cc.keys[KC_RIGHT] ] ) ji.lr =  1.0f ;
		if ( keyState [ cc.keys[KC_UP] ] )    ji.accel = true ;
		if ( keyState [ cc.keys[KC_DOWN] ] )  ji.brake = true ;
		
		if ( keyState [ cc.keys[KC_WHEELIE] ] ) ji.wheelie = true ;
		if ( keyState [ cc.keys[KC_JUMP] ] )    ji.jump = true ;
		if ( keyState [ cc.keys[KC_RESCUE] ] )  ji.rescue = true ;	
		if ( keyState [ cc.keys[KC_FIRE] ] )    ji.fire = true ;
	
		PlayerDriver* driver = dynamic_cast<PlayerDriver*>(World::current()->kart [ i ]->getDriver());
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
