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
#include <plib/pw.h>

#include "loader.hpp"
#include "race_manager.hpp"
#include "start_screen.hpp"
#include "gui/menu_manager.hpp"
#include "plibdrv.hpp"

StartScreen* startScreen = NULL;

StartScreen::StartScreen() 
    : introMaterial(NULL)
{
  menu_manager->switchToMainMenu();
  installMaterial();
  pwSetCallbacks(keystroke, gui_mousefn, gui_motionfn, NULL, NULL);
}

StartScreen::~StartScreen()
{
  delete introMaterial;
  introMaterial= NULL;
}

void
StartScreen::update()
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
  introMaterial -> force () ;

  glBegin ( GL_QUADS ) ;
  glColor3f    ( 1, 1, 1 ) ;
  glTexCoord2f ( 0, 0 ) ; glVertex2i (   -1,   -1 ) ;
  glTexCoord2f ( 1, 0 ) ; glVertex2i (   1,   -1 ) ;
  glTexCoord2f ( 1, 1 ) ; glVertex2i (   1,   1 ) ;
  glTexCoord2f ( 0, 1 ) ; glVertex2i (   -1,   1 ) ;
  glEnd () ;

  glFlush();
  pollEvents() ;
  menu_manager->update();

  // Swapbuffers - and off we go again...
  pwSwapBuffers();
}

void
StartScreen::installMaterial()
{
  /* Make a simplestate for the title screen texture */
  introMaterial = new ssgSimpleState;
  introMaterial -> setTexture(loader->getPath("images/st_title_screen.rgb").c_str(), TRUE, TRUE);
  introMaterial -> enable      ( GL_TEXTURE_2D ) ;
  introMaterial -> disable     ( GL_LIGHTING  ) ;
  introMaterial -> disable     ( GL_CULL_FACE ) ;
  introMaterial -> setOpaque   () ;
  introMaterial -> disable     ( GL_BLEND ) ;
  introMaterial -> setShadeModel ( GL_SMOOTH ) ;
  introMaterial -> disable     ( GL_COLOR_MATERIAL ) ;
  introMaterial -> enable      ( GL_CULL_FACE      ) ;
  introMaterial -> setMaterial ( GL_EMISSION, 0, 0, 0, 1 ) ;
  introMaterial -> setMaterial ( GL_SPECULAR, 0, 0, 0, 1 ) ;
  introMaterial -> setMaterial ( GL_DIFFUSE, 0, 0, 0, 1 ) ;
  introMaterial -> setMaterial ( GL_AMBIENT, 0, 0, 0, 1 ) ;
  introMaterial -> setShininess ( 0 ) ;
}

void
StartScreen::switchToGame()
{
  delete introMaterial;
  introMaterial= NULL;

  race_manager->start();
}

/* EOF */
