//  $Id$
//
//  TuxKart - a fun racing game with go-kart
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

#include <stdexcept>

#include "sdldrv.h"
#include "Loader.h"
#include "RaceSetup.h"
#include "tuxkart.h"
#include "WorldScreen.h"
#include "ScreenManager.h"
#include "RaceManager.h"
#include "TrackManager.h"
#include "StartScreen.h"

StartScreen* startScreen = 0;

StartScreen::StartScreen()
    : introMaterial(0)
{
  guiStack.push_back(GUIS_MAINMENU);
  installMaterial();
}

StartScreen::~StartScreen()
{
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
  //glOrtho        ( 0, 640, 0, 480, 0, 100 ) ;

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
  
  // Swapbuffers - and off we go again...
  pollEvents() ;
  updateGUI();
  swapBuffers();
}

void
StartScreen::installMaterial()
{
  /* Make a simplestate for the title screen texture */

  introMaterial = new ssgSimpleState ;
  ssgTexture* texture = loader->createTexture("title_screen.png", true, true,
          false);
  if(!texture) {
    delete introMaterial;
    introMaterial = 0;
    throw std::runtime_error("Couldn't load title_screen.png");
  }
  introMaterial -> setTexture(texture);
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
  delete introMaterial ;
  introMaterial = 0;
  
  guiStack.clear();
  
  RaceManager::instance()->start();
}

/* EOF */
