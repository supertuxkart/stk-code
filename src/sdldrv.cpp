//  $Id: plibdrv.cpp 757 2006-09-11 22:27:39Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
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

#include <SDL/SDL.h>

#include <plib/ssg.h>
#include <plib/js.h>
#include <plib/fnt.h>

#include "config.hpp"
#include "sdldrv.hpp"
#include "widget_set.hpp"
#include "material_manager.hpp"
#include "kart_manager.hpp"
#include "start_screen.hpp"
#include "screen_manager.hpp"
#include "herring_manager.hpp"
#include "collectable_manager.hpp"
#include "attachment_manager.hpp"
#include "projectile_manager.hpp"
#include "loader.hpp"
#include "screen_manager.hpp"
#include "gui/menu_manager.hpp"
#include "kart_control.hpp"
#include "player.hpp"

SDL_Surface *mainSurface;
long flags;
SDL_Joystick **sticks;

#define DEADZONE_MOUSE 1

// -----------------------------------------------------------------------------
void gui_mousefn ( int button, int updown, int x, int y ) {
  if (button==0 && updown==0) {
    BaseGUI* menu= menu_manager->getCurrentMenu();
    if (menu != NULL) {
      menu->select();
    }
  } else if (button==2 && updown==0 && menu_manager->getMenuStackSize() > 1) {
    menu_manager->popMenu();
  }
  // puMouse(button, updown, x, y ) ;
}

// -----------------------------------------------------------------------------
void updateMenus()
{
}

// -----------------------------------------------------------------------------
void drv_init() {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
  
  flags = SDL_OPENGL | SDL_HWSURFACE;

  if(config->fullscreen)
    flags |= SDL_FULLSCREEN;
  
  mainSurface = SDL_SetVideoMode(config->width, config->height, 0,
			  flags);

  SDL_JoystickEventState(SDL_ENABLE);
			  
  int noSticks = SDL_NumJoysticks();
  sticks = (SDL_Joystick **) malloc(sizeof(SDL_Joystick *) * noSticks);
  for (int i=0;i<noSticks;i++)
    sticks[i] = SDL_JoystickOpen(i);
			  
  SDL_WM_SetCaption("Super Tux Kart", NULL);
  SDL_ShowCursor(SDL_DISABLE);

  ssgInit () ;
  fntInit();
}

// -----------------------------------------------------------------------------
void drv_toggleFullscreen(int resetTextures)
{
  config->fullscreen = !config->fullscreen;

  flags = SDL_OPENGL | SDL_HWSURFACE;

  if(config->fullscreen)
    flags |= SDL_FULLSCREEN;

  SDL_FreeSurface(mainSurface);
  mainSurface = SDL_SetVideoMode(config->width, config->height, 0, flags);

#ifdef WIN32
  if(resetTextures) {
    // Clear plib internal texture cache
    loader->endLoad();
    
    // Windows needs to reload all textures, display lists, ... which means 
    // that all models have to be reloaded. So first, free all textures, 
    // models, then reload the textures from materials.dat, then reload
    // all models, textures etc.
    
    startScreen         -> removeTextures();
    attachment_manager  -> removeTextures();
    projectile_manager  -> removeTextures();
    herring_manager     -> removeTextures();
    kart_manager        -> removeTextures();
    collectable_manager -> removeTextures();

    material_manager->reInit();


    collectable_manager -> loadCollectables();
    kart_manager        -> loadKartData();
    herring_manager     -> loadDefaultHerrings();
    projectile_manager  -> loadData();
    attachment_manager  -> loadModels();

    startScreen         -> installMaterial();
    widgetSet           -> reInit();
  }
#endif
}

// -----------------------------------------------------------------------------
void drv_deinit()
{
  SDL_ShowCursor(SDL_ENABLE);

  int noSticks = SDL_NumJoysticks();
  for (int i=0;i<noSticks;i++)
    SDL_JoystickClose(sticks[i]);
  
  free(sticks);

  SDL_FreeSurface(mainSurface);
  
  SDL_Quit();
}

// -----------------------------------------------------------------------------
void input(InputType type, int id0, int id1, int id2, int value)
{
    BaseGUI* menu= menu_manager->getCurrentMenu();
    if (menu != NULL)
      menu->input(type, id0, id1, id2, value);
}

// -----------------------------------------------------------------------------
void drv_loop()
{
  SDL_Event ev;

  while(SDL_PollEvent(&ev))
  {
    switch(ev.type) {
    case SDL_QUIT:
            screen_manager->abort();
        break;

    case SDL_KEYDOWN:
    case SDL_KEYUP:
            input(IT_KEYBOARD, ev.key.keysym.sym, 0, 0, ev.key.state);
	    break;
        case SDL_MOUSEMOTION:
            // This probably needs better handling
            if (ev.motion.xrel < -DEADZONE_MOUSE)
              input(IT_MOUSEMOTION, 0, AD_NEGATIVE, 0, 1);
            else if(ev.motion.xrel > DEADZONE_MOUSE)
              input(IT_MOUSEMOTION, 0, AD_POSITIVE, 0, 1);
            else
              {
                input(IT_MOUSEMOTION, 0, AD_NEGATIVE, 0, 0);
                input(IT_MOUSEMOTION, 0, AD_POSITIVE, 0, 0);
              }

            if (ev.motion.yrel < -DEADZONE_MOUSE)
              input(IT_MOUSEMOTION, 1, AD_NEGATIVE, 0, 1);
            else if(ev.motion.yrel > DEADZONE_MOUSE)
              input(IT_MOUSEMOTION, 1, AD_POSITIVE, 0, 1);
            else
              {
                input(IT_MOUSEMOTION, 1, AD_NEGATIVE, 0, 0);
                input(IT_MOUSEMOTION, 1, AD_POSITIVE, 0, 0);
              }

	    break;
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
            input(IT_MOUSEBUTTON, ev.button.button, 0, 0, ev.button.state);
	    break;
	case SDL_JOYAXISMOTION:
            if(ev.jaxis.value <= -1000)
              {
                input(IT_STICKMOTION, ev.jaxis.which, ev.jaxis.axis, AD_POSITIVE, 0);
                input(IT_STICKMOTION, ev.jaxis.which, ev.jaxis.axis, AD_NEGATIVE, 1);
              }
            else if(ev.jaxis.value >= 1000)
              {
                input(IT_STICKMOTION, ev.jaxis.which, ev.jaxis.axis, AD_NEGATIVE, 0);
                input(IT_STICKMOTION, ev.jaxis.which, ev.jaxis.axis, AD_POSITIVE, 1);
              }
            else
              {
                input(IT_STICKMOTION, ev.jaxis.which, ev.jaxis.axis, AD_NEGATIVE, 0);
                input(IT_STICKMOTION, ev.jaxis.which, ev.jaxis.axis, AD_POSITIVE, 0);
              }
	    break;
	case SDL_JOYBUTTONDOWN:
	case SDL_JOYBUTTONUP:
	    input(IT_STICKBUTTON, ev.jbutton.which, ev.jbutton.button, 0, 
		  ev.jbutton.state);
	    break;
    }  // switch

    // If the event caused a new screen to be displayed, abort the current event
    // loop. This avoids e.g. the problem of selecting the number of laps twice
    // in the num_laps menu (by rapidly pressing enter), causing the game-start
    // procedure to be done twice (which causes an assertion error in the
    // screen_manager, since the new world_screen is added twice).
    if(screen_manager->screenSwitchPending()) break;
  }   // while (SDL_PollEvent())
  
  updateMenus();
  
}
