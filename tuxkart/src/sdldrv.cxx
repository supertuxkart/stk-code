//  $Id: sdldrv.cxx,v 1.7 2004/08/01 18:52:50 jamesgregory Exp $
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
#include <plib/pu.h>

#include "oldgui.h"
#include "gui.h"
#include "status.h"
#include "tuxkart.h"
#include "Driver.h"
#include "sdldrv.h"

using std::cout;


Uint8 *keyState = 0;
SDL_Surface *sdl_screen = 0;
static jsJoystick *joystick ;

void initVideo(int screenWidth, int screenHeight, bool fullscreen)
{
  int videoFlags = SDL_OPENGL;

  if (fullscreen)
    videoFlags |= SDL_FULLSCREEN;

  if ( SDL_Init(SDL_INIT_VIDEO) == -1 )
    {
      cout << "SDL_Init() failed: " << SDL_GetError();
      exit(1);
    }
    
  const SDL_VideoInfo* info = NULL;

  info = SDL_GetVideoInfo();

  if ( !info ) 
    {
      cout << "SDL_GetVideoInfo() failed: " << SDL_GetError();
      exit(1);
    }
    
  int bpp = info->vfmt->BitsPerPixel;
  
  if ( ( sdl_screen = SDL_SetVideoMode( screenWidth, screenHeight, bpp, videoFlags )) == 0 )
    {
      cout << "SDL_SetVideoMode() failed: " << SDL_GetError();
      exit(1);
    }
    
  keyState = SDL_GetKeyState(NULL);
  
  joystick = new jsJoystick ( 0 ) ;
  joystick -> setDeadBand ( 0, 0.1 ) ;
  joystick -> setDeadBand ( 1, 0.1 ) ;
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
	  
	  puMouse ( puButton, puState, event.button.x, event.button.y );
	  break;
	}
	 
	case SDL_MOUSEMOTION:
	  puMouse ( event.motion.x, event.motion.y );
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
      //FIXME: do something to our widgets
	break;

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
    case SDLK_p : gui->tgl_paused(); return ;

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
