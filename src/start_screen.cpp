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

StartScreen::StartScreen() : introMaterial(NULL) {
  menu_manager->switchToMainMenu();
  installMaterial();
}

// -----------------------------------------------------------------------------
StartScreen::~StartScreen() {
  ssgDeRefDelete(introMaterial->getState());
  introMaterial= NULL;
}   // ~StartScreen

// -----------------------------------------------------------------------------
void StartScreen::update() {
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
void StartScreen::reInit() {
  installMaterial();
}   // reInit

// -----------------------------------------------------------------------------
void StartScreen::installMaterial() {
  /* Make a simplestate for the title screen texture */
  introMaterial = material_manager->getMaterial("st_title_screen.rgb");
  // This ref is necessary: if the window mode is changed (to/from fullscreen)
  // the textures all need to be reloaded. But the current context 
  // (_ssgCurrentContext) still has a pointer to the old ssgTexture object.
  // If the texture for the new title screen is set (as part of force()),
  // ssgSimpleState::setTexture() will ssgDeRefDelete the old texture. The
  // old texture (even if most likely already deleted) will still have the 
  // old texture handle for the title screen, which will then get deleted!
  // Since the new title texture has the same texture handle, the new title
  // screen gets deleted a well.
  // One solution is:
  //  _ssgCurrentContext->getState()->setTexture((ssgTexture*)NULL);
  // before material_manager->reInit() in sdldrv (this way the old tile screen
  // texture gets deleted, which will be deleted in reInit anyway), and when
  // the new texture is set, the old teture is NULL, so nothing will be
  // incorrectly deleted. Or we artifically increase the ref count, so that
  // it does not get freed:
  introMaterial->getState()->ref();
}   // installMaterial

// -----------------------------------------------------------------------------
void StartScreen::switchToGame() {

  race_manager->start();
}   // switchToGame

/* EOF */
