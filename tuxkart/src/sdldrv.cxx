
#include "sdldrv.h"
#include "tuxkart.h"

#include <SDL.h>
#include <plib/pu.h>

#include <iostream>

using std::cout;

//global, you could deglobalise them and prevent them from being changed by making them part of a class or making them static and having a getValue function, if you wanted

Uint8 *keyState;
int paused;

void initSDL (int videoFlags)
{
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
    
  int screenWidth = getScreenWidth();
  int screenHeight = getScreenHeight();
  int bpp = info->vfmt->BitsPerPixel;
  
  if ( SDL_SetVideoMode( screenWidth, screenHeight, bpp, videoFlags ) == 0 )
    {
      cout << "SDL_SetVideoMode() failed: " << SDL_GetError();
      exit(1);
    }
    
  keyState = SDL_GetKeyState(NULL);
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
    case SDLK_p : paused = ! paused ; return ;

    case SDLK_SPACE : if ( gui->isHidden () )
      gui->show () ;
    else
      gui->hide () ;
      return ;
             
    default:
      break;
    }
}

void shutdown()
{
  SDL_Quit( );
  exit (0);
}

/* EOF */
