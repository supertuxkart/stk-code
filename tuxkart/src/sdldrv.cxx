//  $Id: sdldrv.cxx,v 1.12 2004/08/05 10:19:49 jamesgregory Exp $
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
#include <plib/puSDL.h>

#include "sdldrv.h"
#include "WidgetSet.h"
#include "oldgui.h"
#include "BaseGUI.h"
#include "status.h"
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
	  if ( !puKeyboard ( event.key.keysym.sym, PU_DOWN ) )
	  	keyboardInput (event.key.keysym);
	  break;
	
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
	{
	  int puButton = PU_NOBUTTON;
	  int puState  = PU_UP;
	  
	  switch (event.button.button)
	  {
	    case SDL_BUTTON_LEFT:
	      puButton = PU_LEFT_BUTTON;
	      break;
		
	    case SDL_BUTTON_MIDDLE:
	      puButton = PU_MIDDLE_BUTTON;
	      break;
	
	    case SDL_BUTTON_RIGHT:
	      puButton = PU_RIGHT_BUTTON;
	      break;
	  }
	  
	  switch (event.button.state)
	  {
          case SDL_PRESSED:
            puState = PU_DOWN;
            break;
          case SDL_RELEASED:
            puState = PU_UP;
            break;
	  }
	  if (gui)
	  	gui -> click (event.button.button, event.button.x,  getScreenHeight() - event.button.y );
	  puMouse ( puButton, puState, event.button.x, event.button.y );
	  break;
	}
	 
	case SDL_MOUSEMOTION:
	if (gui)
	  	gui -> point ( event.motion.x, getScreenHeight() - event.motion.y );
	  puMouse ( event.motion.x, event.motion.y );
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
	  
	case SDL_QUIT:
	  shutdown ();
	  break;
    }
  }
}

void keyboardInput (const SDL_keysym& key)
{
  static int isWireframe = FALSE ;

  int i;

  if (key.mod & KMOD_CTRL)
    {
      ((PlayerKartDriver*)kart[0])->incomingKeystroke ( key ) ;
      return;
    }

  switch ( key.sym )
    {
    case SDLK_ESCAPE : shutdown() ;
    
    case SDLK_F12:
      fpsToggle() ; return;

    case SDLK_r :
      {
        finishing_position = -1 ;
        for ( i = 0 ; i < num_karts ; i++ )
          kart[i]->reset() ;
        return ;
      }
      break;
 
    case SDLK_w : if ( isWireframe )
      glPolygonMode ( GL_FRONT_AND_BACK, GL_FILL ) ;
    else
      glPolygonMode ( GL_FRONT_AND_BACK, GL_LINE ) ;
      isWireframe = ! isWireframe ;

      return ;
    case SDLK_z : stToggle () ; return ;
    case SDLK_h : hide_status () ; help  () ; return ;
    case SDLK_p : widgetSet -> tgl_paused(); return ;

    case SDLK_SPACE : if ( oldgui->isHidden () )
      oldgui->show () ;
    else
      oldgui->hide () ;
      return ;
             
    default:
      break;
    }
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
