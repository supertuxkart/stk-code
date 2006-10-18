//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKartTeam
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

#if !defined(WIN32) && !defined(__CYGWIN__)
#  include <unistd.h>   // for usleep
#endif
#include <SDL/SDL.h>

#include "loader.hpp"
#include "race_manager.hpp"
#include "start_screen.hpp"
#include "material.hpp"
#include "material_manager.hpp"
#include "gui/menu_manager.hpp"
#include "sound_manager.hpp"

StartScreen* startScreen = NULL;

StartScreen::StartScreen() : introMaterial(NULL) 
{
  menu_manager->switchToMainMenu();
  installMaterial();
}

// -----------------------------------------------------------------------------
StartScreen::~StartScreen() 
{
  introMaterial= NULL;
}   // ~StartScreen

// -----------------------------------------------------------------------------
void StartScreen::update() 
{
  //Setup for boring 2D rendering
  glMatrixMode   ( GL_PROJECTION ) ;
  glLoadIdentity () ;
  glMatrixMode   ( GL_MODELVIEW ) ;
  glLoadIdentity () ;
  glDisable      ( GL_DEPTH_TEST ) ;
  glDisable      ( GL_LIGHTING   ) ;
  glDisable      ( GL_FOG        ) ;
  glDisable      ( GL_CULL_FACE  ) ;
  glDisable      ( GL_ALPHA_TEST ) ;

  // On at least one platform the X server apparently gets overloaded
  // by the large texture, resulting in buffering of key events. This
  // results in the menu being very unresponsive/slow - it can sometimes
  // take (say) half a second before the menu reacts to a pressed key.
  // This is caused by X buffering the key events, delivering them
  // later (and sometimes even several at the same frame). This issue
  // could either be solved by a lazy drawing of the background picture
  // (i.e. draw the background only if something has changed) - which is
  // a lot of implementation work ... or by sleeping for a little while,
  // which apparently reduces the load for the X server, so that no 
  // buffering is done --> all key events are handled in time.
#if !defined(WIN32) && !defined(__CYGWIN__)
  usleep(2000);
#endif
  //Draw the splash screen
  introMaterial->getState()->force();

  glBegin ( GL_QUADS ) ;
    glColor3f   (1, 1, 1 ) ;
    glTexCoord2f(0, 0); glVertex2i(-1, -1);
    glTexCoord2f(1, 0); glVertex2i( 1, -1);
    glTexCoord2f(1, 1); glVertex2i( 1,  1);
    glTexCoord2f(0, 1); glVertex2i(-1,  1);
  glEnd () ;

  glFlush();
  menu_manager->update();
  sound_manager->update();

  // Swapbuffers - and off we go again...
  SDL_GL_SwapBuffers();
}   // update

// -----------------------------------------------------------------------------
void StartScreen::removeTextures()
{
  // The current context (within plib) has a pointer to the last applied
  // texture, which means that the background image will not be deleted
  // as part of material_manager->reInit. To fix this, we load a NULL
  // texture into the current context, which decreases the ref counter
  // of the background image, and allows it to be deleted.
  // Doing it this way is not really nice, so any better way is appreciated :)
  _ssgCurrentContext->getState()->setTexture((ssgTexture*)NULL);
  
}   // removeTextures

// -----------------------------------------------------------------------------
void StartScreen::installMaterial() 
{
  /* Make a simplestate for the title screen texture */
  introMaterial = material_manager->getMaterial("st_title_screen.rgb");
}   // installMaterial

// -----------------------------------------------------------------------------
void StartScreen::switchToGame() 
{
  // As soon as the game is started, display the background picture only
  // so that the user gets feedback that his selection was done.
  introMaterial->getState()->force();

  glBegin ( GL_QUADS ) ;
    glColor3f   (1, 1, 1 ) ;
    glTexCoord2f(0, 0); glVertex2i(-1, -1);
    glTexCoord2f(1, 0); glVertex2i( 1, -1);
    glTexCoord2f(1, 1); glVertex2i( 1,  1);
    glTexCoord2f(0, 1); glVertex2i(-1,  1);
  glEnd () ;

  glFlush();
  SDL_GL_SwapBuffers();
  race_manager->start();
}   // switchToGame

/* EOF */
