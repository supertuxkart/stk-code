//  $Id: sdldrv.cxx,v 1.16 2004/08/06 00:38:26 jamesgregory Exp $
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
#include <plib/pw.h>

#include "sdldrv.h"
#include "WidgetSet.h"
#include "gui/BaseGUI.h"
#include "tuxkart.h"
#include "Driver.h"

using std::cout;


Uint8 *keyState = 0;
SDL_Surface *sdl_screen = 0;
static jsJoystick *joystick ;
static SDL_Joystick *joy;

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
    
  keyState = SDL_GetKeyState(NULL);
  
  joystick = new jsJoystick ( 0 ) ;
  joystick -> setDeadBand ( 0, 0.1 ) ;
  joystick -> setDeadBand ( 1, 0.1 ) ;
  
  if ( SDL_NumJoysticks() > 0 )
  	joy = SDL_JoystickOpen( 0 );
}

void shutdownVideo ()
{	
	if ( SDL_JoystickOpened ( 0 ) )
    		SDL_JoystickClose ( joy );
		
	SDL_Quit( );
}

void pollEvents ()
{
  static SDL_Event event;
  
  if ( SDL_PollEvent(&event) )
  {
    switch (event.type)
    {
      case SDL_KEYDOWN:
	  	keyboardInput (event.key.keysym);
	  break;
	
	case SDL_MOUSEBUTTONDOWN:
		if (gui)
			gui -> select();
	  break;
	 
	case SDL_MOUSEMOTION:
		SDL_ShowCursor(SDL_ENABLE);
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
		if (gui)
			gui -> select();
		break;
	  
	case SDL_QUIT:
	  shutdown ();
	  break;
    }
  }
}

void keyboardInput (const SDL_keysym& key)
{
	if (key.sym == SDLK_p)
		widgetSet -> tgl_paused();
	else if (gui)
		gui -> keybd(key);
}

void kartInput()
{
  static JoyInfo ji ;

  if ( joystick -> notWorking () )
  {
    ji.data[0] = ji.data[1] = 0.0f ;
    ji.buttons = 0 ;
  }
  else
  {
    joystick -> read ( & ji.buttons, ji.data ) ;
    ji.data[0] *= 1.3 ;
  }

  if ( keyState [ SDLK_LEFT ] ) ji.data [0] = -1.0f ;
  if ( keyState [ SDLK_RIGHT ] ) ji.data [0] =  1.0f ;
  if ( keyState [ SDLK_UP ] ) ji.buttons |= 0x01 ;
  if ( keyState [ SDLK_DOWN ] )  ji.buttons |= 0x02 ;

  if ( keyState [ SDLK_f ] ) ji.buttons |= 0x04 ;
  if ( keyState [ SDLK_a ] ) ji.buttons |= 0x20 ;
  if ( keyState [ SDLK_s ] ) ji.buttons |= 0x10 ;
  if ( keyState [ SDLK_d ] ) ji.buttons |= 0x08 ;
  
  // physics debugging keys
  if ( keyState [ SDLK_1 ] ) ji.buttons |= 0x40 ;
  if ( keyState [ SDLK_2 ] ) ji.buttons |= 0x80 ;
  if ( keyState [ SDLK_3 ] ) ji.buttons |= 0x100 ;
  if ( keyState [ SDLK_4 ] ) ji.buttons |= 0x200 ;
  if ( keyState [ SDLK_5 ] ) ji.buttons |= 0x400 ;
  if ( keyState [ SDLK_6 ] ) ji.buttons |= 0x800 ;
  if ( keyState [ SDLK_PLUS ] ) ji.buttons |= 0x1000 ;
  if ( keyState [ SDLK_MINUS ] ) ji.buttons |= 0x2000 ;
  

  ji.hits        = (ji.buttons ^ ji.old_buttons) &  ji.buttons ;
  ji.releases    = (ji.buttons ^ ji.old_buttons) & ~ji.buttons ;
  ji.old_buttons =  ji.buttons ;

  ((PlayerKartDriver *)kart [ 0 ]) -> incomingJoystick ( &ji ) ;
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
