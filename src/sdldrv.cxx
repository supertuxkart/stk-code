//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
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
using std::cerr;
using std::vector;

const unsigned int MOUSE_HIDE_TIME = 2000;

Uint8 *keyState = 0;
SDL_Surface *sdl_screen = 0;
bool fullscreen_mode;

static vector<SDL_Joystick*> joys;

void initVideo(int screenWidth, int screenHeight, bool fullscreen)
{
    int videoFlags = SDL_OPENGL;

    fullscreen_mode = false;
    if (fullscreen)
        {
        videoFlags |= SDL_FULLSCREEN;
        fullscreen_mode = true;
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
    for (i = 0; i != numJoys && i < PLAYERS; ++i)
    {
        joys.push_back( SDL_JoystickOpen( i ) );
        config.player[i].useJoy = true;
    }

    for (; i != PLAYERS; ++i)
        config.player[i].useJoy = false;

}

void shutdownVideo ()
{
    for (unsigned int i = 0; i != joys.size(); ++i)
        SDL_JoystickClose ( joys[i] );

    SDL_Quit( );
}

void toggle_fullscreen()
{
    int videoFlags = SDL_OPENGL;
    int w = sdl_screen->w;
    int h = sdl_screen->h;

    SDL_FreeSurface(sdl_screen);

    fullscreen_mode = !fullscreen_mode;
    if (fullscreen_mode)
        {
        videoFlags |= SDL_FULLSCREEN;
        }

    if ( ( sdl_screen = SDL_SetVideoMode( w, h, 0, videoFlags )) == 0 )
    {
        cout << "SDL_SetVideoMode() failed: " << SDL_GetError();
        exit(1);
    }

// This would be the elegant way to do it:
// (unfortanely, this function is only supported for X11 systems)
/*  
  if(SDL_WM_ToggleFullScreen(sdl_screen))
    fullscreen_mode = !fullscreen_mode;
  else
    cerr << "Error: Could not set fullscreen mode.\n";
*/
}

bool is_fullscreen()
{
  return fullscreen_mode;
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
                    if ( event.key.keysym.sym == config.player[i].keys[KC_FIRE] )
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
            if (gui)
            {
                if (event.jaxis.axis == 0 || event.jaxis.axis == 1)
                    gui -> stick ( event.jaxis.axis,  event.jaxis.value );
            }
            break;

        case SDL_JOYBALLMOTION:
            if (gui)
                gui -> point ( event.jball.xrel,  getScreenHeight() - event.jball.yrel );
            break;

        case SDL_JOYBUTTONDOWN:
            if (gui)
                gui -> joybutton ( event.jbutton.which,  event.jbutton.button );
            break;

        case SDL_QUIT:
            guiStack.clear();
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
            ji.accel   = SDL_JoystickGetButton (joys[i], plyr.buttons[KC_UP]);
            ji.brake   = SDL_JoystickGetButton (joys[i], plyr.buttons[KC_DOWN]);
            ji.wheelie = SDL_JoystickGetButton (joys[i], plyr.buttons[KC_WHEELIE]);
            ji.jump    = SDL_JoystickGetButton (joys[i], plyr.buttons[KC_JUMP]);
            ji.rescue  = SDL_JoystickGetButton (joys[i], plyr.buttons[KC_RESCUE]);
            ji.fire    = SDL_JoystickGetButton (joys[i], plyr.buttons[KC_FIRE]);
        }

        if ( keyState [ plyr.keys[KC_LEFT] ] )
            ji.lr = -1.0f ;
        if ( keyState [ plyr.keys[KC_RIGHT] ] )
            ji.lr =  1.0f ;
        if ( keyState [ plyr.keys[KC_UP] ] )
            ji.accel = true ;
        if ( keyState [ plyr.keys[KC_DOWN] ] )
            ji.brake = true ;

        if ( keyState [ plyr.keys[KC_WHEELIE] ] )
            ji.wheelie = true ;
        if ( keyState [ plyr.keys[KC_JUMP] ] )
            ji.jump = true ;
        if ( keyState [ plyr.keys[KC_RESCUE] ] )
            ji.rescue = true ;
        if ( keyState [ plyr.keys[KC_FIRE] ] )
            ji.fire = true ;

        PlayerDriver* driver = dynamic_cast<PlayerDriver*>(world->getPlayerKart(i)->getDriver());
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
